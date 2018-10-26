#include "gyro_vmu_instr.h"
#include "gyro_vmu_memory.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_display.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_isr.h"
#include "gyro_vmu_osc.h"
#include "gyro_vmu_timers.h"
#include "gyro_vmu_bios.h"
#include "gyro_vmu_util.h"
#include <gyro_system_api.h>
#include "gyro_vmu_disassembler.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>

static int _biosDisassemblyInstrBytes[ROM_SIZE] = { 0 };
static char _biosDisassembly[ROM_SIZE][300] = { { 0 }};

static inline int dbgEnabled(VMUDevice* dev) {
    return gyVmuSerialConnectionType(dev) == VMU_SERIAL_CONNECTION_DC;
}

//#define VMU_DEBUG

#define SGNEXT(n) ((n)&0x80? (n)-0x100:(n))

#define UCHAR_SUB_OV(a, b) \
    ((b < 1)?((UCHAR_MAX + b >= a)?0:0):((b<=a)?0:0))
//    ((b < 1)?((UCHAR_MAX + b >= a)?1:0):((b<=a)?1:0))


static inline int _fetchRegIndAddr(struct VMUDevice* dev, uint8_t reg) {
    assert(reg <= 3); //Make sure we're within bounds
    return (gyVmuMemRead(dev,
                reg |
                ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&(SFR_PSW_IRBK0_MASK|SFR_PSW_IRBK1_MASK))>>0x1u)) //Bits 2-3 come from PSW
   | (reg&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction
}


inline static void _push(VMUDevice* dev, unsigned val) {
    const unsigned sp = dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]+1;
#ifdef VMU_DEBUG
    if(dbgEnabled(dev))
    _gyLog(GY_DEBUG_VERBOSE, "push[%x] = %d", sp, val);
    assert(sp <= RAM_STACK_ADDR_END);
#endif

    if(sp > RAM_STACK_ADDR_END) {
        _gyLog(GY_DEBUG_WARNING, "PUSH: Stack overflow detected!");
    }

    dev->ram[0][++dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]] = val;
}

inline static int _pop(VMUDevice* dev) {
    const unsigned sp = dev->sfr[SFR_OFFSET(SFR_ADDR_SP)];

#ifdef VMU_DEBUG
        if(dbgEnabled(dev))
    _gyLog(GY_DEBUG_VERBOSE, "pop[%x] = %d", dev->sfr[SFR_OFFSET(SFR_ADDR_SP)], dev->ram[0][dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]]);
    assert(sp >= RAM_STACK_ADDR_BASE);
#endif

    if(sp < RAM_STACK_ADDR_BASE) {
        _gyLog(GY_DEBUG_WARNING, "POP: Stack underflow detected!");
    }

    return dev->ram[0][dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]--];
}


static int _serviceInterrupts(VMUDevice* dev) {
    //Check for Port 3 interrupt

    if(!dev->intMask) {

        if((dev->sfr[SFR_OFFSET(SFR_ADDR_P3INT)]&(SFR_P3INT_P31INT_MASK|SFR_P3INT_P30INT_MASK)) ==
                (SFR_P3INT_P31INT_MASK|SFR_P3INT_P30INT_MASK))
            dev->intReq |= 1<<VMU_INT_P3;

    }


    if((dev->sfr[SFR_OFFSET(SFR_ADDR_P7)]&SFR_P7_P70_MASK) && (dev->sfr[SFR_OFFSET(SFR_ADDR_I01CR)]&0x3)) {
        dev->intReq |= 1<<VMU_INT_EXT_INT0;
        _gyLog(GY_DEBUG_VERBOSE, "RAISING EXTERNAL P7 INTERRUPT!!!!");
    }

    uint8_t ie = dev->sfr[SFR_OFFSET(SFR_ADDR_IE)];

    //Check if there are no interrupts, we're already in an interrupt, or we've disabled them.
    if(!dev->intReq || dev->intMask || !(ie&SFR_IE_IE7_MASK)) return 0;

    for(uint16_t r = 0; r < VMU_INT_COUNT; ++r) {
        if(dev->intReq&(1<<r)) {                //pending interrupt request
            dev->intReq &= ~(1<<r);             //clear request
            ++dev->intMask;                     //increment mask/depth
            _push(dev, dev->pc&0xff);
            _push(dev, (dev->pc&0xff00)>>8);    //push return address
#ifdef VMU_DEBUG
                                                    if(dbgEnabled(dev))
            _gyLog(GY_DEBUG_VERBOSE, "INTERRUPT - %d", r);
#endif
            dev->pc = gyVmuIsrAddr(r);          //jump to ISR address
            return 1;
        }
    }

    return 0; //ain't no pending interrupts...
}

static int gyVmuCpuInstrExecute(VMUDevice* dev, const VMUInstr* instr, const VMUInstrOperands* operands) {

    //execute instruction
    switch(_instrMap[instr->instrBytes[INSTR_BYTE_OPCODE]].opcode) {

    case OPCODE_NOP:
    default:
        break;
    case OPCODE_BR:
        dev->pc += SGNEXT(operands->addrRel);
        break;
    case OPCODE_LD:
        gyVmuMemWrite(dev,
                      SFR_ADDR_ACC,
                      gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]));
            break;
    case OPCODE_LD_IND:
        gyVmuMemWrite(dev,
                      SFR_ADDR_ACC,
                      gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND])));
        break;
    case OPCODE_CALL:
        _push(dev, (dev->pc&0xff));
        _push(dev, (dev->pc&0xff00)>>8u);
        dev->pc &= ~0xfff;
        dev->pc |= operands->addrMode[ADDR_MODE_ABS];
        break;
    case OPCODE_CALLR:
        _push(dev, (dev->pc&0xff));
        _push(dev, (dev->pc&0xff00)>>8u);
        dev->pc += (operands->addrMode[ADDR_MODE_REL]%65536)-1;
        break;
    case OPCODE_BRF:
        dev->pc += (operands->addrMode[ADDR_MODE_REL]%65536)-1;
        break;
    case OPCODE_ST:
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]);
        break;
    case OPCODE_ST_IND:
        gyVmuMemWrite(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]), dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]);
        break;
    case OPCODE_CALLF:
        _push(dev, (dev->pc&0xff));
        _push(dev, (dev->pc&0xff00)>>8u);
        dev->pc = operands->addrMode[ADDR_MODE_ABS];
        break;
    case OPCODE_JMPF:
        dev->pc = operands->addrMode[ADDR_MODE_ABS];
        break;
    case OPCODE_MOV:
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], operands->addrMode[ADDR_MODE_IMM]);
        break;
    case OPCODE_MOV_IND:
        gyVmuMemWrite(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]), operands->addrMode[ADDR_MODE_IMM]);
        break;
    case OPCODE_JMP:
        dev->pc &= ~0xfff;
        dev->pc |= operands->addrMode[ADDR_MODE_ABS];
        break;
    case OPCODE_MUL: {
            int temp    =   dev->sfr[SFR_OFFSET(SFR_ADDR_C)] | (dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]<<8);
            temp        *=  dev->sfr[SFR_OFFSET(SFR_ADDR_B)];

            gyVmuMemWrite(dev, SFR_ADDR_C, (temp&0xff));
            gyVmuMemWrite(dev, SFR_ADDR_ACC, ((temp&0xff00)>>8));
            gyVmuMemWrite(dev, SFR_ADDR_B, ((temp&0xff0000)>>16));

            dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]           &=  ~SFR_PSW_CY_MASK;
            if(temp>65535) {
                dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]       |=  SFR_PSW_OV_MASK;
            } else {
                dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]       &=  ~SFR_PSW_OV_MASK;
            }
        }
        break;
    case OPCODE_BEI: {
        int acc = gyVmuMemRead(dev, SFR_ADDR_ACC);
         gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x7f)|(acc<operands->addrMode[ADDR_MODE_IMM]? 0x80:0));
        if(acc == operands->addrMode[ADDR_MODE_IMM]) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_BE: {
        int acc = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int mem = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x7f)|(acc<mem? 0x80:0));
        if(acc == mem) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_BE_IND: {
        int imm = operands->addrMode[ADDR_MODE_IMM];
        int mem = gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]));
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x7f)|(mem<imm? 0x80:0));
        if(mem == imm) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_DIV: {
        int r  =   dev->sfr[SFR_OFFSET(SFR_ADDR_B)];
        int s;

        if(r) {
            int v = dev->sfr[SFR_OFFSET(SFR_ADDR_C)] | (dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]<<8);
            s = v%r;
            r = v/r;
        } else {
            r = 0xff00 | dev->sfr[SFR_OFFSET(SFR_ADDR_C)];
            s = 0;
        }
        gyVmuMemWrite(dev, SFR_ADDR_B, s);
        gyVmuMemWrite(dev, SFR_ADDR_C, r&0xff);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, (r&0xff00)>>8);
        dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]           &=  ~SFR_PSW_CY_MASK;
        if(!s) {
            dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]       |=  SFR_PSW_OV_MASK;
        } else {
            dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]       &=  ~SFR_PSW_OV_MASK;
        }
    }
        break;
    case OPCODE_BNEI:{
        const int acc = gyVmuMemRead(dev, SFR_ADDR_ACC);
        dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)] = (dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&0x7f)|(acc<operands->addrMode[ADDR_MODE_IMM]? 0x80:0);
        if(acc != operands->addrMode[ADDR_MODE_IMM]) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_BNE:{
        int acc = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int mem = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]);
        dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)] = (dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&0x7f)|(acc<mem? 0x80:0);
        if(acc != mem) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_BNE_IND: {
        int mem = gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]));
        dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)] = (dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&0x7f)|(mem < operands->addrMode[ADDR_MODE_IMM]? 0x80:0);
        if(operands->addrMode[ADDR_MODE_IMM] != mem) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_BPC: {
        int mem = gyVmuMemReadLatch(dev, operands->addrMode[ADDR_MODE_DIR]);
        if(mem & (1<<operands->addrMode[ADDR_MODE_BIT])) {
            gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], (mem&~(1<<operands->addrMode[ADDR_MODE_BIT])));
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_LDF: {
        uint32_t flashAddr = ((dev->sfr[SFR_OFFSET(SFR_ADDR_FLASH)]&SFR_FLASH_ADDR_MASK)<<16u)|(dev->sfr[SFR_OFFSET(SFR_ADDR_TRL)] | (dev->sfr[SFR_OFFSET(SFR_ADDR_TRH)]<<8u));
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->flash[flashAddr]);
        break;
    }
    case OPCODE_STF: {
        uint32_t flashAddr = (dev->sfr[SFR_OFFSET(SFR_ADDR_TRL)] | (dev->sfr[SFR_OFFSET(SFR_ADDR_TRH)]<<8u));
        uint8_t acc = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_FLASH)]&SFR_FLASH_UNLOCK_MASK) {
            switch(dev->flashPrg.prgState) {
            case VMU_FLASH_PRG_STATE0:
                if(flashAddr == VMU_FLASH_PRG_STATE0_ADDR && acc == VMU_FLASH_PRG_STATE0_VALUE)
                    dev->flashPrg.prgState = VMU_FLASH_PRG_STATE1;
                break;
            case VMU_FLASH_PRG_STATE1:
                dev->flashPrg.prgState =
                        (flashAddr == VMU_FLASH_PRG_STATE1_ADDR && acc == VMU_FLASH_PRG_STATE1_VALUE)?
                        VMU_FLASH_PRG_STATE2 : VMU_FLASH_PRG_STATE0;
                break;
            case VMU_FLASH_PRG_STATE2:
                if(flashAddr == VMU_FLASH_PRG_STATE2_ADDR && acc == VMU_FLASH_PRG_STATE2_VALUE)
                    dev->flashPrg.prgBytes = VMU_FLASH_PRG_BYTE_COUNT;
                dev->flashPrg.prgState = VMU_FLASH_PRG_STATE0;
                break;
            }
        } else {
            if(dev->flashPrg.prgBytes) {
                flashAddr |= (dev->sfr[SFR_OFFSET(SFR_ADDR_FLASH)]&SFR_FLASH_ADDR_MASK)<<16u;
                dev->flash[flashAddr] = acc;
                --dev->flashPrg.prgBytes;
            } else {
                _gyLog(GY_DEBUG_WARNING, "VMU_CPU: STF Instruction attempting to write byte %d to addr %x while Flash is locked!", acc, flashAddr);
            }
        }
        break;
    }
    case OPCODE_DBNZ: {
        int mem = gyVmuMemReadLatch(dev, operands->addrMode[ADDR_MODE_DIR])-1;
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], mem);
        if(mem != 0) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_DBNZ_IND:{
        int addr = _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]);
        int mem = gyVmuMemRead(dev, addr)-1;
        gyVmuMemWrite(dev, addr, mem);
        if(mem != 0) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    }
    case OPCODE_PUSH:
        _push(dev, gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]));
        break;
    case OPCODE_INC: {
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], gyVmuMemReadLatch(dev, operands->addrMode[ADDR_MODE_DIR])+1);
        break;
    }
    case OPCODE_INC_IND: {
        int addr = _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]);
        gyVmuMemWrite(dev, addr, gyVmuMemReadLatch(dev, addr)+1);
        break;
    }
    case OPCODE_BP:
        if(gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR])&(0x1<<operands->addrMode[ADDR_MODE_BIT])) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    case OPCODE_POP:
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], _pop(dev));
        break;
    case OPCODE_DEC:{
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR],gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR])-1);
        break;
    }
    case OPCODE_DEC_IND:{
        const int addr = _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]);
        gyVmuMemWrite(dev, addr, gyVmuMemRead(dev, addr)-1);
        break;
    }
    case OPCODE_BZ:
        if(!dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    case OPCODE_ADDI: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = operands->addrMode[ADDR_MODE_IMM];
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r+s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADD: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r+s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADD_IND: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]));
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r+s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_BN:
        if(!(gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR])&(0x1<<operands->addrMode[ADDR_MODE_BIT]))) {
            dev->pc += SGNEXT(operands->addrRel);
        }
        break;
    case OPCODE_BNZ: {
        int acc = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        if(acc) {
            dev->pc += SGNEXT(operands->addrRel);
        }
    }
        break;
    case OPCODE_ADDCI: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = operands->addrMode[ADDR_MODE_IMM] + ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&SFR_PSW_CY_MASK)>>SFR_PSW_CY_POS);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r+s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADDC: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]) + ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&SFR_PSW_CY_MASK)>>SFR_PSW_CY_POS);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r+s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADDC_IND: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND])) + ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&SFR_PSW_CY_MASK)>>SFR_PSW_CY_POS);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r+s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_RET: {
        int r   =   _pop(dev)<<8u;
        r       |=  _pop(dev);
        dev->pc = r;
        break;
    }
    case OPCODE_SUBI: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = operands->addrMode[ADDR_MODE_IMM];
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r-s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r-s<0? 0x80:0)|
             ((r&15)-(s&15)<0? 0x40:0)|(UCHAR_SUB_OV(r,s)? 4:0));

        break;
    }
    case OPCODE_SUB:{
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r-s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r-s<0? 0x80:0)|
             ((r&15)-(s&15)<0? 0x40:0)|(UCHAR_SUB_OV(r,s)? 4:0));

        break;
    }
    case OPCODE_SUB_IND:{
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]));
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r-s);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r-s<0? 0x80:0)|
             ((r&15)-(s&15)<0? 0x40:0)|(UCHAR_SUB_OV(r,s)? 4:0));

        break;
    }
    case OPCODE_NOT1: {
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], gyVmuMemReadLatch(dev, operands->addrMode[ADDR_MODE_DIR]) ^ (0x1u<<operands->addrMode[ADDR_MODE_BIT]));
        break;
    }
    case OPCODE_RETI: {
        int r = _pop(dev)<<8u;
        r |= _pop(dev);
        dev->pc = r;
        --dev->intMask;
        break;
    }
    case OPCODE_SUBCI: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = operands->addrMode[ADDR_MODE_IMM];
        int c = ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&SFR_PSW_CY_MASK)>>SFR_PSW_CY_POS);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r-s-c);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r-s-c<0? 0x80:0)|
             ((r&15)-(s&15)-c<0? 0x40:0)|(UCHAR_SUB_OV(r,s-c)? 4:0));
        break;
    }
    case OPCODE_SUBC: {
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]);
        int c = ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&SFR_PSW_CY_MASK)>>SFR_PSW_CY_POS);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r-s-c);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r-s-c<0? 0x80:0)|
             ((r&15)-(s&15)-c<0? 0x40:0)|(UCHAR_SUB_OV(r,s-c)? 4:0));

        break;
    }
    case OPCODE_SUBC_IND:{
        int r = dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)];
        int s = gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]));
        int c = ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&SFR_PSW_CY_MASK)>>SFR_PSW_CY_POS);
        gyVmuMemWrite(dev, SFR_ADDR_ACC, r-s-c);
        gyVmuMemWrite(dev, 0x101, (gyVmuMemRead(dev, 0x101)&0x3b)|(r-s-c<0? 0x80:0)|
             ((r&15)-(s&15)-c<0? 0x40:0)|(UCHAR_SUB_OV(r,s-c)? 4:0));

        break;
    }
    case OPCODE_ROR: {
        int r = gyVmuMemRead(dev, 0x100);
        gyVmuMemWrite(dev, 0x100, (r>>1)|((r&1)<<7));
        break;
    }
    case OPCODE_LDC: //Load from IMEM (flash/rom) not ROM?
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->imem[dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]+(dev->sfr[SFR_OFFSET(SFR_ADDR_TRL)] | (dev->sfr[SFR_OFFSET(SFR_ADDR_TRH)]<<8u))]);
        break;
    case OPCODE_XCH: {
        int acc   = gyVmuMemRead(dev, SFR_ADDR_ACC);
        int mem   = gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]);
        acc                 ^= mem;
        mem                 ^= acc;
        acc                 ^= mem;
        gyVmuMemWrite(dev, SFR_ADDR_ACC, acc);
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], mem);
        break;
    }
    case OPCODE_XCH_IND: {
        const int addr = _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND]);
        int acc   = gyVmuMemRead(dev, SFR_ADDR_ACC);
        int mem   = gyVmuMemRead(dev, addr);
        acc                 ^= mem;
        mem                 ^= acc;
        acc                 ^= mem;
        gyVmuMemWrite(dev, SFR_ADDR_ACC, acc);
        gyVmuMemWrite(dev, addr, mem);
        break;
    }
    case OPCODE_CLR1: {
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], gyVmuMemReadLatch(dev, operands->addrMode[ADDR_MODE_DIR])& ~(0x1u<<operands->addrMode[ADDR_MODE_BIT]));
        break;
    }
    case OPCODE_RORC: {
        int r = gyVmuMemRead(dev, 0x100);
        int s = gyVmuMemRead(dev, 0x101);
        gyVmuMemWrite(dev, 0x101, (s&0x7f)|((r&1)<<7));
        gyVmuMemWrite(dev, 0x100, (r>>1)|(s&0x80));
        break;
    }
    case OPCODE_ORI:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]|operands->addrMode[ADDR_MODE_IMM]);
        break;
    case OPCODE_OR:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]|gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]));
        break;
    case OPCODE_OR_IND:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]|gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND])));
        break;
    case OPCODE_ROL: {
        int r = gyVmuMemRead(dev, 0x100);
        gyVmuMemWrite(dev, 0x100, (r<<1)|((r&0x80)>>7));
        break;
    }
    case OPCODE_ANDI:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]&operands->addrMode[ADDR_MODE_IMM]);
        break;
    case OPCODE_AND:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]&gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]));
        break;
    case OPCODE_AND_IND:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]&gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND])));
        break;
    case OPCODE_SET1: {
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], gyVmuMemReadLatch(dev, operands->addrMode[ADDR_MODE_DIR])|(0x1u<<operands->addrMode[ADDR_MODE_BIT]));
        break;
    }
    case OPCODE_ROLC:  {
        int r = gyVmuMemRead(dev, 0x100);
        int s = gyVmuMemRead(dev, 0x101);
        gyVmuMemWrite(dev, 0x101, (s&0x7f)|(r&0x80));
        gyVmuMemWrite(dev, 0x100, (r<<1)|((s&0x80)>>7));
        break;
    }
    case OPCODE_XORI:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]^operands->addrMode[ADDR_MODE_IMM]);
        break;
    case OPCODE_XOR:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]^gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]));
        break;
    case OPCODE_XOR_IND:
        gyVmuMemWrite(dev, SFR_ADDR_ACC, dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)]^gyVmuMemRead(dev, _fetchRegIndAddr(dev, operands->addrMode[ADDR_MODE_IND])));
        break;
    }

    return 1;
}



int gyVmuCpuInstrExecuteNext(VMUDevice* device) {
    //Fetch instruction
    memset(&device->curInstr, 0, sizeof(VMUInstr));
    gyVmuInstrFetch(&device->imem[device->pc], &device->curInstr);

    //Advance program counter
    device->pc += device->curInstr.bytes;

    //Entire instruction has been loaded
    if(device->curInstr.bytes >= _instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].bytes) {
#ifdef VMU_DEBUG
            if(dbgEnabled(device)) {
        static int instrNum = 0;
        _gyLog(GY_DEBUG_VERBOSE, "*************** [%d] PC - %x **************", ++instrNum, device->pc-_instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].bytes+1);
        _gyPush();
        _gyLog(GY_DEBUG_VERBOSE, "mnemonic - %s", _instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].mnemonic);
            }
#endif
        //If this happens, we're at some unknown instruction, and fuck only knows what is about to happen...
        assert(_instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].mnemonic);

        //Fetch operands
        VMUInstrOperands operands;
        memset(&operands, 0, sizeof(VMUInstrOperands));
        gyVmuInstrDecodeOperands(&device->curInstr, &operands);

        if(gyVmuBiosSystemCodeActive(device)) {
            uint16_t prevPc = device->pc - device->curInstr.bytes;
            gyVmuDisassembleInstruction(device->curInstr, operands, _biosDisassembly[prevPc], prevPc, 1);
            _biosDisassemblyInstrBytes[prevPc] = device->curInstr.bytes;
        }

        //Execute instructions
        gyVmuCpuInstrExecute(device, &device->curInstr, &operands);

        static int wasInFw = 0;

        //Check if we entered the firmware
        if((!device->sfr[SFR_OFFSET(SFR_ADDR_EXT)]&SFR_EXT_MASK)) {
            if(!device->biosLoaded) {
                //handle the BIOS call in software if no firwmare has been loaded
                if((device->pc = gyVmuBiosHandleCall(device)))
                    //jump back to USER mode before resuming execution.
                    gyVmuMemWrite(device, SFR_ADDR_EXT, gyVmuMemRead(device, SFR_ADDR_EXT)|SFR_EXT_USER);
            } else if(!wasInFw){
                if(dbgEnabled(device)) _gyLog(GY_DEBUG_VERBOSE, "Entering firmware: %d", device->pc);
            }
        } else wasInFw = 0;
    }

#ifdef VMU_DEBUG
            if(dbgEnabled(device))
    _gyPop(1);
#endif
    return 1; //keep fetching instruction in next clock-cycle
}


int gyVmuCpuTick(VMUDevice* dev, float deltaTime) {

    //do timing in time domain, so when clock frequency changes, it's automatically handled
    float time = 0.0f;
    int cycle = 0;

    while(time < deltaTime) {
        gyVmuCpuInstrExecuteNext(dev);
        gyVmuTimersUpdate(dev);
       // gyVmuPort1PollRecv(dev);
        _serviceInterrupts(dev);
        float cpuTime = gyVmuOscSecPerCycle(dev)*(float)_instrMap[dev->curInstr.instrBytes[INSTR_BYTE_OPCODE]].cc;
        time += cpuTime;
        //gyVmuSerialUpdate(dev, cpuTime);
        gyVmuDisplayUpdate(dev, cpuTime);
        ++cycle;
    }

    return cycle;
}

int gyVmuCpuReset(VMUDevice* dev) {
    _gyLog(GY_DEBUG_VERBOSE, "Resetting VMU CPU.");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    dev->memMap[VMU_MEM_SEG_XRAM]       = dev->xram[VMU_XRAM_BANK_LCD_TOP];
    dev->memMap[VMU_MEM_SEG_SFR]        = dev->sfr;
    dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]   = RAM_STACK_ADDR_BASE-1;    //Initialize stack pointer
    dev->sfr[SFR_OFFSET(SFR_ADDR_P3)]   = 0xff;                     //Reset all P3 pins (controller buttons)
    dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]  = SFR_PSW_RAMBK0_MASK;


    gyVmuMemWrite(dev, SFR_ADDR_P1FCR,  0xbf);
    gyVmuMemWrite(dev, SFR_ADDR_P3INT,  0xfd);
    gyVmuMemWrite(dev, SFR_ADDR_ISL,    0xc0);
    gyVmuMemWrite(dev, SFR_ADDR_VSEL,   0xf4);
    gyVmuMemWrite(dev, SFR_ADDR_BTCR,   0x41);



    dev->timer0.tscale = 256;
    dev->pc = 0x0;

    if(dev->biosLoaded) {
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)]   |= SFR_P7_P71_MASK;
        dev->memMap[VMU_MEM_SEG_GP1]        = dev->ram[VMU_RAM_BANK0];
        dev->memMap[VMU_MEM_SEG_GP2]        = &dev->ram[VMU_RAM_BANK0][VMU_MEM_SEG_SIZE];
        dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)]  = 0;
        dev->imem = dev->rom;

    } else {

        if(!dev->biosLoaded) {
            //Initialize System Variables
            dev->ram[0][RAM_ADDR_YEAR_MSB_BCD]  = gyVmuToBCD(tm->tm_year/100+19);
            dev->ram[0][RAM_ADDR_YEAR_LSB_BCD]  = gyVmuToBCD(tm->tm_year%100);
            dev->ram[0][RAM_ADDR_MONTH_BCD]     = gyVmuToBCD(tm->tm_mon+1);
            dev->ram[0][RAM_ADDR_DAY_BCD]       = gyVmuToBCD(tm->tm_mday);
            dev->ram[0][RAM_ADDR_HOUR_BCD]      = gyVmuToBCD(tm->tm_hour);
            dev->ram[0][RAM_ADDR_MINUTE_BCD]    = gyVmuToBCD(tm->tm_min);
            dev->ram[0][RAM_ADDR_YEAR_MSB]      = (tm->tm_year+1900)>>8;
            dev->ram[0][RAM_ADDR_YEAR_LSB]      = (tm->tm_year+1900)&0xff;
            dev->ram[0][RAM_ADDR_MONTH]         = tm->tm_mon+1;
            dev->ram[0][RAM_ADDR_DAY]           = tm->tm_mday;
            dev->ram[0][RAM_ADDR_HOUR]          = tm->tm_hour;
            dev->ram[0][RAM_ADDR_MINUTE]        = tm->tm_min;
            dev->ram[0][RAM_ADDR_SEC]           = tm->tm_sec;
            dev->ram[0][RAM_ADDR_CLK_INIT]      = 0xff;

            dev->memMap[VMU_MEM_SEG_GP1]    = dev->ram[VMU_RAM_BANK1];
            dev->memMap[VMU_MEM_SEG_GP2]    = &dev->ram[VMU_RAM_BANK1][VMU_MEM_SEG_SIZE];
            dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)] = 1;
            dev->imem = dev->flash;
            gyVmuMemWrite(dev, SFR_ADDR_XBNK, VMU_XRAM_BANK_ICN);
            gyVmuMemWrite(dev, SFR_ADDR_XRAM_ICN_GAME, 0x10);           //Enable Game Icon

            //SFR values initialized by BIOS (from Sega Documentation)
            gyVmuMemWrite(dev, SFR_ADDR_P1FCR,  0xbf);
            gyVmuMemWrite(dev, SFR_ADDR_P3INT,  0xfd);
            gyVmuMemWrite(dev, SFR_ADDR_ISL,    0xc0);
            gyVmuMemWrite(dev, SFR_ADDR_VSEL,   0xf4);
            gyVmuMemWrite(dev, SFR_ADDR_BTCR,   0x41);

            //dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] = SFR_IE_IE7_MASK;
            gyVmuMemWrite(dev, SFR_ADDR_IE, 0x7c);
            gyVmuMemWrite(dev, SFR_ADDR_OCR, SFR_OCR_OCR7_MASK|SFR_OCR_OCR0_MASK); //stop main clock, divide active clock by 6
            dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] = SFR_P7_P71_MASK;

            gyVmuMemWrite(dev, SFR_ADDR_XBNK, VMU_XRAM_BANK_LCD_TOP);
            gyVmuMemWrite(dev, SFR_ADDR_VCCR, SFR_VCCR_VCCR7_MASK);     //turn on LCD
            gyVmuMemWrite(dev, SFR_ADDR_MCR, SFR_MCR_MCR3_MASK);        //enable LCD update
            gyVmuMemWrite(dev, SFR_ADDR_PCON, 0);                      //Disable HALT/HOLD modes, run CPU normally.

        }
    }

    return 1;
}

void gyVmuBiosDisassemblyPrints(char* buffer) {
    _gyLog(GY_DEBUG_VERBOSE, "===========SHITTING DISASSEMBLY===========");
    int lastValid = -1;
    int spaceAdded = 0;
    buffer[0] = '\0';
    for(unsigned i = 0; i < ROM_SIZE; ++i) {
        if(_biosDisassembly[i][0]) {
            _gyLog(GY_DEBUG_VERBOSE, "%s", _biosDisassembly[i]);
            if(buffer) {
                strcat(buffer, _biosDisassembly[i]);
                strcat(buffer, "\n");
            }
            lastValid = i;
            spaceAdded = 0;
        } else {
            if(!spaceAdded && lastValid != -1) {
                if(i > lastValid + _biosDisassemblyInstrBytes[lastValid]) {
                    _gyLog(GY_DEBUG_VERBOSE, "\n");
                    if(buffer) strcat(buffer, "\n");
                    spaceAdded = 1;
                }
            }


        }
    }
    _gyLog(GY_DEBUG_VERBOSE, "===========DONE SHITTING DISASSEMBLY===========");
}


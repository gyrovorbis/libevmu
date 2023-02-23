#include "gyro_vmu_instr.h"
#include "gyro_vmu_device.h"
#include <gyro_system_api.h>

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <sys/time.h>
#include <time.h>

#include <evmu/hw/evmu_rom.h>
#include "hw/evmu_device_.h"
#include "hw/evmu_memory_.h"
#include "hw/evmu_gamepad_.h"
#include "hw/evmu_timers_.h"
#include "hw/evmu_clock_.h"
#include "hw/evmu_pic_.h"

//#define VMU_DEBUG
#if 0
#define SGNEXT(n) ((n)&0x80? (n)-0x100:(n))
#else
#define SGNEXT(n) n
#endif

#define UCHAR_SUB_OV(a, b) \
    ((b < 1)?((UCHAR_MAX + b >= a)?0:0):((b<=a)?0:0))
//    ((b < 1)?((UCHAR_MAX + b >= a)?1:0):((b<=a)?1:0))


static inline int _fetchRegIndAddr(struct VMUDevice* dev, uint8_t reg) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    assert(reg <= 3); //Make sure we're within bounds
    int addr = (EvmuMemory_readInt(EVMU_MEMORY_PUBLIC_(pDevice_->pMemory),
                reg |
                ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&(EVMU_SFR_PSW_IRBK0_MASK|EVMU_SFR_PSW_IRBK1_MASK))>>0x1u)) //Bits 2-3 come from PSW
   | (reg&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction
    return addr;
}


void _gyVmuPush(VMUDevice* dev, unsigned val) {
        EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    const unsigned sp = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]+1;
#ifdef VMU_DEBUG
    if(dbgEnabled(dev))
    _gyLog(GY_DEBUG_VERBOSE, "push[%x] = %d", sp, val);
    assert(sp <= RAM_STACK_ADDR_END);
#endif

    if(sp > EVMU_ADDRESS_SYSTEM_STACK_END) {
        _gyLog(GY_DEBUG_WARNING, "PUSH: Stack overflow detected!");
    }

    pDevice_->pMemory->ram[0][++pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]] = val;
}

int _gyVmuPop(VMUDevice* dev) {
        EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    const unsigned sp = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)];

#ifdef VMU_DEBUG
        if(dbgEnabled(dev))
    _gyLog(GY_DEBUG_VERBOSE, "pop[%x] = %d", pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)], pDevice_->pMemory->ram[0][pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]]);
    assert(sp >= RAM_STACK_ADDR_BASE);
#endif

    if(sp < EVMU_ADDRESS_SYSTEM_STACK_BASE) {
        _gyLog(GY_DEBUG_WARNING, "POP: Stack underflow detected!");
    }

    return pDevice_->pMemory->ram[0][pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]--];
}

#if 0
#define EVMU_CPU_EXECUTE() return EvmuCpu_execute(pCpu, instr)

#else
#define EVMU_CPU_EXECUTE()
#endif
int gyVmuCpuInstrExecute(VMUDevice* dev, const EvmuDecodedInstruction* instr) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    EvmuDevice* pDevice = EVMU_DEVICE_PRISTINE_PUBLIC(dev);
    EvmuMemory* pMemory = pDevice->pMemory;
    EvmuCpu_* pCpu_ = EVMU_CPU_(pDevice->pCpu);
    EvmuCpu* pCpu = EVMU_CPU_PUBLIC_(pCpu_);
    //execute instruction

    switch(instr->opcode) {
    case OPCODE_NOP:
    default:
        EVMU_CPU_EXECUTE();
        break;
    case OPCODE_BR:
        EVMU_CPU_EXECUTE();
        pCpu_->pc += SGNEXT(instr->operands.relative8);
        break;
    case OPCODE_LD:
        EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory,
                            EVMU_ADDRESS_SFR_ACC,
                            EvmuMemory_readInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory,
                                               instr->operands.direct));
            break;
    case OPCODE_LD_IND:
        EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory,
                            EVMU_ADDRESS_SFR_ACC,
                            EvmuMemory_readInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory,
                                               _fetchRegIndAddr(dev, instr->operands.indirect)));
        break;
    case OPCODE_CALL:
                EVMU_CPU_EXECUTE();
        _gyVmuPush(dev, (pCpu_->pc&0xff));
        _gyVmuPush(dev, (pCpu_->pc&0xff00)>>8u);
        pCpu_->pc &= ~0xfff;
        pCpu_->pc |= (instr->operands.absolute&0xfff);
        break;
    case OPCODE_CALLR:
                EVMU_CPU_EXECUTE();
        _gyVmuPush(dev, (pCpu_->pc&0xff));
        _gyVmuPush(dev, (pCpu_->pc&0xff00)>>8u);
        pCpu_->pc += (instr->operands.relative16/*%65536*/)-1;
        break;
    case OPCODE_BRF:
                EVMU_CPU_EXECUTE();
        pCpu_->pc += (instr->operands.relative16/*%65536*/)-1;
        break;
    case OPCODE_ST:
                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, instr->operands.direct, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]);
        break;
    case OPCODE_ST_IND:
                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect), pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]);
        break;
    case OPCODE_CALLF:
                EVMU_CPU_EXECUTE();
        _gyVmuPush(dev, (pCpu_->pc&0xff));
        _gyVmuPush(dev, (pCpu_->pc&0xff00)>>8u);
        pCpu_->pc = instr->operands.absolute;
        break;
    case OPCODE_JMPF:
                        EVMU_CPU_EXECUTE();
        pCpu_->pc = instr->operands.absolute;
        break;
    case OPCODE_MOV:
                        EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, instr->operands.direct, instr->operands.immediate);
        break;
    case OPCODE_MOV_IND:
                        EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect), instr->operands.immediate);
        break;
    case OPCODE_JMP:
                                EVMU_CPU_EXECUTE();
        pCpu_->pc &= ~0xfff;
        pCpu_->pc |= (instr->operands.absolute&0xfff);
        break;
    case OPCODE_MUL: {
                                EVMU_CPU_EXECUTE();
            int temp    =   pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_C)] | (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]<<8);
            temp        *=  pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_B)];

            EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_C, (temp&0xff));
            EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, ((temp&0xff00)>>8));
            EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_B, ((temp&0xff0000)>>16));

            pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]           &=  ~EVMU_SFR_PSW_CY_MASK;
            if(temp>65535) {
                pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]       |=  EVMU_SFR_PSW_OV_MASK;
            } else {
                pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]       &=  ~EVMU_SFR_PSW_OV_MASK;
            }
        }
        break;
    case OPCODE_BEI: {
                        EVMU_CPU_EXECUTE();
        int acc = EvmuMemory_readInt(pMemory, EVMU_ADDRESS_SFR_ACC);
         EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x7f)|(acc<instr->operands.immediate? 0x80:0));
        if(acc == instr->operands.immediate) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_BE: {
                                EVMU_CPU_EXECUTE();
        int acc = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int mem = EvmuMemory_readInt(pMemory, instr->operands.direct);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x7f)|(acc<mem? 0x80:0));
        if(acc == mem) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_BE_IND: {
                                EVMU_CPU_EXECUTE();
        int imm = instr->operands.immediate;
        int mem = EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect));
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x7f)|(mem<imm? 0x80:0));
        if(mem == imm) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_DIV: {
                                EVMU_CPU_EXECUTE();
        int r  =   pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_B)];
        int s;

        if(r) {
            int v = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_C)] | (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]<<8);
            s = v%r;
            r = v/r;
        } else {
            r = 0xff00 | pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_C)];
            s = 0;
        }
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_B, s);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_C, r&0xff);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, (r&0xff00)>>8);
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]           &=  ~EVMU_SFR_PSW_CY_MASK;
        if(!s) {
            pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]       |=  EVMU_SFR_PSW_OV_MASK;
        } else {
            pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]       &=  ~EVMU_SFR_PSW_OV_MASK;
        }
    }
        break;
    case OPCODE_BNEI:{
                                EVMU_CPU_EXECUTE();
        const int acc = EvmuMemory_readInt(pMemory, EVMU_ADDRESS_SFR_ACC);
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)] = (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&0x7f)|(acc<instr->operands.immediate? 0x80:0);
        if(acc != instr->operands.immediate) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_BNE:{
                                EVMU_CPU_EXECUTE();
        int acc = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int mem = EvmuMemory_readInt(pMemory, instr->operands.direct);
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)] = (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&0x7f)|(acc<mem? 0x80:0);
        if(acc != mem) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_BNE_IND: {
                                EVMU_CPU_EXECUTE();
        int mem = EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect));
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)] = (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&0x7f)|(mem < instr->operands.immediate? 0x80:0);
        if(instr->operands.immediate != mem) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_BPC: {
                                EVMU_CPU_EXECUTE();
        int mem = EvmuMemory_readIntLatch(pMemory, instr->operands.direct);
        if(mem & (1<<instr->operands.bit)) {
            EvmuMemory_writeInt(pMemory, instr->operands.direct, (mem&~(1<<instr->operands.bit)));
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_LDF: {
                                EVMU_CPU_EXECUTE();
        uint32_t flashAddr = ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_FPR)]&EVMU_SFR_FPR_ADDR_MASK)<<16u)|
                              (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_TRL)] |
                              (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_TRH)]<<8u));
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->flash[flashAddr]);
        break;
    }
    case OPCODE_STF: {
        /*
        uint32_t flashAddr = (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_TRL)] | (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_TRH)]<<8u));
        uint8_t acc = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        if(pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_FPR)]&EVMU_SFR_FPR_UNLOCK_MASK) {
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
                flashAddr |= (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_FPR)]&EVMU_SFR_FPR_ADDR_MASK)<<16u;
                pDevice_->pMemory->flash[flashAddr] = acc;
                if(dev->pFnFlashChange)
                    dev->pFnFlashChange(dev, flashAddr);
                --dev->flashPrg.prgBytes;
            } else {
                _gyLog(GY_DEBUG_WARNING, "VMU_CPU: STF Instruction attempting to write byte %d to addr %x while Flash is locked!", acc, flashAddr);
            }
        }*/
        break;
    }
    case OPCODE_DBNZ: {
                                EVMU_CPU_EXECUTE();
        int mem = EvmuMemory_readIntLatch(pMemory, instr->operands.direct)-1;
        EvmuMemory_writeInt(pMemory, instr->operands.direct, mem);
        if(mem != 0) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_DBNZ_IND:{
                                EVMU_CPU_EXECUTE();
        int addr = _fetchRegIndAddr(dev, instr->operands.indirect);
        int mem = EvmuMemory_readInt(pMemory, addr)-1;
        EvmuMemory_writeInt(pMemory, addr, mem);
        if(mem != 0) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    }
    case OPCODE_PUSH:
                                EVMU_CPU_EXECUTE();
        _gyVmuPush(dev, EvmuMemory_readInt(pMemory, instr->operands.direct));
        break;
    case OPCODE_INC: {
                                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, instr->operands.direct, EvmuMemory_readIntLatch(pMemory, instr->operands.direct)+1);
        break;
    }
    case OPCODE_INC_IND: {
                                EVMU_CPU_EXECUTE();
        int addr = _fetchRegIndAddr(dev, instr->operands.indirect);
        EvmuMemory_writeInt(pMemory, addr, EvmuMemory_readIntLatch(pMemory, addr)+1);
        break;
    }
    case OPCODE_BP:
                                EVMU_CPU_EXECUTE();
        if(EvmuMemory_readInt(pMemory, instr->operands.direct)&(0x1<<instr->operands.bit)) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    case OPCODE_POP:
                                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, instr->operands.direct, _gyVmuPop(dev));
        break;
    case OPCODE_DEC:{
                                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, instr->operands.direct,EvmuMemory_readInt(pMemory, instr->operands.direct)-1);
        break;
    }
    case OPCODE_DEC_IND:{
                                EVMU_CPU_EXECUTE();
        const int addr = _fetchRegIndAddr(dev, instr->operands.indirect);
        EvmuMemory_writeInt(pMemory, addr, EvmuMemory_readInt(pMemory, addr)-1);
        break;
    }
    case OPCODE_BZ:
                                EVMU_CPU_EXECUTE();
        if(!pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    case OPCODE_ADDI: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = instr->operands.immediate;
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r+s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADD: {
                             EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, instr->operands.direct);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r+s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADD_IND: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect));
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r+s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_BN:
                                EVMU_CPU_EXECUTE();
        if(!(EvmuMemory_readInt(pMemory, instr->operands.direct)&(0x1<<instr->operands.bit))) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
        break;
    case OPCODE_BNZ: {
                                EVMU_CPU_EXECUTE();
        int acc = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        if(acc) {
            pCpu_->pc += SGNEXT(instr->operands.relative8);
        }
    }
        break;
    case OPCODE_ADDCI: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = instr->operands.immediate + ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r+s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADDC: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, instr->operands.direct) + ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r+s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_ADDC_IND: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect)) + ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r+s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r+s>255? 0x80:0)|
                 ((r&15)+(s&15)>15? 0x40:0)|((0x80&(~r^s)&(s^(r+s)))? 4:0));
        break;
    }
    case OPCODE_RET: {
                                EVMU_CPU_EXECUTE();
        int r   =   _gyVmuPop(dev)<<8u;
        r       |=  _gyVmuPop(dev);
        pCpu_->pc = r;
        break;
    }
    case OPCODE_SUBI: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = instr->operands.immediate;
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r-s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r-s<0? 0x80:0)|
             ((r&15)-(s&15)<0? 0x40:0)|(UCHAR_SUB_OV(r,s)? 4:0));

        break;
    }
    case OPCODE_SUB:{
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, instr->operands.direct);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r-s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r-s<0? 0x80:0)|
             ((r&15)-(s&15)<0? 0x40:0)|(UCHAR_SUB_OV(r,s)? 4:0));

        break;
    }
    case OPCODE_SUB_IND:{
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect));
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r-s);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r-s<0? 0x80:0)|
             ((r&15)-(s&15)<0? 0x40:0)|(UCHAR_SUB_OV(r,s)? 4:0));

        break;
    }
    case OPCODE_NOT1: {
                                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, instr->operands.direct, EvmuMemory_readIntLatch(pMemory, instr->operands.direct) ^ (0x1u<<instr->operands.bit));
        break;
    }
    case OPCODE_RETI: {
                                EVMU_CPU_EXECUTE();
        EvmuPic__retiInstruction(pDevice_->pPic);
        break;
    }
    case OPCODE_SUBCI: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = instr->operands.immediate;
        int c = ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r-s-c);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r-s-c<0? 0x80:0)|
             ((r&15)-(s&15)-c<0? 0x40:0)|(UCHAR_SUB_OV(r,s-c)? 4:0));
        break;
    }
    case OPCODE_SUBC: {
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, instr->operands.direct);
        int c = ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r-s-c);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r-s-c<0? 0x80:0)|
             ((r&15)-(s&15)-c<0? 0x40:0)|(UCHAR_SUB_OV(r,s-c)? 4:0));

        break;
    }
    case OPCODE_SUBC_IND:{
                                EVMU_CPU_EXECUTE();
        int r = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)];
        int s = EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect));
        int c = ((pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, r-s-c);
        EvmuMemory_writeInt(pMemory, 0x101, (EvmuMemory_readInt(pMemory, 0x101)&0x3b)|(r-s-c<0? 0x80:0)|
             ((r&15)-(s&15)-c<0? 0x40:0)|(UCHAR_SUB_OV(r,s-c)? 4:0));

        break;
    }
    case OPCODE_ROR: {
                                EVMU_CPU_EXECUTE();
        int r = EvmuMemory_readInt(pMemory, 0x100);
        EvmuMemory_writeInt(pMemory, 0x100, (r>>1)|((r&1)<<7));
        break;
    }
    case OPCODE_LDC: //Load from IMEM (flash/rom) not ROM?
                                EVMU_CPU_EXECUTE();
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->pExt[pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]+(pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_TRL)] | (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_TRH)]<<8u))]);
        break;
    case OPCODE_XCH: {
        int acc   = EvmuMemory_readInt(pMemory, EVMU_ADDRESS_SFR_ACC);
        int mem   = EvmuMemory_readInt(pMemory, instr->operands.direct);
        acc                 ^= mem;
        mem                 ^= acc;
        acc                 ^= mem;
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, acc);
        EvmuMemory_writeInt(pMemory, instr->operands.direct, mem);
        break;
    }
    case OPCODE_XCH_IND: {
        const int addr = _fetchRegIndAddr(dev, instr->operands.indirect);
        int acc   = EvmuMemory_readInt(pMemory, EVMU_ADDRESS_SFR_ACC);
        int mem   = EvmuMemory_readInt(pMemory, addr);
        acc                 ^= mem;
        mem                 ^= acc;
        acc                 ^= mem;
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, acc);
        EvmuMemory_writeInt(pMemory, addr, mem);
        break;
    }
    case OPCODE_CLR1: {
        EvmuMemory_writeInt(pMemory, instr->operands.direct, EvmuMemory_readIntLatch(pMemory, instr->operands.direct)& ~(0x1u<<instr->operands.bit));
        break;
    }
    case OPCODE_RORC: {
        int r = EvmuMemory_readInt(pMemory, 0x100);
        int s = EvmuMemory_readInt(pMemory, 0x101);
        EvmuMemory_writeInt(pMemory, 0x101, (s&0x7f)|((r&1)<<7));
        EvmuMemory_writeInt(pMemory, 0x100, (r>>1)|(s&0x80));
        break;
    }
    case OPCODE_ORI:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]|instr->operands.immediate);
        break;
    case OPCODE_OR:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]|EvmuMemory_readInt(pMemory, instr->operands.direct));
        break;
    case OPCODE_OR_IND:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]|EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect)));
        break;
    case OPCODE_ROL: {
        int r = EvmuMemory_readInt(pMemory, 0x100);
        EvmuMemory_writeInt(pMemory, 0x100, (r<<1)|((r&0x80)>>7));
        break;
    }
    case OPCODE_ANDI:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]&instr->operands.immediate);
        break;
    case OPCODE_AND:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]&EvmuMemory_readInt(pMemory, instr->operands.direct));
        break;
    case OPCODE_AND_IND:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]&EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect)));
        break;
    case OPCODE_SET1: {
        EvmuMemory_writeInt(pMemory, instr->operands.direct, EvmuMemory_readIntLatch(pMemory, instr->operands.direct)|(0x1u<<instr->operands.bit));
        break;
    }
    case OPCODE_ROLC:  {
        int r = EvmuMemory_readInt(pMemory, 0x100);
        int s = EvmuMemory_readInt(pMemory, 0x101);
        EvmuMemory_writeInt(pMemory, 0x101, (s&0x7f)|(r&0x80));
        EvmuMemory_writeInt(pMemory, 0x100, (r<<1)|((s&0x80)>>7));
        break;
    }
    case OPCODE_XORI:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]^instr->operands.immediate);
        break;
    case OPCODE_XOR:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]^EvmuMemory_readInt(pMemory, instr->operands.direct));
        break;
    case OPCODE_XOR_IND:
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ACC, pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)]^EvmuMemory_readInt(pMemory, _fetchRegIndAddr(dev, instr->operands.indirect)));
        break;
    }

    return 1;
}


#if 0
int gyVmuCpuInstrExecuteNext(VMUDevice* device) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(device);
    EvmuDevice* pDevice   = EVMU_DEVICE_PUBLIC_(pDevice_);
    EvmuCpu*    pCpu      = pDevice->pCpu;
    EvmuCpu_*    pCpu_     = EVMU_CPU_(pCpu);

    //Fetch instruction
    memset(&device->curInstr, 0, sizeof(VMUInstr));
    gyVmuInstrFetch(&pDevice_->pMemory->pExt[EvmuCpu_pc(pCpu)], &device->curInstr);

    if(EvmuCpu_pc(pCpu) == EVMU_BIOS_ADDRESS_FM_WRT_EX|| EvmuCpu_pc(pCpu) == EVMU_BIOS_ADDRESS_FM_WRTA_EX) {
       _gyLog(GY_DEBUG_VERBOSE, "HERE");
    }

    //Advance program counter
    pCpu_->pc += device->curInstr.bytes;

    //Entire instruction has been loaded
    if(device->curInstr.bytes >= _instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].bytes) {
//#ifdef VMU_DEBUG

#if 0
            if(dbgEnabled(device)) {
        static int instrNum = 0;
        _gyLog(GY_DEBUG_VERBOSE, "*************** [%d] PC - %x **************", ++instrNum, EvmuCpu_pc(pCpu)-_instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].bytes+1);
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

        if(EvmuRom_biosActive(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pRom)) {
            uint16_t prevPc = EvmuCpu_pc(pCpu) - device->curInstr.bytes;
            //gyVmuDisassembleInstruction(device->curInstr, operands, _biosDisassembly[prevPc], prevPc, 1);
#ifdef VMU_DEBUG
        _gyLog(GY_DEBUG_VERBOSE, "%s", _biosDisassembly[prevPc]);
        _gyPush();
#endif
        }

        //Execute instructions
//        gyVmuCpuInstrExecute(device, &device->curInstr, &operands);

        static int wasInFw = 0;

        //Check if we entered the firmware
        if(!(pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)]&EVMU_SFR_EXT_MASK)) {
            if(!EvmuRom_biosLoaded(pDevice->pRom)) {
                //handle the BIOS call in software if no firwmare has been loaded
                if((pCpu_->pc = EvmuRom_callBios(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pRom)))
                    //jump back to USER mode before resuming execution.
                    EvmuMemory_writeInt(EVMU_DEVICE_PUBLIC_(pDevice_)->pMemory,
                                        EVMU_ADDRESS_SFR_EXT,
                                        EvmuMemory_readInt(EVMU_DEVICE_PUBLIC_(pDevice_)->pMemory,
                                                           EVMU_ADDRESS_SFR_EXT)|EVMU_SFR_EXT_USER);
            } else if(!wasInFw){
              //  if(dbgEnabled(device)) _gyLog(GY_DEBUG_VERBOSE, "Entering firmware: %d", device->pc);
            }
        } else wasInFw = 0;
    }

#ifdef VMU_DEBUG
            if(dbgEnabled(device))
    _gyPop(1);
#endif
    return 1; //keep fetching instruction in next clock-cycle
}

#endif

double gyVmuCpuTCyc(struct VMUDevice* dev) {
        EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
   return EvmuClock_systemSecsPerCycle(EVMU_CLOCK_PUBLIC_(pDevice_->pClock))*((!(pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PCON)] & EVMU_SFR_PCON_HALT_MASK))?
                        (double)_instrMap[dev->curInstr.instrBytes[INSTR_BYTE_OPCODE]].cc : 1);
}

int gyVmuCpuTick(VMUDevice* dev, double deltaTime) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    //do timing in time domain, so when clock frequency changes, it's automatically handled
    double time = 0.0;
    int cycle = 0;

    while(time < deltaTime) {
        EvmuPic_update(EVMU_PIC_PUBLIC_(pDevice_->pPic));
        EvmuGamepad_poll(EVMU_GAMEPAD_PUBLIC_(pDevice_->pGamepad));
        EvmuTimers_update(EVMU_TIMERS_PUBLIC_(pDevice_->pTimers));
        if(!(pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PCON)] & EVMU_SFR_PCON_HALT_MASK))
       //     gyVmuCpuInstrExecuteNext(dev);

#if 1
#else
        _serviceInterrupts(dev);
#endif
       // gyVmuPort1PollRecv(dev);
        //double cpuTime = gyVmuCpuTCyc(dev);
        //time += cpuTime;
        //gyVmuSerialUpdate(dev, cpuTime);
        //EvmuIBehavior_update(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pLcd), cpuTime*1000000.0);

        ++cycle;
    }

    return cycle;
}


#if 0
int gyVmuCpuReset(VMUDevice* dev) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    EvmuMemory* pMemory = EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory;

    _gyLog(GY_DEBUG_VERBOSE, "Resetting VMU CPU.");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);


    memset(pDevice_->pMemory->ram, 0, EVMU_ADDRESS_SEGMENT_RAM_SIZE*EVMU_ADDRESS_SEGMENT_RAM_BANKS);
    memset(pDevice_->pMemory->sfr, 0, EVMU_ADDRESS_SEGMENT_SFR_SIZE);
    memset(pDevice_->pMemory->wram, 0, EVMU_WRAM_SIZE);
    memset(pDevice_->pMemory->xram, 0, EVMU_ADDRESS_SEGMENT_XRAM_SIZE*EVMU_ADDRESS_SEGMENT_XRAM_BANKS);


    pDevice_->pMemory->pIntMap[VMU_MEM_SEG_XRAM]       = pDevice_->pMemory->xram[EVMU_XRAM_BANK_LCD_TOP];
    pDevice_->pMemory->pIntMap[VMU_MEM_SEG_SFR]        = pDevice_->pMemory->sfr;
    pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]   = EVMU_ADDRESS_SEGMENT_STACK_BASE-1;    //Initialize stack pointer
    pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)]   = 0xff;                     //Reset all P3 pins (controller buttons)
    pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]  = EVMU_SFR_PSW_RAMBK0_MASK;


    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P7, EVMU_SFR_P7_P71_MASK);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IE, 0xff);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IP, 0x00);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P1FCR,  0xbf);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P3INT,  0xfd);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ISL,    0xc0);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_VSEL,   0xfc);
    //EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_BTCR,   0x41);

    pDevice_->pTimers->timer0.tscale = 256;
    pCpu_->pc = 0x0;

    if(dev->biosLoaded) {
       // pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)]   |= SFR_P7_P71_MASK;
        pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP1]        = pDevice_->pMemory->ram[VMU_RAM_BANK0];
        pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP2]        = &pDevice_->pMemory->ram[VMU_RAM_BANK0][VMU_MEM_SEG_SIZE];
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)]  = 0;
        pDevice_->pMemory->pExt = pDevice_->pMemory->rom;

    } //else {

        //Initialize System Variables
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB_BCD]  = GBL_BCD_BYTE_PACK(tm->tm_year/100+19);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB_BCD]  = GBL_BCD_BYTE_PACK(tm->tm_year%100);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH_BCD]     = GBL_BCD_BYTE_PACK(tm->tm_mon+1);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY_BCD]       = GBL_BCD_BYTE_PACK(tm->tm_mday);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR_BCD]      = GBL_BCD_BYTE_PACK(tm->tm_hour);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE_BCD]    = GBL_BCD_BYTE_PACK(tm->tm_min);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB]      = (tm->tm_year+1900)>>8;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB]      = (tm->tm_year+1900)&0xff;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH]         = tm->tm_mon+1;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY]           = tm->tm_mday;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR]          = tm->tm_hour;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE]        = tm->tm_min;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_SEC]           = tm->tm_sec;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DATE_SET]      = 0xff;
        if(!dev->biosLoaded) {
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP1]    = pDevice_->pMemory->ram[VMU_RAM_BANK1];
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP2]    = &pDevice_->pMemory->ram[VMU_RAM_BANK1][VMU_MEM_SEG_SIZE];
            pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 1;
            pDevice_->pMemory->pExt = pDevice_->pMemory->flash;
        }
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_ICON);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_XRAM_ICN_GAME, 0x10);           //Enable Game Icon

        //SFR values initialized by BIOS (from Sega Documentation)
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P1FCR,  0xbf);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P3INT,  0xfd);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ISL,    0xc0);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_VSEL,   0xfc);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_BTCR,   0x41);

        //pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] = SFR_IE_IE7_MASK;
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IE, 0xff);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IP, 0x00);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_OCR, EVMU_SFR_OCR_OCR7_MASK|EVMU_SFR_OCR_OCR0_MASK); //stop main clock, divide active clock by 6
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)] = EVMU_SFR_P7_P71_MASK;

        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_LCD_TOP);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_VCCR, EVMU_SFR_VCCR_VCCR7_MASK);     //turn on LCD
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_MCR, EVMU_SFR_MCR_MCR3_MASK);        //enable LCD update
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_PCON, 0);                      //Disable HALT/HOLD modes, run CPU normally.

 //   }

    return 1;
}
#endif

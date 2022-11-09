#include <evmu/evmu_api.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/events/evmu_memory_event.h>
#include <evmu/hw/evmu_isa.h>
#include "evmu_cpu_.h"
#include "evmu_device_.h"

#define EVMU_WIP

EvmuAddress EvmuCpu_registerIndirectAddress(const EvmuCpu* pSelf, uint8_t reg) {
    EvmuCpu_* pSelf_ = EVMU_CPU_(pSelf);
    return (EvmuMemory__readInt_(pSelf_->pMemory,
                reg |
                ((EvmuMemory__readSfr_(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) &
                  (EVMU_SFR_PSW_IRBK0_MASK|EVMU_SFR_PSW_IRBK1_MASK)) >> 0x1u)) //Bits 2-3 come from PSW
   | (reg&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction
}

EVMU_EXPORT EVMU_RESULT EvmuCpu_executeOpcode(const EvmuCpu* pSelf, EvmuWord opcode, const EvmuOperands* pOperands) {
// EVMU MICRO ISA
#define PC                      pSelf_->pc
#define OP(NAME)                pOperands->NAME
#define SFR(NAME)               EVMU_ADDRESS_SFR_##NAME
#define SFR_MSK(NAME, FIELD)    EVMU_SFR_##NAME##_##FIELD##_MASK
#define SFR_POS(NAME, FIELD)    EVMU_SFR_##NAME##_##FIELD##_POS
#define INDIRECT()              EvmuCpu_registerIndirectAddress(pSelf, OP(indirect))
#define READ(ADDR)              EvmuMemory__readInt_(pSelf_->pMemory, ADDR)
#define READ_LATCH(ADDR)        EvmuMemory__readIntLatch_(pSelf_->pMemory, ADDR)
#define WRITE(ADDR, VAL)        EvmuMemory__writeInt_(pSelf_->pMemory, ADDR, VAL)
#define READ_EXT(ADDR)          EvmuMemory__readExt_(pSelf_->pMemory, ADDR)
#define WRITE_EXT(ADDR, VAL)    EvmuMemory__writeExt_(pSelf_->pMemory, ADDR, VAL)
#define READ_FLASH(ADDR)        EvmuMemory__readFlash_(pSelf_->pMemory, ADDR)
#define WRITE_FLASH(ADDR, VAL)  EvmuMemory__writeFlash_(pSelf_->pMemory, ADDR, VAL)
#define PUSH(VALUE)             EvmuMemory__pushStack_(pSelf_->pMemory, VALUE)
#define POP()                   EvmuMemory__popStack_(pSelf_->pMemory)
#define PUSH_PC()               GBL_STMT_START { PUSH(PC & 0xff); PUSH((PC & 0xff00) >> 8u); } GBL_STMT_END
#define POP_PC()                GBL_STMT_START { PC = POP() << 8u; PC |= POP(); } GBL_STMT_END
#define PSW(FLAG, EXPR)         WRITE(SFR(PSW), (READ(SFR(PSW)) & ~SFR_MSK(PSW, FLAG)) | ((EXPR)? SFR_MSK(PSW, FLAG) : 0))
#define LOGIC_OP(OP, RHS)       WRITE(SFR(ACC), READ(SFR(ACC)) OP (RHS))
#define BR(EXPR, OFFSET)        if((EXPR)) PC += OFFSET
#define OP_ADD_CY_EXP           (a+b+c>255)
#define OP_ADD_AC_EXP           ((a&15)+((b+c)&15)>15)
#define OP_ADD_OV_EXP           (0x80&(~a^(b+c))&((b+c)^(a+(b+c))))
#define OP_ADD_(RVALUE, CY_EN)  OP_ARITH(+, (RVALUE), CY_EN, OP_ADD_CY_EXP, OP_ADD_AC_EXP, OP_ADD_OV_EXP)
#define OP_ADD(RVALUE)          OP_ADD_(RVALUE, 0)
#define OP_ADD_CARRY(RVALUE)    OP_ADD_(RVALUE, 1)
#define OP_SUB_CY_EXP           (a-b-c<0)
#define OP_SUB_AC_EXP           ((a&15)-(b&15)-c<0)
#define UCHAR_SUB_OV(a, b)      ((b < 1)?((UCHAR_MAX + b >= a)?0:0):((b<=a)?0:0)) /*temporarily fucked*/
#define OP_SUB_OV_EXP           (UCHAR_SUB_OV(a,b-c))
#define OP_SUB_(RVALUE, CY_EN)  OP_ARITH(-, (RVALUE), CY_EN, OP_SUB_CY_EXP, OP_SUB_AC_EXP, OP_SUB_OV_EXP)
#define OP_SUB(RVALUE)          OP_SUB_(RVALUE, 0)
#define OP_SUB_CARRY(RVALUE)    OP_SUB_(RVALUE, 1)

#define BR_DEC(ADDR)                                    \
    GBL_STMT_START {                                    \
        const EvmuAddress addr  = (ADDR);               \
        const EvmuWord    value = READ_LATCH(addr) - 1; \
        WRITE(addr, value);                             \
        BR(value != 0, OP(relativeS8));                 \
    } GBL_STMT_END

#define BR_CMP(VALUE1, OPERATOR, VALUE2, OFFSET) \
    GBL_STMT_START {                             \
        const EvmuWord v1 = VALUE1;              \
        const EvmuWord v2 = VALUE2;              \
        PSW(CY, v1 < v2);                        \
        BR(v2 OPERATOR v2, OFFSET);              \
    } GBL_STMT_END

#define OP_ARITH(OP, RVALUE, CY_EN, CY_EXP, AC_EXP, OV_EXP) \
    GBL_STMT_START {                                        \
        const EvmuWord a = READ(SFR(ACC));                  \
        const EvmuWord b = (RVALUE);                        \
        const EvmuWord c = CY_EN?                           \
            (SFR_MSK(PSW, CY) >> SFR_POS(PSW, CY)) : 0;     \
        const EvmuWord r = a OP b OP c;                     \
        EvmuWord p  = READ(SFR(PSW)) & ~(SFR_MSK(PSW, CY) | \
                                        SFR_MSK(PSW, AC)  | \
                                        SFR_MSK(PSW, OV));  \
        p           |= (CY_EXP)? SFR_MSK(PSW, CY) : 0;      \
        p           |= (AC_EXP)? SFR_MSK(PSW, AC) : 0;      \
        p           |= (OV_EXP)? SFR_MSK(PSW, OV) : 0;      \
        WRITE(SFR(ACC), r);                                 \
        WRITE(SFR(PSW), p);                                 \
    } GBL_STMT_END

    GBL_CTX_BEGIN(pSelf);

    EvmuCpu_* pSelf_ = EVMU_CPU_(pSelf);

    switch(opcode) {
    default:
        EVMU_API_ERROR("Invalid opcode!");
        break;
    case EVMU_OPCODE_NOP:
        break;
    case EVMU_OPCODE_BR:
        PC += OP(relativeS8);
        break;
    case EVMU_OPCODE_LD:
        WRITE(EVMU_ADDRESS_SFR_ACC, READ(OP(direct)));
        break;
    case EVMU_OPCODE_LD_IND:
        WRITE(SFR(ACC), READ(INDIRECT()));
        break;
    case EVMU_OPCODE_CALL:
        PUSH_PC();
        PC &= ~0xfff;
        PC |= OP(absolute);
        break;
    case EVMU_OPCODE_CALLR:
        PUSH_PC();
        PC += OP(relative16) - 1;
        break;
    case EVMU_OPCODE_BRF:
        PC += OP(relative16) - 1;
        break;
    case EVMU_OPCODE_ST:
        WRITE(OP(direct), READ(SFR(ACC)));
        break;
    case EVMU_OPCODE_ST_IND:
        WRITE(INDIRECT(), READ(SFR(ACC)));
        break;
    case EVMU_OPCODE_CALLF:
        PUSH_PC();
        PC = OP(absolute);
        break;
    case EVMU_OPCODE_JMPF:
        PC = OP(absolute);
        break;
    case EVMU_OPCODE_MOV:
        WRITE(OP(direct), OP(immediate));
        break;
    case EVMU_OPCODE_MOV_IND:
        WRITE(INDIRECT(), OP(immediate));
        break;
    case EVMU_OPCODE_JMP:
        PC &= ~0xfff;
        PC |= OP(absolute);
        break;
    case EVMU_OPCODE_MUL: {
        const int temp = (READ(SFR(C)) | (READ(SFR(ACC)) << 8)) * READ(SFR(B));
        WRITE(SFR(C),   (temp&0xff));
        WRITE(SFR(ACC), ((temp&0xff00)>>8));
        WRITE(SFR(B),   ((temp&0xff0000)>>16));
        PSW(CY, 0);
        PSW(OV, temp > 65535);
    }
    break;
    case EVMU_OPCODE_BEI:
        BR_CMP(READ(SFR(ACC)), ==, OP(immediate), OP(relativeS8));
        break;
    case EVMU_OPCODE_BE:
        BR_CMP(READ(SFR(ACC)), ==, READ(OP(direct)), OP(relativeS8));
        break;
    case EVMU_OPCODE_BE_IND:
        BR_CMP(READ(INDIRECT()), ==, OP(immediate), OP(relativeS8));
        break;
    case EVMU_OPCODE_DIV: {
        int r  =  READ(SFR(B)), s;
        if(r) {
            const int v = READ(SFR(C)) | (READ(SFR(ACC)) << 8);
            s = v%r;
            r = v/r;
        } else {
            r = 0xff00 | READ(SFR(C));
            s = 0;
        }
        WRITE(SFR(B),   s);
        WRITE(SFR(C),   r & 0xff);
        WRITE(SFR(ACC), (r&0xff00) >> 8);
        PSW(CY, 0);
        PSW(OV, !s);
        break;
    }
    case EVMU_OPCODE_BNEI:
        BR_CMP(READ(SFR(ACC)), !=, OP(immediate), OP(relativeS8));
        break;
    case EVMU_OPCODE_BNE:
        BR_CMP(READ(SFR(ACC)), !=, READ(OP(direct)), OP(relativeS8));
        break;
    case EVMU_OPCODE_BNE_IND:
        BR_CMP(READ(INDIRECT()), !=, OP(immediate), OP(relativeS8));
        break;
    case EVMU_OPCODE_BPC: {
        const EvmuWord value = READ_LATCH(OP(direct));
        const EvmuWord mask = (1u << OP(bit));
        if(value & mask) {
            WRITE(OP(direct), value & (~mask));
            PC += OP(relativeS8);
        }
        break;
    }
    case EVMU_OPCODE_LDF: {
        uint32_t flashAddr  =  (READ(SFR(FPR)) & SFR_MSK(FPR, ADDR)) << 16u;
        flashAddr           |=  READ(SFR(TRH)) << 8u;
        flashAddr           |=  READ(SFR(TRL));
#ifndef EVMU_WIP
        WRITE(SFR(ACC), READ_FLASH(flashAddr));
#endif
        break;
    }
    case EVMU_OPCODE_STF: {
        //#warning "OPCODE_STF NOT DONE!"
        const uint32_t flashAddr = READ(SFR(TRL)) | (READ(SFR(TRH)) << 8u);
        const uint8_t acc = READ(SFR(ACC));
#if 0
        if(READ(SFR(FPR)) & SFR_MSK(FPR, UNLOCK)) {
            switch(pSelf_->peripheral.pDevice->flashPrg.prgState) {
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
#endif
    }
    case EVMU_OPCODE_DBNZ:
        BR_DEC(OP(direct));
        break;
    case EVMU_OPCODE_DBNZ_IND:
        BR_DEC(INDIRECT());
        break;
    case EVMU_OPCODE_PUSH:
        PUSH(READ(OP(direct)));
        break;
    case EVMU_OPCODE_INC:
        WRITE(OP(direct), READ_LATCH(OP(direct)) + 1);
        break;
    case EVMU_OPCODE_INC_IND: {
        const EvmuAddress addr = INDIRECT();
        WRITE(addr, READ_LATCH(addr) + 1);
        break;
    }
    case EVMU_OPCODE_BP:
        BR(READ(OP(direct)) & (0x1 << OP(bit)), OP(relativeS8));
        break;
    case EVMU_OPCODE_POP:
        WRITE(OP(direct), POP());
        break;
    case EVMU_OPCODE_DEC:
        WRITE(OP(direct), READ_LATCH(OP(direct)) - 1);
        break;
    case EVMU_OPCODE_DEC_IND: {
        const EvmuAddress addr = INDIRECT();
        WRITE(addr, READ_LATCH(addr) - 1);
        break;
    }
    case EVMU_OPCODE_BZ:
        BR(!READ(SFR(ACC)), OP(relativeS8));
        break;
    case EVMU_OPCODE_ADDI:
        OP_ADD(OP(immediate));
        break;
    case EVMU_OPCODE_ADD:
        OP_ADD(READ(OP(direct)));
        break;
    case EVMU_OPCODE_ADD_IND:
        OP_ADD(INDIRECT());
        break;
    case EVMU_OPCODE_BN:
        BR(!(READ(OP(direct)) & (0x1 << OP(bit))), OP(relativeS8));
        break;
    case EVMU_OPCODE_BNZ:
        BR(READ(SFR(ACC)), OP(relativeS8));
        break;
    case EVMU_OPCODE_ADDCI:
        OP_ADD_CARRY(OP(immediate));
        break;
    case EVMU_OPCODE_ADDC:
        OP_ADD_CARRY(READ(OP(direct)));
        break;
    case EVMU_OPCODE_ADDC_IND:
        OP_ADD_CARRY(READ(INDIRECT()));
        break;
    case EVMU_OPCODE_RET:
        POP_PC();
        break;
    case EVMU_OPCODE_SUBI:
        OP_SUB(OP(immediate));
        break;
    case EVMU_OPCODE_SUB:
        OP_SUB(READ(OP(direct)));
        break;
    case EVMU_OPCODE_SUB_IND:
        OP_SUB(READ(INDIRECT()));
        break;
    case EVMU_OPCODE_NOT1:
        WRITE(OP(direct), READ_LATCH(OP(direct)) ^ (0x1u << OP(bit)));
        break;
    case EVMU_OPCODE_RETI:
     //   EvmuPic__reti_(pSelf_->pPic);
        break;
    case EVMU_OPCODE_SUBCI:
        OP_SUB_CARRY(OP(immediate));
        break;
    case EVMU_OPCODE_SUBC:
        OP_SUB_CARRY(READ(OP(direct)));
        break;
    case EVMU_OPCODE_SUBC_IND:
        OP_SUB_CARRY(READ(INDIRECT()));
        break;
    case EVMU_OPCODE_ROR: {
        const EvmuWord value = READ(SFR(ACC));
        WRITE(SFR(ACC), ((value & 0x1) << 7u) | (value >> 1u));
        break;
    }
    case EVMU_OPCODE_LDC: {//Load from IMEM (flash/rom) not ROM?
        EvmuAddress address =   READ(SFR(ACC));
        address             +=  READ(SFR(TRL));
        address             |=  READ(SFR(TRH)) << 8u;
        WRITE(SFR(ACC), READ_EXT(address));
        break;
    case EVMU_OPCODE_XCH: {
        EvmuWord acc   = READ(SFR(ACC));
        EvmuWord mem   = READ(OP(direct));
        acc            ^= mem;
        mem            ^= acc;
        acc            ^= mem;
        WRITE(SFR(ACC), acc);
        WRITE(OP(direct), mem);
        break;
    }
    case EVMU_OPCODE_XCH_IND: {
        const EvmuAddress address = INDIRECT();
        EvmuWord acc = READ(SFR(ACC));
        EvmuWord mem = READ(address);
        acc          ^= mem;
        mem          ^= acc;
        acc          ^= mem;
        WRITE(SFR(ACC), acc);
        WRITE(address, mem);
        break;
    }
    case EVMU_OPCODE_CLR1:
        WRITE(OP(direct), READ_LATCH(OP(direct)) & ~(1u << OP(bit)));
        break;
    case EVMU_OPCODE_RORC: {
        const int r = READ(SFR(ACC));
        const int s = READ(SFR(B));
        WRITE(SFR(B), (s&0x7f)|((r&1)<<7));
        WRITE(SFR(ACC), (r>>1)|(s&0x80));
        break;
    }
    case EVMU_OPCODE_ORI:
        LOGIC_OP(|, OP(immediate));
        break;
    case EVMU_OPCODE_OR:
        LOGIC_OP(|, OP(direct));
        break;
    case EVMU_OPCODE_OR_IND:
        LOGIC_OP(|, READ(INDIRECT()));
        break;
    case EVMU_OPCODE_ROL: {
        const EvmuWord value = READ(SFR(ACC));
        WRITE(SFR(ACC), (value << 1u) | ((value & 0x80) >> 7u));
        break;
    }
    case EVMU_OPCODE_ANDI:
        LOGIC_OP(&, OP(immediate));
        break;
    case EVMU_OPCODE_AND:
        LOGIC_OP(&, OP(direct));
        break;
    case EVMU_OPCODE_AND_IND:
        LOGIC_OP(&, READ(INDIRECT()));
        break;
    case EVMU_OPCODE_SET1:
        WRITE(OP(direct), READ_LATCH(OP(direct)) | (1u << OP(bit)));
        break;
    case EVMU_OPCODE_ROLC:  {
        const int r = READ(SFR(ACC));
        const int s = READ(SFR(B));
        WRITE(SFR(B), (s&0x7f)|(r&0x80));
        WRITE(SFR(ACC), (r<<1)|((s&0x80)>>7));
        break;
    }
    case EVMU_OPCODE_XORI:
        LOGIC_OP(^, OP(immediate));
        break;
    case EVMU_OPCODE_XOR:
        LOGIC_OP(^, OP(direct));
        break;
    case EVMU_OPCODE_XOR_IND:
        LOGIC_OP(^, READ(INDIRECT()));
        break;
    }
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuCpu_runCycle_(EvmuCpu* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuCpu_* pSelf_ = EVMU_CPU_(pSelf);

    // First cycle: fetch, decode
    if(!pSelf_->curInstr.elapsedCycles) {
        EvmuWord instrBuffer[EVMU_INSTRUCTION_BYTE_MAX] = { 0 };
        GblSize buffSize = EVMU_INSTRUCTION_BYTE_MAX;

        // Copy Instruction data from EXT to local buffer
#ifndef EVMU_WIP
        GBL_CTX_CALL(EvmuMemory_readExtBytes(EVMU_MEMORY_PUBLIC(pSelf_->pMemory),
                                             pSelf_->pc,
                                             instrBuffer,
                                             &buffSize));
#endif
        // Fetch Instruction from local buffer
        GBL_CTX_VERIFY_CALL(EvmuIsa_fetch(&pSelf_->curInstr.encoded, instrBuffer, &buffSize));

        // Decode instruction
        GBL_CTX_VERIFY_CALL(EvmuIsa_decode(&pSelf_->curInstr.encoded, &pSelf_->curInstr.decoded));

        // Store instruction format
        pSelf_->curInstr.pFormat = EvmuIsa_format(pSelf_->curInstr.decoded.opcode);

        ++pSelf_->curInstr.elapsedCycles;

    // Still operating on instruction...
    } else if(pSelf_->curInstr.pFormat && pSelf_->curInstr.elapsedCycles < pSelf_->curInstr.pFormat->clockCycles) {
        ++pSelf_->curInstr.elapsedCycles;
    } else {
        EVMU_API_ERROR("CPU is in an invalid state!");
    }

    // Complete instruction execution
    if(pSelf_->curInstr.elapsedCycles == pSelf_->curInstr.pFormat->clockCycles) {

        // Update PC to point to the next instruction
        GBL_CTX_VERIFY_CALL(EvmuCpu_setPc(pSelf, pSelf_->pc + pSelf_->curInstr.encoded.byteCount));

        // Execute instruction
        GBL_CTX_VERIFY_CALL(EvmuCpu_executeDecoded(pSelf, &pSelf_->curInstr.decoded));

        // Reset current instruction state so next call to this function begins fetching
        memset(&pSelf_->curInstr, 0, sizeof(pSelf_->curInstr));
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuCpu_clockEvent_(EvmuPeripheral* pSelf, EvmuClockEvent* pEvent) {
    GBL_RESULT result = GBL_RESULT_SUCCESS;

    switch(pEvent->signal) {
    case EVMU_CLOCK_SIGNAL_CYCLE:
        if(EvmuWave_hasChangedEdgeRising(&pEvent->wave))
            result = EvmuCpu_runCycle_(EVMU_CPU(pSelf));
        break;
    default: break;
    }

    return result;
}

static GBL_RESULT EvmuCpu_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(pSelf);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    EVMU_API_INFO("Resetting VMU CPU.");

    EVMU_CPU_(pSelf)->pc = 0x0;

    GBL_CTX_END();
}

static GBL_RESULT EvmuCpuClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_PERIPHERAL_CLASS(pClass)->pFnClockEvent = EvmuCpu_clockEvent_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuCpu_reset_;

    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuCpu_type(void) {
    static GblType type = GBL_INVALID_TYPE;
    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuCpu"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &(const GblTypeInfo) {
                                          .pFnClassInit        = EvmuCpuClass_init_,
                                          .classSize           = sizeof(EvmuCpuClass),
                                          .instanceSize        = sizeof(EvmuCpu),
                                          .instancePrivateSize = sizeof(EvmuCpu_)
                                      },
                                      GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }
    return type;
}


#if 0


/* EVMU_CPU particulars
 *
 *
 *
// log previous X instructions?
// profiling + instrumentation shit

//EvmuCpu_call
 *
 * 1) enable/disable warnings
 *    - stack overflow
 *    - flash write warning (maybe make flash handle it)
 *
 * 2) Events
 *    - instruction executed
 *    - stack changed
 *    - PSW changed
 *    - PC changed
 *    - change instruction
 *
 * 3) Debug commands:
 *    - execute asm directly?
 *    - log every instruction execution
 */



#endif
#if 0
extern int _gyVmuInterruptRetiInstr(struct VMUDevice* dev);


//#define VMU_DEBUG

#define SGNEXT(n) ((n)&0x80? (n)-0x100:(n))

//    ((b < 1)?((UCHAR_MAX + b >= a)?1:0):((b<=a)?1:0))


int gyVmuCpuInstrExecuteNext(VMUDevice* device) {
    //Fetch instruction
    memset(&device->curInstr, 0, sizeof(VMUInstr));
    gyVmuInstrFetch(&device->imem[device->pc], &device->curInstr);

    //Advance program counter
    device->pc += device->curInstr.bytes;

    //Entire instruction has been loaded
    if(device->curInstr.bytes >= _instrMap[device->curInstr.instrBytes[INSTR_BYTE_OPCODE]].bytes) {
//#ifdef VMU_DEBUG
#if 0
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
#ifdef VMU_DEBUG
        _gyLog(GY_DEBUG_VERBOSE, "%s", _biosDisassembly[prevPc]);
        _gyPush();
#endif
            _biosDisassemblyInstrBytes[prevPc] = device->curInstr.bytes;
        }

        //Execute instructions
        gyVmuCpuInstrExecute(device, &device->curInstr, &operands);

        static int wasInFw = 0;

        //Check if we entered the firmware
        if(!(device->sfr[SFR_OFFSET(SFR_ADDR_EXT)]&SFR_EXT_MASK)) {
            if(!device->biosLoaded) {
                //handle the BIOS call in software if no firwmare has been loaded
                if((device->pc = gyVmuBiosHandleCall(device)))
                    //jump back to USER mode before resuming execution.
                    gyVmuMemWrite(device, SFR_ADDR_EXT, gyVmuMemRead(device, SFR_ADDR_EXT)|SFR_EXT_USER);
            } else if(!wasInFw){
               // if(dbgEnabled(device)) _gyLog(GY_DEBUG_VERBOSE, "Entering firmware: %d", device->pc);
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
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_PCON)] & SFR_PCON_HALT_MASK))
            gyVmuCpuInstrExecuteNext(dev);

#if 1
        gyVmuInterruptControllerUpdate(dev);
#else
        _serviceInterrupts(dev);
#endif
        gyVmuTimersUpdate(dev);
       // gyVmuPort1PollRecv(dev);
        float cpuTime = gyVmuOscSecPerCycle(dev)*(float)_instrMap[dev->curInstr.instrBytes[INSTR_BYTE_OPCODE]].cc;
        time += cpuTime;
        //gyVmuSerialUpdate(dev, cpuTime);
        gyVmuDisplayUpdate(dev, cpuTime);
        ++cycle;
    }

    return cycle;
}

#endif

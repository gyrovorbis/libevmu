#include <evmu/evmu_api.h>
#include "evmu_cpu_.h"
#include "evmu_device_.h"
#include <evmu/hw/evmu_sfr.h>

static GBL_RESULT EvmuCpu_constructor_(GblObject* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructor, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuCpu_destructor_(GblBox* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuCpu_constructed_(GblObject* pSelf) {
    GBL_API_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);

    EvmuDevice* pDev = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    GBL_API_VERIFY_EXPRESSION(pDev);

    GBL_API_END();
}

static GBL_RESULT EvmuCpu_reset_(EvmuBehavior* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuBehavior, pFnReset, (EvmuBehavior*)pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuCpu_update_(EvmuBehavior* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuBehavior, pFnUpdate, (EvmuBehavior*)pSelf, ticks);
    GBL_API_END();
}

static GBL_RESULT EvmuCpuClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);

    EVMU_BEHAVIOR_CLASS(pClass)->pFnReset    = EvmuCpu_reset_;
    EVMU_BEHAVIOR_CLASS(pClass)->pFnUpdate   = EvmuCpu_update_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor = EvmuCpu_constructor_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructed = EvmuCpu_constructed_;
    GBL_BOX_CLASS(pClass)->pFnDestructor     = EvmuCpu_destructor_;

    GBL_API_END();
}

GBL_EXPORT GblType EvmuCpu_type(void) {
    static GblType type = GBL_INVALID_TYPE;
    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuCpu"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &(const GblTypeInfo) {
                                          .pFnClassInit        = EvmuCpuClass_init_,
                                          .classSize           = sizeof(EvmuCpuClass),
                                          .instanceSize        = sizeof(EvmuCpu),
                                          .instancePrivateSize = sizeof(EvmuCpu_)
                                      },
                                      GBL_TYPE_FLAGS_NONE);
        GBL_API_VERIFY_LAST_RECORD();
        GBL_API_END_BLOCK();
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

GBL_DECLARE_ENUM(EVMU_CPU_PROPERTY) {
    EVMU_CPU_PROPERTY_PROGRAM_COUNTER = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_CPU_PROPERTY_TICKS,
    EVMU_CPU_PROPERTY_CLOCK_SOURCE, //SCLK, MLK or whatever
    EVMU_CPU_PROPERTY_INSTRUCTION_SOURCE, //Flash or ROM
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPCODE,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPERAND_1,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPERAND_2,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPERAND_3,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_CYCLES,
    EVMU_CPU_PROPERTY_COUNT
};


EVMU_API evmuCpuInstructionCurrent(EvmuCpu hCpu, EvmuAddress* pPc, uint8_t* pOpcode, EvmuDecodedOperands* pOperands, EvmuWord* pIndirectAddress, uint8_t* pElapsedCycles);
EVMU_API evmuCpuProgramCounterSet(EvmuCpu hCpu, EvmuAddress pc);
EVMU_API evmuCpuInstructionExecute(EvmuCpu hCpu, const EvmuInstruction* pInstruction);

EVMU_API evmuCpuRegisterIndirectAddress(EvmuCpu hcpu, uint8_t reg, EvmuAddress* pAddress);
EVMU_API evmuCpuStackPush(EvmuCpu hCpu, const EvmuWord* pWords, EvmuSize* pCount);
EVMU_API evmuCpuStackPop(EvmuCpu hCpu, EvmuWord* pWords, EvmuSize* pCount);

// is halted
// get clock source




EVMU_API gyVmuCpuTick(EvmuCpu* pCpu, EvmuTicks ticks);

#if 0
int gyVmuCpuTick(struct VMUDevice* dev, float deltaTime);
int gyVmuCpuReset(struct VMUDevice* dev);
void gyVmuCpuInstrExecuteNext(struct VMUDevice* dev);
void gyVmuCpuInstrExecute(struct VMUDevice* dev, const VMUInstr* instr, const VMUInstrOperands* operands);
void gyVmuBiosDisassemblyPrints(char* buff);
#endif
#ifdef __cplusplus
}
#endif

EvmuAddress evmuCpuRegisterIndirectAddress_(const EvmuCpu_* pCpu, uint8_t reg) {
    return (evmuMemoryRead_(pCpu->pMemory,
                reg |
                ((evmuMemorySfrRead_(pCpu->pMemory, EVMU_ADDRESS_SFR_PSW) & (EVMU_SFR_PSW_IRBK0_MASK|EVMU_SFR_PSW_IRBK1_MASK)) >> 0x1u)) //Bits 2-3 come from PSW
   | (reg&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction
}

EVMU_API evmuCpuDriverInit_(EvmuCpu_* pCpu) {
    EVMU_API_BEGIN(pCpu);
    //EVMU_API__VERIFY_POINTER(pCpu->peripheral.pDevice);
    //EVMU_API__VERIFY_POINTER(pCpu->peripheral.pDevice->pMemory);
    pCpu->pMemory = pCpu->peripheral.pDevice->pMemory;

    EVMU_API_END();
}

EVMU_API evmuCpuRegisterIndirectAddress(EvmuCpu hCpu, uint8_t reg, EvmuAddress* pAddress) {
    EVMU_API_BEGIN(hCpu);
    EVMU_API_VERIFY_POINTER(pAddress);
    EVMU_API_VERIFY(reg <= 3);

    *pAddress = evmuCpuRegisterIndirectAddress_(hCpu, reg);

    EVMU_API_END();
}

EVMU_API evmuCpuInstructionDecodedExec_(EvmuCpu_* pCpu, uint8_t opcode, const EvmuDecodedOperands* pOperands);

static EVMU_RESULT evmuCpuRunCycle_(EvmuCpu hCpu) {
    EVMU_API_BEGIN(hCpu);

    // First cycle means we gotta decode and shit!
    if(!hCpu->curInstr.elapsedCycles) {
        // FETCH INSTRUCTION
        EvmuWord instrBuffer[EVMU_INSTRUCTION_BYTE_MAX] = { 0 };
        EvmuSize buffSize = EVMU_INSTRUCTION_BYTE_MAX;

        // Copy Instruction data from EXT to local buffer
        evmuMemoryExtRead(&hCpu->pMemory, hCpu->pc, instrBuffer, &buffSize, EVMU_MEMORY_ACCESS_OTHER);

        // Fetch Instruction from local buffer
        EVMU_API_VERIFY(evmuInstructionFetch(&hCpu->curInstr.encoded, instrBuffer, buffSize));

        // Store opcode
        EVMU_API_VERIFY(evmuInstructionPeek(hCpu->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE],
                                           &hCpu->curInstr.opcode));

        // Store decoded operands
        EVMU_API_VERIFY(evmuInstructionDecodeOperands(&hCpu->curInstr.encoded, &hCpu->curInstr.operands));

        // Store instruction format
        EVMU_API_VERIFY(evmuInstructionFormat(hCpu->curInstr.opcode,
                                             &hCpu->curInstr.pFormat));

        ++hCpu->curInstr.elapsedCycles;

    // Still operating on instruction...
    } else if(hCpu->curInstr.pFormat && hCpu->curInstr.elapsedCycles < hCpu->curInstr.pFormat->clockCycles) {

        ++hCpu->curInstr.elapsedCycles;
    } else {
        EVMU_API_ERROR("CPU is in an invalid state!");
    }

    if(hCpu->curInstr.elapsedCycles == hCpu->curInstr.pFormat->clockCycles) {
        // Update PC to point to the next instruction
        EVMU_API_RESULT_ACCUM(evmuCpuProgramCounterSet(hCpu, hCpu->pc + hCpu->curInstr.encoded.byteCount));

        // Execute instruction
        EVMU_API_RESULT_ACCUM(evmuCpuInstructionDecodedExec_(hCpu, hCpu->curInstr.opcode, &hCpu->curInstr.operands));

        // Reset current instruction state so next call to this function begins fetching
        memset(&hCpu->curInstr, 0, sizeof(hCpu->curInstr));

    }

    EVMU_API_END();
}


EVMU_API evmuCpuRun(EvmuCpu hCpu, EvmuCycles cycles) {
    EVMU_API_BEGIN(hCpu);
    for(EvmuCycles c = 0; c < cycles; ++c) {
        EVMU_API_VERIFY(evmuCpuRunCycle_(hCpu));
    }
    EVMU_API_END();
}


EVMU_API evmuCpuDriverUpdate_(EvmuCpu_* pCpu, EvmuTicks ticks) {
    EVMU_API_BEGIN(pCpu);
    EvmuCycles cycles = 0;
    EVMU_API_VERIFY(evmuClockTicksToCycles(EVMU_PERIPHERAL_SIBLING_(pCpu, Clock), EVMU_CLOCK_SYSTEM_2, ticks, &cycles));
    EVMU_API_VERIFY(evmuCpuRun(pCpu, cycles));
    EVMU_API_END();
}



EVMU_API evmuCpuInstructionDecodedExec_(EvmuCpu_* pCpu, uint8_t opcode, const EvmuDecodedOperands* pOperands) {
    EVMU_API_BEGIN(pCpu);

#define PC \
        pCpu->pc

#define OP(NAME) \
    pOperands->NAME

#define SFR(NAME) \
    EVMU_ADDRESS_SFR_##NAME

#define SFR_MSK(NAME, FIELD) \
    EVMU_SFR_##NAME##_##FIELD##_MASK

#define SFR_POS(NAME, FIELD) \
    EVMU_SFR_##NAME##_##FIELD##_POS

#define READ(ADDR) \
    evmuMemoryRead_(pCpu->pMemory, ADDR)

#define READ_LATCH(ADDR) \
    evmuMemoryReadLatch_(pCpu->pMemory, ADDR)

#define WRITE(ADDR, VAL) \
    evmuMemoryWrite_(pCpu->pMemory, ADDR, VAL)

#define READ_EXT(ADDR) \
    evmuMemoryExtRead_(pCpu->pMemory, ADDR)

#define WRITE_EXT(ADDR, VAL) \
    evmuMemoryExtWrite_(pCpu->pMemory, ADDR, VAL)

#define READ_FLASH(ADDR) \
    evmuMemoryFlashRead_(pCpu->pMemory, ADDR)

#define WRITE_FLASH(ADDR, VAL) \
    evmuMemoryFlashWrite_(pCpu->pMemory, ADDR, VAL)

#define INDIRECT() \
    evmuCpuRegisterIndirectAddress_(pCpu, OP(indirect))

#define PUSH(VALUE) \
    evmuMemoryStackPush_(pCpu->pMemory, VALUE)

#define POP() \
    evmuMemoryStackPop_(pCpu->pMemory)

#define PUSH_PC()               \
    PUSH(PC & 0xff);            \
    PUSH((PC & 0xff00) >> 8u)

#define POP_PC()        \
    PC = POP() << 8u;   \
    PC |= POP()

#define PSW(FLAG, EXPR)                        \
    WRITE(SFR(PSW),                            \
    (READ(SFR(PSW)) & ~SFR_MSK(PSW, FLAG)) |   \
        ((EXPR)? SFR_MSK(PSW, FLAG) : 0))

#define LOGIC_OP(OP, RHS) \
    WRITE(SFR(ACC), READ(SFR(ACC)) OP (RHS))

#define BR(EXPR, OFFSET)    \
    if((EXPR)) PC += OFFSET

#define BR_DEC(ADDR)                                \
    const EvmuAddress addr = (ADDR);                \
    const EvmuWord value = READ_LATCH(addr) - 1;    \
    WRITE(addr, value);                             \
    BR(value != 0, OP(relative8.s8))

#define BR_CMP(VALUE1, OPERATOR, VALUE2, OFFSET) \
    const EvmuWord v1 = VALUE1;                  \
    const EvmuWord v2 = VALUE2;                  \
    PSW(CY, v1 < v2);                            \
    BR(v2 OPERATOR v2, OFFSET)


#define OP_ARITH(OP, RVALUE, CY_EN, CY_EXP, AC_EXP, OV_EXP)    \
    const EvmuWord a = READ(SFR(ACC));                  \
    const EvmuWord b = (RVALUE);                        \
    const EvmuWord c = CY_EN?                           \
        (SFR_MSK(PSW, CY) >> SFR_POS(PSW, CY)) :        \
        0;                                              \
    const EvmuWord r = a OP b OP c;                     \
    EvmuWord p  = READ(SFR(PSW)) & ~(SFR_MSK(PSW, CY) | \
                                    SFR_MSK(PSW, AC)  | \
                                    SFR_MSK(PSW, OV));  \
    p           |= (CY_EXP)? SFR_MSK(PSW, CY) : 0;      \
    p           |= (AC_EXP)? SFR_MSK(PSW, AC) : 0;      \
    p           |= (OV_EXP)? SFR_MSK(PSW, OV) : 0;      \
    WRITE(SFR(ACC), r);                                 \
    WRITE(SFR(PSW), p)

#define OP_ADD_CY_EXP (a+b+c>255)
#define OP_ADD_AC_EXP ((a&15)+((b+c)&15)>15)
#define OP_ADD_OV_EXP (0x80&(~a^(b+c))&((b+c)^(a+(b+c))))

#define OP_ADD_(RVALUE, CY_EN) \
    OP_ARITH(+, (RVALUE), CY_EN, OP_ADD_CY_EXP, OP_ADD_AC_EXP, OP_ADD_OV_EXP)

#define OP_ADD(RVALUE) \
    OP_ADD_(RVALUE, 0)

#define OP_ADD_CARRY(RVALUE) \
    OP_ADD_(RVALUE, 1)

#define OP_SUB_CY_EXP (a-b-c<0)
#define OP_SUB_AC_EXP ((a&15)-(b&15)-c<0)
#define UCHAR_SUB_OV(a, b) \
    ((b < 1)?((UCHAR_MAX + b >= a)?0:0):((b<=a)?0:0))
#define OP_SUB_OV_EXP (UCHAR_SUB_OV(a,b-c))

#define OP_SUB_(RVALUE, CY_EN) \
    OP_ARITH(-, (RVALUE), CY_EN, OP_SUB_CY_EXP, OP_SUB_AC_EXP, OP_SUB_OV_EXP)

#define OP_SUB(RVALUE) \
    OP_SUB_(RVALUE, 0)

#define OP_SUB_CARRY(RVALUE) \
    OP_SUB_(RVALUE, 1)

    switch(opcode) {
    case EVMU_OPCODE_NOP:
    default:
        break;
    case EVMU_OPCODE_BR:
        PC += OP(relative8.s8);
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
        int temp    =   READ(SFR(C)) | (READ(SFR(ACC)) << 8);
        temp        *=  READ(SFR(B));

        WRITE(SFR(C),   (temp&0xff));
        WRITE(SFR(ACC), ((temp&0xff00)>>8));
        WRITE(SFR(B),   ((temp&0xff0000)>>16));

        PSW(CY, 0);
        PSW(OV, temp > 65535);
    }
    break;
    case EVMU_OPCODE_BEI: {
        BR_CMP(READ(SFR(ACC)), ==, OP(immediate), OP(relative8.s8));
        break;
    }
    case EVMU_OPCODE_BE: {
        BR_CMP(READ(SFR(ACC)), ==, READ(OP(direct)), OP(relative8.s8));
        break;
    }
    case EVMU_OPCODE_BE_IND: {
        BR_CMP(READ(INDIRECT()), ==, OP(immediate), OP(relative8.s8));
        break;
    }
    case EVMU_OPCODE_DIV: {
        int r  =  READ(SFR(B));
        int s;

        if(r) {
            int v = READ(SFR(C)) | (READ(SFR(ACC)) << 8);
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
    }
        break;
    case EVMU_OPCODE_BNEI: {
        BR_CMP(READ(SFR(ACC)), !=, OP(immediate), OP(relative8.s8));
        break;
    }
    case EVMU_OPCODE_BNE:{
        BR_CMP(READ(SFR(ACC)), !=, READ(OP(direct)), OP(relative8.s8));
        break;
    }
    case EVMU_OPCODE_BNE_IND: {
        BR_CMP(READ(INDIRECT()), !=, OP(immediate), OP(relative8.s8));
        break;
    }
    case EVMU_OPCODE_BPC: {
        const EvmuWord value = READ_LATCH(OP(direct));
        const EvmuWord mask = (1u << OP(bit));
        if(value & mask) {
            WRITE(OP(direct), value & (~mask));
            PC += OP(relative8.s8);
        }
        break;
    }
    case EVMU_OPCODE_LDF: {
        uint32_t flashAddr  =   READ(SFR(FPR)) & SFR_MSK(FPR, ADDR) << 16u;
        flashAddr           |=  READ(SFR(TRH)) << 8u;
        flashAddr           |=  READ(SFR(TRL));
        WRITE(SFR(ACC), READ_FLASH(flashAddr));
        break;
    }
    case EVMU_OPCODE_STF: {
        //#warning "OPCODE_STF NOT DONE!"

        const uint32_t flashAddr = READ(SFR(TRL)) | (READ(SFR(TRH)) << 8u);
        const uint8_t acc = READ(SFR(ACC));
#if 0
        if(READ(SFR(FPR)) & SFR_MSK(FPR, UNLOCK)) {
            switch(pCpu->peripheral.pDevice->flashPrg.prgState) {
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

    case EVMU_OPCODE_DBNZ: {
        BR_DEC(OP(direct));
        break;
    }
    case EVMU_OPCODE_DBNZ_IND: {
        BR_DEC(INDIRECT());
        break;
    }
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
        BR(READ(OP(direct)) & (0x1 << OP(bit)), OP(relative8.s8));
        break;
    case EVMU_OPCODE_POP:
        WRITE(OP(direct), POP());
        break;
    case EVMU_OPCODE_DEC:
        WRITE(OP(direct), READ_LATCH(OP(direct)) - 1);
        break;
    case EVMU_OPCODE_DEC_IND:{
        const EvmuAddress addr = INDIRECT();
        WRITE(addr, READ_LATCH(addr) - 1);
        break;
    }
    case EVMU_OPCODE_BZ:
        BR(!READ(SFR(ACC)), OP(relative8.s8));
        break;
    case EVMU_OPCODE_ADDI: {
        OP_ADD(OP(immediate));
        break;
    }
    case EVMU_OPCODE_ADD: {
        OP_ADD(READ(OP(direct)));
        break;
    }
    case EVMU_OPCODE_ADD_IND: {
        OP_ADD(INDIRECT());
        break;
    }
    case EVMU_OPCODE_BN:
        BR(!(READ(OP(direct)) & (0x1 << OP(bit))), OP(relative8.s8));
        break;
    case EVMU_OPCODE_BNZ:
        BR(READ(SFR(ACC)), OP(relative8.s8));
        break;
    case EVMU_OPCODE_ADDCI: {
        OP_ADD_CARRY(OP(immediate));
        break;
    }
    case EVMU_OPCODE_ADDC: {
        OP_ADD_CARRY(READ(OP(direct)));
        break;
    }
    case EVMU_OPCODE_ADDC_IND: {
        OP_ADD_CARRY(READ(INDIRECT()));
        break;
    }
    case EVMU_OPCODE_RET:
        POP_PC();
        break;
    case EVMU_OPCODE_SUBI: {
        OP_SUB(OP(immediate));
        break;
    }
    case EVMU_OPCODE_SUB: {
        OP_SUB(READ(OP(direct)));
        break;
    }
    case EVMU_OPCODE_SUB_IND:{
        OP_SUB(READ(INDIRECT()));
        break;
    }
    case EVMU_OPCODE_NOT1: {
        const EvmuWord value = READ_LATCH(OP(direct));
        const EvmuWord mask  = (0x1u << OP(bit));
        WRITE(OP(direct), value ^ mask);
        break;
    }
    case EVMU_OPCODE_RETI: {
        evmuPicRetiInstr_(EVMU_PERIPHERAL_SIBLING_(pCpu, Pic));
        break;
    }
    case EVMU_OPCODE_SUBCI: {
        OP_SUB_CARRY(OP(immediate));
        break;
    }
    case EVMU_OPCODE_SUBC: {
        OP_SUB_CARRY(READ(OP(direct)));
        break;
    }
    case EVMU_OPCODE_SUBC_IND:{
        OP_SUB_CARRY(READ(INDIRECT()));
        break;
    }
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
        const
        EvmuAddress address = INDIRECT();
        EvmuWord acc        = READ(SFR(ACC));
        EvmuWord mem        = READ(address);
        acc                 ^= mem;
        mem                 ^= acc;
        acc                 ^= mem;
        WRITE(SFR(ACC), acc);
        WRITE(address, mem);
        break;
    }
    case EVMU_OPCODE_CLR1: {
        WRITE(OP(direct), READ_LATCH(OP(direct)) & ~(1u << OP(bit)));
        break;
    }
    case EVMU_OPCODE_RORC: {
        int r = READ(SFR(ACC));
        int s = READ(SFR(B));
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
    case EVMU_OPCODE_SET1: {
        WRITE(OP(direct), READ_LATCH(OP(direct)) | (1u << OP(bit)));
        break;
    }
    case EVMU_OPCODE_ROLC:  {
        int r = READ(SFR(ACC));
        int s = READ(SFR(B));
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

    EVMU_API_END();
}

#endif
#if 0
extern int _gyVmuInterruptRetiInstr(struct VMUDevice* dev);


//#define VMU_DEBUG

#define SGNEXT(n) ((n)&0x80? (n)-0x100:(n))

//    ((b < 1)?((UCHAR_MAX + b >= a)?1:0):((b<=a)?1:0))


static inline int _fetchRegIndAddr(struct VMUDevice* dev, uint8_t reg) {
    assert(reg <= 3); //Make sure we're within bounds
    return (gyVmuMemRead(dev,
                reg |
                ((dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]&(SFR_PSW_IRBK0_MASK|SFR_PSW_IRBK1_MASK))>>0x1u)) //Bits 2-3 come from PSW
   | (reg&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction
}


void _gyVmuPush(VMUDevice* dev, unsigned val) {
    const unsigned sp = dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]+1;

    if(sp > RAM_STACK_ADDR_END) {
        _gyLog(GY_DEBUG_WARNING, "PUSH: Stack overflow detected!");
    }

    dev->ram[0][++dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]] = val;
}

int _gyVmuPop(VMUDevice* dev) {
    const unsigned sp = dev->sfr[SFR_OFFSET(SFR_ADDR_SP)];

    if(sp < RAM_STACK_ADDR_BASE) {
        _gyLog(GY_DEBUG_WARNING, "POP: Stack underflow detected!");
    }

    return dev->ram[0][dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]--];
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
        _gyVmuPush(dev, (dev->pc&0xff));
        _gyVmuPush(dev, (dev->pc&0xff00)>>8u);
        dev->pc &= ~0xfff;
        dev->pc |= operands->addrMode[ADDR_MODE_ABS];
        break;
    case OPCODE_CALLR:
        _gyVmuPush(dev, (dev->pc&0xff));
        _gyVmuPush(dev, (dev->pc&0xff00)>>8u);
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
        _gyVmuPush(dev, (dev->pc&0xff));
        _gyVmuPush(dev, (dev->pc&0xff00)>>8u);
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
        _gyVmuPush(dev, gyVmuMemRead(dev, operands->addrMode[ADDR_MODE_DIR]));
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
        gyVmuMemWrite(dev, operands->addrMode[ADDR_MODE_DIR], _gyVmuPop(dev));
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
        int r   =   _gyVmuPop(dev)<<8u;
        r       |=  _gyVmuPop(dev);
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
        _gyVmuInterruptRetiInstr(dev);
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

int gyVmuCpuReset(VMUDevice* dev) {
    _gyLog(GY_DEBUG_VERBOSE, "Resetting VMU CPU.");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);


    memset(dev->ram, 0, RAM_BANK_SIZE*RAM_BANK_COUNT);
    memset(dev->sfr, 0, RAM_SFR_SIZE);
    memset(dev->wram, 0, WRAM_SIZE);
    memset(dev->xram, 0, XRAM_BANK_COUNT*XRAM_BANK_SIZE);


    dev->memMap[VMU_MEM_SEG_XRAM]       = dev->xram[VMU_XRAM_BANK_LCD_TOP];
    dev->memMap[VMU_MEM_SEG_SFR]        = dev->sfr;
    dev->sfr[SFR_OFFSET(SFR_ADDR_SP)]   = RAM_STACK_ADDR_BASE-1;    //Initialize stack pointer
    dev->sfr[SFR_OFFSET(SFR_ADDR_P3)]   = 0xff;                     //Reset all P3 pins (controller buttons)
    dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)]  = SFR_PSW_RAMBK0_MASK;


    gyVmuMemWrite(dev, SFR_ADDR_P7, SFR_P7_P71_MASK);
    gyVmuMemWrite(dev, SFR_ADDR_IE, 0xff);
    gyVmuMemWrite(dev, SFR_ADDR_IP, 0x00);
    gyVmuMemWrite(dev, SFR_ADDR_P1FCR,  0xbf);
    gyVmuMemWrite(dev, SFR_ADDR_P3INT,  0xfd);
    gyVmuMemWrite(dev, SFR_ADDR_ISL,    0xc0);
    gyVmuMemWrite(dev, SFR_ADDR_VSEL,   0xf4);
    gyVmuMemWrite(dev, SFR_ADDR_VSEL,   0xfc);
//    gyVmuMemWrite(dev, SFR_ADDR_BTCR,   0x40);

    dev->timer0.tscale = 256;
    dev->pc = 0x0;

    if(dev->biosLoaded) {
       // dev->sfr[SFR_OFFSET(SFR_ADDR_P7)]   |= SFR_P7_P71_MASK;
        dev->memMap[VMU_MEM_SEG_GP1]        = dev->ram[VMU_RAM_BANK0];
        dev->memMap[VMU_MEM_SEG_GP2]        = &dev->ram[VMU_RAM_BANK0][VMU_MEM_SEG_SIZE];
        dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)]  = 0;
        dev->imem = dev->rom;

    } //else {

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
        if(!dev->biosLoaded) {
            dev->memMap[VMU_MEM_SEG_GP1]    = dev->ram[VMU_RAM_BANK1];
            dev->memMap[VMU_MEM_SEG_GP2]    = &dev->ram[VMU_RAM_BANK1][VMU_MEM_SEG_SIZE];
            dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)] = 1;
            dev->imem = dev->flash;
        }
        gyVmuMemWrite(dev, SFR_ADDR_XBNK, VMU_XRAM_BANK_ICN);
        gyVmuMemWrite(dev, SFR_ADDR_XRAM_ICN_GAME, 0x10);           //Enable Game Icon

        //SFR values initialized by BIOS (from Sega Documentation)
        gyVmuMemWrite(dev, SFR_ADDR_P1FCR,  0xbf);
        gyVmuMemWrite(dev, SFR_ADDR_P3INT,  0xfd);
        gyVmuMemWrite(dev, SFR_ADDR_ISL,    0xc0);
        gyVmuMemWrite(dev, SFR_ADDR_VSEL,   0xfc);
        gyVmuMemWrite(dev, SFR_ADDR_BTCR,   0x40);

        //dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] = SFR_IE_IE7_MASK;
        gyVmuMemWrite(dev, SFR_ADDR_IE, 0xff);
        gyVmuMemWrite(dev, SFR_ADDR_IP, 0x00);
        gyVmuMemWrite(dev, SFR_ADDR_OCR, SFR_OCR_OCR7_MASK|SFR_OCR_OCR0_MASK); //stop main clock, divide active clock by 6
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] = SFR_P7_P71_MASK;

        gyVmuMemWrite(dev, SFR_ADDR_XBNK, VMU_XRAM_BANK_LCD_TOP);
        gyVmuMemWrite(dev, SFR_ADDR_VCCR, SFR_VCCR_VCCR7_MASK);     //turn on LCD
        gyVmuMemWrite(dev, SFR_ADDR_MCR, SFR_MCR_MCR3_MASK);        //enable LCD update
        gyVmuMemWrite(dev, SFR_ADDR_PCON, 0);                      //Disable HALT/HOLD modes, run CPU normally.

   // }

    return 1;
}
#endif

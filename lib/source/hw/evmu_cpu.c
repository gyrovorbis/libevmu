#include <evmu/evmu_api.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/events/evmu_memory_event.h>
#include <evmu/hw/evmu_isa.h>
#include "evmu_cpu_.h"
#include "evmu_device_.h"
#include "evmu_memory_.h"
#include "evmu_clock_.h"
#include "evmu_pic_.h"
#include "evmu_gamepad_.h"
#include "evmu_timers_.h"
#include "../types/evmu_peripheral_.h"
#include <gimbal/meta/signals/gimbal_marshal.h>

EVMU_EXPORT EvmuPc EvmuCpu_pc(const EvmuCpu* pSelf) {
    return EVMU_CPU_(pSelf)->pc;
}

EVMU_EXPORT void EvmuCpu_setPc(EvmuCpu* pSelf, EvmuPc address) {
    EvmuCpu_* pSelf_ = EVMU_CPU_(pSelf);

    // Only update if PC actually changed
    if(pSelf_->pc != address) {
        pSelf_->pc = address;

        // Update flag for polling
        pSelf->pcChanged = GBL_TRUE;

        //Notify debugger/UI of next instruction executing
        GblSignal_emit(GBL_INSTANCE(pSelf), "pcChange", address);
    }
}

EVMU_EXPORT EvmuWord EvmuCpu_opcode(const EvmuCpu* pSelf) {
    return EVMU_CPU_(pSelf)->curInstr.pFormat->opcode;
}

EVMU_EXPORT int32_t EvmuCpu_operand(const EvmuCpu* pSelf, size_t operand) {
    int32_t value = 0;

    GBL_CTX_BEGIN(NULL);

    EvmuCpu_* pSelf_ = EVMU_CPU_(pSelf);
    const EvmuInstructionFormat* pFmt = pSelf_->curInstr.pFormat;
    const EvmuOperands* pOperands = &pSelf_->curInstr.decoded.operands;

    GBL_CTX_VERIFY_ARG(operand < EVMU_ISA_ARGC(pFmt->args));

    switch(EVMU_ISA_ARG_FORMAT_EXTRACT(pFmt->args, operand)) {
    case EVMU_ISA_ARG_TYPE_RELATIVE_8:  value = pOperands->relative8;  break;
    case EVMU_ISA_ARG_TYPE_RELATIVE_16: value = pOperands->relative16; break;
    case EVMU_ISA_ARG_TYPE_IMMEDIATE_8: value = pOperands->immediate;  break;
    case EVMU_ISA_ARG_TYPE_DIRECT_9:    value = pOperands->direct;     break;
    case EVMU_ISA_ARG_TYPE_INDIRECT_2:  value = pOperands->indirect;   break; // MAYBE we could fetch this?
    case EVMU_ISA_ARG_TYPE_ABSOLUTE_12:
    case EVMU_ISA_ARG_TYPE_ABSOLUTE_16: value = pOperands->absolute;   break;
    case EVMU_ISA_ARG_TYPE_BIT_3:       value = pOperands->bit;        break;
    default: break;
    }

    GBL_CTX_END_BLOCK();
    return value;
}


EVMU_EXPORT double EvmuCpu_secsPerInstruction(const EvmuCpu* pSelf) {
    EvmuCpu_*    pSelf_  = EVMU_CPU_(pSelf);
    EvmuMemory_* pMemory = pSelf_->pMemory;
    EvmuClock*   pClock  = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf))->pClock;

    return EvmuClock_systemSecsPerCycle(pClock) *
            ((!(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PCON)] & EVMU_SFR_PCON_HALT_MASK))?
                         (double)_instrMap[pSelf_->curInstr.encoded.bytes[INSTR_BYTE_OPCODE]].cc : 1.0);

}

EVMU_EXPORT size_t EvmuCpu_cyclesPerInstruction(const EvmuCpu* pSelf) {
    EvmuCpu_* pSelf_  = EVMU_CPU_(pSelf);
    return EvmuIsa_format(pSelf_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE])->cc;
}


EVMU_EXPORT EVMU_RESULT EvmuCpu_execute(EvmuCpu* pSelf, const EvmuDecodedInstruction* pInstr) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuCpu, pFnExecute, pSelf, pInstr);
    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuCpu_runNext(EvmuCpu* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuCpu, pFnRunNext, pSelf);
    GBL_CTX_END();
}

static EVMU_RESULT EvmuCpu_fetch_(EvmuCpu* pSelf, EvmuPc pc, EvmuInstruction* pInstr) {
    GBL_CTX_BEGIN(NULL);

    EvmuCpu_* pSelf_ = EVMU_CPU_(pSelf);
    size_t sourceSize = 4; //bullshit, fix me
    GBL_CTX_VERIFY_CALL(EvmuIsa_fetch(pInstr,
                                      &pSelf_->pMemory->pExt[pc],
                                      &sourceSize));
    GBL_CTX_END();
}

static EVMU_RESULT EvmuCpu_decode_(EvmuCpu* pSelf, const EvmuInstruction* pEncoded, EvmuDecodedInstruction* pDecoded) {
    GBL_UNUSED(pSelf);
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_CALL(EvmuIsa_decode(pEncoded, pDecoded));

    GBL_CTX_END();
}

static EVMU_RESULT EvmuCpu_runNext_(EvmuCpu* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EvmuCpu_*    pSelf_   = EVMU_CPU_(pSelf);
    EvmuMemory_* pMemory_ = pSelf_->pMemory;
    EvmuMemory*  pMemory  = EVMU_MEMORY_PUBLIC_(pMemory_);
    EvmuDevice*  pDevice  = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuRom*     pRom     = pDevice->pRom;

    // Fet instruction
    GBL_INSTANCE_VCALL(EvmuCpu, pFnFetch, pSelf, pSelf_->pc, &pSelf_->curInstr.encoded);
    pSelf_->curInstr.pFormat = EvmuIsa_format(pSelf_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE]);

    //Decode instruction
    GBL_INSTANCE_VCALL(EvmuCpu, pFnDecode, pSelf, &pSelf_->curInstr.encoded, &pSelf_->curInstr.decoded);

    //Advance program counter
    EvmuCpu_setPc(pSelf, EvmuCpu_pc(pSelf) + pSelf_->curInstr.pFormat->bytes);

    //Execute instructions
    GBL_INSTANCE_VCALL(EvmuCpu, pFnExecute, pSelf, &pSelf_->curInstr.decoded);

    //Check if we entered the firmware
    if(EvmuRom_biosActive(pRom)) {
        if(EvmuRom_biosType(pRom) == EVMU_BIOS_TYPE_EMULATED) {
            //handle the BIOS call in software if no firwmare has been loaded
            if((pSelf_->pc = EvmuRom_callBios(pRom, pSelf_->pc)))
                //jump back to USER mode before resuming execution.
                EvmuMemory_writeData(pMemory,
                                    EVMU_ADDRESS_SFR_EXT,
                                    EvmuMemory_readData(pMemory,
                                                       EVMU_ADDRESS_SFR_EXT) | 0x1);
        }
    }

    GBL_CTX_END();
}

static  EVMU_RESULT EvmuCpu_execute_(EvmuCpu* pSelf, const EvmuDecodedInstruction* pInstr) {
#define PC                      pSelf_->pc
#define OP(NAME)                pOperands->NAME
#define SFR(NAME)               EVMU_ADDRESS_SFR_##NAME
#define SFR_MSK(NAME, FIELD)    EVMU_SFR_##NAME##_##FIELD##_MASK
#define SFR_POS(NAME, FIELD)    EVMU_SFR_##NAME##_##FIELD##_POS
#define INDIRECT()              EvmuMemory_indirectAddress(pMemory, OP(indirect))
#define VIEW(ADDR)              EvmuMemory_viewData(pMemory, ADDR)
#define READ(ADDR)              EvmuMemory_readData(pMemory, ADDR)
#define READ_LATCH(ADDR)        EvmuMemory_readDataLatch(pMemory, ADDR)
#define WRITE(ADDR, VAL)        EvmuMemory_writeData(pMemory, ADDR, VAL)
#define READ_EXT(ADDR)          EvmuMemory_readProgram(pMemory, ADDR)
#define WRITE_EXT(ADDR, VAL)    EvmuMemory_writeProgram(pMemory, ADDR, VAL)
#define READ_FLASH(ADDR)        EvmuMemory_readFlash(pMemory, ADDR)
#define WRITE_FLASH(ADDR, VAL)  EvmuMemory_writeFlash(pMemory, ADDR, VAL)
#define PUSH(VALUE)             EvmuMemory_pushStack(pMemory, VALUE)
#define POP()                   EvmuMemory_popStack(pMemory)
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
#define OP_SUB_OV_(a, b)        (((int8_t)(a^(b)) < 0 && (int8_t)(b^(a-b)) >= 0))
#define OP_SUB_OV_EXP           (OP_SUB_OV_(a,(b+c)))
#define OP_SUB_(RVALUE, CY_EN)  OP_ARITH(-, (RVALUE), CY_EN, OP_SUB_CY_EXP, OP_SUB_AC_EXP, OP_SUB_OV_EXP)
#define OP_SUB(RVALUE)          OP_SUB_(RVALUE, 0)
#define OP_SUB_CARRY(RVALUE)    OP_SUB_(RVALUE, 1)

#define BR_DEC(ADDR)                                    \
    GBL_STMT_START {                                    \
        const EvmuAddress addr  = (ADDR);               \
        const EvmuWord    value = READ_LATCH(addr) - 1; \
        WRITE(addr, value);                             \
        BR(value != 0, OP(relative8));                  \
    } GBL_STMT_END

#define BR_CMP(VALUE1, OPERATOR, VALUE2, OFFSET) \
    GBL_STMT_START {                             \
        const EvmuWord v1 = VALUE1;              \
        const EvmuWord v2 = VALUE2;              \
        PSW(CY, v1 < v2);                        \
        BR(v1 OPERATOR v2, OFFSET);              \
    } GBL_STMT_END

#define OP_ARITH(OP, RVALUE, CY_EN, CY_EXP, AC_EXP, OV_EXP) \
    GBL_STMT_START {                                        \
        const EvmuWord a = READ(SFR(ACC));                  \
        const EvmuWord b = (RVALUE);                        \
        const EvmuWord c = CY_EN?                           \
            ((READ(SFR(PSW)) & SFR_MSK(PSW, CY))            \
                >> SFR_POS(PSW, CY)) : 0;                   \
        const EvmuWord r = a OP b OP c;                     \
        EvmuWord p  = READ(SFR(PSW)) & ~(SFR_MSK(PSW, CY) | \
                                         SFR_MSK(PSW, AC) | \
                                         SFR_MSK(PSW, OV)); \
        p           |= (CY_EXP)? SFR_MSK(PSW, CY) : 0;      \
        p           |= (AC_EXP)? SFR_MSK(PSW, AC) : 0;      \
        p           |= (OV_EXP)? SFR_MSK(PSW, OV) : 0;      \
        WRITE(SFR(ACC), r);                                 \
        WRITE(SFR(PSW), p);                                 \
    } GBL_STMT_END

    GBL_CTX_BEGIN(pSelf);

    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_POINTER(pInstr);

    EvmuCpu_*           pSelf_    = EVMU_CPU_(pSelf);
    EvmuMemory_*        pMemory_  = pSelf_->pMemory;
    EvmuMemory*         pMemory   = EVMU_MEMORY_PUBLIC_(pMemory_);
    EvmuDevice*         pDevice   = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_*        pDevice_  = EVMU_DEVICE_(pDevice);
    VMUDevice*          dev       = pDevice_->pReest;
    const EvmuOperands* pOperands = &pInstr->operands;

    switch(pInstr->opcode) {
    default:
        EVMU_LOG_ERROR("Invalid opcode!");
        break;
    case EVMU_OPCODE_NOP:
        break;
    case EVMU_OPCODE_BR:
        PC += OP(relative8);
        break;
    case EVMU_OPCODE_LD:
        WRITE(SFR(ACC), READ(OP(direct)));
        break;
    case EVMU_OPCODE_LD_IND:
        WRITE(SFR(ACC), READ(INDIRECT()));
        break;
    case EVMU_OPCODE_CALL:
        PUSH_PC();
        PC &= ~0xfff;
        PC |= (OP(absolute)&0xfff);
        break;
    case EVMU_OPCODE_CALLR:
        PUSH_PC();
        PC += (OP(relative16) % 65536) - 1; //unecessary with uint16_t PC
        break;
    case EVMU_OPCODE_BRF:
        PC += (OP(relative16) % 65536) - 1; //unecessary with uint16_t PC
        break;
    case EVMU_OPCODE_ST:
        WRITE(OP(direct), VIEW(SFR(ACC)));
        break;
    case EVMU_OPCODE_ST_IND:
        WRITE(INDIRECT(), VIEW(SFR(ACC)));
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
        PC |= (OP(absolute)&0xfff);
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
        BR_CMP(READ(SFR(ACC)), ==, OP(immediate), OP(relative8));
        break;
    case EVMU_OPCODE_BE:
        BR_CMP(READ(SFR(ACC)), ==, READ(OP(direct)), OP(relative8));
        break;
    case EVMU_OPCODE_BE_IND:
        BR_CMP(READ(INDIRECT()), ==, OP(immediate), OP(relative8));
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
        WRITE(SFR(C),   r&0xff);
        WRITE(SFR(ACC), (r&0xff00) >> 8);
        PSW(CY, 0);
        PSW(OV, !s);
        break;
    }
    case EVMU_OPCODE_BNEI:
        BR_CMP(READ(SFR(ACC)), !=, OP(immediate), OP(relative8));
        break;
    case EVMU_OPCODE_BNE:
        BR_CMP(READ(SFR(ACC)), !=, READ(OP(direct)), OP(relative8));
        break;
    case EVMU_OPCODE_BNE_IND:
        BR_CMP(READ(INDIRECT()), !=, OP(immediate), OP(relative8));
        break;
    case EVMU_OPCODE_BPC: {
        const EvmuWord value = READ_LATCH(OP(direct));
        const EvmuWord mask = (1u << OP(bit));
        if(value & mask) {
            WRITE(OP(direct), value & (~mask));
            PC += OP(relative8);
        }
        break;
    }
    case OPCODE_LDF: {
        const EvmuAddress flashAddr =
                ((READ(SFR(FPR)) & SFR_MSK(FPR, ADDR)) << 16u) |
                 (READ(SFR(TRH)) << 8u) |
                 (READ(SFR(TRL)));
        WRITE(SFR(ACC), READ_FLASH(flashAddr));
        break;
    }
    case EVMU_OPCODE_STF: {
        EvmuAddress flashAddr = (READ(SFR(TRH)) << 8u) | (READ(SFR(TRL)));
        const EvmuWord acc    = READ(SFR(ACC));
        const EvmuWord fpr    = READ(SFR(FPR));
        if(!(READ(SFR(EXT)) & 0x1)) {
            if(fpr & SFR_MSK(FPR, UNLOCK)) {
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
                    dev->flashPrg.prgState =
                            (flashAddr == VMU_FLASH_PRG_STATE2_ADDR && acc == VMU_FLASH_PRG_STATE2_VALUE)?
                        EVMU_FLASH_PROGRAM_STATE_COUNT : VMU_FLASH_PRG_STATE0;
                    break;
                default:
                    dev->flashPrg.prgState = 0;
                    break;
                }
            } else if(dev->flashPrg.prgState < EVMU_FLASH_PROGRAM_STATE_COUNT) {
                GBL_CTX_WARN("[EVMU_CPU]: STF without finishing unlock sequence!");
                dev->flashPrg.prgState = 0;
            } else {
                flashAddr |= ((fpr & SFR_MSK(FPR, ADDR)) << 16u);

                if(dev->flashPrg.prgState == EVMU_FLASH_PROGRAM_STATE_COUNT && (flashAddr & 0x7f)) {
                    GBL_CTX_WARN("[EVMU_CPU]: Unaligned flash write: %x", flashAddr);
                    dev->flashPrg.prgState = 0;
                } else {
                    WRITE_FLASH(flashAddr, acc);
                    if(++dev->flashPrg.prgState == EVMU_FLASH_PROGRAM_BYTE_COUNT + EVMU_FLASH_PROGRAM_STATE_COUNT) {
                        dev->flashPrg.prgState = 0;
                    }
                }
            }
        } else {
            GBL_CTX_WARN("[EVMU_CPU]: Attempted to use SFR instruction while in USER mode!");
        }
        break;
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
        BR(READ(OP(direct)) & (0x1 << OP(bit)), OP(relative8));
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
        BR(!READ(SFR(ACC)), OP(relative8));
        break;
    case EVMU_OPCODE_ADDI:
        OP_ADD(OP(immediate));
        break;
    case EVMU_OPCODE_ADD:
        OP_ADD(READ(OP(direct)));
        break;
    case EVMU_OPCODE_ADD_IND:
        OP_ADD(READ(INDIRECT()));
        break;
    case EVMU_OPCODE_BN:
        BR(!(READ(OP(direct)) & (0x1 << OP(bit))), OP(relative8));
        break;
    case EVMU_OPCODE_BNZ:
        BR(READ(SFR(ACC)), OP(relative8));
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
        EvmuPic__retiInstruction(pDevice_->pPic);
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
        address             +=  (READ(SFR(TRL)) | READ(SFR(TRH)) << 8u);
        if(EvmuMemory_programSource(pMemory) == EVMU_MEMORY_EXT_SRC_FLASH_BANK_1)
            address += EVMU_FLASH_BANK_SIZE;
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
        const unsigned v = READ(SFR(ACC));
        const EvmuWord psw = READ(SFR(PSW));
        WRITE(SFR(PSW), (psw&~(EVMU_SFR_PSW_CY_MASK))|((v&0x1)<<EVMU_SFR_PSW_CY_POS));
        WRITE(SFR(ACC), (v>>1)|(psw&EVMU_SFR_PSW_CY_MASK));
        break;
    }
    case EVMU_OPCODE_ORI:
        LOGIC_OP(|, OP(immediate));
        break;
    case EVMU_OPCODE_OR:
        LOGIC_OP(|, READ(OP(direct)));
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
        LOGIC_OP(&, READ(OP(direct)));
        break;
    case EVMU_OPCODE_AND_IND:
        LOGIC_OP(&, READ(INDIRECT()));
        break;
    case EVMU_OPCODE_SET1:
        WRITE(OP(direct), READ_LATCH(OP(direct)) | (1u << OP(bit)));
        break;
    case EVMU_OPCODE_ROLC:  {
        const unsigned v = READ(SFR(ACC));
        const EvmuWord psw = READ(SFR(PSW));
        WRITE(SFR(PSW), (psw&~(EVMU_SFR_PSW_CY_MASK))|(v&0x80));
        WRITE(SFR(ACC), (v<<1)|((psw&EVMU_SFR_PSW_CY_MASK)>>EVMU_SFR_PSW_CY_POS));
        break;
    }
    case EVMU_OPCODE_XORI:
        LOGIC_OP(^, OP(immediate));
        break;
    case EVMU_OPCODE_XOR:
        LOGIC_OP(^, READ(OP(direct)));
        break;
    case EVMU_OPCODE_XOR_IND:
        LOGIC_OP(^, READ(INDIRECT()));
        break;
    }
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuCpu_IBehavior_update_(EvmuIBehavior* pIBehav, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    EvmuCpu*     pSelf    = EVMU_CPU(pIBehav);
    EvmuDevice*  pDevice  = EvmuPeripheral_device(EVMU_PERIPHERAL(pIBehav));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);
    //do timing in time domain, so when clock frequency changes, it's automatically handled
    double time = 0.0;
    double deltaTime = (double)ticks / 1000000000.0;

    EvmuIBehavior_update(EVMU_IBEHAVIOR(pDevice->pGamepad), ticks);

    while(time < deltaTime) {
        EvmuPic_update(EVMU_PIC_PUBLIC_(pDevice_->pPic));
        EvmuTimers_update(EVMU_TIMERS_PUBLIC_(pDevice_->pTimers));
        if(!(pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PCON)] & EVMU_SFR_PCON_HALT_MASK))
            EvmuCpu_runNext(pSelf);

        const double cpuTime = EvmuCpu_secsPerInstruction(pSelf);
        time += cpuTime;
        EvmuIBehavior_update(EVMU_IBEHAVIOR(pDevice->pLcd), cpuTime*1000000.0);

    }

    GBL_CTX_END();
}


static GBL_RESULT EvmuCpu_IBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(pSelf);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    GBL_CTX_INFO("Resetting VMU CPU.");

    memset(&EVMU_CPU_(pSelf)->curInstr.encoded, 0, sizeof(EvmuInstruction));
    memset(&EVMU_CPU_(pSelf)->curInstr.decoded, 0, sizeof(EvmuInstruction));
    EVMU_CPU_(pSelf)->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);

    GBL_CTX_END();
}

static GBL_RESULT EvmuCpu_GblObject_setProperty_(GblObject* pObject, const GblProperty* pProp, GblVariant* pValue) {
    GBL_CTX_BEGIN(NULL);

    EvmuCpu* pSelf   = EVMU_CPU(pObject);

    switch(pProp->id) {
    case EvmuCpu_Property_Id_pc:
        EvmuCpu_setPc(pSelf, GblVariant_toUint16(pValue));
        break;
    default:
        GBL_CTX_RECORD_SET(GBL_RESULT_ERROR_INVALID_PROPERTY,
                           "Attempt to write unknown EvmuCpu property: [%s]",
                           GblProperty_nameString(pProp));
        break;
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuCpu_GblObject_property_(const GblObject* pObject, const GblProperty* pProp, GblVariant* pValue) {
    GBL_CTX_BEGIN(NULL);

    EvmuCpu* pSelf   = EVMU_CPU(pObject);

    switch(pProp->id) {
    case EvmuCpu_Property_Id_pc:
        GblVariant_setUint16(pValue, EvmuCpu_pc(pSelf));
        break;
    case EvmuCpu_Property_Id_opcode:
        GblVariant_setUint8(pValue, EvmuCpu_opcode(pSelf));
        break;
    case EvmuCpu_Property_Id_operand1:
    case EvmuCpu_Property_Id_operand2:
    case EvmuCpu_Property_Id_operand3:
        GblVariant_setInt32(pValue, EvmuCpu_operand(pSelf, pProp->id - EvmuCpu_Property_Id_operand1));
        break;
    default:
        GBL_CTX_RECORD_SET(GBL_RESULT_ERROR_INVALID_PROPERTY,
                           "Attempt to read unknown EvmuCpu property: [%s]",
                           GblProperty_nameString(pProp));
        break;
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuCpu_GblObject_constructed_(GblObject* pObject) {
    GBL_CTX_BEGIN(NULL);
    GblObject_setName(pObject, EVMU_CPU_NAME);
    GBL_CTX_END();
}

static GBL_RESULT EvmuCpuClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    if(!GblType_classRefCount(GBL_CLASS_TYPEOF(pClass))) {
        GBL_PROPERTIES_REGISTER(EvmuCpu);

        GblSignal_install(EVMU_CPU_TYPE,
                          "pcChange",
                          GblMarshal_CClosure_VOID__INSTANCE_UINT16,
                          1,
                          GBL_UINT16_TYPE);
    }

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuCpu_GblObject_constructed_;
    GBL_OBJECT_CLASS(pClass)    ->pFnProperty    = EvmuCpu_GblObject_property_;
    GBL_OBJECT_CLASS(pClass)    ->pFnSetProperty = EvmuCpu_GblObject_setProperty_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuCpu_IBehavior_update_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuCpu_IBehavior_reset_;
    EVMU_CPU_CLASS(pClass)      ->pFnFetch       = EvmuCpu_fetch_;
    EVMU_CPU_CLASS(pClass)      ->pFnDecode      = EvmuCpu_decode_;
    EVMU_CPU_CLASS(pClass)      ->pFnExecute     = EvmuCpu_execute_;
    EVMU_CPU_CLASS(pClass)      ->pFnRunNext     = EvmuCpu_runNext_;

    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuCpu_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInfo typeInfo = {
        .pFnClassInit        = EvmuCpuClass_init_,
        .classSize           = sizeof(EvmuCpuClass),
        .instanceSize        = sizeof(EvmuCpu),
        .instancePrivateSize = sizeof(EvmuCpu_)
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuCpu"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &typeInfo,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }
    return type;
}


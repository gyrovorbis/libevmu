#ifndef EVMU_CPU__H
#define EVMU_CPU__H

#include <evmu/hw/evmu_cpu.h>

#define SELF    EvmuCpu_* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

typedef enum EVMU_STACK_FRAME_TYPE {
    EVMU_STACK_FRAME_UNKNOWN,
    EVMU_STACK_FRAME_FUNCTION,
    EVMU_STACK_FRAME_FIRMWARE,
    EVMU_STACK_FRAME_SUBROUTINE,
    EVMU_STACK_FRAME_INTERRUPT,
    EVMU_STACK_FRAME_TYPE_COUNT
} EVMU_STACK_FRAME_TYPE;

typedef struct EvmuStackFrame_ {
    EvmuAddress             pcReturn; //callee for callR, callF, hardcoded for interrupts
    EvmuAddress             pcStart;
    EvmuAddress             pc;

    uint8_t                 stackStart;
    EVMU_STACK_FRAME_TYPE   frameType;
    GblBool                 systemMode;
} EvmuStackFrame_;


typedef struct EvmuCpu_ {
    EvmuCpu*                            pPublic;
    EvmuMemory_*                        pMemory;

    EvmuAddress                         pc;
    struct {
        EvmuInstruction                 encoded;
        const EvmuInstructionFormat*    pFormat;
        EvmuDecodedOperands             operands;
        uint8_t                         opcode;
        uint8_t                         elapsedCycles;
    } curInstr;
} EvmuCpu_;


GBL_INLINE EvmuAddress EvmuCpu__pc_(CSELF) GBL_NOEXCEPT {
    return pSelf->pc;
}

GBL_INLINE void EvmuCpu__pcSet_(SELF, EvmuAddress value) GBL_NOEXCEPT {
    pSelf->pc = value;
}



GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_CPU__H

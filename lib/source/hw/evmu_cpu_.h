#ifndef EVMU_CPU__H
#define EVMU_CPU__H

#include <evmu/hw/evmu_cpu.h>

#define EVMU_CPU_(instance)     (GBL_PRIVATE(EvmuCpu, instance))
#define EVMU_CPU_PUBLIC_(priv)  (GBL_PUBLIC(EvmuCpu, priv))

#define GBL_SELF_TYPE EvmuCpu_

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

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
    uint16_t                pc;
    uint8_t                 stackStart;
    EVMU_STACK_FRAME_TYPE   frameType;
    GblBool                 systemMode;
} EvmuStackFrame_;

typedef struct EvmuCpu_ {
    EvmuRam_*       pRam;

    uint16_t        pc;

    struct {
        EvmuInstruction                 encoded;
        EvmuDecodedInstruction          decoded;
        const EvmuInstructionFormat*    pFormat;
    } curInstr;

} EvmuCpu_;


EVMU_INLINE EvmuAddress EvmuCpu__pc_(GBL_CSELF) GBL_NOEXCEPT {
    return pSelf->pc;
}

EVMU_INLINE void EvmuCpu__setPc_(GBL_SELF, EvmuAddress value) GBL_NOEXCEPT {
    pSelf->pc = value;
}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CPU__H

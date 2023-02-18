#ifndef EVMU_CPU__H
#define EVMU_CPU__H

#include <evmu/hw/evmu_cpu.h>
#include <gyro_vmu_instr.h>

#define EVMU_CPU_(instance)     ((EvmuCpu_*)GBL_INSTANCE_PRIVATE(instance, EVMU_CPU_TYPE))
#define EVMU_CPU_PUBLIC_(priv)  ((EvmuCpu*)GBL_INSTANCE_PUBLIC(priv, EVMU_CPU_TYPE))

#define GBL_SELF_TYPE EvmuCpu_

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
    uint16_t                pc;
    uint8_t                 stackStart;
    EVMU_STACK_FRAME_TYPE   frameType;
    GblBool                 systemMode;
} EvmuStackFrame_;


typedef struct EvmuCpu_ {
    EvmuMemory_*    pMemory;

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

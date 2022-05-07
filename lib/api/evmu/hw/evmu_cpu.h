#ifndef EVMU_CPU_H
#define EVMU_CPU_H

#include "../types/evmu_peripheral.h"
#include "evmu_isa.h"

#define EVMU_CPU_TYPE                       (EvmuCpu_type())
#define EVMU_CPU_STRUCT                     EvmuCpu
#define EVMU_CPU_CLASS_STRUCT               EvmuCpuClass
#define EVMU_CPU(inst)                      (GBL_INSTANCE_CAST_PREFIX  (inst,  EVMU_CPU))
#define EVMU_CPU_CHECK(inst)                (GBL_INSTANCE_CHECK_PREFIX (inst,  EVMU_CPU))
#define EVMU_CPU_CLASS(klass)               (GBL_CLASS_CAST_PREFIX     (klass, EVMU_CPU))
#define EVMU_CPU_CLASS_CHECK(klass)         (GBL_CLASS_CHECK_PREFIX    (klass, EVMU_CPU))
#define EVMU_CPU_GET_CLASS(inst)            (GBL_INSTANCE_CAST_CLASS_PREFIX (inst,  EVMU_CPU))

#define SELF    EvmuCpu* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuCpu_);

typedef struct EvmuCpuClass {
    EvmuPeripheralClass base;
} EvmuCpuClass;

typedef struct EvmuCpu {
    union {
        EvmuCpuClass*   pClass;
        EvmuPeripheral  base;
    };
    EvmuCpu_*           pPrivate;
} EvmuCpu;


GBL_EXPORT GblType  EvmuCpu_type               (void) GBL_NOEXCEPT;

EVMU_API            EvmuCpu_instructionCurrent(CSELF,
                                               EvmuAddress*         pProgramCounter,
                                               uint8_t*             pOpCode,
                                               EvmuDecodedOperands* pOperands,
                                               EvmuWord*            pIndirectAddress,
                                               uint8_t*             pEllapsedCycles) GBL_NOEXCEPT;

EVMU_API            EvmuCpu_instructionExecute(CSELF, const EvmuInstruction* pInstruction) GBL_NOEXCEPT;
EvmuAddress         EvmuCpu_registerIndirectAddress(CSELF, uint8_t indirectMode) GBL_NOEXCEPT;


GBL_DECLS_END


#undef CSELF
#undef SELF


#endif // EVMU_CPU_H


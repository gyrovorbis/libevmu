#ifndef EVMU_CPU_H
#define EVMU_CPU_H

#include "../types/evmu_peripheral.h"
#include "evmu_isa.h"

#define EVMU_CPU_TYPE                   (GBL_TYPEOF(EvmuCpu))
#define EVMU_CPU(instance)              (GBL_INSTANCE_CAST(instance, EvmuCpu))
#define EVMU_CPU_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuCpu))
#define EVMU_CPU_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuCpu))

#define GBL_SELF_TYPE EvmuCpu

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuCpu, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuCpu, EvmuPeripheral)

EVMU_EXPORT GblType     EvmuCpu_type                    (void)                                  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuCpu_instructionCurrent      (GBL_CSELF,
                                                         EvmuAddress*         pProgramCounter,
                                                         uint8_t*             pOpCode,
                                                         EvmuDecodedOperands* pOperands,
                                                         EvmuWord*            pIndirectAddress,
                                                         uint8_t*             pEllapsedCycles)  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuCpu_instructionExecute      (GBL_CSELF,
                                                         const EvmuInstruction* pInstruction)   GBL_NOEXCEPT;

EVMU_INLINE EvmuAddress EvmuCpu_registerIndirectAddress (GBL_CSELF, uint8_t indirectMode)       GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CPU_H


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

GBL_PROPERTIES(EvmuCpu,
    (pc,                  GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_UINT32_TYPE),
    (instructionOpcode,   GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (instructionOperand1, GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (instructionOperand2, GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (instructionOperand3, GBL_GENERIC, (READ),                    GBL_UINT8_TYPE)
)

EVMU_EXPORT GblType     EvmuCpu_type                 (void)                                 GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuCpu_pc                   (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuCpu_setPc                (GBL_SELF, EvmuAddress address)        GBL_NOEXCEPT;

EVMU_EXPORT const EvmuDecodedInstruction*
                        EvmuCpu_currentInstruction   (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuCpu_execute              (GBL_CSELF,
                                                      const EvmuDecodedInstruction* pInstr) GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuCpu_indirectAddress      (GBL_CSELF, uint8_t indirectMode)      GBL_NOEXCEPT;
EVMU_EXPORT double      EvmuCpu_secsPerInstruction   (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuCpu_cyclesPerInstruction (GBL_CSELF)                            GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CPU_H


#if 0
EVMU_EXPORT EVMU_RESULT EvmuCpu_instructionInfo (GBL_CSELF,
                                                 EvmuDecodedInstruction* pInstruction,
                                                 EvmuWord*               pIndirectAddress,
                                                 uint8_t*                pEllapsedCycles)  GBL_NOEXCEPT;

//EVMU_EXPORT EVMU_RESULT EvmuCpu_executeInstruction      (GBL_CSELF,
//                                                         const EvmuInstruction* pInstruction)   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuCpu_executeEncoded  (GBL_CSELF,
                                                 const EvmuInstruction* pInstr)            GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuCpu_executeDecoded  (GBL_CSELF,
                                                 const EvmuDecodedInstruction* pInstr)     GBL_NOEXCEPT;
#endif


/*! \file
 *  \brief Sanyo LC86k CPU Core
 *  \ingroup Peripherals
 *
 *  \todo
 *      - Properties
 *      - secs per instruction in msec/nsec, not float
 *      - pull Rom/BIOS update out of CPU update path
 */
#ifndef EVMU_CPU_H
#define EVMU_CPU_H

#include "../types/evmu_peripheral.h"
#include "evmu_isa.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_CPU_TYPE                   (GBL_TYPEOF(EvmuCpu))
#define EVMU_CPU(instance)              (GBL_INSTANCE_CAST(instance, EvmuCpu))
#define EVMU_CPU_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuCpu))
#define EVMU_CPU_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuCpu))

#define GBL_SELF_TYPE EvmuCpu

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuCpu);

typedef uint16_t EvmuPc;

GBL_CLASS_DERIVE(EvmuCpu, EvmuPeripheral)
    EVMU_RESULT (*pFnFetch)  (GBL_SELF,
                              EvmuPc           pc,
                              EvmuInstruction* pEncoded);
    EVMU_RESULT (*pFnDecode) (GBL_SELF,
                              const EvmuInstruction*  pEncoded,
                              EvmuDecodedInstruction* pDecoded);
    EVMU_RESULT (*pFnExecute)(GBL_SELF,
                              const EvmuDecodedInstruction* pInstr);
    EVMU_RESULT (*pFnRunNext)(GBL_SELF);
GBL_CLASS_END

GBL_INSTANCE_DERIVE(EvmuCpu, EvmuPeripheral)
    GblBool halted;
    GblBool haltAfterNext;
    GblBool pcChanged;
GBL_INSTANCE_END

GBL_PROPERTIES(EvmuCpu,
    (pc,                  GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_UINT32_TYPE),
    (instructionOpcode,   GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (instructionOperand1, GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (instructionOperand2, GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (instructionOperand3, GBL_GENERIC, (READ),                    GBL_UINT8_TYPE)
)

GBL_SIGNALS(EvmuCpu,
    (pcChange, (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT16_TYPE, pc))
)

EVMU_EXPORT GblType     EvmuCpu_type                 (void)                                 GBL_NOEXCEPT;

EVMU_EXPORT EvmuPc      EvmuCpu_pc                   (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuCpu_setPc                (GBL_SELF, EvmuPc address)             GBL_NOEXCEPT;

EVMU_EXPORT const EvmuDecodedInstruction*
                        EvmuCpu_currentInstruction   (GBL_CSELF)                            GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuCpu_execute              (GBL_SELF,
                                                      const EvmuDecodedInstruction* pInstr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuCpu_runNext              (GBL_SELF)                             GBL_NOEXCEPT;

EVMU_EXPORT double      EvmuCpu_secsPerInstruction   (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuCpu_cyclesPerInstruction (GBL_CSELF)                            GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CPU_H

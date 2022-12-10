#ifndef EVMU_TIMERS_H
#define EVMU_TIMERS_H

#include "../types/evmu_peripheral.h"

#define EVMU_TIMERS_TYPE                (GBL_TYPEOF(EvmuTimers))
#define EVMU_TIMERS_NAME                "timers"

#define EVMU_TIMERS(instance)           (GBL_INSTANCE_CAST(instance, EvmuTimers))
#define EVMU_TIMERS_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuTimers))
#define EVMU_TIMERS_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuTimers))

#define GBL_SELF_TYPE EvmuTimers

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_TIMER0_MODE) {
    EVMU_TIMER0_MODE_TIMER8_TIMER8,
    EVMU_TIMER0_MODE_TIMER8_COUNTER8,
    EVMU_TIMER0_MODE_TIMER16,
    EVMU_TIMER0_MODE_COUNTER16
};

GBL_DECLARE_ENUM(EVMU_TIMER1_MODE) {
    EVMU_TIMER1_MODE_TIMER8_TIMER8,
    EVMU_TIMER1_MODE_TIMER8_PULSE8,
    EVMU_TIMER1_MODE_TIMER16,
    EVMU_TIMER1_MODE_PULSEVAR
};

GBL_CLASS_DERIVE_EMPTY   (EvmuTimers, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuTimers, EvmuPeripheral)


GBL_PROPERTIES(EvmuTimers,
    (timer0Mode, GBL_GENERIC, (READ), GBL_INT32_TYPE),
    (timer1Mode, GBL_GENERIC, (READ), GBL_INT32_TYPE)
)

EVMU_EXPORT GblType          EvmuTimers_type       (void)      GBL_NOEXCEPT;

EVMU_EXPORT EVMU_TIMER1_MODE EvmuTimers_timer1Mode (GBL_CSELF) GBL_NOEXCEPT;
EVMU_EXPORT void             EvmuTimers_update     (GBL_SELF)  GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_TIMERS_H


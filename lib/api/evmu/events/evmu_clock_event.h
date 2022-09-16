#ifndef EVMU_CLOCK_EVENT_H
#define EVMU_CLOCK_EVENT_H

#include <gimbal/meta/instances/gimbal_event.h>
#include "../hw/evmu_clock.h"

#define EVMU_CLOCK_EVENT_TYPE                  (GBL_TYPEOF(EvmuClockEvent))
#define EVMU_CLOCK_EVENT(instance)             (GBL_INSTANCE_CAST(instance, EvmuClockEvent))
#define EVMU_CLOCK_EVENT_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuClockEvent))
#define EVMU_CLOCK_EVENT_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance, EvmuClockevent))

#define GBL_SELF_TYPE EvmuClockEvent

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY(EvmuClockEvent, GblEvent)

GBL_INSTANCE_DERIVE(EvmuClockEvent, GblEvent)
    EVMU_CLOCK_SIGNAL signal;
    EvmuWave          wave;
GBL_INSTANCE_END

EVMU_EXPORT GblType EvmuClockEvent_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CLOCK_EVENT_H

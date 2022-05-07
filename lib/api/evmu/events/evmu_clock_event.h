#ifndef EVMU_CLOCK_EVENT_H
#define EVMU_CLOCK_EVENT_H

#include <gimbal/objects/gimbal_event.h>
#include "../hw/evmu_clock.h"

#define EVMU_CLOCK_EVENT_TYPE                  (EvmuClockEvent_type())
#define EVMU_CLOCK_EVENT_STRUCT                EvmuClockEvent
#define EVMU_CLOCK_EVENT_CLASS_STRUCT          EvmuClockEventClass
#define EVMU_CLOCK_EVENT(instance)             (GBL_INSTANCE_CAST_PREFIX(instance, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_CHECK(instance)       (GBL_INSTANCE_CHECK_PREFIX(instance, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_CLASS(klass)          (GBL_CLASS_CAST_PREFIX(klass, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_CLASS_CHECK(klass)    (GBL_CLASS_CHECK_PREFIX(klass, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_GET_CLASS(instance)   (GBL_INSTANCE_CAST_CLASS(instance, EVMU_CLOCK_EVENT))

#define SELF    EvmuClockEvent* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

typedef struct EvmuClockEventClass {
    GblEventClass  base;
} EvmuClockEventClass;

typedef struct EvmuClockEvent {
    union {
        EvmuClockEventClass*    pClass;
        GblEvent                base;
    };
    EVMU_CLOCK_SIGNAL           signal;
    EvmuWave                    wave;
} EvmuClockEvent;

GBL_EXPORT GblType              EvmuClockEvent_type(void) GBL_NOEXCEPT;

GBL_EXPORT EVMU_RESULT          EvmuClockEvent_init(SELF, EVMU_CLOCK_SIGNAL signal, EvmuWave wave) GBL_NOEXCEPT;
GBL_EXPORT EVMU_CLOCK_SIGNAL    EvmuClockEvent_signal(CSELF) GBL_NOEXCEPT;
GBL_EXPORT EvmuWave             EvmuClockEvent_wave(CSELF) GBL_NOEXCEPT;

#endif // EVMU_CLOCK_EVENT_H

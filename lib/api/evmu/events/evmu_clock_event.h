#ifndef EVMU_CLOCK_EVENT_H
#define EVMU_CLOCK_EVENT_H

#include <gimbal/objects/gimbal_event.h>
#include "../types/evmu_typedefs.h"

#define EVMU_CLOCK_EVENT_TYPE                  (EvmuClockEvent_type())
#define EVMU_CLOCK_EVENT_STRUCT                EvmuClockEvent
#define EVMU_CLOCK_EVENT_CLASS_STRUCT          EvmuClockEventClass
#define EVMU_CLOCK_EVENT(instance)             (GBL_TYPE_CAST_INSTANCE_PREFIX(instance, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_CHECK(instance)       (GBL_TYPE_CHECK_INSTANCE_PREFIX(instance, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_CLASS(klass)          (GBL_TYPE_CAST_CLASS_PREFIX(klass, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_CLASS_CHECK(klass)    (GBL_TYPE_CHECK_CLASS_PREFIX(klass, EVMU_CLOCK_EVENT))
#define EVMU_CLOCK_EVENT_GET_CLASS(instance)   (GBL_TYPE_CAST_GET_CLASS(instance, EVMU_CLOCK_EVENT))

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

} EvmuClockEvent;
#if 0
typedef struct EvmuClockSignalEvent {
    EvmuClockEvent      base;
    EVMU_CLOCK_SIGNAL   signal;
    EvmuWave            wave;
} EvmuClockSignalEvent;

typedef struct
#endif

GBL_EXPORT GblType EvmuClockEvent_type(void) GBL_NOEXCEPT;

#endif // EVMU_CLOCK_EVENT_H

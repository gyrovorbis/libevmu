#ifndef EVMU_MEMORY_EVENT_H
#define EVMU_MEMORY_EVENT_H

#include <gimbal/objects/gimbal_event.h>
#include "../types/evmu_typedefs.h"

#define EVMU_MEMORY_EVENT_TYPE                  (EvmuMemoryEvent_type())
#define EVMU_MEMORY_EVENT_STRUCT                EvmuMemoryEvent
#define EVMU_MEMORY_EVENT_CLASS_STRUCT          EvmuMemoryEventClass
#define EVMU_MEMORY_EVENT(instance)             (GBL_TYPE_CAST_INSTANCE_PREFIX(instance, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_CHECK(instance)       (GBL_TYPE_CHECK_INSTANCE_PREFIX(instance, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_CLASS(klass)          (GBL_TYPE_CAST_CLASS_PREFIX(klass, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_CLASS_CHECK(klass)    (GBL_TYPE_CHECK_CLASS_PREFIX(klass, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_GET_CLASS(instance)   (GBL_TYPE_CAST_GET_CLASS(instance, EVMU_MEMORY_EVENT))

#define SELF    EvmuMemoryEvent* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

typedef struct EvmuMemoryEventClass {
    GblEventClass  base;
} EvmuMemoryEventClass;

GBL_DECLARE_ENUM(EVMU_MEMORY_EVENT_OP) {
    EVMU_MEMORY_EVENT_OP_READ_LATCH,
    EVMU_MEMORY_EVENT_OP_READ_PORT,
    EVMU_MEMORY_EVENT_OP_WRITE
};

typedef struct EvmuMemoryEvent {
    union {
        EvmuMemoryEventClass*   pClass;
        GblEvent                base;
    };
    EVMU_MEMORY_EVENT_OP        op;
    EvmuAddress                 address;
    EvmuWord                    value;
} EvmuMemoryEvent;


GBL_EXPORT GblType EvmuMemoryEvent_type(void) GBL_NOEXCEPT;



GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_MEMORY_EVENT_H

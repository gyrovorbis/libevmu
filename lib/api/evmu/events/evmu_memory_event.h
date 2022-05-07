#ifndef EVMU_MEMORY_EVENT_H
#define EVMU_MEMORY_EVENT_H

#include <gimbal/objects/gimbal_event.h>
#include "../types/evmu_typedefs.h"

#define EVMU_MEMORY_EVENT_TYPE                  (EvmuMemoryEvent_type())
#define EVMU_MEMORY_EVENT_STRUCT                EvmuMemoryEvent
#define EVMU_MEMORY_EVENT_CLASS_STRUCT          EvmuMemoryEventClass
#define EVMU_MEMORY_EVENT(instance)             (GBL_INSTANCE_CAST_PREFIX(instance, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_CHECK(instance)       (GBL_INSTANCE_CHECK_PREFIX(instance, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_CLASS(klass)          (GBL_CLASS_CAST_PREFIX(klass, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_CLASS_CHECK(klass)    (GBL_CLASS_CHECK_PREFIX(klass, EVMU_MEMORY_EVENT))
#define EVMU_MEMORY_EVENT_GET_CLASS(instance)   (GBL_INSTANCE_CAST_CLASS(instance, EVMU_MEMORY_EVENT))

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
GBL_EXPORT EVMU_MEMORY_EVENT_OP EvmuMemoryEvent_op(CSELF) GBL_NOEXCEPT;
GBL_EXPORT EvmuAddress EvmuMemoryEvent_address(CSELF) GBL_NOEXCEPT;
GBL_EXPORT EvmuWord EvmuMemoryEvent_value(CSELF) GBL_NOEXCEPT;



GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_MEMORY_EVENT_H

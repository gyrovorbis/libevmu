/*! \file
 *  \brief EvmuRamEvent and related API
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_MEMORY_EVENT_H
#define EVMU_MEMORY_EVENT_H

#include <gimbal/meta/instances//gimbal_event.h>
#include "../types/evmu_typedefs.h"

#define EVMU_MEMORY_EVENT_TYPE                  (GBL_TYPEOF(EvmuRamEvent))
#define EVMU_MEMORY_EVENT(instance)             (GBL_INSTANCE_CAST(instance, EvmuRamEvent))
#define EVMU_MEMORY_EVENT_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuRamEvent))
#define EVMU_MEMORY_EVENT_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance, EvmuRamEvent))

#define GBL_SELF_TYPE EvmuRamEvent

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_MEMORY_EVENT_OP) {
    EVMU_MEMORY_EVENT_OP_READ_LATCH,
    EVMU_MEMORY_EVENT_OP_READ_PORT,
    EVMU_MEMORY_EVENT_OP_WRITE
};

/*! \struct EvmuMemoryEventClass
 *  \extends GblEventClass
 *  \brief   GblClass structure for EvmuRamEvent
 */
GBL_CLASS_DERIVE_EMPTY(EvmuMemoryEvent, GblEvent)

/*! \struct EvmuRamEvent
 *  \extends EvmuEvent
 *  \brief   GblEvent-derived type for memory-related events
 */
GBL_INSTANCE_DERIVE(EvmuMemoryEvent, GblEvent)
    EVMU_MEMORY_EVENT_OP op;
    EvmuAddress          address;
    EvmuWord             value;
GBL_INSTANCE_END

EVMU_EXPORT GblType EvmuMemoryEvent_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_MEMORY_EVENT_H

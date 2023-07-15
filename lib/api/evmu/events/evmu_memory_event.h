/*! \file
 *  \brief EvmuRamEvent and related API
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef evmu_memory_event_H
#define evmu_memory_event_H

#include <gimbal/meta/instances//gimbal_event.h>
#include "../types/evmu_typedefs.h"

#define evmu_memory_event_TYPE                  (GBL_TYPEOF(EvmuRamEvent))
#define evmu_memory_event(instance)             (GBL_INSTANCE_CAST(instance, EvmuRamEvent))
#define evmu_memory_event_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuRamEvent))
#define evmu_memory_event_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance, EvmuRamEvent))

#define GBL_SELF_TYPE EvmuRamEvent

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(evmu_memory_event_OP) {
    evmu_memory_event_OP_READ_LATCH,
    evmu_memory_event_OP_READ_PORT,
    evmu_memory_event_OP_WRITE
};

/*! \struct EvmuRamEventClass
 *  \extends GblEventClass
 *  \brief   GblClass structure for EvmuRamEvent
 */
GBL_CLASS_DERIVE_EMPTY(EvmuRamEvent, GblEvent)

/*! \struct EvmuRamEvent
 *  \extends EvmuEvent
 *  \brief   GblEvent-derived type for memory-related events
 */
GBL_INSTANCE_DERIVE(EvmuRamEvent, GblEvent)
    evmu_memory_event_OP op;
    EvmuAddress          address;
    EvmuWord             value;
GBL_INSTANCE_END

EVMU_EXPORT GblType EvmuRamEvent_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // evmu_memory_event_H

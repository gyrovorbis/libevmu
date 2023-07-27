/*! \file
 *  \brief EvmuClockEvent and related API
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_CLOCK_EVENT_H
#define EVMU_CLOCK_EVENT_H

#include <gimbal/meta/instances/gimbal_event.h>
#include "../hw/evmu_clock.h"

#define EVMU_CLOCK_EVENT_TYPE              (GBL_TYPEID(EvmuClockEvent))
#define EVMU_CLOCK_EVENT(self)             (GBL_CAST(EvmuClockEvent, self))
#define EVMU_CLOCK_EVENT_CLASS(klass)      (GBL_CLASS_CAST(EvmuClockEvent, klass))
#define EVMU_CLOCK_EVENT_GET_CLASS(self)   (GBL_CLASSOF(EvmuClockevent, self))

#define GBL_SELF_TYPE EvmuClockEvent

GBL_DECLS_BEGIN

/*! \struct EvmuClockEventClass
 *  \extends GblEventClass
 *  \brief   GblClass structure for EvmuClockEvent
 */
GBL_CLASS_DERIVE_EMPTY(EvmuClockEvent, GblEvent)

/*! \struct EvmuClockEvent
 *  \extends EvmuEvent
 *  \brief   GblEvent-derived type for clock-related events
 */
GBL_INSTANCE_DERIVE(EvmuClockEvent, GblEvent)
    EVMU_CLOCK_SIGNAL signal;
    EvmuWave          wave;
GBL_INSTANCE_END

EVMU_EXPORT GblType EvmuClockEvent_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CLOCK_EVENT_H

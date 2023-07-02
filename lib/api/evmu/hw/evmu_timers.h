/*! \file
 *  \brief BaseTimer, Timer0, and Timer1 Peripherals
 *  \ingroup peripherals
 *
 *  This header provides an API and implementation for the three
 *  different timing peripherals contained within the VMU:
 *  * Timer 0
 *  * Timer 1
 *  * Base Timer
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_TIMERS_H
#define EVMU_TIMERS_H

#include "../types/evmu_peripheral.h"

/*! \name Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_TIMERS_TYPE                (GBL_TYPEOF(EvmuTimers))                        //!< Type UUID for EvmuTimers
#define EVMU_TIMERS(instance)           (GBL_INSTANCE_CAST(instance, EvmuTimers))       //!< Function-style cast for GblInstance
#define EVMU_TIMERS_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuTimers))             //!< Function-style cast for GblClass
#define EVMU_TIMERS_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuTimers))  //!< Get EvmuTimersClass from GblInstance
//! @}

#define EVMU_TIMERS_NAME                "timers"    //!< EvmuTimers GblObject name

#define GBL_SELF_TYPE EvmuTimers

GBL_DECLS_BEGIN

//! Configuration modes for Timer 0
GBL_DECLARE_ENUM(EVMU_TIMER0_MODE) {
    EVMU_TIMER0_MODE_TIMER8_TIMER8,     //!< Timer8 mode 0, dual 8-bit timers
    EVMU_TIMER0_MODE_TIMER8_COUNTER8,
    EVMU_TIMER0_MODE_TIMER16,
    EVMU_TIMER0_MODE_COUNTER16
};

//! Configuration modes for Timer 1
GBL_DECLARE_ENUM(EVMU_TIMER1_MODE) {
    EVMU_TIMER1_MODE_TIMER8_TIMER8,
    EVMU_TIMER1_MODE_TIMER8_PULSE8,
    EVMU_TIMER1_MODE_TIMER16,
    EVMU_TIMER1_MODE_PULSEVAR
};

/*! \struct  EvmuTimersClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass structure for EvmuTimers
 *
 *  No public members.
 *
 *  \todo timeout/reload/tick virtual method?
 *
 *  \sa EvmuTimers
 */
GBL_CLASS_DERIVE_EMPTY(EvmuTimers, EvmuPeripheral)

/*! \struct  EvmuTimers
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   GblInstance structure for EvmuTimers
 *
 *  No public members.
 *
 *  \sa EvmuTimersClass
 */
GBL_INSTANCE_DERIVE_EMPTY(EvmuTimers, EvmuPeripheral)

//! \cond
/*! \todo baseTimer, timer0, timer1 values
 *  \todo timers enable/disabled/active
 */
GBL_PROPERTIES(EvmuTimers,
    (timer0Mode, GBL_GENERIC, (READ), GBL_INT32_TYPE),
    (timer1Mode, GBL_GENERIC, (READ), GBL_INT32_TYPE)
)
//! \endcond

EVMU_EXPORT GblType          EvmuTimers_type       (void)      GBL_NOEXCEPT;

EVMU_EXPORT EVMU_TIMER1_MODE EvmuTimers_timer1Mode (GBL_CSELF) GBL_NOEXCEPT;
EVMU_EXPORT void             EvmuTimers_update     (GBL_SELF)  GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_TIMERS_H


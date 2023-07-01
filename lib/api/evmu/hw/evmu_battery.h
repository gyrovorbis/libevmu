/*! \file
 *  \brief BIOS battery monitor + low battery detection circuit
 *  \ingroup peripherals
 *
 *  EvmuBattery is an EvmuPeripheral which encompasses two things:
 *  * A low battery alarm which is asserted on Pin 7.1 when low voltage
 *    is detected
 *  * A monitor within the BIOS which listens for this alarm and tells
 *    you to change the battery
 *
 *  \warning
 *  The battery alarm is there to ensure that you don't do something like
 *  begin writing to flash with a dying battery, which could result in
 *  loss of data and filesystem corruption.
 *
 *  \todo
 *      - rig up properties
 *
 *  \copyright 2023 Falco Girgis
 */

#ifndef EVMU_BATTERY_H
#define EVMU_BATTERY_H

#include "../types/evmu_peripheral.h"

#define EVMU_BATTERY_TYPE           (GBL_TYPEOF(EvmuBattery))                       //!< GblType UUID for EvmuBattery
#define EVMU_BATTERY(instance)      (GBL_INSTANCE_CAST(instance, EvmuBattery))      //!< Function-style GblInstance cast
#define EVMU_BATTERY_CLASS(klass)   (GBL_CLASS_CAST(klass, EvmuBattery))            //!< Function-style GblClass cast
#define EVMU_BATTERY_GET(instance)  (GBL_INSTANCE_GET_CLASS(instance, EvmuBattery)) //!< Extract EvmuBatteryClass from GblInstance

#define EVMU_BATTERY_NAME           "battery"                                       //!< GblObject peripheral name

#define GBL_SELF_TYPE EvmuBattery

GBL_DECLS_BEGIN

/*! \struct  EvmuBatteryClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass VTable structure for EvmuBattery
 *
 *  Class structure for the EvmuBattery peripheral.
 *  There are no public members.
 *
 *  \sa EvmuBattery
 */
GBL_CLASS_DERIVE_EMPTY(EvmuBattery, EvmuPeripheral)

/*! \struct  EvmuBattery
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   GblInstance structure for the battery peripheral
 *
 *  EvmuBattery represents an instantiable EvmuPeripheral
 *  encapsulating both the low battery alarm/pin as well as the
 *  BIOS logic which monitors it. There are no public members.
 *
 *  \sa EvmuBatteryClass
 */
GBL_INSTANCE_DERIVE_EMPTY(EvmuBattery, EvmuPeripheral)

//!\cond
GBL_PROPERTIES(EvmuBattery,
    (lowAlarm,       GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (monitorEnabled, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE)
)
//!\endcond

/*! Returns the UUID associated with the EvmuBattery type
 *  \relatesalso EvmuBattery
 *  \static
 *  \returns    GblType UUID
 */
EVMU_EXPORT GblType EvmuBattery_type              (void)                      GBL_NOEXCEPT;

/*! Returns true if the low battery detection circuit alarm is on
 *  \relatesalso EvmuBattery
 *  The alarm is reported on Pin7.1.
 *
 *  \returns    GBL_TRUE if the low battery alarm is set
 */
EVMU_EXPORT GblBool EvmuBattery_lowAlarm          (GBL_CSELF)                 GBL_NOEXCEPT;

/*! Sets the low battery detection circuit alarm to the given value
 *  \relatesalso EvmuBattery
 *  \details The alarm is reported on Pin7.1.
 */
EVMU_EXPORT void    EvmuBattery_setLowAlarm       (GBL_SELF, GblBool enabled) GBL_NOEXCEPT;

/*! Returns whether the BIOS battery monitor is enabled
 *  \relatesalso EvmuBattery
 *  \sa EVMU_ADDRESS_SYSTEM_BATTERY_CHECK
 *
 *  \returns     GBL_TRUE if the battery monitor is enabled
 */
EVMU_EXPORT GblBool EvmuBattery_monitorEnabled    (GBL_CSELF)                 GBL_NOEXCEPT;

/*! Enables or disables the BIOS battery monitor
 *  \sa EVMU_ADDRESS_SYSTEM_BATTERY_CHECK
 *  \relatesalso EvmuBattery
 */
EVMU_EXPORT void    EvmuBattery_setMonitorEnabled (GBL_SELF, GblBool enabled) GBL_NOEXCEPT;

// Profiling API

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BATTERY_H


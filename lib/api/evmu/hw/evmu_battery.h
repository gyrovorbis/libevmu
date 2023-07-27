/*! \file
 *  \brief EvmuBattery BIOS monitor + low battery detection circuit
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
 *      - battery lifetime profiler API
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_BATTERY_H
#define EVMU_BATTERY_H

#include "../types/evmu_peripheral.h"

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_BATTERY_TYPE           (GBL_TYPEID(EvmuBattery))            //!< GblType UUID for EvmuBattery
#define EVMU_BATTERY(self)          (GBL_CAST(EvmuBattery, self))        //!< Function-style GblInstance cast
#define EVMU_BATTERY_CLASS(klass)   (GBL_CLASS_CAST(EvmuBattery, klass)) //!< Function-style GblClass cast
#define EVMU_BATTERY_GET(self)      (GBL_CLASSOF(EvmuBattery, self))     //!< Extract EvmuBatteryClass from GblInstance
//! @}

#define EVMU_BATTERY_NAME           "battery"       //!< GblObject peripheral name

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

//! Returns the GblType UUID associated with EvmuBattery
EVMU_EXPORT GblType EvmuBattery_type (void) GBL_NOEXCEPT;

/*! \name Alarm
 *  \brief Methods for getting and setting the alarm state
 *  \relatesalso EvmuBattery
 *  @{
 */
//! Returns GBL_TRUE if the low voltage battery detection signal is asserted
EVMU_EXPORT GblBool EvmuBattery_lowAlarm    (GBL_CSELF)                 GBL_NOEXCEPT;
//! Sets the low battery detection circuit alarm to the \p enabled value
EVMU_EXPORT void    EvmuBattery_setLowAlarm (GBL_SELF, GblBool enabled) GBL_NOEXCEPT;
//! @}

/*! \name Monitor
 *  \brief Methods for getting and setting the monitor state
 *  \relatesalso EvmuBattery
 *  @{
 */
//! Returns GBL_TRUE if the system BIOS low battery monitor is enabled
EVMU_EXPORT GblBool EvmuBattery_monitorEnabled    (GBL_CSELF)                 GBL_NOEXCEPT;
//! Enables or disables the system BIOS low battery monitor, based on the \p enabled value
EVMU_EXPORT void    EvmuBattery_setMonitorEnabled (GBL_SELF, GblBool enabled) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BATTERY_H

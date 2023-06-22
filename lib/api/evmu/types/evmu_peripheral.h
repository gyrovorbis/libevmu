/*! \file
 *  \brief   EvmuPeripheral base VMU component type
 *  \ingroup peripherals
 *
 *  EvmuPeripheral and its APIs comprise the various hardware
 *  peripherals such as timers, serial ports. clocks, etc which
 *  are containe within the Sanyo Potato IC.
 *
 *  \copyright 2023 Falco Girgis
 */

#ifndef EVMU_PERIPHERAL_H
#define EVMU_PERIPHERAL_H

#include "evmu_ibehavior.h"

#define EVMU_PERIPHERAL_TYPE                (GBL_TYPEOF(EvmuPeripheral))                            //!< GblType UUID for EvmuPeripheral
#define EVMU_PERIPHERAL(instance)           (GBL_INSTANCE_CAST(instance, EvmuPeripheral))           //!< Function-style GblInstance cast operator
#define EVMU_PERIPHERAL_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuPeripheral))                 //!< Function-style GblClass cast operator
#define EVMU_PERIPHERAL_GET_CLASS(instance) (GBL_TYPE_INSTANCE_GET_CLASS(instance, EvmuPeripheral)) //!< Get an EvmuPeripheralClass from GblInstance

/*! \defgroup peripherals Peripherals
 *  \brief    Hardware subcomponents of the VMU's SoC
 *
 *  An EvmuDevice is comprised of a collection of EvmuPeripherals
 *  which act as its individual hardware components. Each Peripheral
 *  inherits from the base type, EvmuPeripheral.
 */

#define GBL_SELF_TYPE EvmuPeripheral

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemoryEvent);
GBL_FORWARD_DECLARE_STRUCT(EvmuClockEvent);
GBL_FORWARD_DECLARE_STRUCT(EvmuDevice);
GBL_FORWARD_DECLARE_STRUCT(EvmuPeripheral);

GBL_DECLARE_ENUM(EVMU_PERIPHERAL_LOG_LEVEL) {
    EVMU_PERIPHERAL_LOG_LEVEL_ERROR,
    EVMU_PERIPHERAL_LOG_LEVEL_WARNING,
    EVMU_PERIPHERAL_LOG_LEVEL_VERBOSE,
    EVMU_PERIPHERAL_LOG_LEVEL_DEBUG,
    EVMU_PERIPHERAL_LOG_LEVEL_DISABLED
};

/*! \struct     EvmuPeripheralClass
 *  \extends    GblObjectClass
 *  \implements EvmuIBehaviorClass
 *  \brief      GblClass structure for EvmuPeripherals
 *
 *  GblClass/vtable structure containing virtual functions
 *  for EvmuPeripheral instances.
 *
 *  \sa EvmuPeripheral
 */
GBL_CLASS_DERIVE(EvmuPeripheral, GblObject, EvmuIBehavior)
    //! Called when an EvmuMemoryEvent has been fired
    EVMU_RESULT (*pFnMemoryEvent)(GBL_SELF, EvmuMemoryEvent* pEvent);
    //! Called when an EvmuClockEvent has been fired
    EVMU_RESULT (*pFnClockEvent) (GBL_SELF, EvmuClockEvent* pEvent);
GBL_CLASS_END

/*! \struct     EvmuPeripheral
 *  \ingroup    peripherals
 *  \extends    GblObject
 *  \implements EvmuIBehavior
 *  \brief      Emulated hardware component of an EvmuDevice
 *
 *  Base instance type of all emulated components of the
 *  Visual Memory Unit / Potato IC.
 *
 *  \sa EvmuPeripheralClass
 */
GBL_INSTANCE_DERIVE(EvmuPeripheral, GblObject)
    GblFlags logLevel;  //!< Active log level filter for peripheral
GBL_INSTANCE_END

/*! Returns the GblType UUID corresponding to the EvmuPeripheral type
 *  \relatesalso EvmuPeripheral
 *  \static
 *
 *  \retval             GblType UUID
*/
EVMU_EXPORT GblType     EvmuPeripheral_type        (void)                     GBL_NOEXCEPT;

/*! Returns the EvmuDevice to which the given peripheral is attached to
 *  \relatesalso EvmuPeripheral
 *
 *  \retval             EvmuDevice owning the peripheral
 */
EVMU_EXPORT EvmuDevice* EvmuPeripheral_device      (GBL_CSELF)                GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_PERIPHERAL_H


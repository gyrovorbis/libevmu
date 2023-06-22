/*! \file
 *  \brief EvmuDevice top-level emulated entity
 *
 *  EvmuDevice encompasses everything that a single Visual Memory
 *  Unit/System entails, emulating the Sanyo Potato IC.
 *
 *  \copyright 2023 Falco Girgis
 */

#ifndef EVMU_DEVICE_H
#define EVMU_DEVICE_H

#include "../types/evmu_ibehavior.h"
#include "../hw/evmu_memory.h"
#include "../hw/evmu_cpu.h"
#include "../hw/evmu_clock.h"
#include "../hw/evmu_rom.h"
#include "../hw/evmu_pic.h"
#include "../hw/evmu_flash.h"
#include "../hw/evmu_lcd.h"
#include "../hw/evmu_battery.h"
#include "../hw/evmu_wram.h"
#include "../hw/evmu_buzzer.h"
#include "../hw/evmu_gamepad.h"
#include "../hw/evmu_timers.h"

#define EVMU_DEVICE_TYPE                (GBL_TYPEOF(EvmuDevice))                        //!< UUID for the EvmuDevice type
#define EVMU_DEVICE(instance)           (GBL_INSTANCE_CAST(instance, EvmuDevice))       //!< Function-style GblInstance cast
#define EVMU_DEVICE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuDevice))             //!< Function-style GblClass cast
#define EVMU_DEVICE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuDevice))  //!< Get EvmuDeviceClass from GblInstance

#define GBL_SELF_TYPE EvmuDevice

GBL_DECLS_BEGIN

/*! \struct     EvmuDeviceClass
 *  \extends    GblObjectClass
 *  \implements EvmuIBehaviorClass
 *  \brief      GblClass structure for EvmuDevice
 *
 *  This class contains no public members.
 *
 *  \sa EvmuDevice
 */
GBL_CLASS_DERIVE_EMPTY(EvmuDevice, GblObject, EvmuIBehavior)

/*! \struct     EvmuDevice
 *  \extends    GblObject
 *  \implements EvmuIBehavior
 *  \brief      GblInstance structure for VMU Devices
 *
 *  This structure models the top-level Potato IC, whose
 *  components are accessible as EvmuPeripherals attached
 *  to the device.
 *
 *  \sa EvmuDeviceClass
 */
GBL_INSTANCE_DERIVE(EvmuDevice, GblObject)
    EvmuMemory*  pMemory;   //!< EvmuMemory Peripheral
    EvmuCpu*     pCpu;      //!< EvmuCpu Peripheral
    EvmuClock*   pClock;    //!< EvmuClock Peripheral
    EvmuPic*     pPic;      //!< EvmuPic Peripheral
    EvmuRom*     pRom;      //!< EvmuRom Peripheral
    EvmuFlash*   pFlash;    //!< EvmuFlash Peripheral
    EvmuWram*    pWram;     //!< EvmuWram Peripheral
    EvmuLcd*     pLcd;      //!< EvmuLcd Peripheral
    EvmuBuzzer*  pBuzzer;   //!< EvmuBuzzer Peripheral
    EvmuBattery* pBattery;  //!< EvmuBattery Peripheral
    EvmuGamepad* pGamepad;  //!< EvmuGamepad Peripheral
    EvmuTimers*  pTimers;   //!< EvmuTimers Peripheral
GBL_INSTANCE_END

//! \cond
GBL_PROPERTIES(EvmuDevice,
    (memory,  GBL_GENERIC, (READ), EVMU_MEMORY_TYPE),
    (cpu,     GBL_GENERIC, (READ), EVMU_CPU_TYPE),
    (clock,   GBL_GENERIC, (READ), EVMU_CLOCK_TYPE),
    (pic,     GBL_GENERIC, (READ), EVMU_PIC_TYPE),
    (rom,     GBL_GENERIC, (READ), EVMU_ROM_TYPE),
    (flash,   GBL_GENERIC, (READ), EVMU_FLASH_TYPE),
    (wram,    GBL_GENERIC, (READ), EVMU_WRAM_TYPE),
    (lcd,     GBL_GENERIC, (READ), EVMU_LCD_TYPE),
    (buzzer,  GBL_GENERIC, (READ), EVMU_BUZZER_TYPE),
    (battery, GBL_GENERIC, (READ), EVMU_BATTERY_TYPE),
    (gamepad, GBL_GENERIC, (READ), EVMU_GAMEPAD_TYPE),
    (timers,  GBL_GENERIC, (READ), EVMU_TIMERS_TYPE)
)
//! \endcond

/*! Returns the UUID corresponding to the EvmuDevice type
 *  \relatesalso EvmuDevice
 *  \static
 *
 *  \returns            GblType UUID
 */
EVMU_EXPORT GblType         EvmuDevice_type             (void)                         GBL_NOEXCEPT;

/*! Returns the number of EvmuPeripheral components attached to the device
 *  \relatesalso EvmuDevice
 *
 *  \returns            Number of EvmuPeripherals
 */
EVMU_EXPORT size_t          EvmuDevice_peripheralCount  (GBL_CSELF)                    GBL_NOEXCEPT;

/*! Searches for the give EvmuPeripheral by name
 *  \relatesalso EvmuDevice
 *
 *  \param pName        Peripheral name to search for
 *  \returns            EvmuPeripheral or NULL if not found
 */
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheralByName (GBL_CSELF, const char* pName) GBL_NOEXCEPT;

/*! Returns the EvmuPeripheral at the given index
 *  \relatesalso EvmuDevice
 *
 *  \param index        index of the desired peripheral
 *  \returns            EvmuPeripheral or NULL upon invalid index
 */
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheralAt     (GBL_CSELF, size_t index)      GBL_NOEXCEPT;

//! \cond
GBL_FORWARD_DECLARE_STRUCT(VMUDevice);
EVMU_EXPORT VMUDevice* EvmuDevice_REEST(GBL_CSELF) GBL_NOEXCEPT;
//! \endcond

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_DEVICE_H

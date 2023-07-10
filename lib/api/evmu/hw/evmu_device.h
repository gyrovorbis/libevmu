/*! \file
 *  \brief EvmuDevice top-level emulated entity
 *
 *  EvmuDevice encompasses everything that a single Visual Memory
 *  Unit/System entails, emulating the Sanyo Potato IC.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
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
#include "../fs/evmu_fat.h"
#include "../fs/evmu_file_manager.h"

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_DEVICE_TYPE                (GBL_TYPEOF(EvmuDevice))                        //!< UUID for the EvmuDevice type
#define EVMU_DEVICE(instance)           (GBL_INSTANCE_CAST(instance, EvmuDevice))       //!< Function-style GblInstance cast
#define EVMU_DEVICE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuDevice))             //!< Function-style GblClass cast
#define EVMU_DEVICE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuDevice))  //!< Get EvmuDeviceClass from GblInstance
//! @}

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
    EvmuWram*    pWram;     //!< EvmuWram Peripheral
    EvmuLcd*     pLcd;      //!< EvmuLcd Peripheral
    EvmuBuzzer*  pBuzzer;   //!< EvmuBuzzer Peripheral
    EvmuBattery* pBattery;  //!< EvmuBattery Peripheral
    EvmuGamepad* pGamepad;  //!< EvmuGamepad Peripheral
    EvmuTimers*  pTimers;   //!< EvmuTimers Peripheral
    union {
        EvmuFlash*       pFlash;   //!< EvmuFlash Peripheral
        EvmuFat*         pFat;     //!< EvmuFat Peripheral
        EvmuFileManager* pFileMgr; //!< EvmuFileSystem Peripheral
    };
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
    (timers,  GBL_GENERIC, (READ), EVMU_TIMERS_TYPE),
    (fat,     GBL_GENERIC, (READ), EVMU_FAT_TYPE)
)
//! \endcond

//! Returns the GblType UUID associated with EvmuDevice
EVMU_EXPORT GblType     EvmuDevice_type   (void)     GBL_NOEXCEPT;
//! Creates an EvmuDevice instance and returns a pointer to it
EVMU_EXPORT EvmuDevice* EvmuDevice_create (void)     GBL_NOEXCEPT;
//! Decrements and returns the reference count of the given EvmuDevice, destructing it at 0
EVMU_EXPORT GblRefCount EvmuDevice_unref  (GBL_SELF) GBL_NOEXCEPT;

/*! \name Peripherals
 *  \brief Methods for managing peripheral components
 *  \relatesalso EvmuDevice
 *  @{
 */
//! Returns the number of EvmuPeripheral GblObject children attached to the given device instance
EVMU_EXPORT size_t          EvmuDevice_peripheralCount (GBL_CSELF)                    GBL_NOEXCEPT;
//! Finds a child EvmuPeripheral child attached to the given device, returning a pointer to it or NULL if not found
EVMU_EXPORT EvmuPeripheral* EvmuDevice_findPeripheral  (GBL_CSELF, const char* pName) GBL_NOEXCEPT;
//! Returns the child EvmuPeripheral attached to the given device at the provided \p index
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheral      (GBL_CSELF, size_t index)      GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_DEVICE_H

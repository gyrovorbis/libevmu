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

#define EVMU_DEVICE_TYPE                (GBL_TYPEOF(EvmuDevice))
#define EVMU_DEVICE(instance)           (GBL_INSTANCE_CAST(instance, EvmuDevice))
#define EVMU_DEVICE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuDevice))
#define EVMU_DEVICE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuDevice))

#define GBL_SELF_TYPE EvmuDevice

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuDevice, GblObject, EvmuIBehavior)
GBL_INSTANCE_DERIVE_EMPTY(EvmuDevice, GblObject)

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
    (battery, GBL_GENERIC, (READ), EVMU_BATTERY_TYPE)
)

EVMU_EXPORT GblType         EvmuDevice_type             (void)                         GBL_NOEXCEPT;

EVMU_EXPORT GblSize         EvmuDevice_peripheralCount  (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheralByName (GBL_CSELF, const char* pName) GBL_NOEXCEPT;
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheralAt     (GBL_CSELF, GblSize index)     GBL_NOEXCEPT;

EVMU_EXPORT EvmuMemory*     EvmuDevice_memory           (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuCpu*        EvmuDevice_cpu              (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuClock*      EvmuDevice_clock            (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuPic*        EvmuDevice_pic              (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuRom*        EvmuDevice_rom              (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuFlash*      EvmuDevice_flash            (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuWram*       EvmuDevice_wram             (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuLcd*        EvmuDevice_lcd              (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuBuzzer*     EvmUDevice_buzzer           (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuBattery*    EvmuDevice_battery          (GBL_CSELF)                    GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_DEVICE_H

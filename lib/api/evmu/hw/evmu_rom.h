/*! \file
 *  \brief External ROM chip, BIOS, Firmware routines
 *  \ingroup Peripherals
 *
 *  \todo
 *      - CLEAN UP IMPLEMENTATION IN GENERAL
 *      - remove "reestDevice" references (pending Flash updates get moved)
 *      - EvmuRom_setBiosMode() (set mode button then jump to Colton's address?)
 *      - EvmuRom["biosMode"]: R/W
 *      - EvmuRom["dateTime"]: R/W (pending on ISO8601 in Gimbal)
 *      - Maybe signal when entering/exiting BIOS
 *      - overridable virtuals for whole custom BIOS
 *      - return elapsed ticks/cycles for subroutine call
 *      - return BIOS version information and shit
 */
#ifndef EVMU_ROM_H
#define EVMU_ROM_H

#include "../types/evmu_peripheral.h"

#define EVMU_ROM_TYPE                   (GBL_TYPEOF(EvmuRom))
#define EVMU_ROM_NAME                   "rom"

#define EVMU_ROM(instance)              (GBL_INSTANCE_CAST(instance, EvmuRom))
#define EVMU_ROM_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuRom))
#define EVMU_ROM_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuRom))

#define EVMU_ROM_SIZE                    65536  ///< Total size of external ROM chip

#define EVMU_BIOS_SYS_PROG_ADDRESS_BASE  0x0000 ///< Start of Firmware/Subroutines in bytes
#define EVMU_BIOS_SYS_PROG_SIZE          16384  ///< Size of Firmware/Subroutines in bytes
#define EVMU_BIOS_OS_PROG_ADDRESS_BASE   0xed00 ///< Start address of OS/BIOS program
#define EVMU_BIOS_OS_PROG_SIZE           4096   ///< Size of OS/BIOS program in bytes
#define EVMU_BIOS_SKIP_DATE_TIME_PC      0x2e1  ///< BIOS PC address just after setting date/time

#define GBL_SELF_TYPE EvmuRom

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(GblDateTime);
GBL_FORWARD_DECLARE_STRUCT(EvmuRom);

GBL_DECLARE_ENUM(EVMU_BIOS_SUBROUTINE) {
    EVMU_BIOS_SUBROUTINE_RESET      = 0x000, ///< Regular starting point
    // Firmware calls: utility functions that return execution back to app
    EVMU_BIOS_SUBROUTINE_FM_WRT_EX  = 0x100, ///< Flash memory write, return address variant 1
    EVMU_BIOS_SUBROUTINE_FM_WRTA_EX = 0x108, ///< Flash memory write, return address variant 2
    EVMU_BIOS_SUBROUTINE_FM_VRF_EX  = 0x110, ///< Flash memory page data verify
    EVMU_BIOS_SUBROUTINE_FM_PRD_EX  = 0x120, ///< Flash memory paged read data
    // BIOS routines: control yields to BIOS?
    EVMU_BIOS_SUBROUTINE_TIMER_EX   = 0x130, ///< System time update, used as Base Timer ISR
    EVMU_BIOS_SUBROUTINE_SLEEP_EX   = 0x140, ///< Enable sleep mode
    EVMU_BIOS_SUBROUTINE_EXIT_EX    = 0x1f0  ///< MODE button logic
};

GBL_DECLARE_ENUM(EVMU_BIOS_TYPE) {
    EVMU_BIOS_TYPE_EMULATED,                          ///< Default, no BIOS, software emulation
    EVMU_BIOS_TYPE_AMERICAN_IMAGE_V1_05 = 0xC825003A, ///< CRC for American BIOS
    EVMU_BIOS_TYPE_JAPANESE_IMAGE_V1_04 = 0x8E0F867A, ///< CRC for Japanese BIOS
    EVMU_BIOS_TYPE_UNKNOWN_IMAGE                      ///< Any other unknown image
};

GBL_DECLARE_ENUM(EVMU_BIOS_MODE) {
    EVMU_BIOS_MODE_FILE,    ///< File Manager mode
    EVMU_BIOS_MODE_GAME,    ///< Game/Application mode
    EVMU_BIOS_MODE_TIME,    ///< Clock/time mode
    EVMU_BIOS_MODE_MAPLE,   ///< Connected to DC, Maple slave mode
    EVMU_BIOS_MODE_UNKNOWN, ///< Unknown mode (unknown BIOS)
    EVMU_BIOS_MODE_COUNT    ///< Number of BIOS modes
};

GBL_CLASS_DERIVE(EvmuRom, EvmuPeripheral)
    EVMU_RESULT (*pFnLoadBios)(GBL_SELF, const char* pPath);
    EVMU_RESULT (*pFnCallBios)(GBL_SELF, EvmuAddress entryPc, EvmuAddress* pRetPc);
GBL_CLASS_END

GBL_INSTANCE_DERIVE_EMPTY(EvmuRom, EvmuPeripheral)

GBL_PROPERTIES(EvmuRom,
    (biosActive, GBL_GENERIC, (READ), GBL_BOOL_TYPE),
    (biosType,   GBL_GENERIC, (READ), GBL_ENUM_TYPE),
    (biosMode,   GBL_GENERIC, (READ), GBL_ENUM_TYPE),
    (dateTime,   GBL_GENERIC, (READ), GBL_STRING_TYPE)
)

EVMU_EXPORT GblType        EvmuRom_type          (void)                         GBL_NOEXCEPT;

EVMU_EXPORT GblBool        EvmuRom_biosActive    (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EVMU_BIOS_TYPE EvmuRom_biosType      (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EVMU_BIOS_MODE EvmuRom_biosMode      (GBL_CSELF)                    GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_loadBios      (GBL_SELF, const char* pPath)  GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT    EvmuRom_unloadBios    (GBL_SELF)                     GBL_NOEXCEPT;
EVMU_EXPORT EvmuAddress    EvmuRom_callBios      (GBL_SELF, EvmuAddress entry)  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_skipBiosSetup (GBL_SELF, GblBool enableSkip) GBL_NOEXCEPT;

EVMU_EXPORT GblDateTime*   EvmuRom_dateTime      (GBL_CSELF,
                                                  GblDateTime* pDateTime)       GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_setDateTime   (GBL_SELF,
                                                  const GblDateTime* pDateTime) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_ROM_H

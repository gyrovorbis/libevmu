#ifndef EVMU_ROM_H
#define EVMU_ROM_H

#include "../types/evmu_peripheral.h"

#define EVMU_ROM_TYPE                   (GBL_TYPEOF(EvmuRom))
#define EVMU_ROM_NAME                   "rom"

#define EVMU_ROM(instance)              (GBL_INSTANCE_CAST(instance, EvmuRom))
#define EVMU_ROM_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuRom))
#define EVMU_ROM_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuRom))

#define EVMU_ROM_SIZE                    65536

#define EVMU_BIOS_SYS_PROG_ADDRESS_BASE  0x0000
#define EVMU_BIOS_SYS_PROG_SIZE          16384
#define EVMU_BIOS_OS_PROG_ADDRESS_BASE   0xed00
#define EVMU_BIOS_OS_PROG_SIZE           4096

// Firmware calls: utility functions that return execution back to app
#define EVMU_BIOS_ADDRESS_FM_WRT_EX      0x100
#define EVMU_BIOS_ADDRESS_FM_WRTA_EX     0x108
#define EVMU_BIOS_ADDRESS_FM_VRF_EX      0x110
#define EVMU_BIOS_ADDRESS_FM_PRD_EX      0x120

// BIOS routines: control yields to BIOS?
#define EVMU_BIOS_ADDRESS_TIMER_EX       0x130   // Timer update function used as ISR to populate date/time in system variables
#define EVMU_BIOS_ADDRESS_SLEEP_EX       0x140
#define EVMU_BIOS_ADDRESS_EXIT_EX        0x1f0   // MODE button?

#define GBL_SELF_TYPE EvmuRom

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(GblDateTime);
GBL_FORWARD_DECLARE_STRUCT(EvmuRom);

GBL_DECLARE_ENUM(EVMU_BIOS_SUBROUTINE) {
    EVMU_BIOS_SUBROUTINE_FM_WRT_EX,
    EVMU_BIOS_SUBROUTINE_FM_WRTA_EX,
    EVMU_BIOS_SUBROUTINE_FM_VRF_EX,
    EVMU_BIOS_SUBROUTINE_FM_PRD_EX,
    EVMU_BIOS_SUBROUTINE_TIMER_EX,
    EVMU_BIOS_SUBROUTINE_SLEEP_EX,
    EVMU_BIOS_SUBROUTINE_EXIT_EX,
    EVMU_BIOS_SUBROUTINE_COUNT
};

GBL_DECLARE_ENUM(EVMU_BIOS_TYPE) {
    EVMU_BIOS_TYPE_EMULATED,
    EVMU_BIOS_TYPE_AMERICAN_IMAGE,
    EVMU_BIOS_TYPE_JAPANESE_IMAGE,
    EVMU_BIOS_TYPE_UNKNOWN_IMAGE
};

GBL_DECLARE_ENUM(EVMU_BIOS_MODE) {
    EVMU_BIOS_MODE_UNKNOWN,
    EVMU_BIOS_MODE_MAPLE,   //connected to DC, maple mode
    EVMU_BIOS_MODE_GAME,
    EVMU_BIOS_MODE_FILE,
    EVMU_BIOS_MODE_TIME,
    EVMU_BIOS_MODE_COUNT
};

GBL_CLASS_DERIVE(EvmuRom, EvmuPeripheral)
    EVMU_RESULT (*pFnLoadBios)(GBL_SELF, const char* pPath);
    EVMU_RESULT (*pFnCallBios)(GBL_SELF, EvmuAddress psw);
GBL_CLASS_END

GBL_INSTANCE_DERIVE_EMPTY(EvmuRom, EvmuPeripheral)

GBL_PROPERTIES(EvmuRom,
    (biosActive, GBL_GENERIC, (READ),        GBL_BOOL_TYPE),
    (biosType,   GBL_GENERIC, (READ),        GBL_ENUM_TYPE),
    (biosMode,   GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE)
)

EVMU_EXPORT GblType     EvmuRom_type        (void)                        GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuRom_biosLoaded  (GBL_CSELF)                   GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuRom_biosActive  (GBL_CSELF)                   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuRom_loadBios    (GBL_SELF, const char* pPath) GBL_NOEXCEPT;
EVMU_EXPORT EvmuAddress EvmuRom_callBios    (GBL_SELF)                    GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuRom_dateTime    (GBL_CSELF,
                                             GblDateTime* pDateTime)      GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuRom_setDateTime (GBL_SELF,
                                             const GblDateTime* pDTime)   GBL_NOEXCEPT;


GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_ROM_H


/*
 * WHOLE ADDRESS SPACE: BIOS
 * Standalone Utility Functions: Firmware/OS routines
 * Random routines mid-BIOS: system routines
 * All of this shit: subroutines
 *
 */

/* FLASH MEMORY VARIABLES USED WITH BIOS FW CALL
 *  THIS IS IN RAM BANK 1, IN APP-SPACE!!!
 *  0x7d Fmbank - Specify flash bank to use (guess bit 0 or 1)
 *  0x7e Fmadd_h - Flash memory address (upper 8 bits)
 *  0x7f Fmadd_l - Flash memory address (lower 8 bits)
 */

/*
Document all known static metadata regions in the BIOS
1) BIOS version
2) Maple information
3) Font characters

4) Known harness-able utility functions that can be used via stack return attacks

add a public API that allows you to query and extract this info.

Present at 0x14BE in the BIOS, alongside some build info.
0x14BE JAP BIOS version info
0xAA7 US BIOS version info

Visual Memory Produced By or Under License From SEGA ENTERPRISES,LTD.
Version 1.004,1998/09/30,315-6208-01,SEGA Visual Memory System BIOS Produced by Sue
*/

/* Need a list of BIOS initialization registers with default values!
 *
 */

#if 0
EVMU_EXPORT EvmuAddress    EvmuRom_subroutineAddress    (EVMU_BIOS_SUBROUTINE sub)         GBL_NOEXCEPT;
EVMU_EXPORT EvmuAddress    EvmuRom_subroutineRetAddress (EVMU_BIOS_SUBROUTINE sub)         GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_callSubroutine       (GBL_CSELF,
                                                         EVMU_BIOS_SUBROUTINE subroutine)  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_biosVersionString    (GBL_CSELF, GblStringBuffer* pStr) GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord       EvmuRom_readByte             (GBL_CSELF, EvmuAddress address)   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_readBytes            (GBL_CSELF,
                                                         EvmuAddress base,
                                                         void*       pData,
                                                         GblSize*    pBytes)               GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord       EvmuRom_writeByte            (GBL_CSELF,
                                                         EvmuAddress address,
                                                         EvmuWord    value)                GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT    EvmuRom_writeBytes           (GBL_CSELF,
                                                         EvmuAddress base,
                                                         void*       pData,
                                                         GblSize*    pBytes)               GBL_NOEXCEPT;

EVMU_EXPORT EVMU_BIOS_TYPE EvmuRom_biosType             (GBL_CSELF)                        GBL_NOEXCEPT;
EVMU_EXPORT EVMU_BIOS_MODE EvmuRom_biosMode             (GBL_CSELF)                        GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT    EvmuRom_setBiosMode          (GBL_CSELF, EVMU_BIOS_MODE mode)   GBL_NOEXCEPT;

#endif
// Call this whole fucker "Bios"
// Call the thing that MUXes Flash + Bios "Rom"


// use system clock?

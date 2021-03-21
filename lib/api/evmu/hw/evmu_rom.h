#ifndef EVMU_ROM_H
#define EVMU_ROM_H

#include "../hw/evmu_peripheral.h"

//1 - Read-Only Memory
#define ROM_SIZE                65536
#define ROM_PAGE_SIZE           4096
#define ROM_SYS_PROG_ADDR_BASE  0x0000
#define ROM_SYS_PROG_SIZE       16384
#define ROM_OS_PROG_ADDR_BASE   0xed00
#define ROM_OS_PROG_SIZE        4096

#ifdef __cplusplus
extern "C" {
#endif

/*
 * WHOLE ADDRESS SPACE: BIOS
 * Standalone Utility Functions: Firmware/OS routines
 * Random routines mid-BIOS: system routines
 * All of this shit: subroutines
 *
 */

// Firmware calls: utility functions that return execution back to app
#define BIOS_ADDR_FM_WRT_EX     0x100
#define BIOS_ADDR_FM_WRTA_EX    0x108
#define BIOS_ADDR_FM_VRF_EX     0x110
#define BIOS_ADDR_FM_PRD_EX     0x120

// BIOS routines: control yields to BIOS?
#define BIOS_ADDR_TIMER_EX      0x130   // Timer update function used as ISR to populate date/time in system variables
#define BIOS_ADDR_SLEEP_EX      0x140
#define BIOS_ADDR_EXIT_EX       0x1f0   // MODE button?

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

Visual Memory Produced By or Under License From SEGA ENTERPRISES,LTD.
Version 1.004,1998/09/30,315-6208-01,SEGA Visual Memory System BIOS Produced by Sue
*/

/* Need a list of BIOS initialization registers with default values!
 *
 */



GBL_DEFINE_HANDLE(EvmuRom)

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

GBL_DECLARE_ENUM(EVMU_ROM_BIOS_SOUCE) {
    EVMU_ROM_BIOS_SOURCE_EMULATED,
    EVMU_ROM_BIOS_SOURCE_IMAGE
};

GBL_DECLARE_ENUM(EVMU_ROM_BIOS_MODE) {
    EVMU_ROM_BIOS_MODE_MAPLE,   //connected to DC, maple mode
    EVMU_ROM_BIOS_MODE_GAME,
    EVMU_ROM_BIOS_MODE_FILE,
    EVMU_ROM_BIOS_MODE_TIME,
    EVMU_ROM_BIOS_MODE_COUNT
};

GBL_DECLARE_ENUM(EVMU_ROM_BIOS_PROPERTY) {
    EVMU_ROM_BIOS_IMAGE_LOADED,
    EVMU_ROM_BIOS_SOURCE,
    EVMU_ROM_BIOS_MODE,
    EVMU_ROM_BIOS_PROPERTY_COUNT
} EVMU_ROM_BIOS_PROPERTY;

// load bios?
EVMU_API evmuRomReadBytes(EvmuRom hRom, EvmuAddress address, void* pData, EvmuSize bytes);
EVMU_API evmuRomWriteBytes(EvmuRom hRom, EvmuAddress address, const void* pData, EvmuSize* pBytes);

EVMU_API evmuRomBiosImageLoad(EvmuRom hRom, const void* pData, EvmuSize* pSize);

EVMU_API evmuBiosSubroutineCall(EvmuRom hRom, EVMU_BIOS_SUBROUTINE routine);
EVMU_API evmuBiosSubroutineAddress(EvmuRom hRom, EVMU_BIOS_SUBROUTINE routine, EvmuAddress* pAddress);
EVMU_API evmuBiosSubroutineReturnAddress(EvmuRom hRom, EVMU_BIOS_SUBROUTINE routine, EvmuAddress* pAddress);


// Call this whole fucker "Bios"
// Call the thing that MUXes Flash + Bios "Rom"


// use system clock?




#ifdef __cplusplus
}
#endif


#endif // EVMU_ROM_H

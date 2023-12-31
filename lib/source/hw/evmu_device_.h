#ifndef EVMU_DEVICE__H
#define EVMU_DEVICE__H

#include <evmu/hw/evmu_device.h>

#define EVMU_DEVICE_(instance)              (GBL_PRIVATE(EvmuDevice, instance))
#define EVMU_DEVICE_PUBLIC_(priv)           (GBL_PUBLIC(EvmuDevice, priv))

#define GBL_SELF_TYPE EvmuDevice_

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);
GBL_FORWARD_DECLARE_STRUCT(EvmuCpu_);
GBL_FORWARD_DECLARE_STRUCT(EvmuClock_);
GBL_FORWARD_DECLARE_STRUCT(EvmuLcd_);
GBL_FORWARD_DECLARE_STRUCT(EvmuBattery_);
GBL_FORWARD_DECLARE_STRUCT(EvmuBuzzer_);
GBL_FORWARD_DECLARE_STRUCT(EvmuGamepad_);
GBL_FORWARD_DECLARE_STRUCT(EvmuTimers_);
GBL_FORWARD_DECLARE_STRUCT(EvmuRom_);
GBL_FORWARD_DECLARE_STRUCT(EvmuPic_);
GBL_FORWARD_DECLARE_STRUCT(EvmuFlash_);
GBL_FORWARD_DECLARE_STRUCT(EvmuFat_);
GBL_FORWARD_DECLARE_STRUCT(EvmuWram_);

typedef struct EvmuDevice_ {
    EvmuTicks       remainingTicks;

    EvmuCpu_*       pCpu;
    EvmuRam_*       pRam;
    EvmuClock_*     pClock;
    EvmuLcd_*       pLcd;
    EvmuBattery_*   pBattery;
    EvmuBuzzer_*    pBuzzer;
    EvmuGamepad_*   pGamepad;
    EvmuTimers_*    pTimers;
    EvmuRom_*       pRom;
    EvmuPic_*       pPic;
    EvmuFlash_*     pFlash;
    EvmuFat_*       pFat;
    EvmuWram_*      pWram;
/*

    */
    //LcdAnimations
    //FileSystem

    // Call stack
    // Maple thinger
    // BuzzerTool
    // PerformanceProfiler
    // BatteryProfiler
    // % Memory Usage
    // LUT for IRQ name, address, return address
    // LUT for BIOS entrypoints
    // LUT for BIOS system addresses
    // set/return BIOS current state
    // set/view BIOS SLEEP timer
    // get BIOS version metadata address
    // get current date/time callback for BIOS clock from system clock?


} EvmuDevice_;

#define DEV_(dev) dev->pPrivate

#define DEV_MEMBER_(dev, member) DEV_(dev)->member

#define DEV_MEM_(dev)       DEV_MEMBER_(dev, memory)
#define DEV_CPU_(dev)       DEV_MEMBER_(dev, cpu)
#define DEV_CLOCK_(dev)     DEV_MEMBER_(dev, clock)
#define DEV_PIC_(dev)       DEV_MEMBER_(dev, pPic)
#define DEV_FLASH_(dev)     DEV_MEMBER_(dev, pFlash)


GBL_DECLS_END

#undef GBL_SELF_TYPE


#endif // EVMU_DEVICE__H

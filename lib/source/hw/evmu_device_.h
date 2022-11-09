#ifndef EVMU_DEVICE__H
#define EVMU_DEVICE__H

#include <evmu/hw/evmu_device.h>

#include "evmu_memory_.h"
#include "evmu_cpu_.h"
#include "evmu_clock_.h"
//#include "evmu_pic_.h"
#include "evmu_rom_.h"
#include "evmu_flash_.h"
//#include "evmu_lcd_.h"

#define EVMU_DEVICE_(instance)   ((EvmuDevice_*)GBL_INSTANCE_PRIVATE(instance, EVMU_DEVICE_TYPE))
#define EVMU_DEVICE_PUBLIC(priv) ((EvmuDevice*)GBL_INSTANCE_PUBLIC(priv, EVMU_DEVICE_TYPE))

#define GBL_SELF_TYPE EvmuDevice_

GBL_DECLS_BEGIN

typedef struct EvmuDevice_ {
    EvmuCpu_*       pCpu;
    EvmuMemory_*    pMemory;
    EvmuClock_*     pClock;
//    EvmuPic_*       pPic;
    EvmuRom_*       pRom;
    EvmuFlash_*     pFlash;
//    EvmuWram_*      pWram;
//    EvmuLcd_*       pLcd;
//    EvmuBuzzer_*    pBuzzer;
//    EvmuBattery_*   pBattery;
/*
    EvmuWram_       wram;
    EvmuTimerBase_  timerBase;
    EvmuTimer1_     timer1;
    EvmuTimer0_     timer0;
    EvmuPort1_      port1;
    EvmuPort3_      port3;
    EvmuPort7_      port7;
    EvmuPortExt_    portExt;
    EvmuSerial0_    serial1;
    EvmuSerial1_    serial1;


    EvmuBios_       bios;
    EvmuGamepad_    gamepad;
    EvmuBuzzer_     buzzer;
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

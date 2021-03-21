#ifndef EVMU_DEVICE__H
#define EVMU_DEVICE__H

#include <gimbal/gimbal_container.h>
#include <evmu/hw/evmu_device.h>

#include "evmu_memory_.h"
#include "evmu_clock_.h"
#include "evmu_cpu_.h"
#include "evmu_pic_.h"
#include "evmu_flash_.h"
#include "evmu_lcd_.h"

#define EVMU_DEVICE_ALIGNMENT   1

#ifdef __cplusplus
extern "C" {
#endif


typedef struct EvmuDevice_ {
    EvmuMemory_*    pMemory;
    EvmuCpu_*       pCpu;

    EvmuClock_*     pClock;
    EvmuPic_*       pPic;
    EvmuFlash_*     pFlash;
    EvmuLcd_*       pLcd;
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


    void*                               pUserdata;
    struct EvmuContext_*                pContext;
    EvmuEventHandler                    eventHandler;

    GblVector                           peripherals;
} EvmuDevice_;



inline EvmuPeripheral_* evmuDevicePeripheral_(const EvmuDevice_* pDevice, uint32_t index) {
    EvmuPeripheral_* pPeripheral = NULL;
    return GBL_RESULT_SUCCESS(gblVectorAt(&pDevice->peripherals, index, &pPeripheral)) ?
                pPeripheral :
                NULL;
}


#ifdef __cplusplus
}
#endif


#endif // EVMU_DEVICE__H

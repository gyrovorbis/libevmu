#include "gyro_vmu_device.h"
#include "gyro_vmu_lcd.h"
#include "gyro_vmu_cpu.h"
#include "gyro_vmu_tcp.h"
#include "gyro_vmu_serial.h"
#include "gyro_vmu_port1.h"
#include "gyro_vmu_sfr.h"
#include <gyro_system_api.h>
#include <gyro_vmu_buzzer.h>
#include <gyro_vmu_gamepad.h>
#include <gyro_vmu_osc.h>
#include <gyro_vmu_flash.h>
#include <string.h>
#include <stdlib.h>

VMUDevice* gyVmuDeviceCreate(void) {
    _gyLog(GY_DEBUG_VERBOSE, "Creating VMU Device.");
    VMUDevice* device = malloc(sizeof(VMUDevice));
    memset(device, 0, sizeof(VMUDevice));

    gyVmuFlashFormatDefault(device);
    gyVmuInterruptControllerInit(device);
    gyVmuBuzzerInit(device);
    gyVmuSerialInit(device);
    gyVmuCpuReset(device); //set initial, well-behaved values for internal pointers and shit!

    return device;
}

void gyVmuDeviceDestroy(VMUDevice* dev) {
    _gyLog(GY_DEBUG_VERBOSE, "Destroying VMU Device.");
    gyVmuBuzzerUninit(dev);
    free(dev);
}

int gyVmuDeviceDump(VMUDevice* dev, const char *path) {
    (void)dev;
    (void)path;
    return 0;
}

int gyVmuDeviceRestore(VMUDevice* dev, const char *path) {
    (void)dev;
    (void)path;
    return 0;
}

int gyVmuDeviceUpdate(VMUDevice* device, float deltaTime) {

    if(device->lcdFile) {
        gyVmuGamepadPoll(device);
        gyVmuLcdFileProcessInput(device);
        gyVmuLcdFileUpdate(device, deltaTime);
        gyVmuDisplayUpdate(device, deltaTime);
        return 1;
    } else {

            gyVmuSerialPortUpdate(device);

        if(deltaTime >= gyVmuOscSecPerCycle(device)) {
            //gyVmuSerialUpdate(device, deltaTime);
            gyVmuGamepadPoll(device);

    #ifdef VMU_TRIGGER_SPEED_FACTOR
            if(device->gamepad.lt) deltaTime /= VMU_TRIGGER_SPEED_FACTOR;
            if(device->gamepad.rt) deltaTime *= VMU_TRIGGER_SPEED_FACTOR;
    #endif

            if(!(device->sfr[SFR_OFFSET(SFR_ADDR_PCON)] & SFR_PCON_HOLD_MASK))
                gyVmuCpuTick(device, deltaTime);

            return 1;

        } else {

            return 0;
        }
    }
}

void gyVmuDeviceReset(VMUDevice* device) {
    gyVmuCpuReset(device);
    gyVmuBuzzerInit(device);
    gyVmuSerialInit(device);
    gyVmuBuzzerReset(device);
    gyVmuDisplayInit(device);
    gyVmuInterruptControllerInit(device);
    gyVmuInterruptSignal(device, VMU_INT_RESET);
}

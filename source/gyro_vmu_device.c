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

    gyVmuInterruptControllerInit(device);
    gyVmuBuzzerInit(device);
    gyVmuSerialInit(device);
    gyVmuFlashFormatDefault(device);
    gyVmuFlashRootBlockPrint(device);
    gyVmuCpuReset(device); //set initial, well-behaved values for internal pointers and shit!

    return device;
}

void gyVmuDeviceDestroy(VMUDevice* dev) {
    _gyLog(GY_DEBUG_VERBOSE, "Destroying VMU Device.");
    gyVmuBuzzerUninit(dev);
    free(dev);
}

int gyVmuDeviceSaveState(VMUDevice* dev, void* buffer, size_t size) {
    int success = 1;

    _gyLog(
        GY_DEBUG_VERBOSE,
        "Saving Device State: [%p..%p]",
        buffer,
        (char*)buffer + size);
    _gyPush();

    // Sanity checks on the buffer
    if(buffer == NULL) {
        _gyLog(GY_DEBUG_ERROR, "Invalid buffer: buffer is NULL");
        success = 0;
    } else if(size < GY_VMUDEVICE_SERIALIZE_DATA_SIZE) {
        _gyLog(
            GY_DEBUG_ERROR,
            "Invalid buffer: buffer is %u bytes, but must be %u.",
            size,
            GY_VMUDEVICE_SERIALIZE_DATA_SIZE);
        success = 0;
    } else {
        // Saving state is just backing up the various memory banks that `dev` manages.
        // Don't tell anyone. 😉
        memcpy(buffer, dev, GY_VMUDEVICE_SERIALIZE_DATA_SIZE);
    }
    _gyPop(1);
    return success;
}

int gyVmuDeviceLoadState(VMUDevice* dev, const void* buffer, size_t size) {
    int success = 1;

    _gyLog(GY_DEBUG_VERBOSE, "Loading Device State: [%p..%p]", buffer, buffer + size);
    _gyPush();

    // If the size doesn't match exactly, it may be wrong.
    // In practice, older versions will have an excessive size but our reads
    // will work correctly, so we allow them.
    if(buffer == NULL) {
        _gyLog(GY_DEBUG_ERROR, "Invalid buffer: buffer is NULL");
        success = 0;
    } else if(size < GY_VMUDEVICE_SERIALIZE_DATA_SIZE) {
        _gyLog(
            GY_DEBUG_ERROR,
            "Invalid buffer: buffer is %u bytes, but must be %u.",
            size,
            GY_VMUDEVICE_SERIALIZE_DATA_SIZE);
        success = 0;
    } else {
        if (size != GY_VMUDEVICE_SERIALIZE_DATA_SIZE) {
            _gyLog(
                GY_DEBUG_VERBOSE,
                "Given %u bytes, but only %u are needed. The rest will be ignored!",
                size,
                GY_VMUDEVICE_SERIALIZE_DATA_SIZE);
        }
        // Completely reset everything, and copy over the new data verbatim.
        memset(dev, 0, sizeof(*dev));
        memcpy(dev, buffer, GY_VMUDEVICE_SERIALIZE_DATA_SIZE);

        // Adjust the pointer values
        dev->imem = dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)]? dev->flash : dev->rom;
        dev->memMap[VMU_MEM_SEG_SFR]    = dev->sfr;

        // Be careful with this ordering - gyVmuMemRead uses some of these pointers
        int xramBk = gyVmuMemRead(dev, SFR_ADDR_XBNK);
        dev->memMap[VMU_MEM_SEG_XRAM]   = dev->xram[xramBk];
        int ramBk = (gyVmuMemRead(dev, SFR_ADDR_PSW) & SFR_PSW_RAMBK0_MASK) >> SFR_PSW_RAMBK0_POS;
        dev->memMap[VMU_MEM_SEG_GP1]    = dev->ram[ramBk];
        dev->memMap[VMU_MEM_SEG_GP2]    = &dev->ram[ramBk][VMU_MEM_SEG_SIZE];

        // Force shit to refresh!!
        dev->display.screenChanged = 1;
    }

    _gyPop(1);
    return success;
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

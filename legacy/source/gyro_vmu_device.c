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
#include <libGyro/gyro_file_api.h>
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

int gyVmuDeviceSaveState(VMUDevice* dev, const char* path) {

    _gyLog(GY_DEBUG_VERBOSE, "Saving Device State: [%s]", path);
    _gyPush();

    int     success = 1;
    GYFile* fp      = NULL;
    int     retVal  = gyFileOpen(path, "wb", &fp);

    if(!retVal || !fp) {
        _gyLog(GY_DEBUG_ERROR, "Could not open file for writing!");
        success = 0;
    } else {
        if(!gyFileWrite(fp, dev, sizeof(VMUDevice), 1)) {
            _gyLog(GY_DEBUG_ERROR, "Failed to write entirety of file!");
            success = 0;
        }
        gyFileClose(&fp);
    }

    _gyPop(1);
    return success;
}

int gyVmuDeviceLoadState(VMUDevice* dev, const char *path) {
    int success = 1;

    _gyLog(GY_DEBUG_VERBOSE, "Loading Device State: [%s]", path);
    _gyPush();

    GYFile* fp = NULL;
    int retVal = gyFileOpen(path, "rb", &fp);

    if(!retVal || !fp) {
        _gyLog(GY_DEBUG_ERROR, "Could not open file for reading!");
        success = 0;
    } else {
        // cache shit that needs to persist
        void*               pMemoryUserData = dev->pMemoryUserData;
        VMUMemoryChangeFn   pFnMemoryChange = dev->pFnMemoryChange;
        void*               pFlashUserData = dev->pFlashUserData;
        VMUFlashChangeFn    pFnFlashChange = dev->pFnFlashChange;

        size_t bytesRead = 0;
        retVal = gyFileRead(fp, dev, sizeof(VMUDevice), &bytesRead);

        if(!retVal || bytesRead != sizeof(VMUDevice)) {
            _gyLog(GY_DEBUG_ERROR, "Failed to read entirety of file! [Bytes read: %u/%u]", bytesRead, sizeof(VMUDevice));
            success = 0;
        } else {
            //adjust pointer values
            dev->pMemoryUserData = pMemoryUserData;
            dev->pFnMemoryChange = pFnMemoryChange;
            dev->pFlashUserData = pFlashUserData;
            dev->pFnFlashChange = pFnFlashChange;

            dev->imem = (dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)] & SFR_EXT_MASK)? dev->flash : dev->rom;
            //dev->imem = dev->flash;

            int xramBk = dev->sfr[SFR_OFFSET(SFR_ADDR_XBNK)];//gyVmuMemRead(dev, SFR_ADDR_XBNK);
            int ramBk = (dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)] & SFR_PSW_RAMBK0_MASK) >> SFR_PSW_RAMBK0_POS; //(gyVmuMemRead(dev, SFR_ADDR_PSW) & SFR_PSW_RAMBK0_MASK) >> SFR_PSW_RAMBK0_POS;
            dev->memMap[VMU_MEM_SEG_XRAM]   = dev->xram[xramBk];
            dev->memMap[VMU_MEM_SEG_SFR]    = dev->sfr;
            dev->memMap[VMU_MEM_SEG_GP1]    = dev->ram[ramBk];
            dev->memMap[VMU_MEM_SEG_GP2]    = &dev->ram[ramBk][VMU_MEM_SEG_SIZE];

            //force shit to refresh!!
            dev->display.screenChanged = 1;

            dev->lcdFile = NULL;
        }
        gyFileClose(&fp);

    }

    _gyPop(1);
    return success;
}

int gyVmuDeviceUpdate(VMUDevice* device, double deltaTime) {

    if(device->lcdFile) {
        gyVmuGamepadPoll(device);
        gyVmuLcdFileProcessInput(device);
        gyVmuLcdFileUpdate(device, deltaTime);
        gyVmuDisplayUpdate(device, deltaTime);
        return 1;
    } else {

//            gyVmuSerialPortUpdate(device);

        if(deltaTime >= gyVmuOscSecPerCycle(device)) {
            //gyVmuSerialUpdate(device, deltaTime);
            //gyVmuGamepadPoll(device);

    #ifdef VMU_TRIGGER_SPEED_FACTOR
            if(device->gamepad.lt) deltaTime /= VMU_TRIGGER_SPEED_FACTOR;
            if(device->gamepad.rt) deltaTime *= VMU_TRIGGER_SPEED_FACTOR;
    #endif

            //if(!(device->sfr[SFR_OFFSET(SFR_ADDR_PCON)] & SFR_PCON_HOLD_MASK))
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

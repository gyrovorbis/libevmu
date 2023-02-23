#include "gyro_vmu_device.h"
#include "gyro_vmu_lcd.h"
#include "gyro_vmu_cpu.h"
#include <gyro_system_api.h>
#include <gyro_vmu_flash.h>
#include <libGyro/gyro_file_api.h>
#include <string.h>
#include <stdlib.h>

#include "hw/evmu_device_.h"
#include "hw/evmu_memory_.h"
#include <evmu/hw/evmu_clock.h>

VMUDevice* gyVmuDeviceCreate(void) {
    _gyLog(GY_DEBUG_VERBOSE, "Creating VMU Device.");
    VMUDevice* device = malloc(sizeof(VMUDevice));
    memset(device, 0, sizeof(VMUDevice));
    return device;
}

int gyVmuDeviceInit(VMUDevice* device) {
    _gyLog(GY_DEBUG_VERBOSE, "Initializing VMU Device.");
    //gyVmuInterruptControllerInit(device);
    //gyVmuBuzzerInit(device);
    //gyVmuSerialInit(device);
    gyVmuFlashFormatDefault(device);
    gyVmuFlashRootBlockPrint(device);
//    gyVmuCpuReset(device); //set initial, well-behaved values for internal pointers and shit!
    return 0;
}

void gyVmuDeviceDestroy(VMUDevice* dev) {
    _gyLog(GY_DEBUG_VERBOSE, "Destroying VMU Device.");
    //gyVmuBuzzerUninit(dev);
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
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(dev);
    EvmuDevice* pDevice = EVMU_DEVICE_PUBLIC_(pDevice_);
    EvmuMemory_* pMemory_ = EVMU_MEMORY_(pDevice->pMemory);

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

            switch(EvmuMemory_viewInt(pDevice->pMemory, EVMU_ADDRESS_SFR_EXT)) {
            case EVMU_SFR_EXT_ROM:
                pMemory_->pExt = pMemory_->rom;
                break;
            case EVMU_SFR_EXT_FLASH_BANK_0:
                pMemory_->pExt = pMemory_->flash;
                break;
            case EVMU_SFR_EXT_FLASH_BANK_1:
                pMemory_->pExt = &pMemory_->flash[EVMU_FLASH_BANK_SIZE];
                break;
            default:
                GBL_CTX_WARN("EXT set to unknown value!");
            };

            //pDevice_->pMemory->imem = pDevice_->pMemory->flash;

            int xramBk = pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_XBNK)];//gyVmuMemRead(dev, SFR_ADDR_XBNK);
            int ramBk = (pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)] & EVMU_SFR_PSW_RAMBK0_MASK) >> EVMU_SFR_PSW_RAMBK0_POS; //(gyVmuMemRead(dev, EVMU_SFR_ADDR_PSW) & EVMU_SFR_PSW_RAMBK0_MASK) >> EVMU_SFR_PSW_RAMBK0_POS;
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_XRAM]   = pDevice_->pMemory->xram[xramBk];
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_SFR]    = pDevice_->pMemory->sfr;
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP1]    = pDevice_->pMemory->ram[ramBk];
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP2]    = &pDevice_->pMemory->ram[ramBk][VMU_MEM_SEG_SIZE];

            //force shit to refresh!!
            //dev->display.screenChanged = 1;
            EvmuLcd_setUpdated(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pLcd, GBL_TRUE);

            dev->lcdFile = NULL;
        }
        gyFileClose(&fp);

    }

    _gyPop(1);
    return success;
}

#if 0
int gyVmuDeviceUpdate(VMUDevice* device, double deltaTime) {

    if(device->lcdFile) {
        EvmuGamepad_poll(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pGamepad);
        gyVmuLcdFileProcessInput(device);
        gyVmuLcdFileUpdate(device, deltaTime);
        EvmuIBehavior_update(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pLcd), deltaTime*1000000);
        return 1;
    } else {

//            gyVmuSerialPortUpdate(device);

        if(deltaTime >= EvmuClock_systemSecsPerCycle(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pClock)) {
            //gyVmuSerialUpdate(device, deltaTime);
            //gyVmuGamepadPoll(device);

    #ifdef VMU_TRIGGER_SPEED_FACTOR
            if(EvmuGamepad_buttonPressed(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pGamepad,
                                         EVMU_GAMEPAD_BUTTON_REWIND)) deltaTime /= VMU_TRIGGER_SPEED_FACTOR;
            if(EvmuGamepad_buttonPressed(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pGamepad,
                                         EVMU_GAMEPAD_BUTTON_FAST_FORWARD)) deltaTime *= VMU_TRIGGER_SPEED_FACTOR;
    #endif

            //if(!(device->sfr[EVMU_SFR_OFFSET(SFR_ADDR_PCON)] & EVMU_SFR_PCON_HOLD_MASK))
                gyVmuCpuTick(device, deltaTime);

            return 1;

        } else {

            return 0;
        }
    }
}

void gyVmuDeviceReset(VMUDevice* device) {
    gyVmuCpuReset(device);
    EvmuIBehavior_reset(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pBuzzer));
    //gyVmuSerialInit(device);
    EvmuIBehavior_reset(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pLcd));
    EvmuIBehavior_reset(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pPic));
    EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pPic, EVMU_IRQ_RESET);
}
#endif

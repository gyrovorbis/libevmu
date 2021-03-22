#include <evmu/evmu_api.h>
#include "evmu_device_.h"
#include "evmu_peripheral_.h"
#include "../util/evmu_context_.h"


#define FOREACH_PERIPHERAL(DEVICE, INDEX, POINTER)              \
    EvmuPeripheral_* POINTER = NULL;                            \
    for(uint32_t INDEX = 0;                                     \
        INDEX < peripheralCount_(DEVICE);                       \
        POINTER = evmuDevicePeripheral_(DEVICE, INDEX++))

#define FOREACH_PERIPHERAL_DYNAMIC(DEVICE, INDEX, POINTER)                          \
    EvmuPeripheral_* POINTER = NULL;                                                \
    for(uint32_t INDEX = 0;                                                         \
        INDEX < peripheralDynamicCount_(DEVICE);                                    \
        POINTER = evmuDevicePeripheral_(DEVICE, peripheralDynamicIndex_(INDEX++)))


#define ADD_BUILTIN_PERIPHERAL(ID_SUFFIX, NAME) \
    {                                           \
        EVMU_PERIPHERAL_##ID_SUFFIX,            \
        &evmu##NAME##Driver_,                   \
        offsetof(EvmuDevice_, p##NAME)          \
    }

static struct {
    EVMU_PERIPHERAL_INDEX         index;
    const EvmuPeripheralDriver*   pDriver;
    size_t                        offset;
} peripheralsBuiltin_[] = {
    ADD_BUILTIN_PERIPHERAL(CPU,         Cpu),
    ADD_BUILTIN_PERIPHERAL(MEMORY,      Memory),
    ADD_BUILTIN_PERIPHERAL(CLOCK,       Clock),
    ADD_BUILTIN_PERIPHERAL(PIC,         Pic),
    ADD_BUILTIN_PERIPHERAL(FLASH,       Flash),
    ADD_BUILTIN_PERIPHERAL(LCD,         Lcd)
};

#undef ADD_BUILTIN_PERIPHERAL

#define BUILTIN_PERIPHERAL_COUNT (sizeof(peripheralsBuiltin_)/sizeof(peripheralsBuiltin_[0]))

static EvmuBool peripheralIsBuiltin_(EVMU_PERIPHERAL_INDEX index) {
    for(unsigned i = 0; i < BUILTIN_PERIPHERAL_COUNT; ++i) {
        if(peripheralsBuiltin_[i].index == index)
            return EVMU_TRUE;
    }
    return EVMU_FALSE;
}

uint32_t peripheralCount_(const EvmuDevice_* pDevice) {
    GblSize total;
    return GBL_RESULT_SUCCESS(gblVectorSize(&pDevice->peripherals, &total))?
                (uint32_t)total : 0;
}

uint32_t peripheralDynamicCount_(const EvmuDevice_* pDevice) {
    uint32_t count = 0;
    GblSize total;

    if(GBL_RESULT_SUCCESS(gblVectorSize(&pDevice->peripherals, &total))) {
        if(total > BUILTIN_PERIPHERAL_COUNT) {
            count = total - BUILTIN_PERIPHERAL_COUNT;
        }
    }
    return count;
}

uint32_t peripheralDynamicStartIndex_(const EvmuDevice_* pDevice) {
    (void)pDevice;
    return BUILTIN_PERIPHERAL_COUNT;
}

uint32_t peripheralDynamicIndex_(const EvmuDevice_* pDevice, uint32_t number) {
    return peripheralDynamicStartIndex_(pDevice) + number;
}

// Unconditional remove, internal, doesn't give a shit about builtin or not
static EVMU_RESULT peripheralRemove_(EvmuDevice hDevice, EvmuPeripheral hPeripheral) {
    EVMU_API_BEGIN(hDevice && "Removing Peripheral from Device");

    EVMU_API_RESULT_SET(evmuPeripheralDeinit_(hPeripheral), "Peripheral deinitialization failed!");
    EVMU_API_RESULT_SET(glVectorRemove(&hDevice->peripherals, index), "Failed to remove entry from peripherals vector!");
    EVMU_API_RESULT_SET(evmuContextEventEmit_(NULL, hDevice, NULL, EVMU_EVENT_PERIPHERAL_REMOVED, hPeripheral, sizeof(hPeripheral)), "Failed to emit signal!");
    EVMU_API_FREE(hPeripheral);

    EVMU_API_END();
}

EVMU_API evmuDeviceContext(EvmuDevice hDevice, EvmuContext* phContext) {
    EVMU_API_BEGIN(hDevice);
    EVMU_API_VERIFY_POINTER(phContext);

    *phContext = hDevice->pContext;

    EVMU_API_END();
}
EVMU_API evmuDeviceUserdata(EvmuDevice hDevice, void** ppUserdata) {
    EVMU_API_BEGIN(hDevice);
    EVMU_API_VERIFY_POINTER(phContext);

    *ppUserdata = hDevice->pUserdata;

    EVMU_API_END();
}

EVMU_API evmuDeviceEventHandler(EvmuDevice hDevice, EvmuEventHandler* pHandler) {
    EVMU_API_BEGIN(hCtx);
    EVMU_API_VERIFY_POINTER(pHandler);
    memcpy(pHandler, &hDevice->eventHandler, sizeof(EvmuEventHandler));
    EVMU_API_END();
}

EVMU_API evmuDeviceEventHandlerSet(EvmuContext hDevice, const EvmuEventHandler* pHandler) {
    EVMU_API_BEGIN(hCtx);
    EVMU_API_VERIFY_POINTER(pHandler);
    memcpy(&hDevice->eventHandler, pHandler, sizeof(EvmuEventHandler));
    EVMU_API_END();
}


EVMU_API evmuDevicePeripheralAdd(EvmuDevice hDevice, const EvmuPeripheralDriver* pDriver, EvmuPeripheral* phPeripheral) {
    EVMU_API_BEGIN(hDevice && "Adding Peripheral to Device");
    EVMU_API_VERIFY_POINTER(pDriver);
    EVMU_API_VERIFY_POINTER(phPeriperal);
    EvmuPeripheral_* pPeripheral = EVMU_API_MALLOC(pDriver->instanceSize, 1);
    memset(pPeripheral, 0, sizeof(EvmuPeripheral_));
    pPeripheral->pDevice = hDevice;
    pPeripheral->pDriver = pDriver;
    *phPeripheral = pPeripheral;

    EVMU_API_VERIFY(gblVectorPushBack(&hDevice->peripherals, pPeripheral));

    GblSize size = 0;
    EVMU_API_VERIFY(gblVectorsSize(&hDevice->peripherals, &size));

    pPeripheral->id = (EvmuEnum)size;

    EVMU_API_VERIFY(evmuPeripheralInit_(pPeripheral));

    EVMU_API_VERIFY(evmuContextEventEmit_(NULL, hDevice, NULL, EVMU_EVENT_PERIPHERAL_ADDED, pPeripheral, sizeof(pPeripheral)));

    EVMU_API_END();
}

// API entry point, validates peripheral, ensures it isn't builtin
EVMU_API evmuDevicePeripheralRemove(EvmuDevice hDevice, EvmuPeripheral hPeripheral) {
    EVMU_API_BEGIN(hDevice && "Attempting to remove Peripheral from Device");
    EVMU_API_VERIFY_HANDLE(hPeripheral);
#if 0
    EVMU_API_RESULT_SET_JMP_CND(gblVectorFind(&hDevice->peripherals, hPeripheral, &index) && index != GBL_VECTOR_INDEX_INVALID,
                               EVMU_RESULT_ERROR_INVALID_PERIPHERAL,
                               GBL_API_END_LABEL_DEFAULT,
                               "The peripheral isn't even attached to the given device!");

    EVMU_API_RESULT_SET_JMP_CND(!isBuiltinPeripheral_(hPeripheral->id),
                                EVMU_RESULT_ERROR_INVALID_PERIPHERAL,
                                GBL_API_END_LABEL_DEFAULT,
                                "Cannot remove builtin peripheral!");

    EVMU_API_VERIFY(peripheralRemove_(hDevice, hPeripheral));
#endif
    EVMU_API_END();
}


EVMU_API evmuDevicePeripheralCount(EvmuDevice hDevice, uint32_t* pCount) {
    EVMU_API_BEGIN(hDevice);
    EVMU_API_VERIFY_POINTER(pCount);

    *pCount = 0;
    GblSize size;
    EVMU_API_VERIFY(gblVectorSize(hDevice->peripherals, &size));
    *pCount = (uint32_t)size;

    EVMU_API_END();
}

EVMU_API evmuDevicePeripheral(EvmuDevice hDevice, EvmuEnum index, EvmuPeripheral* phPeripheral) {
    EVMU_API_BEGIN(hDevice);
    EVMU_API_VERIFY_POINTER(phPeripheral);
    EVMU_API_VERIFY(gblVectorAt(&hDevice->peripherals, index, phPeripheral));
    EVMU_API_END();
}

EVMU_API evmuDevicePeripheralFind(EvmuDevice hDevice, const char* pName, EvmuPeripheral* phPeripheral) {
    EVMU_API_BEGIN(hDevice);
    EVMU_API_VERIFY_POINTER(pName);
    EVMU_API_VERIFY_POINTER(phPeripheral);

    *phPeripheral = NULL;
    GblSize size = 0;
    EVMU_API_VERIFY(gblVectorSize(&hDevice->peripherals, &size));

    FOREACH_PERIPHERAL(hDevice, index, pCurrent) {
        if(strcmp(pName, pCurrent->pDriver->pName) == 0) {
            *phPeripheral = pCurrent;
            break;
        }
    }

    EVMU_API_END();
}



/* @TODO:
 * create
    - handle context bullshit
    - create/init peripherals
   state save/load:
    - version checks/reporting
    - serialize peripheral config
    - peripheral fault tolerance
    - one peripheral serializing differently cannot fuck whole format!
*/
EVMU_API evmuDeviceCreate(EvmuDevice* phDevice, const EvmuDeviceCreateInfo* pInfo) {
    EvmuContext_* pContext = pInfo->hContext;
    // create default context

    EVMU_API_BEGIN(pContext);
    EVMU_API_VERIFY_POINTER(phDevice);
    EVMU_API_VERBOSE("Creating Device");
    EVMU_API_PUSH();

    *phDevice = EVMU_API_MALLOC(sizeof(EvmuDevice), EVMU_DEVICE_ALIGNMENT);
    memset(*phDevice, 0, sizeof(EvmuDevice_));
    (*phDevice)->pUserdata          = pInfo->pUserdata;
    (*phDevice)->pContext           = pContext;
    memcpy(&(*phDevice)->eventHandler, &pInfo->eventHandler, sizeof(EvmuEventHandler));
#if 0
    EVMU_API_RESULT_SET_JMP(gblVectorCreate(&(*phDevice)->peripherals, pContext, sizeof(EvmuPeripheral_), NULL, 0, BUILTIN_PERIPHERAL_COUNT),
                           init_fail,
                           "Failed to create Peripheral* vector!");

    EVMU_API_RESULT_SET_JMP(evmuContextDeviceAdd_(pContext, *phDevice),
                           init_fail,
                           "Failed to add the Device to the Context!");
#endif
    // Add builtin peripherals
    EVMU_API_PUSH("Adding Builtin Hardware Peripherals");
    for(uint32_t d = 0; d < BUILTIN_PERIPHERAL_COUNT; ++d) {
#if 0
        EVMU_API_RESULT_ACCUM(evmuDevicePeripheralAdd(*phDevice,
                                                 builtinPeripherals_[d].pDriver,
                                                 (void*)*phDevice + builtinPeripherals_[d].offset));
#endif
    }
    EVMU_API_POP(2);
    EVMU_API_DONE();

init_fail:
    EVMU_API_FREE(phDevice);
    *phDevice = NULL;
    EVMU_API_POP();

    EVMU_API_END();
}

EVMU_API evmuDeviceDestroy(EvmuDevice hDevice) {
    EVMU_API_BEGIN(hDevice);
    EVMU_API_VERIFY_HANDLE(hDevice);

    // Remove from Context
    EVMU_API_RESULT_ACCUM(evmuContextDeviceRemove_(hDevice->pContext, hDevice));

    // Remove all Peripherals
    FOREACH_PERIPHERAL(hDevice, index, pPeripheral) {
        EVMU_API_RESULT_ACCUM(peripheralRemove_(hDevice, pPeripheral));
    }

    //Destroy
    EVMU_API_FREE(hDevice);
    EVMU_API_END();
}

EVMU_API evmuDeviceReset(EvmuDevice hDevice) {
    EVMU_API_BEGIN(hDevice && "Resetting Device");
    EVMU_API_VERIFY_HANDLE(hDevice);

    FOREACH_PERIPHERAL(hDevice, index, pPeripheral) {
        EVMU_API_RESULT_ACCUM(evmuPeripheralReset_(pPeripheral));
    }

   // EVMU_API_RESULT_SET(evmuContextEventEmit_(NULL, hDevice, NULL, EVMU_EVENT_DEVICE_RESET, NULL, 0, "Failed to emit signal!");

    EVMU_API_END();
}

EVMU_API evmuDeviceUpdate(EvmuDevice hDevice, EvmuTicks ticks) {
    EVMU_API_BEGIN(hDevice && "Updating Device");

    EvmuTicks step = 1;
    EvmuTicks elapsed = 0;
    EVMU_API_VERIFY(evmuClockTimestepTicks(hDevice->pClock, &step));


    //Add carry/error accumulation later!
    while(elapsed + step <= ticks) {
        FOREACH_PERIPHERAL(hDevice, index, pPeripheral) {
            EVMU_API_RESULT_ACCUM(evmuPeripheralUpdate_(pPeripheral, step));
            elapsed += step;
        }
    }

    //EVMU_API_RESULT_SET(evmuContextEventEmit_(NULL, hDevice, NULL, EVMU_EVENT_DEVICE_UPDATE, NULL, 0, "Failed to emit signal!");

    EVMU_API_END();
}
#if 0
EVMU_API evmuDeviceStateSave(EvmuDevice hDevice, const char* pPath) {
    EVMU_API_BEGIN(hDevice, "Saving State: [%s]", pPath);
    EVMU_API_VERIFY_HANDLE(hDevice);
    EVMU_API_VERIFY_POINTER(pPath);

    EvmuFile hFile = EVMU_API_FILE_OPEN(pPath);

    // Write version information
    // Write list of peripherals
    // Write peripheral offsets?


    FOREACH_PERIPHERAL(hDevice, index, pPeripheral) {
        EVMU_API_RESULT_ACCUM(evmuPeripheralStateSave_(pPeripheral));
    }

    EVMU_API_FILE_CLOSE(hFile);
    EVMU_API_END();

}

EVMU_API evmuDeviceStateLoad(EvmuDevice hDevice, const char* pPath) {
    EVMU_API_BEGIN(hDevice, "Loading State: [%s", pPath]);
    EVMU_API_VERIFY_HANDLE(hDevice);
    EVMU_API_VERIFY_POINTER(pPath);

    EvmuFile hFile = EVMU_API_FILE_OPEN(pPath);

    //Check version, report version mismatch?
    //Load peripherals?
    //Be fault tolerant to fucked peripherals?

    FOREACH_PERIPHERAL(hDevice, index, pPeripheral) {
        EVMU_API_RESULT_ACCUM(evmuPeripheralStateLoad_(pPeripheral));
    }

    EVMU_API_FILE_CLOSE(hFile);
    EVMU_API_END();
}
#endif











#if 0

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
        size_t bytesRead = 0;
        retVal = gyFileRead(fp, dev, sizeof(VMUDevice), &bytesRead);

        if(!retVal || bytesRead != sizeof(VMUDevice)) {
            _gyLog(GY_DEBUG_ERROR, "Failed to read entirety of file! [Bytes read: %u/%u]", bytesRead, sizeof(VMUDevice));
            success = 0;
        } else {
            //adjust pointer values

            dev->imem = dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)]? dev->flash : dev->rom;
            dev->imem = dev->flash;

            int xramBk = gyVmuMemRead(dev, SFR_ADDR_XBNK);
            int ramBk = (gyVmuMemRead(dev, SFR_ADDR_PSW) & SFR_PSW_RAMBK0_MASK) >> SFR_PSW_RAMBK0_POS;
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

#endif

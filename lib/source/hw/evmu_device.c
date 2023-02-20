#include <evmu/hw/evmu_device.h>
#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_cpu.h>
#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_pic.h>
#include <evmu/hw/evmu_rom.h>
#include <evmu/hw/evmu_flash.h>

#include <gyro_vmu_lcd.h>
#include <gyro_vmu_cpu.h>

#include "evmu_device_.h"
#include "evmu_memory_.h"
#include "evmu_cpu_.h"
#include "evmu_clock_.h"
#include "evmu_lcd_.h"
#include "evmu_battery_.h"
#include "evmu_buzzer_.h"
#include "evmu_gamepad_.h"
#include "evmu_timers_.h"
#include "evmu_rom_.h"
#include "evmu_pic_.h"

static GBL_RESULT EvmuDevice_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuDevice* pDevice = EVMU_DEVICE(pSelf);
    EvmuDevice_* pSelf_ = EVMU_DEVICE_(pDevice);

    // Call parent constructor
    GBL_INSTANCE_VCALL_DEFAULT(GblObject, pFnConstructor, pSelf);

    // Create peripherals
    pDevice->pMemory  = GBL_OBJECT_NEW(EvmuMemory,
                                       "parent", pSelf);

    pDevice->pCpu     = GBL_OBJECT_NEW(EvmuCpu,
                                       "parent", pSelf);

    pDevice->pClock   = GBL_OBJECT_NEW(EvmuClock,
                                       "parent", pSelf);

    pDevice->pLcd     = GBL_OBJECT_NEW(EvmuLcd,
                                       "parent", pSelf);

    pDevice->pBattery = GBL_OBJECT_NEW(EvmuBattery,
                                       "parent", pSelf);

    pDevice->pBuzzer  = GBL_OBJECT_NEW(EvmuBuzzer,
                                       "parent", pSelf);

    pDevice->pGamepad = GBL_OBJECT_NEW(EvmuGamepad,
                                       "parent", pSelf);

    pDevice->pTimers  = GBL_OBJECT_NEW(EvmuTimers,
                                       "parent", pSelf);

    pDevice->pRom     = GBL_OBJECT_NEW(EvmuRom,
                                       "parent", pSelf);

    pDevice->pPic     = GBL_OBJECT_NEW(EvmuPic,
                                       "parent", pSelf);

    // Cache private pointers
    pSelf_->pMemory  = EVMU_MEMORY_(pDevice->pMemory);
    pSelf_->pCpu     = EVMU_CPU_(pDevice->pCpu);
    pSelf_->pClock   = EVMU_CLOCK_(pDevice->pClock);
    pSelf_->pLcd     = EVMU_LCD_(pDevice->pLcd);
    pSelf_->pBattery = EVMU_BATTERY_(pDevice->pBattery);
    pSelf_->pBuzzer  = EVMU_BUZZER_(pDevice->pBuzzer);
    pSelf_->pGamepad = EVMU_GAMEPAD_(pDevice->pGamepad);
    pSelf_->pTimers  = EVMU_TIMERS_(pDevice->pTimers);
    pSelf_->pRom     = EVMU_ROM_(pDevice->pRom);
    pSelf_->pPic     = EVMU_PIC_(pDevice->pPic);

    // Initialize dependencies
    pSelf_->pMemory->pCpu     = pSelf_->pCpu;
    pSelf_->pCpu->pMemory     = pSelf_->pMemory;
    pSelf_->pClock->pMemory   = pSelf_->pMemory;
    pSelf_->pLcd->pMemory     = pSelf_->pMemory;
    pSelf_->pBattery->pMemory = pSelf_->pMemory;
    pSelf_->pBuzzer->pMemory  = pSelf_->pMemory;
    pSelf_->pGamepad->pMemory = pSelf_->pMemory;
    pSelf_->pTimers->pMemory  = pSelf_->pMemory;
    pSelf_->pRom->pMemory     = pSelf_->pMemory;
    pSelf_->pPic->pMemory     = pSelf_->pMemory;

    // Initialize fucking reest
    pSelf_->pReest = gyVmuDeviceCreate();
    pSelf_->pReest->pPristineDevice = pSelf_;
    gyVmuDeviceInit(pSelf_->pReest);

    GBL_CTX_VERIFY_CALL(EvmuIBehavior_reset(EVMU_IBEHAVIOR(pSelf)));

    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EvmuDevice* pDevice = EVMU_DEVICE(pSelf);
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);
#if 0
    GBL_BOX_UNREF(pDevice->pMemory);
    GBL_BOX_UNREF(pDevice->pCpu);
    GBL_BOX_UNREF(pDevice->pClock);
    GBL_BOX_UNREF(pDevice->pLcd);
    GBL_BOX_UNREF(pDevice->pBattery);
    GBL_BOX_UNREF(pDevice->pBuzzer);
    GBL_BOX_UNREF(pDevice->pGamepad);
    GBL_BOX_UNREF(pDevice->pTimers);
    GBL_BOX_UNREF(pDevice->pRom);
    GBL_BOX_UNREF(pDevice->pPic);
#endif
    gyVmuDeviceDestroy(pDevice_->pReest);

    GBL_INSTANCE_VCALL_DEFAULT(GblObject, base.pFnDestructor, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_reset_(EvmuIBehavior* pIBehavior) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pIBehavior);
    //EvmuPic_raiseIrq(EVMU_DEVICE(pIBehavior)->pPic, EVMU_IRQ_RESET);
    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_update_(EvmuIBehavior* pIBehavior, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    EvmuDevice*  pSelf  = EVMU_DEVICE(pIBehavior);
    VMUDevice*   device = EVMU_DEVICE_REEST(pSelf);

    // fuck the base implementation, do it manually
    //GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);

    EvmuTicks timestep = EvmuClock_systemTicksPerCycle(pSelf->pClock);

  //  pSelf_->remainingTicks += ticks;

  //  while(pSelf_->remainingTicks >= timestep) {
        double deltaTime = ticks / 1000000000.0;

        if(device->lcdFile) {
            EvmuGamepad_poll(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pGamepad);
            gyVmuLcdFileProcessInput(device);
            gyVmuLcdFileUpdate(device, deltaTime);
            EvmuIBehavior_update(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pLcd), timestep);
        } else {

            if(EvmuGamepad_buttonPressed(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pGamepad,
                                         EVMU_GAMEPAD_BUTTON_REWIND))
                ticks /= VMU_TRIGGER_SPEED_FACTOR;
            if(EvmuGamepad_buttonPressed(EVMU_DEVICE_PRISTINE_PUBLIC(device)->pGamepad,
                                         EVMU_GAMEPAD_BUTTON_FAST_FORWARD))
                ticks *= VMU_TRIGGER_SPEED_FACTOR;

            EvmuIBehavior_update(EVMU_IBEHAVIOR(pSelf->pCpu), ticks);
        }

     //   pSelf_->remainingTicks -= timestep;
    //}

    GBL_CTX_END();
}

GBL_EXPORT GblSize EvmuDevice_peripheralCount(const EvmuDevice* pSelf) {
    GblSize count = 0;
    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_INSTANCE_CHECK(pIter, EvmuPeripheral)) ++count;
    }
    return count;
}

GBL_EXPORT EvmuPeripheral* EvmuDevice_peripheralAt(const EvmuDevice* pSelf, GblSize index) {
    EvmuPeripheral* pPeripheral = NULL;
    GblSize count = 0;

    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_INSTANCE_CHECK(pIter, EvmuPeripheral)) {
            if(count++ == index) {
                pPeripheral = EVMU_PERIPHERAL(pIter);
                break;
            }
        }
    }

    return pPeripheral;
}

GBL_EXPORT EvmuPeripheral* EvmuDevice_peripheralByName(const EvmuDevice* pSelf, const char* pName) {
    return EVMU_PERIPHERAL(GblObject_findChildByName(GBL_OBJECT(pSelf), pName));
}

static GBL_RESULT EvmuDeviceClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset   = EvmuDevice_reset_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate  = EvmuDevice_update_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor = EvmuDevice_constructor_;
    GBL_BOX_CLASS(pClass)->pFnDestructor     = EvmuDevice_destructor_;

    GBL_CTX_END();
}


GBL_EXPORT GblType EvmuDevice_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInterfaceMapEntry ifaceEntries[] = {
        {
            .classOffset   = offsetof(EvmuDeviceClass, EvmuIBehaviorImpl)
        }
    };

    const static GblTypeInfo info = {
        .pFnClassInit         = EvmuDeviceClass_init_,
        .classSize            = sizeof(EvmuDeviceClass),
        .instanceSize         = sizeof(EvmuDevice),
        .instancePrivateSize  = sizeof(EvmuDevice_),
        .interfaceCount       = 1,
        .pInterfaceMap        = ifaceEntries
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        ifaceEntries[0].interfaceType = EVMU_IBEHAVIOR_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuDevice"),
                                      GBL_OBJECT_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}

EVMU_EXPORT VMUDevice* EvmuDevice_REEST(const EvmuDevice* pSelf) {
    return EVMU_DEVICE_REEST(pSelf);
}

#include <evmu/hw/evmu_device.h>
#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_cpu.h>
#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_pic.h>
#include <evmu/hw/evmu_rom.h>
#include <evmu/hw/evmu_flash.h>

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
#include "evmu_flash_.h"
#include "../fs/evmu_fat_.h"

EVMU_EXPORT EvmuDevice* EvmuDevice_create(void) {
    return GBL_NEW(EvmuDevice);
}

EVMU_EXPORT GblRefCount EvmuDevice_unref(EvmuDevice* pSelf) {
    return GBL_UNREF(pSelf);
}

static GBL_RESULT EvmuDevice_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuDevice* pDevice = EVMU_DEVICE(pSelf);
    EvmuDevice_* pSelf_ = EVMU_DEVICE_(pDevice);

    // Call parent constructor
    GBL_INSTANCE_VCALL_DEFAULT(GblObject, pFnConstructor, pSelf);

    // Create peripherals
    pDevice->pMemory  = GBL_NEW(EvmuMemory,
                                "parent", pSelf);

    pDevice->pCpu     = GBL_NEW(EvmuCpu,
                                "parent", pSelf);

    pDevice->pClock   = GBL_NEW(EvmuClock,
                                "parent", pSelf);

    pDevice->pLcd     = GBL_NEW(EvmuLcd,
                                "parent", pSelf);

    pDevice->pBattery = GBL_NEW(EvmuBattery,
                                "parent", pSelf);

    pDevice->pBuzzer  = GBL_NEW(EvmuBuzzer,
                                "parent", pSelf);

    pDevice->pGamepad = GBL_NEW(EvmuGamepad,
                                "parent", pSelf);

    pDevice->pTimers  = GBL_NEW(EvmuTimers,
                                "parent", pSelf);

    pDevice->pRom     = GBL_NEW(EvmuRom,
                                "parent", pSelf);

    pDevice->pPic     = GBL_NEW(EvmuPic,
                                "parent", pSelf);

    pDevice->pFileMgr = GBL_NEW(EvmuFileManager,
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
    pSelf_->pFlash   = EVMU_FLASH_(pDevice->pFlash);
    pSelf_->pFat     = EVMU_FAT_(pDevice->pFat);

    // Initialize dependencies
    pSelf_->pMemory->pCpu     = pSelf_->pCpu;
    pSelf_->pMemory->pFlash   = pSelf_->pFlash;
    pSelf_->pCpu->pMemory     = pSelf_->pMemory;
    pSelf_->pClock->pMemory   = pSelf_->pMemory;
    pSelf_->pLcd->pMemory     = pSelf_->pMemory;
    pSelf_->pBattery->pMemory = pSelf_->pMemory;
    pSelf_->pBuzzer->pMemory  = pSelf_->pMemory;
    pSelf_->pGamepad->pMemory = pSelf_->pMemory;
    pSelf_->pTimers->pMemory  = pSelf_->pMemory;
    pSelf_->pTimers->pBuzzer  = pSelf_->pBuzzer;
    pSelf_->pRom->pMemory     = pSelf_->pMemory;
    pSelf_->pPic->pMemory     = pSelf_->pMemory;
    pSelf_->pFat->pMemory     = pSelf_->pMemory;

    //!\todo move this to EvmuFat
    GBL_CTX_CALL(EvmuFat_format(pDevice->pFat, NULL));
    EvmuFat_log(pDevice->pFat);

    GBL_CTX_VERIFY_CALL(EvmuIBehavior_reset(EVMU_IBEHAVIOR(pSelf)));

    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EvmuDevice* pDevice = EVMU_DEVICE(pSelf);

    GBL_UNREF(pDevice->pMemory);
    GBL_UNREF(pDevice->pCpu);
    GBL_UNREF(pDevice->pClock);
    GBL_UNREF(pDevice->pLcd);
    GBL_UNREF(pDevice->pBattery);
    GBL_UNREF(pDevice->pBuzzer);
    GBL_UNREF(pDevice->pGamepad);
    GBL_UNREF(pDevice->pTimers);
    GBL_UNREF(pDevice->pRom);
    GBL_UNREF(pDevice->pPic);
    GBL_UNREF(pDevice->pFlash);
    GBL_UNREF(pDevice->pFat);

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

    // fuck the base implementation, do it manually
    //GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);

    if(pSelf->pGamepad->slowMotion)
        ticks /= 10.0f;
    if(pSelf->pGamepad->fastForward)
        ticks *= 10.0f;

    EvmuIBehavior_update(EVMU_IBEHAVIOR(pSelf->pCpu), ticks);

    GBL_CTX_END();
}

EVMU_EXPORT size_t EvmuDevice_peripheralCount(const EvmuDevice* pSelf) {
    size_t count = 0;
    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_INSTANCE_CHECK(pIter, EvmuPeripheral)) ++count;
    }
    return count;
}

EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheral(const EvmuDevice* pSelf, size_t index) {
    EvmuPeripheral* pPeripheral = NULL;
    size_t count = 0;

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

EVMU_EXPORT EvmuPeripheral* EvmuDevice_findPeripheral(const EvmuDevice* pSelf, const char* pName) {
    return EVMU_PERIPHERAL(GblObject_findChildByName(GBL_OBJECT(pSelf), pName));
}

static GBL_RESULT EvmuDeviceClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuDevice_reset_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuDevice_update_;
    GBL_OBJECT_CLASS(pClass)    ->pFnConstructor = EvmuDevice_constructor_;
    GBL_BOX_CLASS(pClass)       ->pFnDestructor  = EvmuDevice_destructor_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuDevice_type(void) {
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

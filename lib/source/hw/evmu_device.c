#include <evmu/hw/evmu_device.h>
#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_cpu.h>
#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_pic.h>
#include <evmu/hw/evmu_rom.h>
#include <evmu/hw/evmu_flash.h>
#include <gimbal/meta/instances/gimbal_module.h>

#include "evmu_device_.h"
#include "evmu_clock_.h"


static GBL_RESULT EvmuDevice_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuDevice_* pSelf_ = EVMU_DEVICE_(pSelf);

    GBL_INSTANCE_VCALL_DEFAULT(GblObject, pFnConstructor, pSelf);

    // Create components
    EvmuMemory* pMem  = GBL_OBJECT_NEW(EvmuMemory,
                                       "parent", pSelf,
                                       "name",   "memory");
#if 0
    EvmuCpu* pCpu = GBL_OBJECT_NEW(EvmuCpu,
                                   "parent", pSelf,
                                   "name",   "cpu");
#endif
    EvmuClock* pClock = GBL_OBJECT_NEW(EvmuClock,
                                       "parent", pSelf,
                                       "name",   "clock");
#if 0
    EvmuPic* pPic = GBL_OBJECT_NEW(EvmuPic,
                                   "parent", pSelf,
                                   "name",   "pic");

    EvmuRom* pRom = GBL_OBJECT_NEW(EvmuRom,
                                   "parent", pSelf,
                                   "name",   "rom");

    EvmuFlash* pFlash = GBL_OBJECT_NEW(EvmuFlash,
                                       "parent", pSelf,
                                       "name",   "flash");

    EvmuWram* pWram = GBL_OBJECT_NEW(EvmuWram,
                                     "parent", pSelf,
                                     "name",   "wram");

    EvmuLcd* pLcd = GBL_OBJECT_NEW(EvmuLcd,
                                   "parent", pSelf,
                                   "name",   "lcd");

    EvmuBuzzer* pBuzzer = GBL_OBJECT_NEW(EvmuBuzzer,
                                         "parent", pSelf,
                                         "name",   "buzzer");

    EvmuBattery* pBattery = GBL_OBJECT_NEW(EvmuBattery,
                                           "parent", pSelf,
                                           "name",   "battery");
#endif
//    pSelf_->pCpu     = EVMU_CPU_(pCpu);
    pSelf_->pMemory  = EVMU_MEMORY_(pMem);
    pSelf_->pClock   = EVMU_CLOCK_(pClock);
//    pSelf_->pPic     = EVMU_PIC_(pPic);
//    pSelf_->pRom     = EVMU_ROM_(pRom);
//    pSelf_->pFlash   = EVMU_FLASH_(pFlash);
//    pSelf_->pWram    = EVMU_WRAM_(pWram);
//    pSelf_->pLcd     = EVMU_LCD_(pLcd);
//    pSelf_->pBuzzer  = EVMU_BUZZER_(pBuzzer);
//    pSelf_->pBattery = EVMU_BATTERY_(pBattery);

    // Initialize dependencies
    pSelf_->pMemory->pCpu   = pSelf_->pCpu;
//    pSelf_->pCpu->pMemory   = pSelf_->pMemory;
    pSelf_->pClock->pMemory = pSelf_->pMemory;

    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(GblObject, base.pFnDestructor, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuDevice_update_(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);
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

GBL_EXPORT EvmuMemory* EvmuDevice_memory(const EvmuDevice* pSelf) {
    return (EvmuMemory*)GBL_INSTANCE_PUBLIC(EVMU_DEVICE_(pSelf)->pMemory, EVMU_MEMORY_TYPE);
}

GBL_EXPORT EvmuCpu* EvmuDevice_cpu(const EvmuDevice* pSelf) {
    //return (EvmuCpu*)GBL_INSTANCE_PUBLIC(EVMU_DEVICE_(pSelf)->pCpu, EVMU_CPU_TYPE);
}
GBL_EXPORT EvmuClock* EvmuDevice_clock(const EvmuDevice* pSelf) {
    return (EvmuClock*)GBL_INSTANCE_PUBLIC(EVMU_DEVICE_(pSelf)->pClock, EVMU_CLOCK_TYPE);
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

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        ifaceEntries[0].interfaceType = EVMU_IBEHAVIOR_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuDevice"),
                                      GBL_OBJECT_TYPE,
                                      &(const GblTypeInfo) {
                                          .pFnClassInit         = EvmuDeviceClass_init_,
                                          .classSize            = sizeof(EvmuDeviceClass),
                                          .instanceSize         = sizeof(EvmuDevice),
                                          .instancePrivateSize  = sizeof(EvmuDevice_),
                                          .interfaceCount       = 1,
                                          .pInterfaceMap        = ifaceEntries
                                      },
                                      GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}

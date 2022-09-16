#include <evmu/hw/evmu_device.h>
#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_cpu.h>
#include <evmu/hw/evmu_clock.h>
#include <gimbal/meta/instances/gimbal_module.h>

#include "evmu_device_.h"
#include "evmu_clock_.h"


static GBL_RESULT EvmuDevice_constructor_(GblObject* pSelf) {
    GBL_API_BEGIN(NULL);

    EvmuDevice_* pSelf_ = EVMU_DEVICE_(pSelf);

    GBL_INSTANCE_VCALL(GblObject, pFnConstructor, pSelf);

    // Create components
    EvmuMemory* pMem = EVMU_MEMORY(GblObject_create(EVMU_MEMORY_TYPE,
                                                    "parent", pSelf,
                                                    "name",   "memory",
                                                    NULL));
    pSelf_->pMemory = EVMU_MEMORY_(pMem);


    EvmuCpu* pCpu = EVMU_CPU(GblObject_create(EVMU_CPU_TYPE,
                                              "parent", pSelf,
                                              "name",   "cpu",
                                              NULL));
    pSelf_->pCpu = EVMU_CPU_(pCpu);

    EvmuClock* pClock = EVMU_CLOCK(GblObject_create(EVMU_CLOCK_TYPE,
                                                    "parent", pSelf,
                                                    "name",   "clock",
                                                    NULL));
    pSelf_->pClock = EVMU_CLOCK_(pClock);


    // Initialize dependencies
    pSelf_->pMemory->pCpu   = pSelf_->pCpu;
    pSelf_->pCpu->pMemory   = pSelf_->pMemory;
    pSelf_->pClock->pMemory = pSelf_->pMemory;

    GBL_API_END();
}

static GBL_RESULT EvmuDevice_destructor_(GblBox* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(GblObject, base.pFnDestructor, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuDevice_reset_(EvmuBehavior* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuBehavior, pFnReset, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuDevice_update_(EvmuBehavior* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuBehavior, pFnUpdate, pSelf, ticks);
    GBL_API_END();
}

static GBL_RESULT EvmuDeviceClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);

    EVMU_BEHAVIOR_CLASS(pClass)->pFnReset    = EvmuDevice_reset_;
    EVMU_BEHAVIOR_CLASS(pClass)->pFnUpdate   = EvmuDevice_update_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor = EvmuDevice_constructor_;
    GBL_BOX_CLASS(pClass)->pFnDestructor     = EvmuDevice_destructor_;

    GBL_API_END();
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
    return (EvmuCpu*)GBL_INSTANCE_PUBLIC(EVMU_DEVICE_(pSelf)->pCpu, EVMU_CPU_TYPE);
}
GBL_EXPORT EvmuClock* EvmuDevice_clock(const EvmuDevice* pSelf) {
    return (EvmuClock*)GBL_INSTANCE_PUBLIC(EVMU_DEVICE_(pSelf)->pClock, EVMU_CLOCK_TYPE);
}


GBL_EXPORT GblType EvmuDevice_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInterfaceMapEntry ifaceEntries[] = {
        {
            .classOffset   = offsetof(GblObjectClass, GblITableImpl)
        }
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        ifaceEntries[0].interfaceType = EVMU_BEHAVIOR_TYPE;

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
        GBL_API_VERIFY_LAST_RECORD();
        GBL_API_END_BLOCK();
    }

    return type;
}

#include <evmu/hw/evmu_device.h>
#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_cpu.h>
#include <evmu/hw/evmu_clock.h>

#include "evmu_device_.h"
#include "evmu_clock_.h"

#if 0
GBL_DECLARE_ENUM(EVMU_DEVICE_EMULATION_MODE) {
    EVMU_DEVICE_SOFTWARE_MODE_DISABLED,
    EVMU_DEVICE_SOFTWARE_MODE_EMULATION,
    EVMU_DEVICE_SOFTWARE_MODE_LCD_PLAYBACK
};

GBL_DECLARE_ENUM(EVMU_DEVICE_PROPERTY) {
//    EVMU_DEVICE_PROPERTY_BIOS_MODE? GAME, FILE, CLOCK, etc?
    EVMU_DEVICE_PROPERTY_PIN_5V,
    EVMU_DEVICE_PROPERTY_PIN_SIO0_OUTPUT_ENABLE,
    EVMU_DEVICE_PROPERTY_PIN_SIO0_INPUT,
    EVMU_DEVICE_PROPERTY_PIN_SIO0_OUTPUT,
    EVMU_DEVICE_PROPERTY_PIN_SIO1_OUTPUT_ENABLE,
    EVMU_DEVICE_PROPERTY_PIN_SIO1_INPUT,
    EVMU_DEVICE_PROPERTY_PIN_SIO1_OUTPUT,
    EVMU_DEVICE_PROPERTY_PIN_ID0,
    EVMU_DEVICE_PROPERTY_PIN_ID1,
    EVMU_DEVICE_PROPERTY_COUNT
};
#endif


static GBL_RESULT EvmuDevice_constructor_(EvmuDevice* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                             base.pFnConstructor, (void*)pSelf);
    pSelf->pPrivate = GBL_API_MALLOC(sizeof(EvmuDevice_));
    memset(pSelf->pPrivate, 0, sizeof(EvmuDevice_));

    DEV_(pSelf)->pPublic = pSelf;

    EvmuMemory* pMem = (EvmuMemory*)GblObject_new(EVMU_MEMORY_TYPE,
                                                  "parent", pSelf,
                                                  "name", "memory",
                                                  NULL);

    EvmuCpu* pCpu = (EvmuCpu*)GblObject_new(EVMU_CPU_TYPE,
                                            "parent", pSelf,
                                            "name", "cpu",
                                            NULL);

    // initialize dependencies
    DEV_MEM_(pSelf).pCpu    = pCpu->pPrivate;
    DEV_CPU_(pSelf).pMemory = pMem->pPrivate;

    //DEV_CPU_(pSelf) = pCpu->pPrivate;

    EvmuClock* pClock = (EvmuClock*)GblObject_new(EVMU_CLOCK_TYPE,
                                                  "parent", pSelf,
                                                  "name", "clock",
                                                  NULL);
    DEV_CLOCK_(pSelf).pMemory = pMem->pPrivate;

    //DEV_CLOCK_(pSelf) = pClock->pPrivate;

    GBL_API_END();
}

static GBL_RESULT EvmuDevice_destructor_(EvmuDevice* pSelf) {
    GBL_API_BEGIN(NULL);

    GBL_API_FREE(DEV_(pSelf));


    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                              base.pFnDestructor, (void*)pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuDevice_reset_(EvmuDevice* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                              pFnReset, (void*)pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuDevice_update_(EvmuDevice* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                              pFnUpdate, (void*)pSelf, ticks);
    GBL_API_END();
}

static GBL_RESULT EvmuDeviceClass_init_(EvmuDeviceClass* pClass, void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);
    pClass->base.pFnReset               = (void*)EvmuDevice_reset_;
    pClass->base.pFnUpdate              = (void*)EvmuDevice_update_;
    pClass->base.base.pFnConstructor    = (void*)EvmuDevice_constructor_;
    pClass->base.base.pFnDestructor     = (void*)EvmuDevice_destructor_;
    GBL_API_END();
}


GBL_EXPORT GblType EvmuDevice_type(void) {
    static GblType type = GBL_TYPE_INVALID;
    if(type == GBL_TYPE_INVALID) {
        type = gblTypeRegisterStatic(EVMU_ENTITY_TYPE,
                                     "EvmuDevice",
                                     &((const GblTypeInfo) {
                                         .pFnClassInit  = (GblTypeClassInitFn)EvmuDeviceClass_init_,
                                         .classSize     = sizeof(EvmuDeviceClass),
                                         .classAlign    = GBL_ALIGNOF(EvmuDeviceClass),
                                         .instanceSize  = sizeof(EvmuDevice),
                                         .instanceAlign = GBL_ALIGNOF(EvmuDevice)
                                     }),
                                     GBL_TYPE_FLAGS_NONE);

    }
    return type;
}

GBL_EXPORT GblSize EvmuDevice_peripheralCount(const EvmuDevice* pSelf) {
    return EvmuEntity_childCountByType(EVMU_ENTITY(pSelf), EVMU_PERIPHERAL_TYPE);
}
GBL_EXPORT EvmuPeripheral* EvmuDevice_peripheralFindByIndex(const EvmuDevice* pSelf, GblSize index) {
    return EVMU_PERIPHERAL(EvmuEntity_childFindByTypeIndex(EVMU_ENTITY(pSelf), EVMU_PERIPHERAL_TYPE, index));
}

GBL_EXPORT EvmuPeripheral* EvmuDevice_peripheralFindByName(const EvmuDevice* pSelf, const char* pName) {
    return EVMU_PERIPHERAL(EvmuEntity_childFindByTypeName(EVMU_ENTITY(pSelf), EVMU_PERIPHERAL_TYPE, pName));
}

GBL_EXPORT EvmuMemory* EvmuDevice_memory(const EvmuDevice* pSelf) {
    return DEV_MEM_(pSelf).pPublic;
}
GBL_EXPORT EvmuCpu* EvmuDevice_cpu(const EvmuDevice* pSelf) {
    return DEV_CPU_(pSelf).pPublic;
}
GBL_EXPORT EvmuClock* EvmuDevice_clock(const EvmuDevice* pSelf) {
    return DEV_CLOCK_(pSelf).pPublic;
}
GBL_EXPORT EvmuPic* EvmuDevice_pic(const EvmuDevice* pSelf) {
    //return GblObject_childFindByType(GBL_OBJECT(pSelf), EVMU_PIC_TYPE);
}
GBL_EXPORT EvmuFlash* EvmuDevice_flash(const EvmuDevice* pSelf) {
    //return GblObject_childFindByType(GBL_OBJECT(pSelf), EVMU_FLASH_TYPE);
}
GBL_EXPORT EvmuLcd* EvmuDevice_lcd(const EvmuDevice* pSelf) {
    //return GblObject_childFindByType(GBL_OBJECT(pSelf), EVMU_LCD_TYPE);
}


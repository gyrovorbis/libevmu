#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_memory_.h"
#include "evmu_device_.h"

static GBL_RESULT EvmuMemory_constructor_(EvmuMemory* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_PERIPHERAL_TYPE, EvmuPeripheralClass,
                             base.base.pFnConstructor, (void*)pSelf);
////GBL_API_MALLOC(sizeof(EvmuMemory_));
    pSelf->pPrivate = &DEV_MEM_(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf)));
    //memset(pSelf->pPrivate, 0, sizeof(EvmuMemory_));

    pSelf->pPrivate->pPublic = pSelf;

    GBL_API_END();
}

static GBL_RESULT EvmuMemory_destructor_(EvmuMemory* pSelf) {
    GBL_API_BEGIN(NULL);

    //GBL_API_FREE(pSelf->pPrivate);


    GBL_INSTANCE_VCALL_SUPER(EVMU_PERIPHERAL_TYPE, EvmuPeripheralClass,
                             base.base.pFnDestructor, (void*)pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuMemory_reset_(EvmuMemory* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                             pFnReset, (void*)pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuMemoryClass_init_(EvmuMemoryClass* pClass, void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);
    pClass->base.base.pFnReset               = (void*)EvmuMemory_reset_;
    pClass->base.base.base.pFnConstructor    = (void*)EvmuMemory_constructor_;
    pClass->base.base.base.pFnDestructor     = (void*)EvmuMemory_destructor_;
    GBL_API_END();
}


GBL_EXPORT GblType EvmuMemory_type(void) {
    static GblType type = GBL_INVALID_TYPE;
    if(type == GBL_INVALID_TYPE) {
        type = GblType_registerStatic(EVMU_PERIPHERAL_TYPE,
                                     "EvmuMemory",
                                     &((const GblTypeInfo) {
                                         .pFnClassInit          = (GblTypeClassInitializeFn)EvmuMemoryClass_init_,
                                         .classSize             = sizeof(EvmuMemoryClass),
                                         .instanceSize          = sizeof(EvmuMemory)
                                     }),
                                     GBL_TYPE_FLAGS_NONE);

    }
    return type;
}


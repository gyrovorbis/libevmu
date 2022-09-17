#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_memory_.h"
#include "evmu_device_.h"

static GBL_RESULT EvmuMemory_constructor_(GblObject* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructor, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuMemory_destructor_(GblBox* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuMemory_reset_(EvmuIBehavior* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);
    GBL_API_END();
}

static GBL_RESULT EvmuMemoryClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset    = EvmuMemory_reset_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor = EvmuMemory_constructor_;
    GBL_BOX_CLASS(pClass)->pFnDestructor     = EvmuMemory_destructor_;

    GBL_API_END();
}


GBL_EXPORT GblType EvmuMemory_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuMemory"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &((const GblTypeInfo) {
                                          .pFnClassInit          = EvmuMemoryClass_init_,
                                          .classSize             = sizeof(EvmuMemoryClass),
                                          .instanceSize          = sizeof(EvmuMemory),
                                          .instancePrivateSize   = sizeof(EvmuMemory_)
                                      }),
                                      GBL_TYPE_FLAGS_NONE);
        GBL_API_VERIFY_LAST_RECORD();
        GBL_API_END_BLOCK();
    }

    return type;
}


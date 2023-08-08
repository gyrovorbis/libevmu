#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_device.h>
#include "evmu_peripheral_.h"
#include "../hw/evmu_device_.h"

GBL_EXPORT EvmuDevice* EvmuPeripheral_device(const EvmuPeripheral* pSelf) {
    GblObject* pParent = GblObject_parent(GBL_OBJECT(pSelf));
    return pParent && GBL_TYPECHECK(EvmuDevice, pParent)?
                EVMU_DEVICE(pParent) : NULL;

}

static GBL_RESULT EvmuPeripheral_constructed_(GblObject* pObject) {
    GBL_CTX_BEGIN(NULL);
    EvmuPeripheral* pSelf = EVMU_PERIPHERAL(pObject);
    pSelf->logLevel = ~0;
    EvmuPeripheral_* pSelf_ = EVMU_PERIPHERAL_(pSelf);
    pSelf_->pDevice_ = EVMU_DEVICE_(EvmuPeripheral_device(pSelf));
    GBL_CTX_END();
}

static GBL_RESULT EvmuPeripheralClass_init(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);
    GBL_OBJECT_CLASS(pClass)->pFnConstructed = EvmuPeripheral_constructed_;
    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuPeripheral_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblInterfaceImpl ifaceEntries[] = {
        {
            .classOffset   = offsetof(EvmuPeripheralClass, EvmuIBehaviorImpl)
        }
    };

    const static GblTypeInfo info = {
        .classSize            = sizeof(EvmuPeripheralClass),
        .pFnClassInit         = EvmuPeripheralClass_init,
        .instanceSize         = sizeof(EvmuPeripheral),
        .instancePrivateSize  = sizeof(EvmuPeripheral_),
        .interfaceCount       = 1,
        .pInterfaceImpls        = ifaceEntries
    };

    if(type == GBL_INVALID_TYPE) {
        ifaceEntries[0].interfaceType = EVMU_IBEHAVIOR_TYPE;

        type = GblType_register(GblQuark_internStatic("EvmuPeripheral"),
                                GBL_OBJECT_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }
    return type;
}

#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_device.h>

GBL_EXPORT EvmuDevice* EvmuPeripheral_device(const EvmuPeripheral* pSelf) {
    GblObject* pParent = GblObject_parent(GBL_OBJECT(pSelf));
    return pParent && GBL_INSTANCE_CHECK(pParent, EvmuDevice)?
                EVMU_DEVICE(pParent) : NULL;

}

GBL_EXPORT GblEnum EvmuPeripheral_logLevel(const EvmuPeripheral* pSelf) GBL_NOEXCEPT {
    return pSelf->logLevel;
}

GBL_EXPORT void EvmuPeripheral_logLevelSet(EvmuPeripheral* pSelf, GblEnum level) GBL_NOEXCEPT {
    pSelf->logLevel = level;
}

GBL_EXPORT GblType EvmuPeripheral_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInterfaceMapEntry ifaceEntries[] = {
        {
            .classOffset   = offsetof(EvmuPeripheralClass, EvmuIBehaviorImpl)
        }
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        ifaceEntries[0].interfaceType = EVMU_IBEHAVIOR_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuPeripheral"),
                                      GBL_OBJECT_TYPE,
                                      &(const GblTypeInfo) {
                                          .classSize      = sizeof(EvmuPeripheralClass),
                                          .instanceSize   = sizeof(EvmuPeripheral),
                                          .interfaceCount = 1,
                                          .pInterfaceMap  = ifaceEntries
                                      },
                                      GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }
    return type;
}

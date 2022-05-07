#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_device.h>


GBL_EXPORT GblType EvmuPeripheral_type(void) {
    static GblType type = GBL_INVALID_TYPE;
    if(type == GBL_INVALID_TYPE) {
        type = GblType_registerStatic(EVMU_ENTITY_TYPE,
                                     "EvmuPeripheral",
                                     &((const GblTypeInfo) {
                                         .classSize     = sizeof(EvmuPeripheralClass),
                                         .instanceSize  = sizeof(EvmuPeripheral),
                                     }),
                                     GBL_TYPE_FLAGS_NONE);
    }
    return type;
}

GBL_EXPORT EvmuDevice* EvmuPeripheral_device(const EvmuPeripheral* pSelf) {
    GblObject* pParent = GblObject_parent(GBL_OBJECT(pSelf));
    return pParent && EVMU_DEVICE_CHECK(pParent)?
                EVMU_DEVICE(pParent) : NULL;

}

GBL_EXPORT GblEnum EvmuPeripheral_logLevel(const EvmuPeripheral* pSelf) GBL_NOEXCEPT {
    return pSelf->logLevel;
}

GBL_EXPORT void EvmuPeripheral_logLevelSet(EvmuPeripheral* pSelf, GblEnum level) GBL_NOEXCEPT {
    pSelf->logLevel = level;
}

#include <evmu/types/evmu_peripheral.h>
#include <evmu/hw/evmu_device.h>


GBL_EXPORT GblType EvmuPeripheral_type(void) {
    static GblType type = GBL_TYPE_INVALID;
    if(type == GBL_TYPE_INVALID) {
        type = gblTypeRegisterStatic(EVMU_ENTITY_TYPE,
                                     "EvmuPeripheral",
                                     &((const GblTypeInfo) {
                                         .classSize     = sizeof(EvmuPeripheralClass),
                                         .classAlign    = GBL_ALIGNOF(EvmuPeripheralClass),
                                         .instanceSize  = sizeof(EvmuPeripheral),
                                         .instanceAlign = GBL_ALIGNOF(EvmuPeripheral)
                                     }),
                                     GBL_TYPE_FLAGS_NONE);
    }
    return type;
}

GBL_EXPORT EvmuDevice* EvmuPeripheral_device(const EvmuPeripheral* pSelf) {
    GblObject* pParent = GblObject_parentGet(GBL_OBJECT(pSelf));
    return pParent && EVMU_DEVICE_CHECK(pParent)?
                EVMU_DEVICE(pParent) : NULL;

}

GBL_EXPORT GblEnum EvmuPeripheral_logLevel(const EvmuPeripheral* pSelf) GBL_NOEXCEPT {
    return pSelf->logLevel;
}

GBL_EXPORT void EvmuPeripheral_logLevelSet(EvmuPeripheral* pSelf, GblEnum level) GBL_NOEXCEPT {
    pSelf->logLevel = level;
}

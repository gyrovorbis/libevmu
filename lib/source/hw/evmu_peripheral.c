#include "evmu_peripheral_.h"
#include <evmu/evmu_types.h>



EVMU_API evmuPeripheralDevice(EvmuPeripheral hPeripheral, EvmuDevice** ppDevice) {
    EVMU_API_BEGIN(hPeripheral);
    GBL_API_VERIFY_POINTER(ppDevice);

    *ppDevice = hPeripheral->pDevice;

    GBL_API_END();
}

EVMU_API evmuPeripheralDriver(EvmuPeripheral hPeripheral, EvmuPeripheralDriver** ppDriver) {
    EVMU_API_BEGIN(hPeripheral);
    GBL_API_VERIFY_POINTER(ppDriver);

    *ppDriver = hPeripheral->pDriver;

    GBL_API_END();
}
EVMU_API evmuPeripheralUserdata(EvmuPeripheral hPeripheral, void** ppUserdata) {
    EVMU_API_BEGIN(hPeripheral);
    GBL_API_VERIFY_POINTER(ppUserdata);

    *ppUserdata = hPeripheral->pUserdata;

    GBL_API_END();
}

EVMU_API evmuPeripheralReset(EvmuPeripheral hPeripheral) {
    EVMU_PERIPHERAL__DISPATCH(hPeripheral, Reset, hPeripheral);
}

EVMU_API evmuPeripheralUpdate(EvmuPeripheral hPeripheral, EvmuTicks ticks) {
    EVMU_PERIPHERAL__DISPATCH(hPeripheral, Update, hPeripheral, ticks);
}

EVMU_API evmuPeripheralPropertyValue(EvmuPeripheral hPeripheral, EvmuEnum propertyId, void* pData, EvmuSize* pSize) {
    EVMU_API_BEGIN(hPeripheral);
    GBL_API_VERIFY_POINTER(pSize);

    EvmuBool found = EVMU_FALSE;
    if(hPeripheral->pDriver->dispatchTable.pFnProperty) {
        const EVMU_RESULT result = hPeripheral->pDriver->dispatchTable.pFnProperty(hPeripheral, propertyId, pData, pSize);
        if(GBL_RESULT_SUCCESS(result)) found = EVMU_TRUE;
    }

#define CASE_BUILTIN_PROPERTY(NAME, PROPERTY)                   \
    case EVMU_PERIPHERAL_PROPERTY_##NAME: {                     \
        const EvmuSize size = sizeof(hPeripheral->PROPERTY);    \
        if(!pData) {                                            \
            *pSize = size;                                      \
        } else {                                                \
            GBL_API_VERIFY(*pSize >= size,                      \
                           "Buffer is too small!");             \
            memcpy(pData, hPeripheral->PROPERTY, size);         \
            *pSize = SIZE;                                      \
        }                                                       \
        break;                                                  \
    }

    if(!found) {
        switch(propertyId) {
        CASE_BUILTIN_PROPERTY(ID, id);
        CASE_BUILTIN_PROPERTY(MODE, mode);
        CASE_BUILTIN_PROPERTY(LOG_LEVEL, logLevel);
        default:
            GBL_API_RESULT_SET(EVMU_RESULT_UNKNOWN_PROPERTY, "Uknown Peripheral property: [%u]", propertyId);
        }
    }

#undef CASE_BUILTIN_PROPERTY
    GBL_API_END();
}

EVMU_API evmuPeripheralPropertyValueSet(EvmuPeripheral hPeripheral, EvmuEnum propertyId, const void* pData, EvmuSize* pSize) {
    EVMU_API_BEGIN(hPeripheral);
    GBL_API_VERIFY_POINTER(pData);
    GBL_API_VERIFY_POINTER(pSize);

    EvmuBool found = EVMU_FALSE;
    if(hPeripheral->pDriver->dispatchTable.pFnPropertySet) {
        const EVMU_RESULT result = hPeripheral->pDriver->dispatchTable.pFnPropertySet(hPeripheral, propertyId, pData, pSize);
        if(GBL_RESULT_SUCCESS(result)) found = EVMU_TRUE;
    }

#define CASE_BUILTIN_PROPERTY(NAME, PROPERTY)                   \
    case EVMU_PERIPHERAL_PROPERTY_##NAME: {                     \
        const EvmuSize size = sizeof(hPeripheral->PROPERTY);    \
            GBL_API_VERIFY(*pSize == size,                      \
                           "Incorrect Buffer Size");            \
            memcpy(hPeripheral->PROPERTY, pData, size);         \
        break;                                                  \
    }

    if(!found) {
        *pSize = 0;
        switch(propertyId) {
        CASE_BUILTIN_PROPERTY(ID, id);
        CASE_BUILTIN_PROPERTY(MODE, mode);
        CASE_BUILTIN_PROPERTY(LOG_LEVEL, logLevel);
        default:
            GBL_API_RESULT_SET(EVMU_RESULT_UNKNOWN_PROPERTY, "Uknown Peripheral property: [%u]", propertyId);
        }
    }

#undef CASE_BUILTIN_PROPERTY
    GBL_API_END();
}


#if 0
EVMU_API evmuPeripheralProperty(EvmuPeripheral hPeripheral, EvmuEnum propertyId, EvmuPeripheralProperty* pProperty, EvmuSize* pSize) {


}
#endif




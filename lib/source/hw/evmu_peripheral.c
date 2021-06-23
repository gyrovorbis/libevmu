#include "evmu_peripheral_.h"
#include <evmu/evmu_types.h>
#include <evmu/evmu_api.h>
#include <string.h>
EVMU_API evmuPeripheralDevice(EvmuPeripheral hPeripheral, EvmuDevice* ppDevice) {
    EVMU_API_BEGIN(hPeripheral);
    EVMU_API_VERIFY_POINTER(ppDevice);

    *ppDevice = hPeripheral->pDevice;

    EVMU_API_END();
}

EVMU_API evmuPeripheralDriver(EvmuPeripheral hPeripheral, const EvmuPeripheralDriver** ppDriver) {
    EVMU_API_BEGIN(hPeripheral);
    EVMU_API_VERIFY_POINTER(ppDriver);

    *ppDriver = hPeripheral->pDriver;

    EVMU_API_END();
}
EVMU_API evmuPeripheralUserdata(EvmuPeripheral hPeripheral, void** ppUserdata) {
    EVMU_API_BEGIN(hPeripheral);
    EVMU_API_VERIFY_POINTER(ppUserdata);

    *ppUserdata = hPeripheral->pUserdata;

    EVMU_API_END();
}

EVMU_API evmuPeripheralReset(EvmuPeripheral hPeripheral) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, Reset, hPeripheral);
}

EVMU_API evmuPeripheralUpdate(EvmuPeripheral hPeripheral, EvmuTicks ticks) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, Update, hPeripheral, ticks);
}

EVMU_API evmuPeripheralPropertyValue(EvmuPeripheral hPeripheral, EvmuEnum propertyId, void* pData, EvmuSize* pSize) {
    EVMU_API_BEGIN(hPeripheral);
    EVMU_API_VERIFY_POINTER(pSize);

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
            EVMU_API_VERIFY(*pSize >= size &                    \
                           "Buffer is too small!");             \
            memcpy(pData, &hPeripheral->PROPERTY, size);        \
            *pSize = size;                                      \
        }                                                       \
        break;                                                  \
    }

    if(!found) {
        switch(propertyId) {
        CASE_BUILTIN_PROPERTY(ID, id);
        CASE_BUILTIN_PROPERTY(MODE, mode);
        CASE_BUILTIN_PROPERTY(LOG_LEVEL, logLevel);
        default:
            EVMU_API_RESULT_SET(EVMU_RESULT_ERROR_PROPERTY_UNKNOWN, "Unknown Peripheral property: [%u]", propertyId);
        }
    }

#undef CASE_BUILTIN_PROPERTY
    EVMU_API_END();
}

EVMU_API evmuPeripheralPropertyValueSet(EvmuPeripheral hPeripheral, EvmuEnum propertyId, const void* pData, EvmuSize* pSize) {
    EVMU_API_BEGIN(hPeripheral);
    EVMU_API_VERIFY_POINTER(pData);
    EVMU_API_VERIFY_POINTER(pSize);

    EvmuBool found = EVMU_FALSE;
    if(hPeripheral->pDriver->dispatchTable.pFnPropertySet) {
        const EVMU_RESULT result = hPeripheral->pDriver->dispatchTable.pFnPropertySet(hPeripheral, propertyId, pData, pSize);
        if(GBL_RESULT_SUCCESS(result)) found = EVMU_TRUE;
    }

#define CASE_BUILTIN_PROPERTY(NAME, PROPERTY)                   \
    case EVMU_PERIPHERAL_PROPERTY_##NAME: {                     \
        const EvmuSize size = sizeof(hPeripheral->PROPERTY);    \
            EVMU_API_VERIFY(*pSize == size &&                   \
                           "Incorrect Buffer Size");            \
            memcpy(&hPeripheral->PROPERTY, pData, size);        \
        break;                                                  \
    }

    if(!found) {
        *pSize = 0;
        switch(propertyId) {
        CASE_BUILTIN_PROPERTY(ID, id);
        CASE_BUILTIN_PROPERTY(MODE, mode);
        CASE_BUILTIN_PROPERTY(LOG_LEVEL, logLevel);
        default:
            EVMU_API_RESULT_SET(EVMU_RESULT_ERROR_PROPERTY_UNKNOWN, "Uknown Peripheral property: [%u]", propertyId);
        }
    }

#undef CASE_BUILTIN_PROPERTY
    EVMU_API_END();
}


#if 0
EVMU_API evmuPeripheralProperty(EvmuPeripheral hPeripheral, EvmuEnum propertyId, EvmuPeripheralProperty* pProperty, EvmuSize* pSize) {


}
#endif




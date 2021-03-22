#ifndef EVMU_DEVICE_H
#define EVMU_DEVICE_H

#include "../evmu_types.h"
//#include "../util/evmu_context.h"
//#include "../evmu_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct EvmuPeripheralDriver;

GBL_DECLARE_ENUM(EVMU_DEVICE_EMULATION_MODE) {
    EVMU_DEVICE_SOFTWARE_MODE_DISABLED,
    EVMU_DEVICE_SOFTWARE_MODE_EMULATION,
    EVMU_DEVICE_SOFTWARE_MODE_LCD_PLAYBACK
};
/*
typedef struct EvmuConnector {


} EvmuConnector;

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
};*/

GBL_DECLARE_ENUM(EVMU_DEVICE_PROPERTY) {
    EVMU_DEVICE_PROPERTY_SOFTWARE_MODE
};

GBL_DECLARE_HANDLE(EvmuContext);

GBL_DECLARE_HANDLE(EvmuDevice);


typedef struct EvmuDeviceCreateInfo {
    void*                               pUserdata;
    EvmuContext                         hContext;
    EvmuEventHandler                    eventHandler;
} EvmuDeviceCreateInfo;


// IMPLEMENTED AT LEAST PARTIALLY
EVMU_API    evmuDeviceCreate(EvmuDevice* phDevice, const EvmuDeviceCreateInfo* pInfo);
EVMU_API    evmuDeviceDestroy(EvmuDevice hDevice);

EVMU_API    evmuDeviceContext(EvmuDevice hDevice, EvmuContext *phContext);
EVMU_API    evmuDeviceUserdata(EvmuDevice hDevice, void** pUserData);
EVMU_API    evmuDeviceEventHandler(EvmuDevice hDevice, EvmuEventHandler* pHandler);
EVMU_API    evmuDeviceEventHandlerSet(EvmuContext hDevice, const EvmuEventHandler* pHandler);

EVMU_API    evmuDevicePeripheralCount(EvmuDevice hDevice, uint32_t* pCount);
EVMU_API    evmuDevicePeripheral(EvmuDevice hDevice, EvmuEnum index, EvmuPeripheral* phPeripheral);
EVMU_API    evmuDevicePeripheralFind(EvmuDevice hDevice, const char* pName, EvmuPeripheral* phPeripheral);
EVMU_API    evmuDevicePeripheralAdd(EvmuDevice hDevice, const struct EvmuPeripheralDriver* pDriver, EvmuPeripheral* phPeripheral);
EVMU_API    evmuDevicePeripheralRemove(EvmuDevice hDevice, EvmuPeripheral hPeripheral);


EVMU_API    evmuDeviceReset(EvmuDevice hDevice);
EVMU_API    evmuDeviceStateSave(EvmuDevice hDevice, void* pData, EvmuSize* pSize);
EVMU_API    evmuDeviceStateLoad(EvmuDevice hDevice, const void* pData, EvmuSize size);





// IMPLEMENT ME NEXT!


//EVMU_API    evmuDeviceProperty(EvmuDevice hDevice, EvmuEnum propertyId, EvmuSize* pSize);
EVMU_API    evmuDevicePropertyValue(EvmuDevice hDevice, EvmuEnum propertyId, void* pData, EvmuSize* pSize);
EVMU_API    evmuDevicePropertyValueSet(EvmuDevice hDevice, EvmuEnum propertyId, void* pData, EvmuSize size);




// We'll see
EVMU_API    evmuDeviceUpdate(EvmuDevice hDevice, EvmuTicks ticks);
EVMU_API    evmuDeviceSleep(EvmuDevice hDevice);
EVMU_API    evmuDeviceResume(EvmuDevice hDevice);









#ifdef __cplusplus
}
#endif

#endif // EVMU_DEVICE_H


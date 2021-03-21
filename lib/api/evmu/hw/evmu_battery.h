#ifndef EVMU_BATTERY_H
#define EVMU_BATTERY_H

#include <gimbal/gimbal_api.h>
#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

GBL_DEFINE_HANDLE(EvmuBattery)

GBL_DECLARE_ENUM(EVMU_BATTERY_PROPERTY) {
    EVMU_BATTERY_PROPERTY_LOW_ALARM = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_BATTERY_PROPERTY_MONITOR_ENABLED,
    EVMU_BATTERY_PROPERTY_COUNT
};





#if 0
EVMU_API evmuBatteryProperty(EvmuPeripheral* pPeripheral, GblEnum propertyId, EvmuPeripheralProperty* pProperty, GblSize* pSize);
EVMU_API evmuPeripheralPropertyValue(EvmuPeripheral* pPeripheral, GblEnum propertyId, void* pData, GblSize* pSize);
EVMU_API evmuPeripheralPropertyValueSet(EvmuPeripheral* pPeripheral, GblEnum propertyId, void* pData, GblSize size);

EVMU_API    evmuBattery

EVMU_API    evmuBatteryLow(const EvmuDevice* pDev, GblBool* pLow);
EVMU_API    evmuBatteryLowSet(EvmuDevice* pDev, GblBool low);
EVMU_API    evmuBatteryMonitorEnabled(const EvmuDevice* pDev, GblBool* pEnabled);
EVMU_API    evmuBatteryMonitorEnabledSet(EvmuDevice* pDev, GblBool enabled);
#endif



// BATTERY PROFILING TOOLS


#ifdef __cplusplus
}
#endif


#endif // EVMU_BATTERY_H


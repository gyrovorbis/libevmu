#ifndef EVMU_BATTERY__H
#define EVMU_BATTERY__H

#include <evmu/hw/evmu_battery.h>

#define EVMU_BATTERY_(instance)     ((EvmuBattery_*)GBL_INSTANCE_PRIVATE(instance, EVMU_BATTERY_TYPE))
#define EVMU_BATTERY_PUBLIC(priv)   ((EvmuBattery*)GBL_INSTANCE_PUBLIC(priv, EVMU_BATTERY_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_STRUCT(EvmuBattery_) {
    EvmuMemory_* pMemory;
};

GBL_DECLS_END

#endif // EVMU_BATTERY__H

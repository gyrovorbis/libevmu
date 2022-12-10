#ifndef EVMU_BATTERY_H
#define EVMU_BATTERY_H

#include "../types/evmu_peripheral.h"

#define EVMU_BATTERY_TYPE           (GBL_TYPEOF(EvmuBattery))
#define EVMU_BATTERY_NAME           "battery"

#define EVMU_BATTERY(instance)      (GBL_INSTANCE_CAST(instance, EvmuBattery))
#define EVMU_BATTERY_CLASS(klass)   (GBL_CLASS_CAST(klass, EvmuBattery))
#define EVMU_BATTERY_GET(instance)  (GBL_INSTANCE_GET_CLASS(instance, EvmuBattery))

#define GBL_SELF_TYPE EvmuBattery

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuBattery, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuBattery, EvmuPeripheral)

GBL_PROPERTIES(EvmuBattery,
    (lowAlarm,       GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (monitorEnabled, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE)
)

EVMU_EXPORT GblType EvmuBattery_type              (void)                      GBL_NOEXCEPT;

EVMU_EXPORT GblBool EvmuBattery_lowAlarm          (GBL_CSELF)                 GBL_NOEXCEPT;
EVMU_EXPORT void    EvmuBattery_setLowAlarm       (GBL_SELF, GblBool enabled) GBL_NOEXCEPT;

EVMU_EXPORT GblBool EvmuBattery_monitorEnabled    (GBL_CSELF)                 GBL_NOEXCEPT;
EVMU_EXPORT void    EvmuBattery_setMonitorEnabled (GBL_SELF, GblBool enabled) GBL_NOEXCEPT;

// Profiling API

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BATTERY_H


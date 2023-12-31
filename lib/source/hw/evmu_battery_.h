#ifndef EVMU_BATTERY__H
#define EVMU_BATTERY__H

#include <evmu/hw/evmu_battery.h>

#define EVMU_BATTERY_(instance)     (GBL_PRIVATE(EvmuBattery, instance))
#define EVMU_BATTERY_PUBLIC(priv)   (GBL_PUBLIC(EvmuBattery, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuBattery_) {
    EvmuRam_* pRam;
};

GBL_DECLS_END

#endif // EVMU_BATTERY__H

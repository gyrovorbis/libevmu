#ifndef EVMU_PERIPHERAL__H
#define EVMU_PERIPHERAL__H

#include <evmu/types/evmu_peripheral.h>

#define EVMU_PERIPHERAL_(instance)  ((EvmuPeripheral_*)GBL_INSTANCE_PRIVATE(instance, EVMU_PERIPHERAL_TYPE))

GBL_FORWARD_DECLARE_STRUCT(EvmuDevice_);

GBL_DECLARE_STRUCT(EvmuPeripheral_) {
    EvmuDevice_* pDevice_;
};

#endif // EVMU_PERIPHERAL__H

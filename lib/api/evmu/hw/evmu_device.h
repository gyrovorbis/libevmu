#ifndef EVMU_DEVICE_H
#define EVMU_DEVICE_H

#include "../types/evmu_behavior.h"

#define EVMU_DEVICE_TYPE                (GBL_TYPEOF(EvmuDevice))
#define EVMU_DEVICE(instance)           (GBL_INSTANCE_CAST(instance, EvmuDevice))
#define EVMU_DEVICE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuDevice))
#define EVMU_DEVICE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuDevice))

#define GBL_SELF_TYPE EvmuDevice

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuDevice, GblObject, EvmuBehavior)
GBL_INSTANCE_DERIVE_EMPTY(EvmuDevice, GblObject)

EVMU_EXPORT GblType         EvmuDevice_type             (void)                         GBL_NOEXCEPT;

EVMU_EXPORT GblSize         EvmuDevice_peripheralCount  (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheralByName (GBL_CSELF, const char* pName) GBL_NOEXCEPT;
EVMU_EXPORT EvmuPeripheral* EvmuDevice_peripheralAt     (GBL_CSELF, GblSize index)     GBL_NOEXCEPT;

EVMU_EXPORT EvmuMemory*     EvmuDevice_memory           (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuCpu*        EvmuDevice_cpu              (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuClock*      EvmuDevice_clock            (GBL_CSELF)                    GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_DEVICE_H

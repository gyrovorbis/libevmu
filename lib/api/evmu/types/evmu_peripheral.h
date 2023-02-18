#ifndef EVMU_PERIPHERAL_H
#define EVMU_PERIPHERAL_H

#include "evmu_ibehavior.h"

#define EVMU_PERIPHERAL_TYPE                (GBL_TYPEOF(EvmuPeripheral))
#define EVMU_PERIPHERAL(instance)           (GBL_INSTANCE_CAST(instance, EvmuPeripheral))
#define EVMU_PERIPHERAL_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuPeripheral))
#define EVMU_PERIPHERAL_GET_CLASS(instance) (GBL_TYPE_INSTANCE_GET_CLASS(instance, EvmuPeripheral))

#define GBL_SELF_TYPE EvmuPeripheral

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemoryEvent);
GBL_FORWARD_DECLARE_STRUCT(EvmuClockEvent);
GBL_FORWARD_DECLARE_STRUCT(EvmuDevice);
GBL_FORWARD_DECLARE_STRUCT(EvmuPeripheral);

GBL_DECLARE_ENUM(EVMU_PERIPHERAL_LOG_LEVEL) {
    EVMU_PERIPHERAL_LOG_LEVEL_ERROR,
    EVMU_PERIPHERAL_LOG_LEVEL_WARNING,
    EVMU_PERIPHERAL_LOG_LEVEL_VERBOSE,
    EVMU_PERIPHERAL_LOG_LEVEL_DEBUG,
    EVMU_PERIPHERAL_LOG_LEVEL_DISABLED
};

GBL_CLASS_DERIVE(EvmuPeripheral, GblObject, EvmuIBehavior)
    EVMU_RESULT (*pFnMemoryEvent)(GBL_SELF, EvmuMemoryEvent* pEvent);
    EVMU_RESULT (*pFnClockEvent) (GBL_SELF, EvmuClockEvent* pEvent);
GBL_CLASS_END

GBL_INSTANCE_DERIVE(EvmuPeripheral, GblObject)
    GblFlags logLevel;
GBL_INSTANCE_END

EVMU_EXPORT GblType     EvmuPeripheral_type        (void)                     GBL_NOEXCEPT;
EVMU_EXPORT EvmuDevice* EvmuPeripheral_device      (GBL_CSELF)                GBL_NOEXCEPT;
EVMU_EXPORT GblFlags    EvmuPeripheral_logLevel    (GBL_CSELF)                GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuPeripheral_setLogLevel (GBL_SELF, GblFlags level) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_PERIPHERAL_H


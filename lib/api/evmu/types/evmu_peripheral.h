#ifndef EVMU_PERIPHERAL_H
#define EVMU_PERIPHERAL_H

#include "evmu_entity.h"

#define EVMU_PERIPHERAL_TYPE                    (EvmuPeripheral_type())
#define EVMU_PERIPHERAL_STRUCT                  EvmuPeripheral
#define EVMU_PERIPHERAL_CLASS_STRUCT            EvmuPeripheralClass
#define EVMU_PERIPHERAL(instance)               (GBL_INSTANCE_CAST_PREFIX  (instance, EVMU_PERIPHERAL))
#define EVMU_PERIPHERAL_CHECK(instance)         (GBL_INSTANCE_CHECK_PREFIX (instance, EVMU_PERIPHERAL))
#define EVMU_PERIPHERAL_CLASS(klass)            (GBL_CLASS_CAST_PREFIX     (klass,    EVMU_PERIPHERAL))
#define EVMU_PERIPHERAL_CLASS_CHECK(klass)      (GBL_CLASS_CHECK_PREFIX    (klass,    EVMU_PERIPHERAL))
#define EVMU_PERIPHERAL_GET_CLASS(instance)     (GBL_TYPE_INSTANCE_CLASS_PREFIX (instance, EVMU_PERIPHERAL))

#define EVMU_PERIPHERAL_VERBOSE(...)
#define EVMU_PERIPHERAL_DEBUG(...)
#define EVMU_PERIPHERAL_WARNING(...)
#define EVMU_PERIPHERAL_ERROR(...)

#if 0
#define EVMU_PERI_GET_DEV(pSelf)
#define EVMU_PERI_GET_CPU
#define EVMU_PERI_GET_MEM
#define EVMU_PERI_GET_
#endif

#define SELF    EvmuPeripheral* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

typedef enum EVMU_PERIPHERAL_LOG_LEVEL {
    EVMU_PERIPHERAL_LOG_LEVEL_ERROR,
    EVMU_PERIPHERAL_LOG_LEVEL_WARNING,
    EVMU_PERIPHERAL_LOG_LEVEL_VERBOSE,
    EVMU_PERIPHERAL_LOG_LEVEL_DEBUG,
    EVMU_PERIPHERAL_LOG_LEVEL_DISABLED,
} EVMU_PERIPHERAL_LOG_LEVEL;

typedef struct EvmuPeripheralClass {
    EvmuEntityClass base;
    EVMU_RESULT (*pFnEventMemory)(SELF, EvmuMemoryEvent* pEvent);
    EVMU_RESULT (*pFnEventClock) (SELF, EvmuClockEvent* pEvent);
} EvmuPeripheralClass;

typedef struct EvmuPeripheral {
    EvmuEntity  object;
    GblEnum     logLevel;
} EvmuPeripheral;

GBL_EXPORT GblType      EvmuPeripheral_type         (void)                  GBL_NOEXCEPT;

GBL_EXPORT EvmuDevice*  EvmuPeripheral_device       (CSELF)                 GBL_NOEXCEPT;

GBL_EXPORT GblEnum      EvmuPeripheral_logLevel     (CSELF)                 GBL_NOEXCEPT;
GBL_EXPORT void         EvmuPeripheral_logLevelSet  (SELF, GblEnum level)   GBL_NOEXCEPT;


GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_PERIPHERAL_H


#ifndef EVMU_DEVICE_H
#define EVMU_DEVICE_H

#include "../types/evmu_entity.h"

#define EVMU_DEVICE_TYPE                        (EvmuDevice_type())
#define EVMU_DEVICE_STRUCT                      EvmuDevice
#define EVMU_DEVICE_CLASS_STRUCT                EvmuDeviceClass
#define EVMU_DEVICE(instance)                   (GBL_INSTANCE_CAST_PREFIX(instance, EVMU_DEVICE))
#define EVMU_DEVICE_CHECK(instance)             (GBL_INSTANCE_CHECK(instance, EVMU_DEVICE_TYPE))
#define EVMU_DEVICE_CLASS(klass)                (GBL_CLASS_CAST_PREFIX(klass, EVMU_DEVICE))
#define EVMU_DEVICE_CLASS_CHECK(klass)          (GBL_CLASS_CHECK(klass, EVMU_DEVICE_TYPE))
#define EVMU_DEVICE_GET_CLASS(instance)         (GBL_INSTANCE_CAST_CLASS_PREFIX(instance, EVMU_DEVICE))

#define SELF    EvmuDevice* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuDevice_);

typedef struct EvmuDeviceClass {
    EvmuEntityClass     base;
} EvmuDeviceClass;

typedef struct EvmuDevice {
    union {
        EvmuDeviceClass*    pClass;
        EvmuEntity          base;
    };
    EvmuDevice_*            pPrivate;
} EvmuDevice;

GBL_EXPORT GblType         EvmuDevice_type                 (void)                     GBL_NOEXCEPT;

GBL_EXPORT GblSize         EvmuDevice_peripheralCount      (CSELF)                    GBL_NOEXCEPT;
GBL_EXPORT EvmuPeripheral* EvmuDevice_peripheralFindByName (CSELF, const char* pName) GBL_NOEXCEPT;
GBL_EXPORT EvmuPeripheral* EvmuDevice_peripheralFindByIndex(CSELF, GblSize index)     GBL_NOEXCEPT;

GBL_EXPORT EvmuMemory*     EvmuDevice_memory               (CSELF)                    GBL_NOEXCEPT;
GBL_EXPORT EvmuCpu*        EvmuDevice_cpu                  (CSELF)                    GBL_NOEXCEPT;
GBL_EXPORT EvmuClock*      EvmuDevice_clock                (CSELF)                    GBL_NOEXCEPT;
GBL_EXPORT EvmuPic*        EvmuDevice_pic                  (CSELF)                    GBL_NOEXCEPT;
GBL_EXPORT EvmuFlash*      EvmuDevice_flash                (CSELF)                    GBL_NOEXCEPT;
GBL_EXPORT EvmuLcd*        EvmuDevice_lcd                  (CSELF)                    GBL_NOEXCEPT;

GBL_DECLS_END

#undef CSELF
#undef SELF







#endif // EVMU_DEVICE_H


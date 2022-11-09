#ifndef EVMU_EMULATOR_H
#define EVMU_EMULATOR_H

#include <gimbal/meta/instances/gimbal_module.h>
#include "evmu_ibehavior.h"

#define EVMU_EMULATOR_TYPE                  (GBL_TYPEOF(EvmuEmulator))
#define EVMU_EMULATOR(instance)             (GBL_INSTANCE_CAST(instance,  EvmuEmulator))
#define EVMU_EMULATOR_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuEmulator))
#define EVMU_EMULATOR_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance, EvmuEmulator))

#define GBL_SELF_TYPE   EvmuEmulator

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuDevice);

GBL_CLASS_DERIVE_EMPTY   (EvmuEmulator, GblContext, EvmuIBehavior)
GBL_INSTANCE_DERIVE_EMPTY(EvmuEmulator, GblContext)

EVMU_EXPORT GblType       EvmuEmulator_type         (void)                          GBL_NOEXCEPT;
EVMU_EXPORT GblVersion    EvmuEmulator_version      (void)                          GBL_NOEXCEPT;

EVMU_EXPORT EvmuEmulator* EvmuEmulator_create       (GblContext* pContext)          GBL_NOEXCEPT;
EVMU_EXPORT void          EvmuEmulator_destroy      (GBL_SELF)                      GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuEmulator_addDevice    (GBL_SELF, EvmuDevice* pDevice) GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuEmulator_removeDevice (GBL_SELF, EvmuDevice* pDevice) GBL_NOEXCEPT;
EVMU_EXPORT GblSize       EvmuEmulator_deviceCount  (GBL_CSELF)                     GBL_NOEXCEPT;
EVMU_EXPORT EvmuDevice*   EvmuEmulator_deviceAt     (GBL_CSELF, GblSize index)      GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CONTEXT_H


/*
typedef struct EvmuEmulator {
    EvmuEntity              base;
    //virtual gettimeofday
    //virtual fileOpen
    //virtual fileRead
    //virtual fileWrite
    //virtual fileSeek
} EvmuEmulator;
*/

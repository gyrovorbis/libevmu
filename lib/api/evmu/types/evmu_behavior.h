#ifndef EVMU_ENTITY_H
#define EVMU_ENTITY_H

#include "evmu_typedefs.h"
#include <gimbal/meta/instances/gimbal_object.h>

#define EVMU_BEHAVIOR_TYPE                  (GBL_TYPEOF(EvmuBehavior))
#define EVMU_BEHAVIOR(instance)             (GBL_INSTANCE_CAST(instance, EvmuBehavior))
#define EVMU_BEHAVIOR_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuBehavior))
#define EVMU_BEHAVIOR_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance,  EvmuBehavior))

#define GBL_SELF_TYPE EvmuBehavior

GBL_DECLS_BEGIN

GBL_INTERFACE_DERIVE(EvmuBehavior)
    EVMU_RESULT (*pFnInitialize)(GBL_SELF);
    EVMU_RESULT (*pFnFinalize)  (GBL_SELF);
    EVMU_RESULT (*pFnReset)     (GBL_SELF);
    EVMU_RESULT (*pFnUpdate)    (GBL_SELF, EvmuTicks ticks);
    EVMU_RESULT (*pFnSaveState) (GBL_CSELF, GblStringBuffer* pString);
    EVMU_RESULT (*pFnLoadState) (GBL_SELF, const GblStringBuffer* pString);
GBL_INTERFACE_END

EVMU_EXPORT GblType       EvmuBehavior_type       (void)                                     GBL_NOEXCEPT;

EVMU_EXPORT EvmuEmulator* EvmuBehavior_emulator   (GBL_CSELF)                                GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT   EvmuBehavior_initialize (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuBehavior_finalize   (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuBehavior_reset      (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuBehavior_update     (GBL_SELF, EvmuTicks ticks)                GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuBehavior_saveState  (GBL_SELF, GblStringBuffer* pString)       GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuBehavior_loadState  (GBL_SELF, const GblStringBuffer* pString) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_ENTITY_H

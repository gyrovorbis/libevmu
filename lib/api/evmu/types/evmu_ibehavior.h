#ifndef EVMU_IBEHAVIOR_H
#define EVMU_IBEHAVIOR_H

#include "evmu_typedefs.h"
#include <gimbal/meta/instances/gimbal_object.h>

#define EVMU_IBEHAVIOR_TYPE                  (GBL_TYPEOF(EvmuIBehavior))
#define EVMU_IBEHAVIOR(instance)             (GBL_INSTANCE_CAST(instance, EvmuIBehavior))
#define EVMU_IBEHAVIOR_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuIBehavior))
#define EVMU_IBEHAVIOR_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance,  EvmuIBehavior))

#define GBL_SELF_TYPE EvmuIBehavior

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuEmulator);

GBL_INTERFACE_DERIVE(EvmuIBehavior)
    EVMU_RESULT (*pFnInitialize)(GBL_SELF);
    EVMU_RESULT (*pFnFinalize)  (GBL_SELF);
    EVMU_RESULT (*pFnReset)     (GBL_SELF);
    EVMU_RESULT (*pFnUpdate)    (GBL_SELF, EvmuTicks ticks);
    EVMU_RESULT (*pFnSaveState) (GBL_CSELF, GblStringBuffer* pString);
    EVMU_RESULT (*pFnLoadState) (GBL_SELF, const GblStringBuffer* pString);
GBL_INTERFACE_END

EVMU_EXPORT GblType       EvmuIBehavior_type       (void)                                     GBL_NOEXCEPT;
EVMU_EXPORT EvmuEmulator* EvmuIBehavior_emulator   (GBL_CSELF)                                GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_initialize (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_finalize   (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_reset      (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_update     (GBL_SELF, EvmuTicks ticks)                GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_saveState  (GBL_SELF, GblStringBuffer* pString)       GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_loadState  (GBL_SELF, const GblStringBuffer* pString) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_IBEHAVIOR_H

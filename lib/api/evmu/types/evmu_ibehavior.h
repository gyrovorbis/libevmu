/*! \file
 *  \brief EvmuIBehavior interface for emulated entities
 *
 *  \todo
 *      - static typeinfo
 *
 *  \author 2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_IBEHAVIOR_H
#define EVMU_IBEHAVIOR_H

#include "evmu_typedefs.h"
#include <gimbal/meta/instances/gimbal_object.h>

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_IBEHAVIOR_TYPE                  (GBL_TYPEOF(EvmuIBehavior))
#define EVMU_IBEHAVIOR(instance)             (GBL_INSTANCE_CAST(instance, EvmuIBehavior))
#define EVMU_IBEHAVIOR_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuIBehavior))
#define EVMU_IBEHAVIOR_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance,  EvmuIBehavior))
//! @}

#define GBL_SELF_TYPE EvmuIBehavior

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuEmulator);

/*! \struct  EvmuIBehaviorClass
 *  \extends GblInterface
 *  \brief   GblInterface/VTable for all EvmuBehaviors
 *
 *  EvmuIBehaviorClass is the virtual table which implements each
 *  emulation event/trigger for a given class.
 */
GBL_INTERFACE_DERIVE(EvmuIBehavior)
    //! Called when the reset event is fired
    EVMU_RESULT (*pFnReset)     (GBL_SELF);
    //! Called when the update event is fired
    EVMU_RESULT (*pFnUpdate)    (GBL_SELF, EvmuTicks ticks);
    //! Called to save the state of the associated entity
    EVMU_RESULT (*pFnSaveState) (GBL_CSELF, FILE* pFile);
    //! Called to load the state of the associated entity
    EVMU_RESULT (*pFnLoadState) (GBL_SELF, FILE* pFile);
GBL_INTERFACE_END

/*! \struct EvmuIBehavior
 *  \brief Standard events for all emulated entities
 *  \ingroup evmu_ibehavior.h
 *
 *  EvmuIBehavior is a common interface which is inherited by all
 *  emulated Entities within ElysianVMU, providing basic event-driven
 *  logic for each hardware block.
 */

EVMU_EXPORT GblType       EvmuIBehavior_type       (void)                                     GBL_NOEXCEPT;
EVMU_EXPORT EvmuEmulator* EvmuIBehavior_emulator   (GBL_CSELF)                                GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_reset      (GBL_SELF)                                 GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_update     (GBL_SELF, EvmuTicks ticks)                GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_saveState  (GBL_SELF, GblStringBuffer* pString)       GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuIBehavior_loadState  (GBL_SELF, const GblStringBuffer* pString) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_IBEHAVIOR_H

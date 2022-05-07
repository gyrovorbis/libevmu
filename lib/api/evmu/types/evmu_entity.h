#ifndef EVMU_ENTITY_H
#define EVMU_ENTITY_H

#include "evmu_typedefs.h"
#include <gimbal/objects/gimbal_object.h>

#define EVMU_ENTITY_TYPE                    (EvmuEntity_type())
#define EVMU_ENTITY_STRUCT                  EvmuEntity
#define EVMU_ENTITY_CLASS_STRUCT            EvmuEntityClass
#define EVMU_ENTITY(instance)               (GBL_INSTANCE_CAST_PREFIX  (instance,  EVMU_ENTITY))
#define EVMU_ENTITY_CHECK(instance)         (GBL_INSTANCE_CHECK_PREFIX (instance,  EVMU_ENTITY))
#define EVMU_ENTITY_CLASS(klass)            (GBL_CLASS_CAST_PREFIX     (klass,     EVMU_ENTITY))
#define EVMU_ENTITY_CLASS_CHECK(klass)      (GBL_CLASS_CHECK_PREFIX    (klass,     EVMU_ENTITY))
#define EVMU_ENTITY_GET_CLASS(instance)     (GBL_INSTANCE_CAST_CLASS_PREFIX (instance,  EVMU_ENTITY))

#define SELF    EvmuEntity* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

typedef struct EvmuEntityClass {
    GblObjectClass  base;
    GBL_RESULT      (*pFnInit)(SELF);
    GBL_RESULT      (*pFnDeinit)(SELF);
    GBL_RESULT      (*pFnReset) (SELF);
    GBL_RESULT      (*pFnUpdate)(SELF, EvmuTicks ticks);
    GBL_RESULT      (*pFnStateSave)(CSELF, GblString* pString);
    GBL_RESULT      (*pFnStateLoad)(SELF, const GblString* pString);
} EvmuEntityClass;

typedef struct EvmuEntity {
    union {
        EvmuEntityClass*    pClass;
        GblObject           base;
    };
} EvmuEntity;

GBL_EXPORT GblType          EvmuEntity_type                 (void)                                   GBL_NOEXCEPT;

GBL_EXPORT GblType          EvmuEntity_entityType           (CSELF)                                  GBL_NOEXCEPT;
GBL_EXPORT EvmuSimulation*  EvmuEntity_findSimulation       (CSELF)                                  GBL_NOEXCEPT;

GBL_EXPORT GblObject*       EvmuEntity_childFindByTypeIndex (CSELF, GblType type, GblSize index)     GBL_NOEXCEPT;
GBL_EXPORT GblObject*       EvmuEntity_childFindByTypeName  (CSELF, GblType type, const char* pName) GBL_NOEXCEPT;
GBL_EXPORT GblSize          EvmuEntity_childCountByType     (CSELF, GblType type)                    GBL_NOEXCEPT;

GBL_EXPORT GBL_RESULT       EvmuEntity_vReset               (SELF)                                   GBL_NOEXCEPT;
GBL_EXPORT GBL_RESULT       EvmuEntity_vUpdate              (SELF, EvmuTicks ticks)                  GBL_NOEXCEPT;

GBL_DECLS_END

#undef CSELF
#undef SELF


#endif // EVMU_ENTITY_H

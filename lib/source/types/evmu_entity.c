#include "gimbal/meta/gimbal_instance.h"
#include <evmu/types/evmu_entity.h>
#include <evmu/types/evmu_simulation.h>

static GBL_RESULT EvmuEntity_reset_(EvmuEntity* pSelf) {
    GBL_API_BEGIN(NULL);
    for(GblObject* pObject = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObject != NULL;
        pObject = GblObject_siblingNext(GBL_OBJECT(pSelf)))
    {
        if(EVMU_ENTITY_CHECK(pObject)) {
            GBL_API_CALL(EvmuEntity_vReset(EVMU_ENTITY(pObject)));
        }
    }
    GBL_API_END();
}

static GBL_RESULT EvmuEntity_update_(EvmuEntity* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    for(GblObject* pObject = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObject != NULL;
        pObject = GblObject_siblingNext(GBL_OBJECT(pSelf)))
    {
        if(EVMU_ENTITY_CHECK(pObject)) {
            GBL_API_CALL(EvmuEntity_vUpdate(EVMU_ENTITY(pObject), ticks));
        }
    }
    GBL_API_END();
}

static GBL_RESULT EvmuEntityClass_init_(EvmuEntityClass* pClass, void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);
    pClass->pFnReset = EvmuEntity_reset_;
    pClass->pFnUpdate = EvmuEntity_update_;
    GBL_API_END();
}

GBL_EXPORT GblType EvmuEntity_type(void) GBL_NOEXCEPT {
    static GblType type = GBL_INVALID_TYPE;
    if(type == GBL_INVALID_TYPE) {
        type = GblType_registerStatic(GBL_OBJECT_TYPE,
                                     "EvmuEntity",
                                     &((const GblTypeInfo) {
                                         .pFnClassInit  = (GblTypeClassInitializeFn)EvmuEntityClass_init_,
                                         .classSize     = sizeof(EvmuEntityClass),
                                         .instanceSize  = sizeof(EvmuEntity),
                                     }),
                                     GBL_TYPE_FLAGS_NONE);

    }
    return type;
}

GBL_EXPORT GblType EvmuEntity_entityType(const EvmuEntity* pSelf) GBL_NOEXCEPT {
    GblType type = GBL_INVALID_TYPE;
    for(GblUint d = 0; d <= GblType_depth(GBL_INSTANCE_TYPE(pSelf)); ++d) {
        if(GblType_base(GBL_INSTANCE_TYPE(pSelf), d) == EVMU_ENTITY_TYPE) {
            type = GblType_base(GBL_INSTANCE_TYPE(pSelf), d+1);
            break;
        }
    }
    return type;
}

GBL_EXPORT EvmuSimulation* EvmuEntity_findSimulation(const EvmuEntity* pSelf) GBL_NOEXCEPT {
    return EVMU_SIMULATION(GblObject_ancestorFindByType(&pSelf->base, EVMU_SIMULATION_TYPE));
}
GBL_EXPORT GblObject* EvmuEntity_childFindByTypeIndex(const EvmuEntity* pSelf, GblType type, GblSize index) {
    GblObject* pChild = NULL;
    GblSize count = 0;
    GBL_API_BEGIN(GblType_context(GBL_INSTANCE_TYPE(pSelf)));
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPE(pObj), type)) {
            if(count++ == index) {
                pChild = pObj;
            }
        }
    }
    GBL_API_END_BLOCK();
    return pChild;
}

GBL_EXPORT GblObject* EvmuEntity_childFindByTypeName(const EvmuEntity* pSelf, GblType type, const char* pName) GBL_NOEXCEPT {
    GblObject* pChild = NULL;
    GBL_API_BEGIN(GblType_context(GBL_INSTANCE_TYPE(pSelf)));
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPE(pObj), type)) {
            const char* pObjName = GblObject_nameGet(pObj);
            if(pObjName && strcmp(pObjName, pName) == 0) {
                pChild = pObj;
                break;
            }
        }
    }
    GBL_API_END_BLOCK();
    return pChild;
}

GBL_EXPORT GblSize EvmuEntity_childCountByType(const EvmuEntity* pSelf, GblType type) {
    GblSize count = 0;
    GBL_API_BEGIN(GblType_context(GBL_INSTANCE_TYPE(pSelf)));
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPE(pObj), type)) {
            ++count;
        }
    }
    GBL_API_END_BLOCK();
    return count;
}



GBL_EXPORT GBL_RESULT EvmuEntity_vReset(EvmuEntity* pSelf) GBL_NOEXCEPT {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_PREFIX(EVMU_ENTITY, pFnReset, pSelf);
    GBL_API_END();
}

GBL_EXPORT GBL_RESULT EvmuEntity_vUpdate(EvmuEntity* pSelf, EvmuTicks ticks) GBL_NOEXCEPT {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_PREFIX(EVMU_ENTITY, pFnUpdate, pSelf, ticks);
    GBL_API_END();
}

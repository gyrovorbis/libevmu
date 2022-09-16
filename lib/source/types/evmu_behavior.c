#include <gimbal/meta/instances/gimbal_instance.h>
#include <evmu/types/evmu_behavior.h>
#include <evmu/types/evmu_emulator.h>

static GBL_RESULT EvmuBehavior_reset_(EvmuBehavior* pSelf) {
    GBL_API_BEGIN(NULL);
    for(GblObject* pObject = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObject != NULL;
        pObject = GblObject_siblingNext(GBL_OBJECT(pSelf)))
    {
        if(GBL_INSTANCE_CHECK(pObject, EvmuBehavior)) {
            GBL_API_CALL(EvmuBehavior_reset(EVMU_BEHAVIOR(pObject)));
        }
    }
    GBL_API_END();
}

static GBL_RESULT EvmuBehavior_update_(EvmuBehavior* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    for(GblObject* pObject = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObject != NULL;
        pObject = GblObject_siblingNext(GBL_OBJECT(pSelf)))
    {
        if(GBL_INSTANCE_CHECK(pObject, EvmuBehavior)) {
            GBL_API_CALL(EvmuBehavior_update(EVMU_BEHAVIOR(pObject), ticks));
        }
    }
    GBL_API_END();
}

static GBL_RESULT EvmuBehaviorClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);

    EVMU_BEHAVIOR_CLASS(pClass)->pFnReset = EvmuBehavior_reset_;
    EVMU_BEHAVIOR_CLASS(pClass)->pFnUpdate = EvmuBehavior_update_;

    GBL_API_END();
}

GBL_EXPORT EvmuEmulator* EvmuBehavior_emulator(const EvmuBehavior* pSelf) GBL_NOEXCEPT {
    return EVMU_EMULATOR(GblObject_findAncestorByType(GBL_OBJECT(pSelf), EVMU_EMULATOR_TYPE));
}

GBL_EXPORT GblObject* EvmuBehavior_childFindByTypeIndex(const EvmuBehavior* pSelf, GblType type, GblSize index) {
    GblObject* pChild = NULL;
    GblSize count = 0;
    GBL_API_BEGIN(NULL);
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPEOF(pObj), type)) {
            if(count++ == index) {
                pChild = pObj;
            }
        }
    }
    GBL_API_END_BLOCK();
    return pChild;
}

GBL_EXPORT GblObject* EvmuBehavior_childFindByTypeName(const EvmuBehavior* pSelf, GblType type, const char* pName) {
    GblObject* pChild = NULL;
    GBL_API_BEGIN(NULL);
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPEOF(pObj), type)) {
            const char* pObjName = GblObject_name(pObj);
            if(pObjName && strcmp(pObjName, pName) == 0) {
                pChild = pObj;
                break;
            }
        }
    }
    GBL_API_END_BLOCK();
    return pChild;
}

GBL_EXPORT GblSize EvmuBehavior_childCountByType(const EvmuBehavior* pSelf, GblType type) {
    GblSize count = 0;
    GBL_API_BEGIN(NULL);
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPEOF(pObj), type)) {
            ++count;
        }
    }
    GBL_API_END_BLOCK();
    return count;
}

GBL_EXPORT GBL_RESULT EvmuBehavior_reset(EvmuBehavior* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuBehavior, pFnReset, pSelf);
    GBL_API_END();
}

GBL_EXPORT GBL_RESULT EvmuBehavior_update(EvmuBehavior* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuBehavior, pFnUpdate, pSelf, ticks);
    GBL_API_END();
}

GBL_EXPORT GblType EvmuBehavior_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblType dependencies[1];

    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        dependencies[0] = GBL_OBJECT_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuBehavior"),
                                      GBL_INTERFACE_TYPE,
                                      &(const GblTypeInfo) {
                                          .pFnClassInit      = EvmuBehaviorClass_init_,
                                          .classSize         = sizeof(EvmuBehaviorClass),
                                          .dependencyCount  = 1,
                                          .pDependencies    = dependencies
                                     },
                                     GBL_TYPE_FLAGS_NONE);
        GBL_API_VERIFY_LAST_RECORD();

        GBL_API_END_BLOCK();
    }
    return type;
}

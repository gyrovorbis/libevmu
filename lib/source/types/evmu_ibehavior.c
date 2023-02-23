#include <gimbal/meta/instances/gimbal_instance.h>
#include <evmu/types/evmu_ibehavior.h>
#include <evmu/types/evmu_emulator.h>

static GBL_RESULT EvmuIBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);
    for(GblObject* pObject = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObject != NULL;
        pObject = GblObject_siblingNext(GBL_OBJECT(pObject)))
    {
        if(GBL_INSTANCE_CHECK(pObject, EvmuIBehavior)) {
            GBL_CTX_CALL(EvmuIBehavior_reset(EVMU_IBEHAVIOR(pObject)));
        }
    }
    GBL_CTX_END();
}

static GBL_RESULT EvmuIBehavior_update_(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);
    for(GblObject* pObject = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObject != NULL;
        pObject = GblObject_siblingNext(GBL_OBJECT(pObject)))
    {
        if(GBL_INSTANCE_CHECK(pObject, EvmuIBehavior)) {
            GBL_CTX_CALL(EvmuIBehavior_update(EVMU_IBEHAVIOR(pObject), ticks));
        }
    }
    GBL_CTX_END();
}

static GBL_RESULT EvmuIBehaviorClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset = EvmuIBehavior_reset_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate = EvmuIBehavior_update_;

    GBL_CTX_END();
}

GBL_EXPORT EvmuEmulator* EvmuIBehavior_emulator(const EvmuIBehavior* pSelf) GBL_NOEXCEPT {
    return EVMU_EMULATOR(GblObject_findAncestorByType(GBL_OBJECT(pSelf), EVMU_EMULATOR_TYPE));
}

GBL_EXPORT GblObject* EvmuIBehavior_childFindByTypeIndex(const EvmuIBehavior* pSelf, GblType type, GblSize index) {
    GblObject* pChild = NULL;
    GblSize count = 0;
    GBL_CTX_BEGIN(NULL);
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
    GBL_CTX_END_BLOCK();
    return pChild;
}

GBL_EXPORT GblObject* EvmuIBehavior_childFindByTypeName(const EvmuIBehavior* pSelf, GblType type, const char* pName) {
    GblObject* pChild = NULL;
    GBL_CTX_BEGIN(NULL);
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
    GBL_CTX_END_BLOCK();
    return pChild;
}

GBL_EXPORT GblSize EvmuIBehavior_childCountByType(const EvmuIBehavior* pSelf, GblType type) {
    GblSize count = 0;
    GBL_CTX_BEGIN(NULL);
    for(GblObject* pObj = GblObject_childFirst(GBL_OBJECT(pSelf));
        pObj != NULL;
        pObj = GblObject_siblingNext(GBL_OBJECT(pObj)))
    {
        if(GblType_check(GBL_INSTANCE_TYPEOF(pObj), type)) {
            ++count;
        }
    }
    GBL_CTX_END_BLOCK();
    return count;
}

GBL_EXPORT GBL_RESULT EvmuIBehavior_reset(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuIBehavior, pFnReset, pSelf);
    GBL_CTX_END();
}

GBL_EXPORT GBL_RESULT EvmuIBehavior_update(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuIBehavior, pFnUpdate, pSelf, ticks);
    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuIBehavior_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblType dependencies[1];

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        dependencies[0] = GBL_OBJECT_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuIBehavior"),
                                      GBL_INTERFACE_TYPE,
                                      &(const GblTypeInfo) {
                                          .pFnClassInit      = EvmuIBehaviorClass_init_,
                                          .classSize         = sizeof(EvmuIBehaviorClass),
                                          .dependencyCount  = 1,
                                          .pDependencies    = dependencies
                                     },
                                     GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();

        GBL_CTX_END_BLOCK();
    }
    return type;
}

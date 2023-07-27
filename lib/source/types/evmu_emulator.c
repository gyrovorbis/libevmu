#include <evmu/types/evmu_emulator.h>
#include <evmu/hw/evmu_device.h>
#include <gimbal/utils/gimbal_version.h>

EVMU_EXPORT GblVersion EvmuEmulator_version(void) {
    return GBL_VERSION_MAKE(EVMU_VERSION_MAJOR, EVMU_VERSION_MINOR, EVMU_VERSION_PATCH);
}

EVMU_EXPORT EvmuEmulator* EvmuEmulator_create(void) {
    return GBL_NEW(EvmuEmulator);
}

EVMU_EXPORT GblRefCount EvmuEmulator_unref(EvmuEmulator* pSelf) {
    return GBL_UNREF(pSelf);
}

EVMU_EXPORT EVMU_RESULT EvmuEmulator_addDevice(EvmuEmulator* pSelf, EvmuDevice* pDevice) {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_ARG(!GblObject_parent(GBL_OBJECT(pDevice)));

    GblObject_addChild(GBL_OBJECT(pSelf), GBL_OBJECT(pDevice));

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuEmulator_removeDevice(EvmuEmulator* pSelf, EvmuDevice* pDevice) {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pDevice);

    const GblBool removed = GblObject_removeChild(GBL_OBJECT(pSelf), GBL_OBJECT(pDevice));

    GBL_CTX_VERIFY(removed,
                   GBL_RESULT_ERROR_INVALID_OPERATION,
                   "EvmuEmulator_removeDevice(): attempt to remove non-child device!");
    GBL_CTX_END();
}

EVMU_EXPORT size_t EvmuEmulator_deviceCount(const EvmuEmulator* pSelf) {
    size_t count = 0;
    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_TYPECHECK(EvmuDevice, pIter)) ++count;
    }
    return count;
}

GBL_DECLARE_STRUCT(EvmuEmulatorDeviceIterClosure_) {
    size_t      count;
    size_t      index;
    EvmuDevice* pResult;
};

static GblBool EvmuEmulator_deviceIter_(const EvmuEmulator* pSelf, EvmuDevice* pDevice, void* pClosure) {
    EvmuEmulatorDeviceIterClosure_* pIterClosure = pClosure;

    if(pIterClosure->count++ == pIterClosure->index) {
        pIterClosure->pResult = pDevice;
        return GBL_TRUE;
    }

    return GBL_FALSE;
}

EVMU_EXPORT EvmuDevice* EvmuEmulator_device(const EvmuEmulator* pSelf, size_t index) {
    EvmuEmulatorDeviceIterClosure_ closure = {
        .index = index
    };

    EvmuEmulator_foreachDevice(pSelf, EvmuEmulator_deviceIter_, &closure);

    return closure.pResult;
}

EVMU_EXPORT GblBool EvmuEmulator_foreachDevice(const EvmuEmulator* pSelf, EvmuEmulatorIterFn pFnIt, void* pClosure) {
    EvmuDevice* pDevice = NULL;
    size_t      count   = 0;

    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_TYPECHECK(EvmuDevice, pIter))
            if(pFnIt(pSelf, EVMU_DEVICE(pIter), pClosure))
                return GBL_TRUE;
    }

    return GBL_FALSE;
}


static GBL_RESULT EvmuEmulator_GblModule_unload_(GblModule* pModule) {
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_END();
}

static GBL_RESULT EvmuEmulator_GblModule_load_(GblModule* pModule) {
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_END();
}

static GBL_RESULT EvmuEmulator_GblBox_destructor_(GblBox* pBox) {
    GBL_CTX_BEGIN(NULL);
    GBL_VCALL_DEFAULT(GblModule, base.base.base.pFnDestructor, pBox);
    GBL_CTX_END();
}

static GBL_RESULT EvmuEmulator_init_(GblInstance* pInstance) {
    GBL_CTX_BEGIN(NULL);

    GblModule* pModule    = GBL_MODULE(pInstance);

    pModule->version      = EvmuEmulator_version();
    pModule->pPrefix      = GblStringRef_create("Evmu");
    pModule->pAuthor      = GblStringRef_create("Falco Girgis");
    pModule->pDescription = GblStringRef_create("ElysianVMU Emulation Core");

    GBL_CTX_END();
}

static GBL_RESULT EvmuEmulatorClass_init_(GblClass* pClass, const void* pUd) {
    GBL_CTX_BEGIN(NULL);

    GBL_BOX_CLASS(pClass)   ->pFnDestructor = EvmuEmulator_GblBox_destructor_;
    GBL_MODULE_CLASS(pClass)->pFnLoad       = EvmuEmulator_GblModule_load_;
    GBL_MODULE_CLASS(pClass)->pFnUnload     = EvmuEmulator_GblModule_unload_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuEmulator_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblInterfaceImpl ifaceEntries[] = {
        {
            .classOffset   = offsetof(EvmuEmulatorClass, EvmuIBehaviorImpl)
        }
    };

    static const GblTypeInfo info = {
        .pFnClassInit    = EvmuEmulatorClass_init_,
        .classSize       = sizeof(EvmuEmulatorClass),
        .pFnInstanceInit = EvmuEmulator_init_,
        .instanceSize    = sizeof(EvmuEmulator),
        .interfaceCount  = 1,
        .pInterfaceImpls   = ifaceEntries
    };

    if(type == GBL_INVALID_TYPE) {
        ifaceEntries[0].interfaceType = EVMU_IBEHAVIOR_TYPE;

        type = GblType_register(GblQuark_internStringStatic("EvmuEmulator"),
                                      GBL_MODULE_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }
    return type;
}


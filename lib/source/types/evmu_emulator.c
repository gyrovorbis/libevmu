#include <evmu/types/evmu_emulator.h>
#include <evmu/hw/evmu_device.h>
#include <gimbal/meta/instances/gimbal_context.h>

EVMU_EXPORT EvmuEmulator* EvmuEmulator_create(GblContext* pContext) {
    EvmuEmulator* pInstance = NULL;
    GBL_API_BEGIN(pContext);

    pInstance = EVMU_EMULATOR(GblObject_create(EVMU_EMULATOR_TYPE,
                                               "parent", pContext,
                                               NULL));

    GBL_API_END_BLOCK();
    return pInstance;
}

EVMU_EXPORT void EvmuEmulator_destroy(EvmuEmulator* pSelf) {
    GBL_API_BEGIN(pSelf);
    GBL_API_VERIFY_POINTER(pSelf);
    const GblRefCount refCount = GBL_BOX_UNREF(pSelf);
    GBL_API_VERIFY(!refCount, GBL_RESULT_ERROR,
                   "Attempting to destroy EvmuEmulator "
                   "with remaining references: %u", refCount);
    GBL_API_END_BLOCK();
}

EVMU_EXPORT EVMU_RESULT EvmuEmulator_addDevice(EvmuEmulator* pSelf, EvmuDevice* pDevice) {
    GBL_API_BEGIN(pSelf);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_ARG(!GblObject_parent(GBL_OBJECT(pDevice)));

    GblObject_setParent(GBL_OBJECT(pDevice), GBL_OBJECT(pSelf));

    GBL_API_END();
}

EVMU_EXPORT EVMU_RESULT EvmuEmulator_removeDevice(EvmuEmulator* pSelf, EvmuDevice* pDevice) {
    GBL_API_BEGIN(pSelf);
    GBL_API_VERIFY_POINTER(pDevice);

    const GblBool removed = GblObject_removeChild(GBL_OBJECT(pSelf), GBL_OBJECT(pDevice));

    GBL_API_VERIFY(removed,
                   GBL_RESULT_ERROR_INVALID_OPERATION,
                   "EvmuEmulator_removeDevice(): attempt to remove non-child device!");
    GBL_API_END();
}

EVMU_EXPORT GblSize EvmuEmulator_deviceCount(const EvmuEmulator* pSelf) {
    GblSize count = 0;
    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_INSTANCE_CHECK(pIter, EvmuDevice)) ++count;
    }
    return count;
}

EVMU_EXPORT EvmuDevice* EvmuEmulator_deviceAt(const EvmuEmulator* pSelf, GblSize index) {
    EvmuDevice* pDevice = NULL;
    GblSize count = 0;

    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_INSTANCE_CHECK(pIter, EvmuDevice)) {
            if(count++ == index) {
                pDevice = EVMU_DEVICE(pIter);
                break;
            }
        }
    }

    return pDevice;
}

EVMU_EXPORT GblType EvmuEmulator_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInterfaceMapEntry ifaceEntries[] = {
        {
            .classOffset   = offsetof(EvmuEmulatorClass, EvmuIBehaviorImpl)
        }
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        ifaceEntries[0].interfaceType = EVMU_IBEHAVIOR_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuEmulator"),
                                      GBL_CONTEXT_TYPE,
                                      &(const GblTypeInfo) {
                                          .classSize      = sizeof(EvmuEmulatorClass),
                                          .instanceSize   = sizeof(EvmuEmulator),
                                          .interfaceCount = 1,
                                          .pInterfaceMap  = ifaceEntries
                                      },
                                      GBL_TYPE_FLAGS_NONE);
        GBL_API_VERIFY_LAST_RECORD();
        GBL_API_END_BLOCK();
    }
    return type;
}


#include <evmu/types/evmu_emulator.h>
#include <evmu/hw/evmu_device.h>
#include <gimbal/meta/instances/gimbal_context.h>

GBL_EXPORT EvmuEmulator* EvmuEmulator_create(GblContext* pContext) {
    EvmuEmulator* pInstance = NULL;
    GBL_API_BEGIN(pContext);
    // dirty hack for now
    //if(pContext) GblContext_setGlobal(pContext);
    pInstance = EVMU_EMULATOR(GblObject_create(EVMU_EMULATOR_TYPE, NULL));
    GBL_API_END_BLOCK();
    return pInstance;
}

GBL_EXPORT void EvmuEmulator_destroy(EvmuEmulator* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    const GblRefCount refCount = GBL_BOX_UNREF(pSelf);
    GBL_API_VERIFY(!refCount, GBL_RESULT_ERROR,
                   "Attempting to destroy EvmuEmulator "
                   "with remaining references: %u", refCount);
    GBL_API_END_BLOCK();
}

GBL_EXPORT EvmuDevice* EvmuEmulator_deviceCreate(EvmuEmulator* pSelf) {
    EvmuDevice* pDevice = NULL;
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    pDevice = (EvmuDevice*)GblObject_create(EVMU_DEVICE_TYPE, "parent", pSelf,
                                                            NULL);


    GBL_API_END_BLOCK();
    return pDevice;
}

GBL_EXPORT void EvmuEmulator_deviceDestroy(EvmuEmulator* pInstance, EvmuDevice* pDevice) {
    GBL_UNUSED(pInstance);
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pInstance);
    GBL_API_VERIFY_POINTER(pDevice);
    const GblRefCount refCount = GBL_BOX_UNREF(pDevice);
    GBL_API_VERIFY(!refCount, GBL_RESULT_ERROR,
                   "Attempting to destroy EvmuDevice "
                   "with remaining references: %u", refCount);
    GBL_API_END_BLOCK();
}

GBL_EXPORT GblSize EvmuEmulator_deviceCount(const EvmuEmulator* pSelf) {
    GblSize count = 0;
    for(GblObject* pIter = GblObject_childFirst(GBL_OBJECT(pSelf));
        pIter;
        pIter = GblObject_siblingNext(pIter))
    {
        if(GBL_INSTANCE_CHECK(pIter, EvmuDevice)) ++count;
    }
    return count;
}

GBL_EXPORT EvmuDevice* EvmuEmulator_deviceAt(const EvmuEmulator* pSelf, GblSize index) {
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

GblType EvmuEmulator_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblTypeInterfaceMapEntry ifaceEntries[] = {
        {
            .classOffset   = offsetof(GblObjectClass, GblITableImpl)
        }
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        ifaceEntries[0].interfaceType = EVMU_BEHAVIOR_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuEmulator"),
                                      GBL_MODULE_TYPE,
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


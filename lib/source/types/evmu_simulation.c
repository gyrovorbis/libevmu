#include <evmu/types/evmu_simulation.h>
#include <evmu/hw/evmu_device.h>
#include <gimbal/objects/gimbal_context.h>

GblType EvmuSimulation_type(void) {
    static GblType type = GBL_TYPE_INVALID;
    if(type == GBL_TYPE_INVALID) {
        type = gblTypeRegisterStatic(EVMU_ENTITY_TYPE,
                                     "EvmuSimulation",
                                     &((const GblTypeInfo) {
                                         .classSize     = sizeof(EvmuSimulationClass),
                                         .classAlign    = GBL_ALIGNOF(EvmuSimulationClass),
                                         .instanceSize  = sizeof(EvmuSimulation),
                                         .instanceAlign = GBL_ALIGNOF(EvmuSimulation)
                                     }),
                                     GBL_TYPE_FLAGS_NONE);
    }
    return type;
}

GBL_EXPORT EvmuSimulation* EvmuSimulation_create(GblContext* pContext) {
    EvmuSimulation* pInstance = NULL;
    GBL_API_BEGIN(pContext);
    // dirty hack for now
    if(pContext) GblContext_globalSet(pContext);
    pInstance = EVMU_SIMULATION(GblObject_new(EVMU_SIMULATION_TYPE, NULL));
    GBL_API_END_BLOCK();
    return pInstance;
}

GBL_EXPORT void EvmuSimulation_destroy(EvmuSimulation* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    const GblRefCount refCount = GBL_OBJECT_UNREF(pSelf);
    GBL_API_VERIFY(!refCount, GBL_RESULT_ERROR,
                   "Attempting to destroy EvmuSimulation "
                   "with remaining references: %u", refCount);
    GBL_API_END_BLOCK();
}

GBL_EXPORT EvmuDevice* EvmuSimulation_deviceCreate(EvmuSimulation* pSelf) {
    EvmuDevice* pDevice = NULL;
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    pDevice = (EvmuDevice*)GblObject_new(EVMU_DEVICE_TYPE, "parent", pSelf,
                                                            NULL);


    GBL_API_END_BLOCK();
    return pDevice;
}

GBL_EXPORT void EvmuSimulation_deviceDestroy(EvmuSimulation* pInstance, EvmuDevice* pDevice) {
    GBL_UNUSED(pInstance);
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pInstance);
    GBL_API_VERIFY_POINTER(pDevice);
    const GblRefCount refCount = GBL_OBJECT_UNREF(pDevice);
    GBL_API_VERIFY(!refCount, GBL_RESULT_ERROR,
                   "Attempting to destroy EvmuDevice "
                   "with remaining references: %u", refCount);
    GBL_API_END_BLOCK();
}

GBL_EXPORT GblSize EvmuSimulation_deviceCount(const EvmuSimulation* pSelf) {
    return EvmuEntity_childCountByType(EVMU_ENTITY(pSelf), EVMU_DEVICE_TYPE);
}

GBL_EXPORT EvmuDevice* EvmuSimulation_deviceFindByIndex(const EvmuSimulation* pSelf, GblSize index) {
    return EVMU_DEVICE(EvmuEntity_childFindByTypeIndex(EVMU_ENTITY(pSelf), EVMU_DEVICE_TYPE, index));
}

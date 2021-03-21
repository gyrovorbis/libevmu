#ifndef EVMU_CONTEXT__H
#define EVMU_CONTEXT__H

#include <evmu/util/evmu_context.h>
#include <gimbal/gimbal_container.h>
#include <evmu/evmu_api.h>

#ifdef __cplusplus
extern "C" {
#endif

GBL_FORWARD_DECLARE_STRUCT(EvmuDevice_)
GBL_DECLARE_HANDLE(EvmuPeripheralDriver)

#define EVMU_CONTEXT__DEVICE_INDEX_INVALID  UINT32_MAX

typedef struct EvmuContext_ {
    EvmuError               lastError;
    EvmuContextCreateInfo   createInfo;
    EvmuEventHandler        eventHandler;

    GblVector               devices;
} EvmuContext_;

inline EVMU_RESULT evmuEventHandlerEmit_(const EvmuEventHandler* pHandler,
                                        EvmuContext hContext,
                                        EvmuDevice hDevice,
                                        EvmuPeripheral hPeripheral,
                                        EvmuEnum type,
                                        const EvmuEvent* pData,
                                        EvmuSize dataSize)
{
    EVMU_API_BEGIN(hContext);
    GBL_API_VERIFY_POINTER(pHandler);
    if(pHandler->pFnCallback &&
       (pHandler->monitoredEvents & type))
    {
        GBL_API_VERIFY(pHandler->pFnCallback(pHandler->pUserdata, hCtx, hDev, hPeripheral, type, pData, dataSize));
    }

    GBL_API_END();
}


inline EVMU_RESULT   evmuContextEventEmit_(EvmuContext hContext,
                                           EvmuDevice hDevice,
                                           EvmuPeripheral hPeripheral,
                                           EvmuEnum type,
                                           const EvmuEvent* pData,
                                           EvmuSize dataSize)
{
    EVMU_API_BEGIN(hCtx);

    if(hPeripheral && !hDev) {
        hDevice = hPeripheral->pDevice;
        GBL_API_VERIFY_POINTER(hDev);
    }

    if(hDevice && !hContext) {
        hContext = hDevice->pContext;
        GBL_API_VERIFY_POINTER(hContext);
        GBL_API_ACCUM(evmuEventHandlerEmit_(&hDevice->eventHandler, hContext, hDevice, hPeripheral, type, pData, dataSize));
    }

    GBL_API_VERIFY(hContext);
    GBL_API_ACCUM(evmuEventHandlerEmit_(&hContext->eventHandler, hContext, hDevice, hPeripheral, type, pData, dataSize));

    EVMU_API_END();
}


inline EVMU_RESULT evmuContextDeviceAdd_(EvmuContext_* pCtx, EvmuDevice_* pDev) {
    EVMU_API_BEGIN(pCtx);
    GBL_API_VERIFY_HANDLE(pDev);

    GBL_VERIFY(gblVectorPushBack(&pCtx->devices, pDev));
    GBL_VERIFY(evmuContextEventEmit_(pCtx, NULL, NULL, EVMU_EVENT_DEVICE_CREATE, pDev, sizeof(pDev)));

    EVMU_API_END();
}

inline EVMU_RESULT evmuContextDeviceRemove_(EvmuContext_* pCtx, EvmuDevice_* pDev) {
    EVMU_API_BEGIN(pCtx);

    EvmuSize index;
    GBL_VERIFY(gblVectorFind(&pCtx->devices, pDev, &index));
    GBL_API_RESULT_SET_CND(index != GBL_VECTOR_INDEX_INVALID, EVMU_RESULT_ERROR_INVALID_DEVICE);

    GBL_VERIFY(evmuContextEventEmit_(pCtx, NULL, NULL, EVMU_EVENT_DEVICE_REMOVE, pDev, sizeof(pDev)));

    GBL_VERIFY(gblVectorErase(&pCtx->devices, index));
    EVMU_API_END();
}




#ifdef __cplusplus
}
#endif

#endif // EVMU_CONTEXT__H

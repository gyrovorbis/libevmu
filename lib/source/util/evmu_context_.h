#ifndef EVMU_CONTEXT__H
#define EVMU_CONTEXT__H

#include <evmu/util/evmu_context.h>
#include <gimbal/gimbal_container.h>
#include <evmu/evmu_api.h>
#include "../hw/evmu_device_.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVMU_CONTEXT__DEVICE_INDEX_INVALID  UINT32_MAX

typedef struct EvmuContext_ {
    GblContext              baseGblContext;

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
#if 0
    GBL_API_VERIFY_POINTER(pHandler);
    if(pHandler->pFnCallback &&
       (pHandler->monitoredEvents & type))
    {
        GBL_API_VERIFY(pHandler->pFnCallback(pHandler->pUserdata, hCtx, hDev, hPeripheral, type, pData, dataSize));
    }
#endif
    EVMU_API_END();
}


inline EVMU_RESULT   evmuContextEventEmit_(EvmuContext hContext,
                                           EvmuDevice hDevice,
                                           EvmuPeripheral hPeripheral,
                                           EvmuEnum type,
                                           const EvmuEvent* pData,
                                           EvmuSize dataSize)
{
    EVMU_API_BEGIN(hCtx);
#if 0
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
#endif
    EVMU_API_END();
}


inline EVMU_RESULT evmuContextDeviceAdd_(EvmuContext_* pCtx, EvmuDevice_* pDev) {
    EVMU_API_BEGIN(pCtx);
#if 0
    GBL_API_VERIFY_HANDLE(pDev);

    GBL_VERIFY(gblVectorPushBack(&pCtx->devices, pDev));
    GBL_VERIFY(evmuContextEventEmit_(pCtx, NULL, NULL, EVMU_EVENT_DEVICE_CREATE, pDev, sizeof(pDev)));
#endif
    EVMU_API_END();
}

inline EVMU_RESULT evmuContextDeviceRemove_(EvmuContext_* pCtx, EvmuDevice_* pDev) {
    EVMU_API_BEGIN(pCtx);
#if 0
    EvmuSize index;
    GBL_VERIFY(gblVectorFind(&pCtx->devices, pDev, &index));
    GBL_API_RESULT_SET_CND(index != GBL_VECTOR_INDEX_INVALID, EVMU_RESULT_ERROR_INVALID_DEVICE);

    GBL_VERIFY(evmuContextEventEmit_(pCtx, NULL, NULL, EVMU_EVENT_DEVICE_REMOVE, pDev, sizeof(pDev)));

    GBL_VERIFY(gblVectorErase(&pCtx->devices, index));
#endif
    EVMU_API_END();
}




#ifdef __cplusplus
}
#endif

#endif // EVMU_CONTEXT__H

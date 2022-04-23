#ifndef EVMU_CONTEXT__H
#define EVMU_CONTEXT__H

#include <evmu/util/evmu_context.h>
#include <gimbal/gimbal_container.h>
#include "../hw/evmu_device_.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct EvmuContext_ {
    struct GblContext_      gblContext;
    GblVector               devices;
} EvmuContext_;


GBL_INLINE EVMU_RESULT evmuContextDeviceAdd_(struct EvmuContext_* pCtx, EvmuDevice_* pDev) {
    EVMU_API_BEGIN(pCtx);
    EVMU_API_VERIFY_DEVICE(pDev);
    EVMU_API_CALL(gblVectorPushBack(&pCtx->devices, pDev));
    EVMU_API_END();
}

GBL_INLINE EVMU_RESULT evmuContextDeviceRemove_(struct EvmuContext_* pCtx, EvmuDevice_* pDev) {
    EvmuSize size = 0;
    EvmuBool found = EVMU_FALSE;
    EVMU_API_BEGIN(pCtx);
    EVMU_API_VERIFY_DEVICE(pDev);
    EVMU_API_CALL(gblVectorSize(&pCtx->devices, &size));
    for(GblSize d = 0; d < size; ++d) {
        EvmuDevice hDevice = GBL_HANDLE_INVALID;
        EVMU_API_CALL(gblVectorAt(&pCtx->devices, d, (void**)&hDevice));
        if(hDevice == pDev) {
            EVMU_API_CALL(gblVectorErase(&pCtx->devices, d, 1));
            found = EVMU_TRUE;
            break;
        }
    }
    EVMU_API_VERIFY_EXPRESSION(found, "Device %p not found!", pDev);
    EVMU_API_END();
}





#if 0

#define FOREACH_PERIPHERAL(DEVICE, INDEX, POINTER)              \
    EvmuPeripheral_* POINTER = NULL;                            \
    for(uint32_t INDEX = 0;                                     \
        INDEX < peripheralCount_(DEVICE);                       \
        POINTER = evmuDevicePeripheral_(DEVICE, INDEX++))


GBL_API_INLINE GblSize gblVectorSize_()

GBL_API_VECTOR_SIZE(gblVec) \

const GblStackFrame*, pFrame

#define GBL_EXT_FN_DEFINE_(prefixName, ...) \
    GBL_API gblExt##prefixName (GBL_MAP_LIST(GBL_DECL_VAR_PAIR, __VA_ARGS__)) { \
        GBL_ASSERT(pFrame); \
        return gblContext##prefixName (pFrame->hContext, GBL_MAP_LIST(GBL_DECL_VAR_PAIR_NAME, __VA_ARGS__)); \
    }


GBL_API__CALL_DECLARE(retPair, apiFunction, ...) \
    GBL_MAYBE_UNUSED GBL_INLINE GBL_DECL_VAR_PAIR_TYPE(retPair) apiFunction##_ (const GblStackFrame* pFrame, GBL_MAP_LIST(GBL_DECL_VAR_PAIR, __VA_ARGS__)) { \

}

#define GBL_API_INLINE(MethodPrefix, ReturnType, ...) \
    GBL_INLINE ReturnType GBL_API_INLINE_##MethodPrefix##_(GBL_API_FRAME_DECLARE, SrcLoc srcLoc, ##__VA_ARGS__) { \
        ReturnType GBL_API_INLINE_RETURN_VALUE_NAME;

#define GBL_API_INLINE_BEGIN(InitialRetValue) \
        GBL_API_INLINE_RETVAL() = InitialRetValue; \
        GBL_API_SOURCE_LOC_PUSH(srcLoc);


#define GBL_API_INLINE_END()                \
        goto GBL_API_END_LABEL;             \
        GBL_API_END_LABEL: do ; while(0);   \
    }

#define GBL_API_INLINE_RETURN() \
    GBL_API_SOURCE_POP();  \
    return GBL_API_INLINE_RETVAL()


GBL_MAYBE_UNUSED GBL_API_INLINE(MALLOC, void*, GblSize size, GblSize align, const char* pDebugStr) {
    GBL_API_INLINE_BEGIN(GBL_NULL);
    GBL_ASSERT(size % align == 0);
    GBL_API_EXT(MALLOC, size, align, pDebugStr, &GBL_API_INLINE_RETVAL());
    GBL_API_INLINE_END();
    // modify/set return value based on result
    GBL_API_INLINE_RETURN();
}




GBL_VECTOR_FOREACH_KV_PAIR(it, gblVec) \
for( struct { GblSize i; void* pData; } it = { 0, NULL}; \
    it.k < GBL_API_INLINE_CALL(gblVectorSize(gblVec, 0)); \



    EVMU_API_CALL(gblVectorAt))

#define GBL_VECTOR_FOREACH(gblVec, it) \
    for(struct { })

    do {
        GblSize size = 0;
        for(GblSize i = 0; i < size; ++i) {
            GBL_API_CALL(gblVectorAt((gblVec), i, (void**)(it)));

        }

    } while(0)
#endif


#ifdef __cplusplus
}
#endif

#endif // EVMU_CONTEXT__H

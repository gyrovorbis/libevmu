#include <evmu/util/evmu_context.h>
#include <evmu/hw/evmu_device.h>
#include "evmu_context_.h"

EVMU_API    evmuContextDeviceCount(EvmuContext hCtx,
                                   uint32_t* pCount)
{
    EVMU_API_BEGIN(hCtx);
    GBL_API_VERIFY_POINTER(pCount);

    GBL_API_VERIFY(gblVectorSize(hCtx->devices, pCount));

    GBL_API_END();
}

EVMU_API     evmuContextDevice(EvmuContext hCtx,
                               uint32_t index,
                               EvmuDevice* phDevice)
{
    EVMU_API_BEGIN(hCtx);
    GBL_API_VERIFY_POINTER(hDevice);

    GBL_API_VERIFY(gblVectorAt(hCtx->devices, index, phDevice));

    GBL_API_END();

}
EVMU_API    evmuContextUserdata(EvmuContext hCtx,
                                void** ppUserdata)
{
    EVMU_API_BEGIN(hCtx);
    GBL_API_VERIFY_POINTER(ppUserdata);

    *ppUserdata = hCtx->createInfo.baseInfo.pUserdata;

    GBL_API_END();
}

EVMU_API evmuContextEventHandler(EvmuContext hCtx, EvmuEventHandler* pHandler) {
    GBL_API_BEGIN(hCtx);
    GBL_API_VERIFY_POINTER(pHandler);
    memcpy(pHandler, &hCtx->createInfo.eventHandler, sizeof(EvmuEventHandler));
    GBL_API_END();
}
EVMU_API evmuContextEventHandlerSet(EvmuContext hCtx, const EvmuEventHandler* pHandler) {
    GBL_API_BEGIN(hCtx);
    GBL_API_VERIFY_POINTER(pHandler);
    memcpy(&hCtx->createInfo.eventHandler, pHandler, sizeof(EvmuEventHandler));
    GBL_API_END();
}

EVMU_API evmuContextInit(EvmuContext hCtx, EvmuContextCreateInfo* pInfo) {
    if(!hCtx) {

    }

    EVMU_RESULT_CTX_FOLD(pCtx, VALIDATE_HANDLE);
    EVMU_RESULT result = EVMU_RESULT_SUCCESS;

    memset(pCtx, 0, sizeof(EvmuContext));

    if(pInfo) {
        memcpy(&pCtx->createInfo, pInfo, sizeof(EvmuContextCreateInfo));
    }

    gblVectorInit(&hCtx->devices, hCtx, sizeof(EvmuContext), NULL, 0, 0);
    memcpy(&hCtx->eventHandler, &pInfo->eventHandler, sizeof(EvmuEventHandler));

    if(!EVMU_CTX_UD(pCtx)) {
        void* pPtr = NULL;
        result = evmuMalloc_(pCtx, sizeof(CtxDefaultUserdata_), 1, &pPtr);
        if(EVMU_RESULT_SUCCESS(result)) {
            memset(pPtr, 0, sizeof(CtxDefaultUserdata_));
            EVMU_CTX_UD(pCtx) = pPtr;
        }
    }

    GBL_API_VERIFY(evmuContextEventEmit_(hCtx, NULL, NULL, EVMU_EVENT_CONTEXT_CREATE, hCtx, sizeof(hCtx)));

    EVMU_API_END();
}

EVMU_API evmuContextDeinit(EvmuContext hCtx) {
    EVMU_API_BEGIN(hCtx);
    if(pCtx) {

         GBL_API_VERIFY(evmuContextEventEmit_(hCtx, NULL, NULL, EVMU_EVENT_CONTEXT_DESTROY, hCtx, sizeof(hCtx)));
        GBL_API_RESULT_ACCUM(gblVectorUninit(&hCtx->devices));
    }
    EVMU_API_END();
}


EVMU_API evmuContextUpdate(EvmuContext hCtx, EvmuTicks ticks) {
    GBL_API_BEGIN(hCtx);

    GBL_API_RETURN_CND(hCtx->deviceCount);
    GBL_API_RETURN_CND(hCtx->ticks);

    // THIS IS NOT IN PARALLEL!!!
    for(uint32_t d = 0; d < hCtx->deviceCount; ++d) {
        EVMU_RESULT result = evmuDeviceUpdate(hCtx->ppDevices[d], ticks);
        GBL_API_RESULT_SET_CND(!GBL_RESULT_SUCCESS(result), result);
    }

    GBL_API_END();
}

#if 0

#include <evmu/util/evmu_context.h>
#include <evmu/util/evmu_result.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define EVMU_LOG_LEVEL_WARNING_PREFIX       "! - "
#define EVMU_LOG_LEVEL_ERROR_PREFIX         "X - "
#define EVMU_LOG_LEVEL_DEBUG_PREFIX         "@ - "
#define EVMU_LOG_LEVEL_DEFAULT_PREFIX       ""
#define EVMU_LOG_LEVEL_TRUNCATED_PREFIX     "T - "
#define EVMU_LOG_LEVEL_TAB_PREFIX           '\t'
#define EVMU_LOG_WRITE_DEFAULT_BUFFER_SIZE  256

typedef struct CtxDefaultUserdata_ {
    uint32_t logStackDepth;
} CtxDefaultUserdata_;

CtxDefaultUserdata_* ctxDefaultUserdata_(const EVMUContext* pCtx) {
    CtxDefaultUserdata_* pUd = NULL;

    if(pCtx) {
        pUd = (CtxDefaultUserdata_*)&pCtx->createInfo.pUserdata;
        EVMU_RESULT_HANDLER(pCtx, pUd, INVALID_HANDLE, "Failed to retrieve default UD ptr from context!");
    }

    return pUd;
}


EVMU_API EVMU_RESULT evmuLogWriteDefault(const EVMUContext* pCtx,
                                const char* pFileName,
                                const char* pFunction,
                                uint64_t line,
                                EVMU_LOG_LEVEL level,
                                const char* pFormat,
                                ...)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFormat);
    EVMU_RESULT_VALIDATE_ARG(pCtx, level, level >= 0 && level <= EVMU_LOG_LEVEL_ERROR);

    const FILE* pFile = (level >= EVMU_LOG_LEVEL_ERROR)?
                stderr : stdout;

    const char* pPrefix = NULL;

    EVMU_RESULT result = EVMU_RESULT_SUCCESS;

    switch(level) {
    case EVMU_LOG_LEVEL_WARNING: pPrefix = EVMU_LOG_LEVEL_WARNING_PREFIX; break;
    case EVMU_LOG_LEVEL_ERROR:   pPrefix = EVMU_LOG_LEVEL_ERROR_PREFIX; break;
    case EVMU_LOG_LEVEL_DEBUG:   pPrefix = EVMU_LOG_LEVEL_DEBUG_PREFIX; break;
    default:                     pPrefix = EVMU_LOG_LEVEL_DEFAULT_PREFIX; break;
    }

    EVMU_VA_SNPRINTF(pFormat);
    if(vsnprintfBytes > (int)sizeof(buffer)) {
        pPrefix = EVMU_LOG_LEVEL_TRUNCATED_PREFIX;
        result = EVMU_RESULT_INCOMPLETE;
    }

    char tabBuff[EVMU_LOG_WRITE_DEFAULT_BUFFER_SIZE];

    CtxDefaultUserdata_* pUd = ctxDefaultUserdata_(pCtx);

    EVMU_RESULT_ASSIGN_GOTO(pCtx,
                            pUd,
                            INVALID_HANDLE,
                            end);
    memset(tabBuff, EVMU_LOG_LEVEL_TAB_PREFIX, pUd->logStackDepth);
    tabBuff[pUd->logStackDepth] = '\0';

    EVMU_RESULT_ASSIGN_GOTO(pCtx,
                            (fprintf(pFile, "%s%s%s [%s:%zu %s]\n", tabBuff, pPrefix, buffer, pFileName, pFunction, line) >= 0),
                            FILE_WRITE,
                            end);

    EVMU_RESULT_ASSIGN_GOTO(pCtx, fflush(pFile) == 0, FILE_WRITE, end);

done:
    return result;
}


EVMU_API EVMU_RESULT evmuLogWrite_(const EVMUContext* pCtx,
                                   const char* pFile, const char* pFunction, uint64_t line,
                                   EVMU_LOG_LEVEL level, const char* pFormat, ...)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFormat);
    EVMU_RESULT_VALIDATE_ARG(pCtx, level, level >= 0 && level <= EVMU_LOG_LEVEL_ERROR);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFile);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFunction);

    EVMU_VA_SNPRINTF(pFormat);
    const EVMULogWriteFn pWrite = pCtx->createInfo.logCallbacks.pWriteFn;

    if(pWrite) {
        return pWrite(EVMU_CTX_UD(pCtx),
                    pFile, pFunction, line,
                    level, buffer);
    } else {
        return evmuLogWriteDefault(pCtx, pFile, pFunction, line, level, buffer);
    }
}

EVMU_API EVMU_RESULT evmuLogPush_(const EVMUContext* pCtx)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);

    const EVMULogPushFn pPush = pCtx->createInfo.logCallbacks.pPushFn;

    if(pPush) {
        return pPush(EVMU_CTX_UD(pCtx));
    } else {
        CtxDefaultUserdata_* pUd = ctxDefaultUserdata_(pCtx);

        EVMU_RESULT_ASSIGN_GOTO(pCtx,
                                pUd,
                                INVALID_HANDLE,
                                end);

        const uint32_t result = pUd->logStackDepth + 1;
        EVMU_RESULT_ASSIGN_GOTO(pCtx, result > pUd->logStackDepth, LOG_PUSH_OVERFLOW, done, "Overflowing log stack!");
        ++pUd->logStackDepth;
    }
done:
    return result;
}


EVMU_API EVMU_RESULT evmuLogPop_(const EVMUContext* pCtx,
                                    uint32_t count)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, count);

    EVMU_RESULT result = EVMU_RESULT_SUCCESS;
    const EVMULogPopFn pPop = pCtx->createInfo.logCallbacks.pPopFn;

    if(pPop) {
        return pPop(EVMU_CTX_UD(pCtx), count);
    } else {
        CtxDefaultUserdata_* pUd = ctxDefaultUserdata_(pCtx);

        EVMU_RESULT_ASSIGN_GOTO(pCtx,
                                pUd,
                                INVALID_HANDLE,
                                end);
        EVMU_RESULT_ASSIGN_GOTO(pCtx, pUd->logStackDepth == 0, LOG_POP_UNDERFLOW, done, "Underflowing log stack!");

        --pUd->logStackDepth;

    }
done:
    return result;
}


EVMU_API EVMU_RESULT evmuMalloc_(const EVMUContext* pCtx,
                                    size_t size,
                                    size_t alignment,
                                    void** ppData)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, ppData);
    EVMU_RESULT_VALIDATE_ARG(size);
    EVMU_RESULT_VALIDATE_ARG(alignment);

    const EVMUMallocFn pMalloc = pCtx->createInfo.allocCallbacks.pMallocFn;
    if(pMalloc) {
        return pMalloc(EVMU_CTX_UD(pCtx), size, alignment, ppData);
    } else {
        EVMU_LOG(WARNING, "Malloc %zu bytes ignoring alignment: %zu", size, alignment);
        *ppData = malloc(size);
        EVMU_RESULT_RETURN(pCtx, ppData, MEM_MALLOC);
    }
    return EVMU_RESULT_SUCCESS;
}

EVMU_API EVMU_RESULT evmuRealloc_(const EVMUContext* pCtx,
                                     void* pExistingData,
                                     size_t newSize,
                                     size_t newAlignment,
                                     void* ppNewData)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pExistingData);
    EVMU_RESULT_VALIDATE_ARG(pCtx, newSize);
    EVMU_RESULT_VALIDATE_ARG(pCtx, newAlignment);
    EVMU_RESULT_VALIDATE_ARG(pCtx, ppNewData);

    const EVMUReallocFn pRealloc = pCtx->createInfo.allocCallbacks.pReallocFn;
    if(pRealloc) {
        return pRealloc(EVMU_CTX_UD(pCtx), pExistingData, newSize, newAlignment, ppNewData);
    } else {
        EVMU_LOG(WARNING, "Realloc %zu bytes ignoring alignment: %zu", newSize, newAlignment);
        *ppData = realloc(pExistingData, newSize);
        EVMU_RESULT_RETURN(pCtx, ppData, MEM_REALLOC);
    }
    return EVMU_RESULT_SUCCESS;
}


EVMU_API EVMU_RESULT evmuFree_(const EVMUContext* pCtx,
                                  void* pData)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    const EVMUFreeFn pFree = pCtx->createInfo.allocCallbacks.pFreeFn;
    if(pFree) {
        return pFree(EVMU_CTX_UD(pCtx), pData);
    } else {
        free(pData);
    }
    return EVMU_RESULT_SUCCESS;
}




typedef struct CtxDefaultUserdata_ {
    uint32_t logStackDepth;
} CtxDefaultUserdata_;

CtxDefaultUserdata_* ctxDefaultUserdata_(const EVMUContext* pCtx) {
    CtxDefaultUserdata_* pUd = NULL;

    if(pCtx) {
        pUd = (CtxDefaultUserdata_*)&pCtx->createInfo.pUserdata;
        EVMU_RESULT_CTX_FOLD(pCtx, HANDLER, pUd, INVALID_HANDLE,  "Failed to retrieve default UD ptr from context!");
    }

    return pUd;
}


EVMU_API EVMU_RESULT evmuLogWriteDefault(const EVMUContext* pCtx,
                                const char* pFileName,
                                const char* pFunction,
                                uint64_t line,
                                EVMU_LOG_LEVEL level,
                                const char* pFormat,
                                ...)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_CTX_FOLD(pCtx, VALIDATE_ARG, pFormat);
    EVMU_RESULT_CTX_FOLD(pCtx, VALIDATE_ARG, level, level >= 0 && level <= EVMU_LOG_LEVEL_ERROR);

    FILE* const pFile = (level >= EVMU_LOG_LEVEL_ERROR)?
                stderr : stdout;

    const char* pPrefix = NULL;

    EVMU_RESULT result = EVMU_RESULT_SUCCESS;

    switch(level) {
    case EVMU_LOG_LEVEL_WARNING: pPrefix = "! - "; break;
    case EVMU_LOG_LEVEL_ERROR:   pPrefix = "X - "; break;
    case EVMU_LOG_LEVEL_DEBUG:   pPrefix = "@ - "; break;
    default:                     pPrefix = "";     break;
    }

    EVMU_VA_SNPRINTF(pFormat);
    if(vsnprintfBytes > (int)sizeof(buffer)) {
        pPrefix = "T - "; //Truncated prefix!
        result = EVMU_RESULT_INCOMPLETE;
    }

    char tabBuff[EVMU_LOG_WRITE_DEFAULT_BUFFER_SIZE] = { '\t' };

    CtxDefaultUserdata_* pUd = ctxDefaultUserdata_(pCtx);

    EVMU_RESULT_CTX_FOLD(pCtx,
                         ASSIGN_GOTO,
                            pUd,
                            INVALID_HANDLE,
                            done,
                         "Failed to retrieve default context userdata!");

    tabBuff[pUd->logStackDepth] = '\0';

    EVMU_RESULT_ASSIGN_GOTO(pCtx,
                            (fprintf(pFile, "%s%s%s [%s:%zu %s]\n", tabBuff, pPrefix, buffer, pFileName, line, pFunction) >= 0),
                            FILE_WRITE,
                            done);

    EVMU_RESULT_ASSIGN_GOTO(pCtx, fflush(pFile) == 0, FILE_WRITE, done);

done:
    return result;
}


EVMU_API EVMU_RESULT evmuLogWrite_(const EVMUContext* pCtx,
                                   const char* pFile, const char* pFunction, uint64_t line,
                                   EVMU_LOG_LEVEL level, const char* pFormat, ...)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFormat);
    EVMU_RESULT_VALIDATE_ARG(pCtx, level, level >= 0 && level <= EVMU_LOG_LEVEL_ERROR);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFile);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pFunction);

    EVMU_VA_SNPRINTF(pFormat);
    const EVMULogWriteFn pWrite = pCtx->createInfo.logCallbacks.pWriteFn;

    if(pWrite) {
        return pWrite(EVMU_CTX_UD(pCtx),
                    pFile, pFunction, line,
                    level, buffer);
    } else {
        return evmuLogWriteDefault(pCtx, pFile, pFunction, line, level, buffer);
    }
}

EVMU_API EVMU_RESULT evmuLogPush_(const EVMUContext* pCtx)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);

    EVMU_RESULT result = EVMU_RESULT_SUCCESS;
    const EVMULogPushFn pPush = pCtx->createInfo.logCallbacks.pPushFn;

    if(pPush) {
        return pPush(EVMU_CTX_UD(pCtx));
    } else {        CtxDefaultUserdata_* pUd = ctxDefaultUserdata_(pCtx);

        EVMU_RESULT_ASSIGN_GOTO(pCtx,
                                pUd,
                                INVALID_HANDLE,
                                done,
                                "Failed to retrieve default context userdata!");

        const uint32_t newDepth = pUd->logStackDepth + 1;
        EVMU_RESULT_ASSIGN_GOTO(pCtx, (newDepth > pUd->logStackDepth), LOG_PUSH_OVERFLOW, done, "Overflowing log stack!");
        ++pUd->logStackDepth;
    }
done:
    return result;
}


EVMU_API EVMU_RESULT evmuLogPop_(const EVMUContext* pCtx,
                                    uint32_t count)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, count);

    EVMU_RESULT result = EVMU_RESULT_SUCCESS;
    const EVMULogPopFn pPop = pCtx->createInfo.logCallbacks.pPopFn;

    if(pPop) {
        return pPop(EVMU_CTX_UD(pCtx), count);
    } else {
        CtxDefaultUserdata_* pUd = ctxDefaultUserdata_(pCtx);

        EVMU_RESULT_ASSIGN_GOTO(pCtx,
                                pUd,
                                INVALID_HANDLE,
                                end);
        EVMU_RESULT_ASSIGN_GOTO(pCtx, pUd->logStackDepth == 0, LOG_POP_UNDERFLOW, end, "Underflowing log stack!");

        --pUd->logStackDepth;

    }
end:
    return result;
}


EVMU_API EVMU_RESULT evmuMalloc_(const EVMUContext* pCtx,
                                    size_t size,
                                    size_t alignment,
                                    void** ppData)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, ppData);
    EVMU_RESULT_VALIDATE_ARG(size);
    EVMU_RESULT_VALIDATE_ARG(alignment);

    const EVMUMallocFn pMalloc = pCtx->createInfo.allocCallbacks.pMallocFn;
    if(pMalloc) {
        return pMalloc(EVMU_CTX_UD(pCtx), size, alignment, ppData);
    } else {
        EVMU_LOG(WARNING, "Malloc %zu bytes ignoring alignment: %zu", size, alignment);
        *ppData = malloc(size);
        EVMU_RESULT_RETURN(pCtx, ppData, MEM_MALLOC);
    }
    return EVMU_RESULT_SUCCESS;
}

EVMU_API EVMU_RESULT evmuRealloc_(const EVMUContext* pCtx,
                                     void* pExistingData,
                                     size_t newSize,
                                     size_t newAlignment,
                                     void* ppNewData)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    EVMU_RESULT_VALIDATE_ARG(pCtx, pExistingData);
    EVMU_RESULT_VALIDATE_ARG(pCtx, newSize);
    EVMU_RESULT_VALIDATE_ARG(pCtx, newAlignment);
    EVMU_RESULT_VALIDATE_ARG(pCtx, ppNewData);

    const EVMUReallocFn pRealloc = pCtx->createInfo.allocCallbacks.pReallocFn;
    if(pRealloc) {
        return pRealloc(EVMU_CTX_UD(pCtx), pExistingData, newSize, newAlignment, ppNewData);
    } else {
        EVMU_LOG(WARNING, "Realloc %zu bytes ignoring alignment: %zu", newSize, newAlignment);
        *ppData = realloc(pExistingData, newSize);
        EVMU_RESULT_RETURN(pCtx, ppData, MEM_REALLOC);
    }
    return EVMU_RESULT_SUCCESS;
}


EVMU_API EVMU_RESULT evmuFree_(const EVMUContext* pCtx,
                                  void* pData)
{
    EVMU_RESULT_VALIDATE_HANDLE(pCtx);
    const EVMUFreeFn pFree = pCtx->createInfo.allocCallbacks.pFreeFn;
    if(pFree) {
        return pFree(EVMU_CTX_UD(pCtx), pData);
    } else {
        free(pData);
    }
    return EVMU_RESULT_SUCCESS;
}

#endif

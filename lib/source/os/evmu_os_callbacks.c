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



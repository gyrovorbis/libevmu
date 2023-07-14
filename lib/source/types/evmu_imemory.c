#include <evmu/types/evmu_imemory.h>
#include <evmu/types/evmu_peripheral.h>
#include "evmu_marshal_.h"

EVMU_EXPORT EvmuWord EvmuIMemory_readByte(const EvmuIMemory* pSelf,
                                          EvmuAddress        address)
{
    EvmuWord value = 0;
    size_t bytes = 1;
    EvmuIMemory_readBytes(pSelf, address, &value, &bytes);
    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuIMemory_writeByte(EvmuIMemory* pSelf,
                                              EvmuAddress  address,
                                              EvmuWord     value)
{
    size_t bytes = 1;
    return EvmuIMemory_writeBytes(pSelf, address, &value, &bytes);
}

EVMU_EXPORT EVMU_RESULT EvmuIMemory_readBytes(const EvmuIMemory* pSelf,
                                              EvmuAddress        base,
                                              void*              pData,
                                              size_t*            pBytes)
{
    GBL_CTX_BEGIN(NULL);

    const size_t capacity = EVMU_IMEMORY_GET_CLASS(pSelf)->capacity;
    if(base < capacity && base + *pBytes > capacity) {
        *pBytes -= base + *pBytes - capacity - 1;
        GBL_CTX_RECORD_SET(GBL_RESULT_TRUNCATED);
    }

    GBL_CTX_VERIFY(*pBytes,
                   GBL_RESULT_ERROR_OUT_OF_RANGE);


    GBL_INSTANCE_VCALL(EvmuIMemory, pFnRead, pSelf, base, pData, pBytes);

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuIMemory_writeBytes(EvmuIMemory* pSelf,
                                               EvmuAddress  base,
                                               const void*  pData,
                                               size_t*      pBytes)
{
    GBL_CTX_BEGIN(NULL);

    const size_t capacity = EVMU_IMEMORY_GET_CLASS(pSelf)->capacity;
    if(base < capacity && base + *pBytes > capacity) {
        *pBytes -= base + *pBytes - capacity - 1;
        GBL_CTX_RECORD_SET(GBL_RESULT_TRUNCATED);
    }

    GBL_CTX_VERIFY(*pBytes,
                   GBL_RESULT_ERROR_OUT_OF_RANGE);

    GBL_INSTANCE_VCALL(EvmuIMemory, pFnWrite, pSelf, base, pData, pBytes);

    GBL_CTX_END();
}

static EVMU_RESULT EvmuIMemory_write_(EvmuIMemory* pSelf,
                                      EvmuAddress  base,
                                      const void*  pData,
                                      size_t*      pBytes)
{
    GblObject_setProperty(GBL_OBJECT(pSelf), "dataChanged", GBL_TRUE);
    GBL_EMIT(pSelf, "dataChange", base, *pBytes, pData);
    return GBL_RESULT_SUCCESS;
}

static GBL_RESULT EvmuIMemoryClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_CTX_BEGIN(NULL);

    EVMU_IMEMORY_CLASS(pClass)->pFnWrite = EvmuIMemory_write_;

    if(!GblType_classRefCount(EVMU_IMEMORY_TYPE)) {
        GBL_PROPERTIES_REGISTER(EvmuIMemory);

        GBL_CTX_CALL(
            GblSignal_install(GblClass_typeOf(pClass),
                              "dataChange",
                               GblMarshal_CClosure_VOID__INSTANCE_UINT32_SIZE_POINTER,
                               3,
                               GBL_UINT32_TYPE,
                               GBL_SIZE_TYPE,
                               GBL_POINTER_TYPE));
    }

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuIMemory_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblType deps[1];

    static GblTypeInfo info = {
        .pFnClassInit    = EvmuIMemoryClass_init_,
        .classSize       = sizeof(EvmuIMemoryClass),
        .pDependencies   = deps,
        .dependencyCount = 1
    };

    if(type == GBL_INVALID_TYPE) {
        deps[0] = EVMU_PERIPHERAL_TYPE;

        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuIMemory"),
                                      GBL_INTERFACE_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC |
                                      GBL_TYPE_FLAG_ABSTRACT);
    }

    return type;
}

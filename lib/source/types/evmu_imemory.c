#include <evmu/types/evmu_imemory.h>
#include <evmu/types/evmu_peripheral.h>
#include "evmu_marshal_.h"

#define EVMU_IMEMORY_FILL_BUFFER_SIZE_  512

EVMU_EXPORT size_t EvmuIMemory_capacity(const EvmuIMemory* pSelf) {
    return EVMU_IMEMORY_GET_CLASS(pSelf)->capacity;
}

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


    GBL_VCALL(EvmuIMemory, pFnRead, pSelf, base, pData, pBytes);

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuIMemory_fillBytes(EvmuIMemory* pSelf,
                                              EvmuAddress  base,
                                              size_t       regionSize,
                                              const void*  pData,
                                              size_t       dataBytes)
{
    GBL_CTX_BEGIN(NULL);

    const size_t capacity = EvmuIMemory_capacity(pSelf);

    GBL_CTX_VERIFY(base < capacity,
                   GBL_RESULT_ERROR_OUT_OF_RANGE);

    size_t byte = 0;
    while(byte < regionSize) {
        const EvmuAddress chunkAddr = base + byte;
        const size_t      remaining = regionSize - byte;
              size_t      chunkSize = remaining >= dataBytes?
                                      dataBytes : remaining;

        GBL_CTX_VERIFY_CALL(
            EvmuIMemory_writeBytes(pSelf, chunkAddr, pData, &chunkSize)
        );

        byte += chunkSize;
    }

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

    GBL_VCALL(EvmuIMemory, pFnWrite, pSelf, base, pData, pBytes);

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

static GBL_RESULT EvmuIMemoryClass_init_(GblClass* pClass, const void* pUd) {
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

        type = GblType_register(GblQuark_internStringStatic("EvmuIMemory"),
                                      GBL_INTERFACE_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC |
                                      GBL_TYPE_FLAG_ABSTRACT);
    }

    return type;
}

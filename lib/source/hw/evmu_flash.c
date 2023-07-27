#include <evmu/hw/evmu_flash.h>
#include <evmu/hw/evmu_ram.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_address_space.h>
#include "evmu_flash_.h"

EVMU_EXPORT EvmuAddress EvmuFlash_programAddress(EVMU_FLASH_PROGRAM_STATE state) {
    static const EvmuAddress prgAddressLut[] = {
        EVMU_FLASH_PROGRAM_STATE_0_ADDRESS,
        EVMU_FLASH_PROGRAM_STATE_1_ADDRESS,
        EVMU_FLASH_PROGRAM_STATE_2_ADDRESS
    };

    GBL_ASSERT(state < EVMU_FLASH_PROGRAM_STATE_COUNT,
               "Invalid flash program state!");

    return prgAddressLut[state];
}

EVMU_EXPORT EvmuWord EvmuFlash_programValue(EVMU_FLASH_PROGRAM_STATE state) {
    static const EvmuWord prgValueLut[] = {
        EVMU_FLASH_PROGRAM_STATE_0_VALUE,
        EVMU_FLASH_PROGRAM_STATE_1_VALUE,
        EVMU_FLASH_PROGRAM_STATE_2_VALUE
    };

    GBL_ASSERT(state < EVMU_FLASH_PROGRAM_STATE_COUNT,
               "Invalid flash program state!");

    return prgValueLut[state];
}

EVMU_EXPORT EVMU_FLASH_PROGRAM_STATE EvmuFlash_programState(const EvmuFlash* pSelf) {
    return EVMU_FLASH_(pSelf)->prgState;
}

EVMU_EXPORT size_t EvmuFlash_programBytes(const EvmuFlash* pSelf) {
    return EVMU_FLASH_(pSelf)->prgBytes;
}

EVMU_EXPORT EvmuAddress EvmuFlash_targetAddress(const EvmuFlash* pSelf) {
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));

    return ((EvmuRam_readData(pDevice->pRam, EVMU_ADDRESS_SFR_FPR)
                                                 & EVMU_SFR_FPR_ADDR_MASK) << 16) |
            (EvmuRam_readData(pDevice->pRam, EVMU_ADDRESS_SFR_TRL)   << 8)  |
             EvmuRam_readData(pDevice->pRam, EVMU_ADDRESS_SFR_TRL);
}

EVMU_EXPORT GblBool EvmuFlash_unlocked(const EvmuFlash* pSelf) {
    return EvmuFlash_programState(pSelf) == EVMU_FLASH_PROGRAM_STATE_COUNT;
}

EVMU_EXPORT EvmuWord EvmuFlash_readByte(const EvmuFlash* pSelf, EvmuAddress address) {
    EvmuWord value = 0;
    size_t   bytes = 1;

    EvmuFlash_readBytes(pSelf, address, &value, &bytes);

    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuFlash_writeByte(EvmuFlash*  pSelf,
                                            EvmuAddress address,
                                            EvmuWord    value)
{
    size_t bytes = 1;

    return EvmuFlash_writeBytes(pSelf, address, &value, &bytes);

}

EVMU_EXPORT EVMU_RESULT EvmuFlash_readBytes(const EvmuFlash* pSelf,
                                            EvmuAddress      address,
                                            void*            pBuffer,
                                            size_t*          pBytes)
{
    return EvmuIMemory_readBytes(EVMU_IMEMORY(pSelf),
                                 address,
                                 pBuffer,
                                 pBytes);
}


EVMU_EXPORT EVMU_RESULT EvmuFlash_writeBytes(EvmuFlash*   pSelf,
                                             EvmuAddress  address,
                                             const void*  pBuffer,
                                             size_t*      pBytes)
{
    return EvmuIMemory_writeBytes(EVMU_IMEMORY(pSelf),
                                  address,
                                  pBuffer,
                                  pBytes);
}

static EVMU_RESULT EvmuFlash_IMemory_readBytes_(const EvmuIMemory* pSelf,
                                                EvmuAddress        address,
                                                void*              pBuffer,
                                                size_t*            pBytes)
{
    GBL_CTX_BEGIN(NULL);
    EvmuFlash_*  pSelf_  = EVMU_FLASH_(pSelf);

    if(!GBL_RESULT_SUCCESS(
        GblByteArray_read(pSelf_->pStorage, address, *pBytes, pBuffer)
    )) {
        *pBytes = 0;
        GBL_CTX_VERIFY_LAST_RECORD();
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuFlash_IMemory_writeBytes_(EvmuIMemory* pSelf,
                                                 EvmuAddress  address,
                                                 const void*  pBuffer,
                                                 size_t*      pBytes)
{
    // Clear our call record
    GBL_CTX_BEGIN(NULL);

    // Cache private data
    EvmuFlash_*  pSelf_   = EVMU_FLASH_(pSelf);

    // Attempt to write to flash byte array
    if(!GBL_RESULT_SUCCESS(
            GblByteArray_write(pSelf_->pStorage, address, *pBytes, pBuffer)
    )) {
        // Return 0 if the write failed, proxy last error
        *pBytes = 0;
        GBL_CTX_VERIFY_LAST_RECORD();
    }

    GBL_VCALL_DEFAULT(EvmuIMemory, pFnWrite, pSelf, address, pBuffer, pBytes);

    // End call record, return result
    GBL_CTX_END();
}

static GBL_RESULT EvmuFlash_GblBox_destructor_(GblBox* pBox) {
    GBL_CTX_BEGIN(NULL);

    GblByteArray_unref(EVMU_FLASH_(pBox)->pStorage);
    GBL_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pBox);

    GBL_CTX_END();
}

static GBL_RESULT EvmuFlash_init_(GblInstance* pInstance) {
    GBL_CTX_BEGIN(NULL);

    EvmuFlash* pSelf   = EVMU_FLASH(pInstance);
    EvmuFlash_* pSelf_ = EVMU_FLASH_(pSelf);

    pSelf_->pStorage   = GblByteArray_create(
                            EVMU_IMEMORY_GET_CLASS(pSelf)->capacity
                         );

    memset(pSelf_->pStorage->pData, 0, pSelf_->pStorage->size);

    GBL_CTX_END();
}

static GBL_RESULT EvmuFlashClass_init_(GblClass* pClass, const void* pUd) {
    GBL_CTX_BEGIN(NULL);

    GBL_BOX_CLASS(pClass)     ->pFnDestructor = EvmuFlash_GblBox_destructor_;
    EVMU_IMEMORY_CLASS(pClass)->pFnRead       = EvmuFlash_IMemory_readBytes_;
    EVMU_IMEMORY_CLASS(pClass)->pFnWrite      = EvmuFlash_IMemory_writeBytes_;
    EVMU_IMEMORY_CLASS(pClass)->capacity      = EVMU_FLASH_SIZE;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuFlash_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblInterfaceImpl ifaces[] = {
        { .classOffset = offsetof(EvmuFlashClass, EvmuIMemoryImpl) }
    };

    static const GblTypeInfo info = {
        .pFnClassInit        = EvmuFlashClass_init_,
        .classSize           = sizeof(EvmuFlashClass),
        .pFnInstanceInit     = EvmuFlash_init_,
        .instanceSize        = sizeof(EvmuFlash),
        .instancePrivateSize = sizeof(EvmuFlash_),
        .pInterfaceImpls       = ifaces,
        .interfaceCount      = 1
    };

    if(type == GBL_INVALID_TYPE) {
        ifaces[0].interfaceType = EVMU_IMEMORY_TYPE;

        type = GblType_register(GblQuark_internStringStatic("EvmuFlash"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);

    }

    return type;
}

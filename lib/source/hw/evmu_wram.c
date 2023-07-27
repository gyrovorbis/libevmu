#include <evmu/hw/evmu_wram.h>
#include "evmu_wram_.h"
#include "evmu_ram_.h"

EVMU_EXPORT EvmuAddress EvmuWram_accessAddress(const EvmuWram* pSelf) {
    EvmuRam_* pRam_ = EVMU_WRAM_(pSelf)->pRam;

    return pRam_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] << 8 |
           pRam_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)];
}

EVMU_EXPORT EVMU_RESULT EvmuWram_setAccessAddress(EvmuWram* pSelf, EvmuAddress addr) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY(addr < EVMU_IMEMORY_GET_CLASS(pSelf)->capacity,
                   GBL_RESULT_ERROR_OUT_OF_RANGE,
                   "Attempt to set out-of-range WRAM access address: [%x]",
                   addr);

    EvmuRam_* pRam_ = EVMU_WRAM_(pSelf)->pRam;

    // Set high bit 9/bank number
    pRam_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] &= ~0x1;
    if(addr & 0x100)
        pRam_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] |= 0x1;

    //Set low byte
    pRam_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)] = addr & 0xff;

    GBL_CTX_END();
}

EVMU_EXPORT GblBool EvmuWram_mapleTransferring(const EvmuWram* pSelf) {
    return EVMU_WRAM_(pSelf)->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VSEL)] & EVMU_SFR_VSEL_ASEL_MASK;
}

EVMU_EXPORT EvmuWord EvmuWram_readByte(const EvmuWram* pSelf, EvmuAddress address) {
    EvmuWord value = 0;
    size_t   bytes = 1;

    EvmuWram_readBytes(pSelf, address, &value, &bytes);

    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuWram_writeByte(EvmuWram*  pSelf,
                                           EvmuAddress address,
                                           EvmuWord    value)
{
    size_t bytes = 1;

    return EvmuWram_writeBytes(pSelf, address, &value, &bytes);

}

EVMU_EXPORT EVMU_RESULT EvmuWram_readBytes(const EvmuWram* pSelf,
                                            EvmuAddress      address,
                                            void*            pBuffer,
                                            size_t*          pBytes)
{
    return EvmuIMemory_readBytes(EVMU_IMEMORY(pSelf),
                                 address,
                                 pBuffer,
                                 pBytes);
}


EVMU_EXPORT EVMU_RESULT EvmuWram_writeBytes(EvmuWram*   pSelf,
                                            EvmuAddress  address,
                                            const void*  pBuffer,
                                            size_t*      pBytes)
{
    return EvmuIMemory_writeBytes(EVMU_IMEMORY(pSelf),
                                  address,
                                  pBuffer,
                                  pBytes);
}


static EVMU_RESULT EvmuWram_IBehavior_reset_(EvmuIBehavior* pBehav) {
    GBL_CTX_BEGIN(NULL);
    EvmuWram_* pSelf_ = EVMU_WRAM_(pBehav);
    memset(pSelf_->pStorage->pData, 0, pSelf_->pStorage->size);
    GBL_CTX_END();
}

static EVMU_RESULT EvmuWram_IMemory_readBytes_(const EvmuIMemory* pSelf,
                                               EvmuAddress        address,
                                               void*              pBuffer,
                                               size_t*            pBytes)
{
    GBL_CTX_BEGIN(NULL);
    EvmuWram_*  pSelf_  = EVMU_WRAM_(pSelf);

    if(!GBL_RESULT_SUCCESS(
            GblByteArray_read(pSelf_->pStorage, address, *pBytes, pBuffer)
            )) {
        *pBytes = 0;
        GBL_CTX_VERIFY_LAST_RECORD();
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuWram_IMemory_writeBytes_(EvmuIMemory* pSelf,
                                                EvmuAddress  address,
                                                const void*  pBuffer,
                                                size_t*      pBytes)
{
    // Clear our call record
    GBL_CTX_BEGIN(NULL);

    // Cache private data
    EvmuWram_*  pSelf_   = EVMU_WRAM_(pSelf);

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

static GBL_RESULT EvmuWram_GblBox_destructor_(GblBox* pBox) {
    GBL_CTX_BEGIN(NULL);

    GblByteArray_unref(EVMU_WRAM_(pBox)->pStorage);
    GBL_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pBox);

    GBL_CTX_END();
}

static GBL_RESULT EvmuWram_init_(GblInstance* pInstance) {
    GBL_CTX_BEGIN(NULL);

    EvmuWram* pSelf   = EVMU_WRAM(pInstance);
    EvmuWram_* pSelf_ = EVMU_WRAM_(pSelf);

    pSelf_->pStorage   = GblByteArray_create(
                             EVMU_IMEMORY_GET_CLASS(pSelf)->capacity
                         );

    memset(pSelf_->pStorage->pData, 0, pSelf_->pStorage->size);

    GBL_CTX_END();
}

static GBL_RESULT EvmuWramClass_init_(GblClass* pClass, const void* pUd) {
    GBL_CTX_BEGIN(NULL);

    GBL_BOX_CLASS(pClass)       ->pFnDestructor = EvmuWram_GblBox_destructor_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset      = EvmuWram_IBehavior_reset_;
    EVMU_IMEMORY_CLASS(pClass)  ->pFnRead       = EvmuWram_IMemory_readBytes_;
    EVMU_IMEMORY_CLASS(pClass)  ->pFnWrite      = EvmuWram_IMemory_writeBytes_;
    EVMU_IMEMORY_CLASS(pClass)  ->capacity      = EVMU_WRAM_SIZE;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuWram_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblInterfaceImpl ifaces[] = {
        { .classOffset = offsetof(EvmuWramClass, EvmuIMemoryImpl) }
    };

    static const GblTypeInfo info = {
        .pFnClassInit        = EvmuWramClass_init_,
        .classSize           = sizeof(EvmuWramClass),
        .pFnInstanceInit     = EvmuWram_init_,
        .instanceSize        = sizeof(EvmuWram),
        .instancePrivateSize = sizeof(EvmuWram_),
        .pInterfaceImpls       = ifaces,
        .interfaceCount      = 1
    };

    if(type == GBL_INVALID_TYPE) {
        ifaces[0].interfaceType = EVMU_IMEMORY_TYPE;

        type = GblType_register(GblQuark_internStringStatic("EvmuWram"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);

    }

    return type;
}

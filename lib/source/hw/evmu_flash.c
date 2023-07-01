#include <evmu/hw/evmu_flash.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_address_space.h>
#include "evmu_flash_.h"

EVMU_EXPORT EvmuAddress EvmuFlash_programAddress(EVMU_FLASH_PROGRAM_STATE state) {
    static const EvmuAddress prgAddressLut[] = {
        EVMU_FLASH_PROGRAM_STATE_0_ADDRESS,
        EVMU_FLASH_PROGRAM_STATE_1_ADDRESS,
        EVMU_FLASH_PROGRAM_STATE_2_ADDRESS
    };

    GBL_ASSERT(state < EVMU_FLASH_PROGRAM_STATE_COUNT, "Invalid flash program state!");

    return prgAddressLut[state];
}

EVMU_EXPORT EvmuWord EvmuFlash_programValue(EVMU_FLASH_PROGRAM_STATE state) {
    static const EvmuWord prgValueLut[] = {
        EVMU_FLASH_PROGRAM_STATE_0_VALUE,
        EVMU_FLASH_PROGRAM_STATE_1_VALUE,
        EVMU_FLASH_PROGRAM_STATE_2_VALUE
    };

    GBL_ASSERT(state < EVMU_FLASH_PROGRAM_STATE_COUNT, "Invalid flash program state!");

    return prgValueLut[state];
}

EVMU_EXPORT EVMU_FLASH_PROGRAM_STATE EvmuFlash_programState(const EvmuFlash* pSelf) {
    return EVMU_FLASH_(pSelf)->prgState;
}

EVMU_EXPORT size_t EvmuFlash_programBytes(const EvmuFlash* pSelf) {
    return EVMU_FLASH_(pSelf)->prgBytes;
}

EVMU_EXPORT size_t EvmuFlash_programCycles(const EvmuFlash* pSelf) {
    return 0;
}

EVMU_EXPORT EvmuAddress EvmuFlash_targetAddress(const EvmuFlash* pSelf) {
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));

    return ((EvmuMemory_readData(pDevice->pMemory, EVMU_ADDRESS_SFR_FPR) & EVMU_SFR_FPR_ADDR_MASK) << 16) |
            (EvmuMemory_readData(pDevice->pMemory, EVMU_ADDRESS_SFR_TRL) << 8) |
             EvmuMemory_readData(pDevice->pMemory, EVMU_ADDRESS_SFR_TRL);
}

EVMU_EXPORT GblBool EvmuFlash_unlocked(const EvmuFlash* pSelf) {
    return EvmuFlash_programState(pSelf) == EVMU_FLASH_PROGRAM_STATE_COUNT;
}

EVMU_EXPORT EvmuWord EvmuFlash_readByte(const EvmuFlash* pSelf, EvmuAddress address) {
    return EvmuMemory_readFlash(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf))->pMemory, address);
}

EVMU_EXPORT EVMU_RESULT EvmuFlash_writeByte(const EvmuFlash* pSelf,
                                            EvmuAddress address,
                                            EvmuWord    value)
{
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_VERIFY(EvmuFlash_unlocked(pSelf),
                   EVMU_RESULT_ERROR_FLASH_LOCKED);

    GBL_CTX_VERIFY_CALL(EvmuMemory_writeFlash(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf))->pMemory,
                                              address,
                                              value));
    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuFlash_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static const GblTypeInfo info = {
        .classSize           = sizeof(EvmuFlashClass),
        .instanceSize        = sizeof(EvmuFlash),
        .instancePrivateSize = sizeof(EvmuFlash_)
    };

    if(type == GBL_INVALID_TYPE) {
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuFlash"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);

    }

    return type;
}

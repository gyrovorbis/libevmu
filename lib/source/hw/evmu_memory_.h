#ifndef EVMU_MEMORY__H
#define EVMU_MEMORY__H

#ifdef __cplusplus
extern "C" {
#endif

#include <evmu/hw/evmu_memory.h>
#include "evmu_peripheral_.h"

typedef struct EvmuMemory_ {
    EvmuPeripheral      peripheral;
    EvmuWord   flash   [FLASH_SIZE];
    EvmuWord   rom     [ROM_SIZE];
    EvmuWord   xram    [XRAM_BANK_COUNT][XRAM_BANK_SIZE];
    EvmuWord   wram    [WRAM_SIZE];
    EvmuWord   sfr     [RAM_SFR_SIZE];                     //not including XRAM
    EvmuWord   ram     [RAM_BANK_COUNT][RAM_BANK_SIZE];    //general-purpose RAM
    EvmuWord*  pMemMap [VMU_MEM_SEG_COUNT];                //contiguous RAM address space
    EvmuWord*  pExt;
} EvmuMemory_;

// INLINE!!!
EvmuWord     evmuMemoryRead_(const EvmuMemory_* pMemory, EvmuAddress addr);
EvmuWord     evmuMemoryReadLatch_(const EvmuMemory_* pMemory, EvmuAddress addr);
void         evmuMemoryWrite_(EvmuMemory_* pMemory, EvmuAddress addr, EvmuWord val);

// Fast helper function for CPU
EvmuWord    evmuMemorySfrRead_(const EvmuMemory_* pMemory, EvmuAddress address);
EvmuWord    evmuMemoryExtRead_(const EvmuMemory_* pMemory, EvmuAddress address);
EvmuWord    evmuMemoryExtWrite_(EvmuMemory_* pMemory, EvmuAddress address, EvmuWord value);

EvmuWord    evmuMemoryFlashRead_(const EvmuMemory_* pMemory, EvmuAddress address);
EvmuWord    evmuMemoryFlashWrite_(EvmuMemory_* pMemory, EvmuAddress address, EvmuWord value);


void evmuMemoryStackPush_(struct EvmuMemory_* pMemory, EvmuWord val) {
    EVMU_API_BEGIN(pMemory);
    const EvmuWord sp = evmuMemorySfrRead_(pMemory, EVMU_ADDRESS_SFR_SP) + 1;

    if(sp > RAM_STACK_ADDR_END) {
        GBL_API_WARN("PUSH[%u]: Stack overflow detected!", val);
    }

    pMemory->ram[0][++pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]] = val;
}

EvmuWord evmuMemoryStackPop_(struct EvmuMemory_* pMemory) {
    EVMU_API_BEGIN(pMemory);
    const EvmuWord sp = evmuMemorySfrRead_(pMemory, EVMU_ADDRESS_SFR_SP);

    const EvmuWord val = pMemory->ram[0][pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]];

    if(sp < EVMU_ADDRESS_SEGMENT_STACK_BASE) {
        GBL_API_WARN("POP[%u]: Stack underflow detected!", val);
    }

    --pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)];
    return val;

}


#ifdef __cplusplus
}
#endif

#endif // EVMU_MEMORY__H

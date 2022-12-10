#ifndef EVMU_MEMORY__H
#define EVMU_MEMORY__H

//#include <evmu/evmu_api.h>
#include <gimbal/algorithms/gimbal_numeric.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_address_space.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_flash.h>
#include <evmu/hw/evmu_wram.h>
#include <evmu/hw/evmu_rom.h>
#include "evmu_cpu_.h"

#define EVMU_MEMORY_(instance)      ((EvmuMemory_*)GBL_INSTANCE_PRIVATE(instance, EVMU_MEMORY_TYPE))
#define EVMU_MEMORY_PUBLIC_(priv)   ((EvmuMemory*)GBL_INSTANCE_PUBLIC(priv, EVMU_MEMORY_TYPE))

#define EVMU_MEMORY__INT_SEGMENT_SIZE_  128

#define GBL_SELF_TYPE EvmuMemory_

GBL_DECLS_BEGIN

typedef enum EVMU_MEMORY__INT_SEGMENT_ {
    EVMU_MEMORY__INT_SEGMENT_GP1_,
    EVMU_MEMORY__INT_SEGMENT_GP2_,
    EVMU_MEMORY__INT_SEGMENT_SFR_,
    EVMU_MEMORY__INT_SEGMENT_XRAM_,
    EVMU_MEMORY__INT_SEGMENT_COUNT_
} EVMU_MEMORY__INT_SEGMENT_;

typedef struct EvmuMemory_ {
    EvmuCpu_*       pCpu;

    // Internal Memory BUS
    EvmuWord        ram     [EVMU_ADDRESS_SEGMENT_RAM_BANKS][EVMU_ADDRESS_SEGMENT_RAM_SIZE];    //general-purpose RAM
    EvmuWord        sfr     [EVMU_ADDRESS_SEGMENT_SFR_SIZE];                     //not including XRAM
    EvmuWord        xram    [EVMU_ADDRESS_SEGMENT_XRAM_BANKS][EVMU_ADDRESS_SEGMENT_XRAM_SIZE];

    // External Memory BUS
    EvmuWord        flash   [EVMU_FLASH_SIZE];
    EvmuWord        rom     [EVMU_ROM_SIZE];

    EvmuWord        wram    [EVMU_WRAM_SIZE];

    EvmuWord*       pIntMap [EVMU_MEMORY__INT_SEGMENT_COUNT_];                //contiguous RAM address space
    EvmuWord*       pExt;
} EvmuMemory_;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_MEMORY__H

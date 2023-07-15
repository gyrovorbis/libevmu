#ifndef EVMU_RAM__H
#define EVMU_RAM__H

//#include <evmu/evmu_api.h>
#include <gimbal/algorithms/gimbal_numeric.h>
#include <evmu/hw/evmu_ram.h>
#include <evmu/hw/evmu_address_space.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_flash.h>
#include <evmu/hw/evmu_wram.h>
#include <evmu/hw/evmu_rom.h>
#include "evmu_cpu_.h"
#include "evmu_flash_.h"

#define EVMU_RAM_(instance)      ((EvmuRam_*)GBL_INSTANCE_PRIVATE(instance, EVMU_RAM_TYPE))
#define EVMU_RAM_PUBLIC_(priv)   ((EvmuRam*)GBL_INSTANCE_PUBLIC(priv, EVMU_RAM_TYPE))

#define EVMU_RAM__INT_SEGMENT_SIZE_  128

#define GBL_SELF_TYPE EvmuRam_

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRom_);

typedef enum EVMU_RAM__INT_SEGMENT_ {
    EVMU_RAM__INT_SEGMENT_GP1_,
    EVMU_RAM__INT_SEGMENT_GP2_,
    EVMU_RAM__INT_SEGMENT_SFR_,
    EVMU_RAM__INT_SEGMENT_XRAM_,
    EVMU_RAM__INT_SEGMENT_COUNT_
} EVMU_RAM__INT_SEGMENT_;

typedef struct EvmuRam_ {
    EvmuCpu_*   pCpu;
    EvmuFlash_* pFlash;
    EvmuRom_*   pRom;

    // Internal Memory BUS
    EvmuWord  ram     [EVMU_ADDRESS_SEGMENT_RAM_BANKS][EVMU_ADDRESS_SEGMENT_RAM_SIZE];   //general-purpose RAM
    EvmuWord  sfr     [EVMU_ADDRESS_SEGMENT_SFR_SIZE];                                   //Excluding XRAM
    EvmuWord  xram    [EVMU_ADDRESS_SEGMENT_XRAM_BANKS][EVMU_ADDRESS_SEGMENT_XRAM_SIZE]; //VRAM

    // Memory-Map for current internal BUS address space
    EvmuWord* pIntMap [EVMU_RAM__INT_SEGMENT_COUNT_];                //contiguous RAM address space
    // Memory-Map for current external BUS address space
    EvmuWord* pExt;
} EvmuRam_;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_RAM__H

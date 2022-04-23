#ifndef EVMU_MEMORY_H
#define EVMU_MEMORY_H

#include "../types/evmu_peripheral.h"

#define EVMU_MEMORY_TYPE                    (EvmuMemory_type())
#define EVMU_MEMORY_STRUCT                  EvmuMemory
#define EVMU_MEMORY_CLASS_STRUCT            EvmuMemoryClass
#define EVMU_MEMORY(inst)                   (GBL_TYPE_CAST_INSTANCE_PREFIX  (inst,  EVMU_MEMORY))
#define EVMU_MEMORY_COMPATIBLE(inst)        (GBL_TYPE_CHECK_INSTANCE_PREFIX (inst,  EVMU_MEMORY))
#define EVMU_MEMORY_CLASS(klass)            (GBL_TYPE_CAST_CLASS_PREFIX     (klass, EVMU_MEMORY))
#define EVMU_MEMORY_CLASS_COMPATIBLE(klass) (GBL_TYPE_CHECK_CLASS_PREFIX    (klass, EVMU_MEMORY))
#define EVMU_MEMORY_GET_CLASS(inst)         (GBL_TYPE_CAST_GET_CLASS_PREFIX (inst,  EVMU_MEMORY))

#define SELF    EvmuMemory* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_ENUM(EVMU_MEMORY_SEGMENT) {
    EVMU_MEMORY_SEGMENT_RAM,
    EVMU_MEMORY_SEGMENT_SFR,
    EVMU_MEMORY_SEGMENT_XRAM,
    EVMU_MEMORY_SEGMENT_WRAM,
    EVMU_MEMORY_SEGMENT_ROM,
    EVMU_MEMORY_SEGMENT_FLASH,
    EVMU_MEMORY_SEGMENT_COUNT
};

GBL_DECLARE_ENUM(EVMU_MEMORY_BANK) {
    EVMU_MEMORY_BANK_INVALID,
    EVMU_MEMORY_BANK_CURRENT,
    EVMU_MEMORY_BANK_0,
    EVMU_MEMORY_BANK_1,
    EVMU_MEMORY_BANK_2,
    EVMU_MEMORY_BANK_COUNT
};

GBL_DECLARE_ENUM(EVMU_MEMORY_PINS) {
    EVMU_MEMORY_PINS_PORT,
    EVMU_MEMORY_PINS_LATCH
};

typedef struct EvmuMemoryClass {
    EvmuPeripheralClass base;
} EvmuMemoryClass;

typedef struct EvmuMemory {
    union {
        EvmuMemoryClass*    pClass;
        EvmuPeripheral      base;
    };
    EvmuMemory_*            pPrivate;
} EvmuMemory;


GBL_EXPORT GblType       EvmuMemory_type(void) GBL_NOEXCEPT;

GBL_EXPORT EVMU_RESULT   EvmuMemory_segmentInfo(EVMU_MEMORY_SEGMENT segment, GblSize* pBankSize, uint8_t* pBankCount) GBL_NOEXCEPT;

// Read/write generically into any bank of any segment
GBL_EXPORT EVMU_RESULT   EvmuMemory_segmentReadBytes(CSELF, EVMU_MEMORY_SEGMENT segment, EVMU_MEMORY_BANK bank, EVMU_MEMORY_PINS pins, EvmuAddress base, EvmuWord* pData, GblSize* pBytes)    GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT   EvmuMemory_segmentWriteBytes(CSELF, EVMU_MEMORY_SEGMENT segment, EVMU_MEMORY_BANK bank, EVMU_MEMORY_PINS pins, EvmuAddress base, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;

// Read/write into external ROM address space
GBL_EXPORT EVMU_RESULT   EvmuMemory_extReadBytes(CSELF, EvmuAddress base, EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT   EvmuMemory_extWriteBytes(CSELF, EvmuAddress base, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;

// Read/write into internal RAM address space
GBL_EXPORT EVMU_RESULT   EvmuMemory_intReadBytes(CSELF, EVMU_MEMORY_PINS pins, EvmuAddress base, EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT   EvmuMemory_intWriteBytes(CSELF, EVMU_MEMORY_PINS pins, EvmuAddress base, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;

// Read/write into only RAM (current bank, SYSTEM, or APPLICATION RAM bank
GBL_EXPORT EVMU_RESULT   EvmuMemory_ramReadBytes(CSELF, EVMU_MEMORY_BANK bank, EvmuAddress base, EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT   EvmuMemory_ramWriteBytes(CSELF, EVMU_MEMORY_BANK bank, EvmuAddress base, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;

// Read/write into only the SFR segment of the internal address space
GBL_EXPORT EVMU_RESULT   EvmuMemory_sfrReadBytes(CSELF, EVMU_MEMORY_PINS pins, EvmuAddress base, EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT   EvmuMemory_sfrWriteBytes(CSELF, EVMU_MEMORY_PINS pins, EvmuAddress base, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;

// Push/pop operations to manipulate stack from API
GBL_EXPORT EVMU_RESULT   EvmuMemory_stackPushBytes(CSELF, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT   EvmuMemory_stackPopBytes(CSELF, const EvmuWord* pData, GblSize* pBytes) GBL_NOEXCEPT;

GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_MEMORY_H


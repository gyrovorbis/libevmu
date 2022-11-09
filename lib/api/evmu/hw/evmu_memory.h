#ifndef EVMU_MEMORY_H
#define EVMU_MEMORY_H

#include "../types/evmu_peripheral.h"

#define EVMU_MEMORY_TYPE                (GBL_TYPEOF(EvmuMemory))
#define EVMU_MEMORY(instance)           (GBL_INSTANCE_CAST(instance, EvmuMemory))
#define EVMU_MEMORY_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuMemory))
#define EVMU_MEMORY_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuMemory))

#define GBL_SELF_TYPE EvmuMemory

GBL_DECLS_BEGIN

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

GBL_CLASS_DERIVE_EMPTY   (EvmuMemory, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuMemory, EvmuPeripheral)

GBL_PROPERTIES(EvmuMemory,
    (stackPointer, GBL_GENERIC, (READ), GBL_UINT8_TYPE),
    (extBusSource, GBL_GENERIC, (READ), GBL_ENUM_TYPE),
    (intBusSource, GBL_GENERIC, (READ), GBL_ENUM_TYPE)
)

EVMU_EXPORT GblType     EvmuMemory_type          (void)                           GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_segmentInfo   (EVMU_MEMORY_SEGMENT segment,
                                                  GblSize*            pBankSize,
                                                  uint8_t*            pBankCount) GBL_NOEXCEPT;

// Read/write generically into any bank of any segment
EVMU_EXPORT EVMU_RESULT EvmuMemory_readBytes     (GBL_CSELF,
                                                  EVMU_MEMORY_SEGMENT segment,
                                                  EVMU_MEMORY_BANK    bank,
                                                  EVMU_MEMORY_PINS    pins,
                                                  EvmuAddress         base,
                                                  EvmuWord*           pData,
                                                  GblSize*            pBytes)     GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeBytes    (GBL_CSELF,
                                                  EVMU_MEMORY_SEGMENT segment,
                                                  EVMU_MEMORY_BANK    bank,
                                                  EVMU_MEMORY_PINS    pins,
                                                  EvmuAddress         base,
                                                  const EvmuWord*     pData,
                                                  GblSize*            pBytes)    GBL_NOEXCEPT;

// Read/write into external ROM address space
EVMU_EXPORT EVMU_RESULT EvmuMemory_readExtBytes  (GBL_CSELF,
                                                  EvmuAddress         base,
                                                  EvmuWord*           pData,
                                                  GblSize*            pBytes)    GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeExtBytes (GBL_CSELF,
                                                  EvmuAddress         base,
                                                  const EvmuWord*     pData,
                                                  GblSize*            pBytes)    GBL_NOEXCEPT;

// Read/write into internal RAM address space
EVMU_EXPORT EVMU_RESULT EvmuMemory_readIntBytes  (GBL_CSELF,
                                                  EVMU_MEMORY_PINS    pins,
                                                  EvmuAddress         base,
                                                  EvmuWord*           pData,
                                                  GblSize*            pBytes)    GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeIntBytes (GBL_CSELF,
                                                  EVMU_MEMORY_PINS    pins,
                                                  EvmuAddress         base,
                                                  const EvmuWord*     pData,
                                                  GblSize*            pBytes)    GBL_NOEXCEPT;

// Read/write into only RAM (current bank, SYSTEM, or APPLICATION RAM bank)
EVMU_EXPORT EVMU_RESULT EvmuMemory_readRamBytes  (GBL_CSELF,
                                                  EVMU_MEMORY_BANK    bank,
                                                  EvmuAddress         base,
                                                  EvmuWord*           pData,
                                                  GblSize*            pBytes)   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeRamBytes (GBL_CSELF,
                                                  EVMU_MEMORY_BANK    bank,
                                                  EvmuAddress         base,
                                                  const EvmuWord*     pData,
                                                  GblSize*            pBytes)   GBL_NOEXCEPT;

// Read/write into only the SFR segment of the internal address space
EVMU_EXPORT EVMU_RESULT EvmuMemory_readSfrBytes  (GBL_CSELF,
                                                  EVMU_MEMORY_PINS    pins,
                                                  EvmuAddress         base,
                                                  EvmuWord*           pData,
                                                  GblSize*            pBytes)   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeSfrBytes (GBL_CSELF,
                                                  EVMU_MEMORY_PINS    pins,
                                                  EvmuAddress         base,
                                                  const EvmuWord*     pData,
                                                  GblSize*            pBytes)   GBL_NOEXCEPT;

// Push/pop operations to manipulate stack from API
EVMU_EXPORT EVMU_RESULT EvmuMemory_pushStackBytes(GBL_CSELF,
                                                  const EvmuWord*    pData,
                                                  GblSize*           pBytes)    GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_popStackBytes (GBL_CSELF,
                                                  const EvmuWord*    pData,
                                                  GblSize*           pBytes)    GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_MEMORY_H


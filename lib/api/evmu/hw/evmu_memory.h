#ifndef EVMU_MEMORY_H
#define EVMU_MEMORY_H

#include "../types/evmu_peripheral.h"
#include "../hw/evmu_sfr.h"

#define EVMU_MEMORY_TYPE                (GBL_TYPEOF(EvmuMemory))
#define EVMU_MEMORY_NAME                "FIXME"

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

GBL_DECLARE_ENUM(EVMU_MEMORY_EXT_SRC) {
    EVMU_MEMORY_EXT_SRC_ROM          = EVMU_SFR_EXT_ROM,
    EVMU_MEMORY_EXT_SRC_FLASH_BANK_0 = EVMU_SFR_EXT_FLASH_BANK_0,
    EVMU_MEMORY_EXT_SRC_FLASH_BANK_1 = EVMU_SFR_EXT_FLASH_BANK_1
};

GBL_CLASS_DERIVE_EMPTY   (EvmuMemory, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuMemory, EvmuPeripheral)

GBL_PROPERTIES(EvmuMemory,
    (stackPointer, GBL_GENERIC, (READ), GBL_UINT8_TYPE),
    (extBusSource, GBL_GENERIC, (READ), GBL_ENUM_TYPE),
    (intBusSource, GBL_GENERIC, (READ), GBL_ENUM_TYPE)
)

EVMU_EXPORT GblType     EvmuMemory_type         (void)                        GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuMemory_indirectAddress
                                                (GBL_CSELF, uint8_t mode)     GBL_NOEXCEPT;

// Read/write into internal RAM address space (RAM, SFRs, XRAM)
EVMU_EXPORT EvmuWord    EvmuMemory_readInt      (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuMemory_readIntLatch (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuMemory_viewInt      (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeInt     (GBL_SELF,
                                                 EvmuAddress address,
                                                 EvmuWord    value)           GBL_NOEXCEPT;

// External addres space (ROM/Flash)
EVMU_EXPORT EvmuWord    EvmuMemory_readExt      (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeExt     (GBL_SELF,
                                                 EvmuAddress addr,
                                                 EvmuWord    value)           GBL_NOEXCEPT;

EVMU_EXPORT EVMU_MEMORY_EXT_SRC
                        EvmuMemory_extSource    (GBL_CSELF)                   GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuMemory_setExtSource (GBL_SELF,
                                                 EVMU_MEMORY_EXT_SRC src)     GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord    EvmuMemory_readFlash    (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeFlash   (GBL_SELF,
                                                 EvmuAddress addr,
                                                 EvmuWord    value)           GBL_NOEXCEPT;

EVMU_EXPORT int         EvmuMemory_stackDepth   (GBL_CSELF)                   GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuMemory_viewStack    (GBL_CSELF, GblSize depth)    GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuMemory_popStack     (GBL_SELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuMemory_pushStack    (GBL_SELF, EvmuWord value)    GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord    EvmuMemory_readWram     (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeWram    (GBL_SELF,
                                                 EvmuAddress addr,
                                                 EvmuWord    value)           GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_MEMORY_H


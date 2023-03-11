#ifndef EVMU_MEMORY_H
#define EVMU_MEMORY_H

#include "../types/evmu_peripheral.h"
#include "../hw/evmu_sfr.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_MEMORY_TYPE                (GBL_TYPEOF(EvmuMemory))
#define EVMU_MEMORY_NAME                "FIXME"

#define EVMU_MEMORY(instance)           (GBL_INSTANCE_CAST(instance, EvmuMemory))
#define EVMU_MEMORY_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuMemory))
#define EVMU_MEMORY_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuMemory))

#define GBL_SELF_TYPE EvmuMemory

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory);

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

GBL_CLASS_DERIVE(EvmuMemory, EvmuPeripheral)
    EVMU_RESULT (*pFnReadData)      (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnReadDataLatch) (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnWriteData)     (GBL_SELF, EvmuAddress addr, EvmuWord value);
    EVMU_RESULT (*pFnWriteDataLatch)(GBL_SELF, EvmuAddress addr, EvmuWord value);
    EVMU_RESULT (*pFnReadProgram)   (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnWriteProgram)  (GBL_SELF, EvmuAddress addr, EvmuWord value);
    EVMU_RESULT (*pFnReadFlash)     (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnWriteFlash)    (GBL_SELF, EvmuAddress addr, EvmuWord value);
GBL_CLASS_END

GBL_INSTANCE_DERIVE(EvmuMemory, EvmuPeripheral)
    uint32_t ramChanged   : 1;
    uint32_t sfrChanged   : 1;
    uint32_t xramChanged  : 1;
    uint32_t flashChanged : 1;
    uint32_t romChanged   : 1;
    uint32_t wramChanged  : 1;
GBL_INSTANCE_END

GBL_PROPERTIES(EvmuMemory,
    (ramBank,        GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (xramBank,       GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (programSource,  GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (stackPointer,   GBL_GENERIC, (READ),        GBL_UINT8_TYPE),
    (vselIncrement,  GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (vrmad,          GBL_GENERIC, (READ, WRITE), GBL_UINT16_TYPE),
    (vtrbf,          GBL_GENERIC, (READ, WRITE), GBL_UINT8_TYPE)
)

GBL_SIGNALS(EvmuMemory,
    (ramValueChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address), (GBL_ENUM_TYPE, bank)),
    (ramBankChange,       (GBL_INSTANCE_TYPE, pReceiver), (GBL_ENUM_TYPE, bank)),
    (sfrValueChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address)),
    (xramValueChange,     (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address), (GBL_ENUM_TYPE, bank)),
    (xramBankChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_ENUM_TYPE, bank)),
    (flashValueChange,    (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address)),
    (romValueChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address)),
    (programSourceChange, (GBL_INSTANCE_TYPE, pReceiver), (GBL_BOOL_TYPE, flash)),
    (wramValueChange,     (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address)),
    (stackPush,           (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT8_TYPE, value)),
    (stackPop,            (GBL_INSTANCE_TYPE, pReceiver))
)

EVMU_EXPORT GblType     EvmuMemory_type         (void)                        GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuMemory_indirectAddress
                                                (GBL_CSELF, uint8_t mode)     GBL_NOEXCEPT;

// Read/write into internal RAM address space (RAM, SFRs, XRAM)
EVMU_EXPORT EvmuWord    EvmuMemory_readData      (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuMemory_readDataLatch (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuMemory_viewData      (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeData     (GBL_SELF,
                                                  EvmuAddress address,
                                                  EvmuWord    value)           GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeDataLatch    (GBL_SELF,
                                                  EvmuAddress address,
                                                  EvmuWord    value)           GBL_NOEXCEPT;

// External addres space (ROM/Flash)
EVMU_EXPORT EvmuWord    EvmuMemory_readProgram      (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeProgram     (GBL_SELF,
                                                 EvmuAddress addr,
                                                 EvmuWord    value)           GBL_NOEXCEPT;

EVMU_EXPORT EVMU_MEMORY_EXT_SRC
                        EvmuMemory_programSource    (GBL_CSELF)                   GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuMemory_setProgramSource (GBL_SELF,
                                                 EVMU_MEMORY_EXT_SRC src)     GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord    EvmuMemory_readFlash    (GBL_CSELF, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeFlash   (GBL_SELF,
                                                 EvmuAddress addr,
                                                 EvmuWord    value)           GBL_NOEXCEPT;
//READ/WRITE ROM

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


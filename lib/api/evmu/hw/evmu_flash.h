#ifndef EVMU_FLASH_H
#define EVMU_FLASH_H

#include "../types/evmu_peripheral.h"

#define EVMU_FLASH_TYPE                     (GBL_TYPEOF(EvmuFlash))

#define EVMU_FLASH(instance)                (GBL_INSTANCE_CAST(instance, EvmuFlash))
#define EVMU_FLASH_CLASS(klass)             (GBL_CLASS_CAST(klass, EvmuFlash))
#define EVMU_FLASH_GET_CLASS(instance)      (GBL_INSTANCE_GET_CLASS(instance, EvmuFlash))

#define EVMU_FLASH_BANK_SIZE                65536
#define EVMU_FLASH_BANKS                    2
#define EVMU_FLASH_SIZE                     (EVMU_FLASH_BANK_SIZE*EVMU_FLASH_BANKS)

#define EVMU_FLASH_PROGRAM_BYTE_COUNT       128     //number of bytes software can write to flash once unlocked
#define EVMU_FLASH_PROGRAM_STATE_0_ADDRESS  0x5555  //Key-value pairs for flash unlock sequence used by STF instruction
#define EVMU_FLASH_PROGRAM_STATE_0_VALUE    0xaa
#define EVMU_FLASH_PROGRAM_STATE_1_ADDRESS  0x2aaa
#define EVMU_FLASH_PROGRAM_STATE_1_VALUE    0x55
#define EVMU_FLASH_PROGRAM_STATE_2_ADDRESS  0x5555
#define EVMU_FLASH_PROGRAM_STATE_2_VALUE    0xa0

#define GBL_SELF_TYPE EvmuFlash

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_FLASH_PROGRAM_STATE) {
    EVMU_FLASH_PROGRAM_STATE_0,
    EVMU_FLASH_PROGRAM_STATE_1,
    EVMU_FLASH_PROGRAM_STATE_2,
    EVMU_FLASH_PROGRAM_STATE_COUNT
};

GBL_CLASS_DERIVE_EMPTY   (EvmuFlash, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuFlash, EvmuPeripheral)

GBL_PROPERTIES(EvmuFlash,
    (programUnlocked, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (programState,    GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (programBytes,    GBL_GENERIC, (READ, WRITE), GBL_UINT8_TYPE),
    (targetAddress,   GBL_GENERIC, (READ, WRITE), GBL_UINT32_TYPE)
)

EVMU_EXPORT GblType     EvmuFlash_type           (void)                           GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuFlash_programAddress (EVMU_FLASH_PROGRAM_STATE state) GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuFlash_programValue   (EVMU_FLASH_PROGRAM_STATE state) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_FLASH_PROGRAM_STATE
                        EvmuFlash_programState   (GBL_CSELF)                      GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuFlash_programBytes   (GBL_CSELF)                      GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuFlash_programCycles  (GBL_CSELF)                      GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuFlash_targetAddress  (GBL_CSELF)                      GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuFlash_unlocked       (GBL_CSELF)                      GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord    EvmuFlash_readByte       (GBL_CSELF, EvmuAddress address) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuFlash_readBytes      (GBL_CSELF,
                                                  EvmuAddress base,
                                                  void*       pData,
                                                  GblSize*    pBytes)             GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuFlash_writeByte      (GBL_CSELF,
                                                  EvmuAddress address,
                                                  EvmuWord    value)              GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuFlash_writeBytes     (GBL_CSELF,
                                                  EvmuAddress base,
                                                  const void* pData,
                                                  GblSize*    pBytes)             GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FLASH_H


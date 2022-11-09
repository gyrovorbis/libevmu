#ifndef EVMU_WRAM_H
#define EVMU_WRAM_H

#include "../types/evmu_peripheral.h"

#define EVMU_WRAM_TYPE                  (GBL_TYPEOF(EvmuWram))

#define EVMU_WRAM(instance)             (GBL_INSTANCE_CAST(instance, EvmuWram))
#define EVMU_WRAM_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuWram))
#define EVMU_WRAM_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance, EvmuWram))

#define EVMU_WRAM_BANK_COUNT            2
#define EVMU_WRAM_BANK_SIZE             256
#define EVMU_WRAM_SIZE                  (EVMU_WRAM_BANK_COUNT*EVMU_WRAM_BANK_SIZE)

/* SFRs owned:
 *  VSEL - Configuration, needed by Serial communications too?
 *  VRMAD1, VRMAD2 - Low and high byte of address to read/write from
 *  VTRBF - Register to read/write from to access WRAM[VRMAD1<<8|VRMAD2]
 *            Can autoincrement shitbased on register VSEL.INCE!
 */

#define GBL_SELF_TYPE EvmuWram

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuWram, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuWram, EvmuPeripheral)

GBL_PROPERTIES(EvmuWram,
    (mode,           GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (autoIncAddress, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (targetAddress,  GBL_GENERIC, (READ, WRITE), GBL_UINT16_TYPE),
    (targetValue,    GBL_GENERIC, (READ, WRITE), GBL_UINT8_TYPE),
    (transferring,   GBL_GENERIC, (READ),        GBL_BOOL_TYPE)
)

EVMU_EXPORT GblType     EvmuWram_type       (void)                GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord    EvmuWram_readByte   (GBL_CSELF,
                                             EvmuAddress address) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuWram_readBytes  (GBL_CSELF,
                                             EvmuAddress address,
                                             void*       pData,
                                             GblSize*    pSize)   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuWram_writeByte  (GBL_CSELF,
                                             EvmuAddress address,
                                             EvmuWord byte)       GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuWram_writeBytes (GBL_CSELF,
                                             EvmuAddress address,
                                             const void* pData,
                                             GblSize*    pBytes)  GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_WRAM_H

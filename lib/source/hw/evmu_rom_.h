#ifndef EVMU_ROM__H
#define EVMU_ROM__H

#include <evmu/hw/evmu_rom.h>
#include <gimbal/utils/gimbal_byte_array.h>

#define EVMU_ROM_(instance)     (GBL_PRIVATE(EvmuRom, instance))
#define EVMU_ROM_PUBLIC_(priv)  (GBL_PUBLIC(EvmuRom, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

typedef struct EvmuRom_ {
    EvmuRam_*      pRam;

    GblByteArray*  pStorage;
    EVMU_BIOS_TYPE eBiosType;
    GblBool        bSetupSkipEnabled;
} EvmuRom_;

GBL_DECLS_END

#endif // EVMU_ROM__H

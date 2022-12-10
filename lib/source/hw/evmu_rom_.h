#ifndef EVMU_ROM__H
#define EVMU_ROM__H

#include <evmu/hw/evmu_rom.h>

#define EVMU_ROM_(instance)     ((EvmuRom_*)GBL_INSTANCE_PRIVATE(instance, EVMU_ROM_TYPE))
#define EVMU_ROM_PUBLIC_(priv)  ((EvmuRom*)GBL_INSTANCE_PUBLIC(priv, EVMU_ROM_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

typedef struct EvmuRom_ {
    EvmuMemory_* pMemory;
} EvmuRom_;

GBL_DECLS_END

#endif // EVMU_ROM__H

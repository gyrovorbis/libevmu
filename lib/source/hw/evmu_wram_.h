#ifndef EVMU_WRAM__H
#define EVMU_WRAM__H

#include <evmu/hw/evmu_wram.h>
#include <gimbal/utils/gimbal_byte_array.h>

#define EVMU_WRAM_(instance)   ((EvmuWram_*)GBL_INSTANCE_PRIVATE(instance, EVMU_WRAM_TYPE))
#define EVMU_WRAM_PUBLIC(priv) ((EvmuWram*)GBL_INSTANCE_PUBLIC(priv, EVMU_WRAM_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuWram_) {
    EvmuRam_*     pRam;
    GblByteArray* pStorage;
};

GBL_DECLS_END

#endif // EVMU_WRAM__H

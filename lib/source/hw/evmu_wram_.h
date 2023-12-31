#ifndef EVMU_WRAM__H
#define EVMU_WRAM__H

#include <evmu/hw/evmu_wram.h>
#include <gimbal/utils/gimbal_byte_array.h>

#define EVMU_WRAM_(instance)   (GBL_PRIVATE(EvmuWram, instance))
#define EVMU_WRAM_PUBLIC(priv) (GBL_PUBLIC(EvmuWram, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuWram_) {
    EvmuRam_*     pRam;
    GblByteArray* pStorage;
};

GBL_DECLS_END

#endif // EVMU_WRAM__H

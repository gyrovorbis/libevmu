#ifndef EVMU_FAT__H
#define EVMU_FAT__H

#include <evmu/fs/evmu_fat.h>

#define EVMU_FAT_(instance)     ((EvmuFat_*)GBL_INSTANCE_PRIVATE(instance, EVMU_FAT_TYPE))
#define EVMU_FAT_PUBLIC(priv)   ((EvmuFat*)GBL_INSTANCE_PUBLIC(priv, EVMU_FAT_TYPE))

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuFat_) {
    EvmuRam_*       pRam;
    EvmuRootBlock*  pRoot;
    size_t          blockSize;
};

#define GBL_SELF_TYPE EvmuFat

GBL_DECLS_BEGIN

EVMU_EXPORT void EvmuFat_logRoot        (GBL_CSELF) GBL_NOEXCEPT;
EVMU_EXPORT void EvmuFat_logTable       (GBL_CSELF) GBL_NOEXCEPT;
EVMU_EXPORT void EvmuFat_logDirectory   (GBL_CSELF) GBL_NOEXCEPT;
EVMU_EXPORT void EvmuFat_logMemoryUsage (GBL_CSELF) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FAT__H

#ifndef EVMU_ROM__H
#define EVMU_ROM__H

#include <evmu/hw/evmu_flash.h>

#define EVMU_ROM_(instance) ((EvmuRom_*)GBL_INSTANCE_PRIVATE(instance, EVMU_ROM_TYPE))


GBL_DECLS_BEGIN

typedef struct EvmuRom_ {

} EvmuRom_;

GBL_DECLS_END

#endif // EVMU_ROM__H

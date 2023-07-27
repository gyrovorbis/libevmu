#ifndef EVMU_FLASH__H
#define EVMU_FLASH__H

#include <evmu/hw/evmu_flash.h>
#include <gimbal/utils/gimbal_byte_array.h>

#define EVMU_FLASH_(instance)    (GBL_PRIVATE(EvmuFlash, instance))
#define EVMU_FLASH_PUBLIC_(priv) (GBL_PUBLIC(EvmuFlash, priv))

GBL_DECLS_BEGIN

//Flash controller for VMU (note actual flash blocks are stored within device)
GBL_DECLARE_STRUCT(EvmuFlash_) {
    EVMU_FLASH_PROGRAM_STATE prgState;
    uint8_t                  prgBytes;
    GblByteArray*            pStorage;
};

GBL_DECLS_END

#endif // EVMU_FLASH__H

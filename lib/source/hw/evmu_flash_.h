#ifndef EVMU_FLASH__H
#define EVMU_FLASH__H

#include <evmu/hw/evmu_flash.h>

#define EVMU_FLASH_(instance)   ((EvmuFlash_*)GBL_INSTANCE_PRIVATE(instance, EVMU_FLASH_TYPE))

#ifdef __cplusplus
extern "C" {
#endif

//Flash controller for VMU (note actual flash blocks are stored within device)
typedef struct EvmuFlash_ {
    uint8_t prgBytes;
    EVMU_FLASH_PROGRAM_STATE prgState;
} EvmuFlash_;

#ifdef __cplusplus
}
#endif

#endif // EVMU_FLASH__H

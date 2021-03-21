#ifndef EVMU_FLASH__H
#define EVMU_FLASH__H

#include <evmu/hw/evmu_peripheral.h>

#ifdef __cplusplus
extern "C" {
#endif

//Flash controller for VMU (note actual flash blocks are stored within device)
typedef struct EvmuFlash_ {
    EvmuPeripheral  peripheral;
    uint8_t prgBytes;
    EVMU_FLASH_PROGRAM_STATE prgState;
} EvmuFlash_;

#ifdef __cplusplus
}
#endif

#endif // EVMU_FLASH__H

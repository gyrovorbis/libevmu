#ifndef EVMU_FLASH__H
#define EVMU_FLASH__H

#include <evmu/hw/evmu_flash.h>
#include "evmu_peripheral_.h"

#ifdef __cplusplus
extern "C" {
#endif

//Flash controller for VMU (note actual flash blocks are stored within device)
typedef struct EvmuFlash_ {
    EvmuPeripheral_  peripheral;
    uint8_t prgBytes;
    EVMU_FLASH_PROGRAM_STATE prgState;
} EvmuFlash_;

static const EvmuPeripheralDriver evmuFlashDriver_ = {
    EVMU_PERIPHERAL_FLASH,
    "Flash Subystem",
    "Clocks!!!",
};

#ifdef __cplusplus
}
#endif

#endif // EVMU_FLASH__H

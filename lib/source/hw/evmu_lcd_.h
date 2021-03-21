#ifndef EVMU_LCD__H
#define EVMU_LCD__H

#include <evmu/hw/evmu_peripheral.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EvmuLcd_ {
    EvmuPeripheral peripheral;
    int     lcdBuffer[VMU_DISP_PIXEL_HEIGHT][VMU_DISP_PIXEL_WIDTH];
    float   refreshElapsed;
    int     ghostingEnabled;
    int     screenChanged;
} EvmuLcd_;

#ifdef __cplusplus
}
#endif

#endif // EVMU_LCD__H

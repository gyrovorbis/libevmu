#ifndef EVMU_LCD__H
#define EVMU_LCD__H

#include "evmu_peripheral_.h"
#include <evmu/hw/evmu_lcd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EvmuLcd_ {
    EvmuPeripheral_ peripheral;
    int     lcdBuffer[EVMU_LCD_PIXEL_HEIGHT][EVMU_LCD_PIXEL_WIDTH];
    float   refreshElapsed;
    int     ghostingEnabled;
    int     screenChanged;
} EvmuLcd_;

static const EvmuPeripheralDriver evmuLcdDriver_ = {
    EVMU_PERIPHERAL_LCD,
    "Clock Subystem",
    "Clocks!!!",
};

#ifdef __cplusplus
}
#endif

#endif // EVMU_LCD__H

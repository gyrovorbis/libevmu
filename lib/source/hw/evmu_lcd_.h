#ifndef EVMU_LCD__H
#define EVMU_LCD__H

#include <evmu/hw/evmu_lcd.h>

#define EVMU_LCD_(instance)         ((EvmuLcd_*)GBL_INSTANCE_PRIVATE(instance, EVMU_LCD_TYPE))
#define EVMU_LCD_PUBLIC(instance)   ((EvmuLcd*)GBL_INSTANCE_PUBLIC(instance, EVMU_LCD_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_STRUCT(EvmuLcd_) {
    int             pixelBuffer[EVMU_LCD_PIXEL_HEIGHT][EVMU_LCD_PIXEL_WIDTH];
    EVMU_LCD_ICONS  icons;
    EvmuTicks       refreshElapsed;
    EvmuMemory_*    pMemory;
};

GBL_DECLS_END

#endif // EVMU_LCD__H

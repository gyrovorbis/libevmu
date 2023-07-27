#ifndef EVMU_LCD__H
#define EVMU_LCD__H

#include <evmu/hw/evmu_lcd.h>

#define EVMU_LCD_(self)         (GBL_PRIVATE(EvmuLcd, self))
#define EVMU_LCD_PUBLIC_(priv)  (GBL_PUBLIC(EvmuLcd, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuLcd_) {
    int             pixelBuffer[EVMU_LCD_PIXEL_HEIGHT][EVMU_LCD_PIXEL_WIDTH];
    EVMU_LCD_ICONS  icons;
    EvmuTicks       refreshElapsed;
    EvmuRam_*       pRam;
};

GBL_DECLS_END

#endif // EVMU_LCD__H

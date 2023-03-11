/*! \file
 *  \brief LCD display circuit with emulated effects + back-end rendering signals
 *  \ingroup Peripherals
 *  \todo
 *      - Pixel ghosting still needs some work
 *      - update screen when in sleep mode
 *      - access/control XRAM bank
 */

#ifndef EVMU_LCD_H
#define EVMU_LCD_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_LCD_TYPE                       (GBL_TYPEOF(EvmuLcd))
#define EVMU_LCD_NAME                       "lcd"

#define EVMU_LCD(instance)                  (GBL_INSTANCE_CAST(instance, EvmuLcd))
#define EVMU_LCD_CLASS(klass)               (GBL_CLASS_CAST(klass, EvmuLcd))
#define EVMU_LCD_GET_CLASS(instance)        (GBL_INSTANCE_GET_CLASS(instance, EvmuLcd))

#define EVMU_XRAM_BANK_SIZE                 0x80
#define EVMU_XRAM_BANK_COUNT                3
#define EVMU_LCD_PIXEL_WIDTH                48
#define EVMU_LCD_PIXEL_HEIGHT               32
#define EVMU_LCD_ICON_COUNT                 4
#define EVMU_LCD_GHOSTING_FRAMES            25
#define EVMU_LCD_SCREEN_REFRESH_DIVISOR     50

#define GBL_SELF_TYPE EvmuLcd

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuLcd);

GBL_DECLARE_ENUM(EVMU_LCD_REFRESH_RATE) {
    EVMU_LCD_REFRESH_83HZ,
    EVMU_LCD_REFRESH_166HZ
};

GBL_DECLARE_ENUM(EVMU_XRAM_BANK) {
    EVMU_XRAM_BANK_LCD_TOP,
    EVMU_XRAM_BANK_LCD_BOTTOM,
    EVMU_XRAM_BANK_ICON
};

GBL_DECLARE_FLAGS(EVMU_LCD_ICONS) {
    EVMU_LCD_ICONS_NONE  = 0x0,
    EVMU_LCD_ICON_FILE   = 0x1,
    EVMU_LCD_ICON_GAME   = 0x2,
    EVMU_LCD_ICON_CLOCK  = 0x4,
    EVMU_LCD_ICON_FLASH  = 0x8,
    EVMU_LCD_ICONS_ALL   = 0xf
};

GBL_CLASS_DERIVE(EvmuLcd, EvmuPeripheral)
    EVMU_RESULT (*pFnRefreshScreen)(GBL_SELF);
GBL_CLASS_END

GBL_INSTANCE_DERIVE(EvmuLcd, EvmuPeripheral)
    GblSize screenRefreshDivisor; ///< How many hardware refreshes before software refresh
    uint32_t screenChanged   : 1; ///< User-driven toggle for knowing when to redraw
    uint32_t ghostingEnabled : 1; ///< Emulate pixel ghosting/fade effect
    uint32_t filterEnabled   : 1; ///< Enable linear filtering
    uint32_t invertColors    : 1; ///< Swap black and white pixel values
GBL_INSTANCE_END

GBL_PROPERTIES(EvmuLcd,
    (screenEnabled,   GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (refreshEnabled,  GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (refreshRate,     GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (ghostingEnabled, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (filterEnabled,   GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (invertColors,    GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (icons,           GBL_GENERIC, (READ, WRITE), GBL_FLAGS_TYPE)
)

GBL_SIGNALS(EvmuLcd,
    (screenRefresh, (GBL_INSTANCE_TYPE, pReceiver)),
    (screenToggle,  (GBL_INSTANCE_TYPE, pReceiver), (GBL_BOOL_TYPE,  enabled)),
    (iconsChange,   (GBL_INSTANCE_TYPE, pReceiver), (GBL_FLAGS_TYPE, flags))
)

EVMU_EXPORT GblType   EvmuLcd_type              (void)                                 GBL_NOEXCEPT;

EVMU_EXPORT GblBool   EvmuLcd_screenEnabled     (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT void      EvmuLcd_setScreenEnabled  (GBL_SELF, GblBool enabled)            GBL_NOEXCEPT;

EVMU_EXPORT GblBool   EvmuLcd_refreshEnabled    (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT void      EvmuLcd_setRefreshEnabled (GBL_SELF, GblBool enabled)            GBL_NOEXCEPT;

EVMU_EXPORT EVMU_LCD_REFRESH_RATE
                      EvmuLcd_refreshRate       (GBL_CSELF)                            GBL_NOEXCEPT;
EVMU_EXPORT void      EvmuLcd_setRefreshRate    (GBL_SELF, EVMU_LCD_REFRESH_RATE rate) GBL_NOEXCEPT;
EVMU_EXPORT EvmuTicks EvmuLcd_refreshRateTicks  (GBL_CSELF)                            GBL_NOEXCEPT;

EVMU_EXPORT EVMU_LCD_ICONS
                      EvmuLcd_icons             (GBL_CSELF)                            GBL_NOEXCEPT;

EVMU_EXPORT void      EvmuLcd_setIcons          (GBL_SELF, EVMU_LCD_ICONS icons)       GBL_NOEXCEPT;

EVMU_EXPORT GblBool   EvmuLcd_pixel             (GBL_CSELF, GblSize row, GblSize col)  GBL_NOEXCEPT;

EVMU_EXPORT void      EvmuLcd_setPixel          (GBL_SELF,
                                                 GblSize row,
                                                 GblSize col,
                                                 GblBool enabled)                      GBL_NOEXCEPT;

EVMU_EXPORT uint8_t   EvmuLcd_decoratedPixel    (GBL_CSELF, GblSize row, GblSize col)  GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_LCD_H


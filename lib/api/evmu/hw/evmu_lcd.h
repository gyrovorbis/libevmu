#ifndef EVMU_LCD_H
#define EVMU_LCD_H

#include "../types/evmu_peripheral.h"

#define EVMU_LCD_TYPE                   (GBL_TYPEOF(EvmuLcd))
#define EVMU_LCD_NAME                   "lcd"

#define EVMU_LCD(instance)              (GBL_INSTANCE_CAST(instance, EvmuLcd))
#define EVMU_LCD_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuLcd))
#define EVMU_LCD_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuLcd))

#define EVMU_XRAM_BANK_SIZE             0x80
#define EVMU_XRAM_BANK_COUNT            3
#define EVMU_LCD_PIXEL_WIDTH            48
#define EVMU_LCD_PIXEL_HEIGHT           32
#define EVMU_LCD_GHOSTING_FRAMES        25

#define GBL_SELF_TYPE EvmuLcd

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_LCD_REFRESH_RATE) {
    EVMU_LCD_REFRESH_83HZ,
    EVMU_LCD_REFRESH_166HZ
};

GBL_DECLARE_ENUM(EVMU_XRAM_BANK) {
    EVMU_XRAM_BANK_LCD_TOP,
    EVMU_XRAM_BANK_LCD_BOTTOM,
    EVMU_XRAM_BANK_ICON
};

GBL_DECLARE_ENUM(EVMU_LCD_ICON) {
    EVMU_LCD_ICON_FILE,
    EVMU_LCD_ICON_GAME,
    EVMU_LCD_ICON_CLOCK,
    EVMU_LCD_ICON_FLASH,
    EVMU_LCD_ICON_COUNT
};

GBL_DECLARE_ENUM(EVMU_LCD_FILTER) {
    EVMU_LCD_FILTER_NONE,
    EVMU_LCD_FILTER_LINEAR,
    EVMU_LCD_FILTER_COUNT
};

GBL_CLASS_DERIVE_EMPTY   (EvmuLcd, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuLcd, EvmuPeripheral)

GBL_PROPERTIES(EvmuLcd,
    (enabled,         GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (refreshEnabled,  GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (refreshRate,     GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (ghostingEnabled, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (changed,         GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE)
)

EVMU_EXPORT GblType     EvmuLcd_type               (void)                                                GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuLcd_displayEnabled     (GBL_CSELF)                                           GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setDisplayEnabled  (GBL_SELF, GblBool enabled)                           GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuLcd_refreshEnabled     (GBL_CSELF)                                           GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setRefreshEnabled  (GBL_SELF, GblBool enabled)                           GBL_NOEXCEPT;

EVMU_EXPORT EVMU_LCD_REFRESH_RATE
                        EvmuLcd_refreshRate        (GBL_CSELF)                                           GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setRefreshRate     (GBL_SELF, EVMU_LCD_REFRESH_RATE rate)                GBL_NOEXCEPT;
EVMU_EXPORT EvmuTicks   EvmuLcd_refreshRateTicks   (GBL_CSELF)                                           GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuLcd_iconEnabled        (GBL_CSELF, EVMU_LCD_ICON icon)                       GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setIconEnabled     (GBL_SELF, EVMU_LCD_ICON icon, GblBool enabled)       GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuLcd_pixel              (GBL_CSELF, GblSize row, GblSize col)                 GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setPixel           (GBL_SELF, GblSize row, GblSize col, GblBool enabled) GBL_NOEXCEPT;

EVMU_EXPORT uint8_t     EvmuLcd_decoratedPixel     (GBL_CSELF, GblSize row, GblSize col)                 GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuLcd_ghostingEnabled    (GBL_CSELF)                                           GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setGhostingEnabled (GBL_SELF, GblBool enabled)                           GBL_NOEXCEPT;

EVMU_EXPORT EVMU_LCD_FILTER
                        EvmuLcd_filter             (GBL_CSELF)                                           GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setFilter          (GBL_SELF, EVMU_LCD_FILTER filter)                    GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuLcd_updated            (GBL_CSELF)                                           GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuLcd_setUpdated         (GBL_SELF, GblBool updated)                           GBL_NOEXCEPT;
//EVMU_EXPORT uint8_t   EvmuLcd_filteredPixel      (CSELF, GblSize row, GblSize col)                      GBL_NOEXCEPT;


GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_LCD_H


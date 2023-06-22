/*! \file
 *  \brief LCD display circuit with emulated effects + back-end rendering signals
 *  \ingroup peripherals
 *
 *  Encompasses the API for rendering to and reading from the VMU's
 *  liquid crystal display screen. The API offers:
 *  - reading from and writing to pixels
 *  - enabling and disabling icons
 *  - enabling and disabling filters and effects
 *  - enabling and disabling screen refresh
 *  - changing screen refresh rate
 *  - synchronous event-driven callbacks for back-end drawing
 *
 *  \todo
 *      - Pixel ghosting still needs some work
 *      - update screen when in sleep mode
 *      - access/control XRAM bank
 *
 *  \copyright 2023 Falco Girgis
 */

#ifndef EVMU_LCD_H
#define EVMU_LCD_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_LCD_TYPE                   (GBL_TYPEOF(EvmuLcd))                       //!< Type UUID for EvmuLcd
#define EVMU_LCD(instance)              (GBL_INSTANCE_CAST(instance, EvmuLcd))      //!< Function-style GblInstance cast
#define EVMU_LCD_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuLcd))            //!< Function-style GblClass cast
#define EVMU_LCD_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuLcd)) //!< Get EvmuLcdClass from GblInstance

#define EVMU_LCD_NAME                   "lcd"   //!< Peripheral GblObject name
#define EVMU_LCD_PIXEL_WIDTH            48      //!< Screen resolution (width/rows)
#define EVMU_LCD_PIXEL_HEIGHT           32      //!< Screen resolution (height/columns)
#define EVMU_LCD_ICON_COUNT             4       //!< Number of icons
#define EVMU_LCD_GHOSTING_FRAMES        255     //!< Frame duration for pixel ghosting effect
#define EVMU_LCD_SCREEN_REFRESH_DIVISOR 100      //!< Number of physical refreshes to skip before redrawing

#define GBL_SELF_TYPE EvmuLcd

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuLcd);

//! Refresh rate for the LCD Screen
GBL_DECLARE_ENUM(EVMU_LCD_REFRESH_RATE) {
    EVMU_LCD_REFRESH_83HZ,  //!< 83Hz Refresh rate
    EVMU_LCD_REFRESH_166HZ  //!< 166Hz Refresh rate
};

//! XRAM Bank Numbers
GBL_DECLARE_ENUM(EVMU_XRAM_BANK) {
    EVMU_XRAM_BANK_LCD_TOP,     //!< Top (0) Bank
    EVMU_XRAM_BANK_LCD_BOTTOM,  //!< Bottom (1) Bank
    EVMU_XRAM_BANK_ICON         //!< Icon (2) Bank
};

//! LCD Screen Icons
GBL_DECLARE_FLAGS(EVMU_LCD_ICONS) {
    EVMU_LCD_ICONS_NONE  = 0x0, //!< No Icon
    EVMU_LCD_ICON_FILE   = 0x1, //!< File Icon (Notepad)
    EVMU_LCD_ICON_GAME   = 0x2, //!< Game Icon (Card)
    EVMU_LCD_ICON_CLOCK  = 0x4, //!< Clock Icon (Analog Clock)
    EVMU_LCD_ICON_FLASH  = 0x8, //!< Flash Icon (!)
    EVMU_LCD_ICONS_ALL   = 0xf  //!< All Icons
};

/*! \struct  EvmuLcdClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass for EvmuLcd
 *
 *  EvmuLcdClass contains a single overridable virtual method
 *  for triggering a renderer back-end to redraw the framebuffer.
 *  This will only be called in the event that a pixel has actually
 *  changed, and will be synchronized with the refresh rate.
 *
 *  \note
 *  The default implementation simply emits the "screenRefresh" signal.
 *
 *  \sa EvmuLcd
 */
GBL_CLASS_DERIVE(EvmuLcd, EvmuPeripheral)
    EVMU_RESULT (*pFnRefreshScreen)(GBL_SELF); //!< Called when screen updates
GBL_CLASS_END

/*! \struct  EvmuLcd
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   EvmuLcd screen and framebuffer peripheral
 *
 *  EvmuLcd is the instance structure providing an API around the VMU's
 *  LCD screen.
 *
 *  \sa EvmuLcdClass
 */
GBL_INSTANCE_DERIVE(EvmuLcd, EvmuPeripheral)
    size_t   screenRefreshDivisor; //!< How many hardware refreshes before software refresh
    uint32_t screenChanged   : 1;  //!< User-driven toggle for knowing when to redraw
    uint32_t ghostingEnabled : 1;  //!< Emulate pixel ghosting/fade effect
    uint32_t filterEnabled   : 1;  //!< Enable linear filtering
    uint32_t invertColors    : 1;  //!< Swap black and white pixel values
GBL_INSTANCE_END

//! \cond
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
//! \endcond

/*! Returns the UUID for the EvmuLcd type
 *  \relatesalso EvmuLcd
 *  \static
 *
 *  \returns            GblType UUID
 */
EVMU_EXPORT GblType   EvmuLcd_type              (void)                                 GBL_NOEXCEPT;

/*! Returns whether the LCD screen is enabled
 *  \relatesalso EvmuLcd
 *
 *  \returns            GBL_TRUE if the LCD is enabled, GBL_FALSE otherwise
 *
 *  \sa EvmuLcd_setScreenEnabled
 */
EVMU_EXPORT GblBool   EvmuLcd_screenEnabled     (GBL_CSELF)                            GBL_NOEXCEPT;

/*! Enables of disables the LCD screen
 *  \relatesalso EvmuLcd
 *
 *  \param enabled      GBL_TRUE to enable the screen, GBL_FALSE to disable
 *
 *  \sa EvmuLcd_screenEnabled
 */
EVMU_EXPORT void      EvmuLcd_setScreenEnabled  (GBL_SELF, GblBool enabled)            GBL_NOEXCEPT;

/*! Returns whether LCD screen refreshing is enabled
 *  \relatesalso EvmuLcd
 *
 *  \returns            GBL_TRUE if the refreshing is enabled, GBL_FALSE otherwise
 *
 *  \sa EvmuLcd_setRefreshEnabled
 */
EVMU_EXPORT GblBool   EvmuLcd_refreshEnabled    (GBL_CSELF)                            GBL_NOEXCEPT;

/*! Enables of disables refreshing of the LCD screen
 *  \relatesalso EvmuLcd
 *
 *  \param enabled      GBL_TRUE to enable refreshing, GBL_FALSE to disable
 *
 *  \sa EvmuLcd_refreshEnabled
 */
EVMU_EXPORT void      EvmuLcd_setRefreshEnabled (GBL_SELF, GblBool enabled)            GBL_NOEXCEPT;

/*! Returns the refresh rate of the LCD screen
 *  \relatesalso EvmuLcd
 *
 *  \returns            EVMU_LCD_REFRESH_RATE
 *
 *  \sa EvmuLcd_setRefreshRate, EvmuLcd_refreshRateTicks
 */
EVMU_EXPORT EVMU_LCD_REFRESH_RATE
                      EvmuLcd_refreshRate       (GBL_CSELF)                            GBL_NOEXCEPT;

/*! Sets the refresh rate of the LCD screen
 *  \relatesalso EvmuLcd
 *
 *  \param rate      EVMU_LCD_REFRESH_RATE
 *
 *  \sa EvmuLcd_refreshRate
 */
EVMU_EXPORT void      EvmuLcd_setRefreshRate    (GBL_SELF, EVMU_LCD_REFRESH_RATE rate) GBL_NOEXCEPT;

/*! Returns the refresh rate (msec)
 *  \relatesalso EvmuLcd
 *
 *  \returns          refresh rate (in milliseconds)
 *
 *  \sa EvmuLcd_refreshRate
 */
EVMU_EXPORT EvmuTicks EvmuLcd_refreshRateTicks  (GBL_CSELF)                            GBL_NOEXCEPT;

/*! Returns the active icon mask
 *  \relatesalso EvmuLcd
 *
 *  \returns          Mask containing all active icons
 *
 *  \sa EvmuLcd_setIcons
 */
EVMU_EXPORT EVMU_LCD_ICONS
                      EvmuLcd_icons             (GBL_CSELF)                            GBL_NOEXCEPT;

/*! Sets the active icons to the given mask
 *  \relatesalso EvmuLcd
 *
 *  \param icons      Bitmask of all active icons OR'd together
 *
 *  \sa EvmuLcd_icons
 */
EVMU_EXPORT void      EvmuLcd_setIcons          (GBL_SELF, EVMU_LCD_ICONS icons)       GBL_NOEXCEPT;

/*! Returns the raw pixel value for the given screen coordinate
 *  \relatesalso EvmuLcd
 *
 *  \note
 *  This value does not not include any filtering or optional effects.
 *
 *  \param row      Screen Y coordinate
 *  \param col      Screen X coordinate
 *  \returns        GBL_TRUE if the pixel is lit, GBL_FALSE otherwise
 *
 *  \sa EvmuLcd_setPixel, EvmuLcd_decoratedPixel
 */
EVMU_EXPORT GblBool   EvmuLcd_pixel             (GBL_CSELF, size_t row, size_t col)    GBL_NOEXCEPT;

/*! Sets the raw pixel value for the given screen coordinate
 *  \relatesalso EvmuLcd
 *
 *  \param row      Screen Y coordinate
 *  \param col      Screen X coordinate
 *  \param enabled  GBL_TRUE if the pixel is lit, GBL_FALSE otherwise
 *
 *  \sa EvmuLcd_pixel
 */
EVMU_EXPORT void      EvmuLcd_setPixel          (GBL_SELF,
                                                 size_t row,
                                                 size_t col,
                                                 GblBool enabled)                      GBL_NOEXCEPT;

/*! Retrieves the decorated pixel value for the given screen coordinate
 *  \relatesalso EvmuLcd
 *
 *  \note
 *  This pixel value takes into account all of the optional effects such as
 *  linear filtering and pixel ghosting.
 *
 *  \param row      Screen Y coordinate
 *  \param col      Screen X coordinate
 *  \returns        Grayscale intensity value (0-255) for the given coordinate
 *
 *  \sa EvmuLcd_pixel
 */
EVMU_EXPORT uint8_t   EvmuLcd_decoratedPixel    (GBL_CSELF, size_t row, size_t col)    GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_LCD_H


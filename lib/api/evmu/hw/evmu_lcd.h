/*! \file
 *  \brief EvmuLcd display circuit + back-end rendering signals
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
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_LCD_H
#define EVMU_LCD_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_LCD_TYPE            (GBL_TYPEID(EvmuLcd))            //!< Type UUID for EvmuLcd
#define EVMU_LCD(self)           (GBL_CAST(EvmuLcd, self))        //!< Casts GblInstance to EvmuLcd
#define EVMU_LCD_CLASS(klass)    (GBL_CLASS_CAST(EvmuLcd, klass)) //!< Casts GblClass to EvmuLcdClass
#define EVMU_LCD_GET_CLASS(self) (GBL_CLASSOF(EvmuLcd, self))     //!< Get EvmuLcdClass from GblInstance
//! @}

#define EVMU_LCD_NAME   "lcd"   //!< Peripheral GblObject name

/*! \name  Display Constants
 *  \brief Constants used to define display characteristics
 *  @{
 */
#define EVMU_LCD_PIXEL_WIDTH    48  //!< Screen resolution (width/rows)
#define EVMU_LCD_PIXEL_HEIGHT   32  //!< Screen resolution (height/columns)
#define EVMU_LCD_ICON_COUNT     4   //!< Number of icons
//! @}

/*! \name  Emulator Settings
 *  \brief Constants used to define emulation behavior
 *  @{
 */
#define EVMU_LCD_GHOSTING_FRAMES        100 //!< Frame duration for pixel ghosting effect
#define EVMU_LCD_SCREEN_REFRESH_DIVISOR 199 //!< Number of physical refreshes to skip before redrawing
//! @}

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

//! Returns the GblType UUID associated with EvmuLcd
EVMU_EXPORT GblType EvmuLcd_type (void) GBL_NOEXCEPT;

/*! \name Configuration
 *  \brief Methods for querying the display configuration
 *  \relatesalso EvmuLcd
 *  @{
 */
//! Returns GBL_TRUE if the display is enabled/powered, GBL_FALSE otherwise
EVMU_EXPORT GblBool   EvmuLcd_screenEnabled    (GBL_CSELF) GBL_NOEXCEPT;
//! Returns GBL_TRUE if screen refreshing is currently enabled for the display, GBL_FALSE otherwise
EVMU_EXPORT GblBool   EvmuLcd_refreshEnabled   (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the refresh rate configuration for how frequently the display updates
EVMU_EXPORT EVMU_LCD_REFRESH_RATE
                      EvmuLcd_refreshRate      (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the refresh rate / update time of the LCD screen in milliseconds
EVMU_EXPORT EvmuTicks EvmuLcd_refreshRateTicks (GBL_CSELF) GBL_NOEXCEPT;
//! @}

/*! \name Configuring
 *  \brief Methods for setting and altering the display configuration
 *  \relatesalso EvmuLcd
 *  @{
 */
//! Enables or disables the VMU's LCD display, depending on the value of \p enabled
EVMU_EXPORT void EvmuLcd_setScreenEnabled  (GBL_SELF, GblBool enabled)            GBL_NOEXCEPT;
//! Enables or disables automatic screen refreshing for the display, depending on the value of \p enabled
EVMU_EXPORT void EvmuLcd_setRefreshEnabled (GBL_SELF, GblBool enabled)            GBL_NOEXCEPT;
//! Sets the refresh rate of the LCD screen to the given \p rate value
EVMU_EXPORT void EvmuLcd_setRefreshRate    (GBL_SELF, EVMU_LCD_REFRESH_RATE rate) GBL_NOEXCEPT;
//! @}

/*! \name Display Reading
 *  \brief Methods to retrieve display values for rendering
 *  \relatesalso EvmuLcd
 *  @{
 */
//! Returns a value containing the bitmasks of all of the enabled icons OR'd together
EVMU_EXPORT EVMU_LCD_ICONS
                    EvmuLcd_icons          (GBL_CSELF)                         GBL_NOEXCEPT;
//! Returns the raw pixel value for the given screen coordinate, GBL_TRUE being black and GBL_FALSE being white
EVMU_EXPORT GblBool EvmuLcd_pixel          (GBL_CSELF, size_t row, size_t col) GBL_NOEXCEPT;
//! Retrieves the decorated pixel value for the given screen coordinate, with all effects enabled
EVMU_EXPORT uint8_t EvmuLcd_decoratedPixel (GBL_CSELF, size_t row, size_t col) GBL_NOEXCEPT;
//! @}

/*! \name Display Rendering
 *  \brief Methods to set and modify display values
 *  \relatesalso EvmuLcd
 *  @{
 */
//! Sets the active icons to the mask given by \p icons, which has individual icon masks OR'd together
EVMU_EXPORT void EvmuLcd_setIcons (GBL_SELF, EVMU_LCD_ICONS icons)                    GBL_NOEXCEPT;
//! Sets the raw pixel value for the given screen coordinate, with \p enabled signifying a black pixel
EVMU_EXPORT void EvmuLcd_setPixel (GBL_SELF, size_t row, size_t col, GblBool enabled) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_LCD_H


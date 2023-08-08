#include "hw/evmu_lcd_.h"
#include "hw/evmu_device_.h"
#include "hw/evmu_ram_.h"
#include <evmu/hw/evmu_address_space.h>
#include <gimbal/meta/signals/gimbal_marshal.h>

#define EVMU_LCD_REFRESH_TICKS_83HZ_     12
#define EVMU_LCD_REFRESH_TICKS_166HZ_    6

// 6 bytes per row (8 bits per byte) = 48 bits per row
// rows are in groups of 2
// after each group of 2, next row starts after 4 bytes
// maps a row and a byte (col/8) to an address
static int xramAddrLut_[16][6] = {
    { 0x180, 0x181, 0x182, 0x183, 0x184, 0x185 },
    { 0x186, 0x187,	0x188, 0x189, 0x18A, 0x18B },
    { 0x190, 0x191,	0x192, 0x193, 0x194, 0x195 },
    { 0x196, 0x197,	0x198, 0x199, 0x19A, 0x19B },
    { 0x1A0, 0x1A1,	0x1A2, 0x1A3, 0x1A4, 0x1A5 },
    { 0x1A6, 0x1A7,	0x1A8, 0x1A9, 0x1AA, 0x1AB },
    { 0x1B0, 0x1B1,	0x1B2, 0x1B3, 0x1B4, 0x1B5 },
    { 0x1B6, 0x1B7,	0x1B8, 0x1B9, 0x1BA, 0x1BB },
    { 0x1C0, 0x1C1,	0x1C2, 0x1C3, 0x1C4, 0x1C5 },
    { 0x1C6, 0x1C7,	0x1C8, 0x1C9, 0x1CA, 0x1CB },
    { 0x1D0, 0x1D1,	0x1D2, 0x1D3, 0x1D4, 0x1D5 },
    { 0x1D6, 0x1D7,	0x1D8, 0x1D9, 0x1DA, 0x1DB },
    { 0x1E0, 0x1E1,	0x1E2, 0x1E3, 0x1E4, 0x1E5 },
    { 0x1E6, 0x1E7,	0x1E8, 0x1E9, 0x1EA, 0x1EB },
    { 0x1F0, 0x1F1,	0x1F2, 0x1F3, 0x1F4, 0x1F5 },
    { 0x1F6, 0x1F7,	0x1F8, 0x1F9, 0x1FA, 0x1FB }
};

#define FOREACH_ICON_BIT_(varName, curIconName, icons) \
    for(size_t curIconName = 0, varName = 0; curIconName < EVMU_LCD_ICON_COUNT; ++curIconName) \
        if((varName = iconBit_(icons & GBL_BIT_MASK(1, curIconName))) == GBL_NPOS) continue; \
        else


GBL_INLINE size_t iconBit_(GblFlags icon) {
    size_t bit = GBL_NPOS;

    switch(icon) {
    case EVMU_LCD_ICON_FILE:
        bit = 6;
        break;
    case EVMU_LCD_ICON_GAME:
        bit = 4;
        break;
    case EVMU_LCD_ICON_CLOCK:
        bit = 2;
        break;
    case EVMU_LCD_ICON_FLASH:
        bit = 0;
        break;
    default:
        bit = GBL_NPOS;
        break;
    }

    return bit;
}

GBL_INLINE void xramBitFromRowCol_(int x, int y, unsigned* bank, int* addr, unsigned* bit) {
    *bank = y/16;
    unsigned row = y%16;
    unsigned col = x/8;
    *addr = xramAddrLut_[row][col];
    *bit = 7-x%8;
}

static void updateLcdBuffer_(EvmuLcd* pLcd) {
    EvmuLcd_* pLcd_ = EVMU_LCD_(pLcd);

    unsigned char *sfr = pLcd_->pRam->sfr;
    unsigned char (*xram)[0x80] = pLcd_->pRam->xram;
      int y, x, b=0, p=0;

    const int pixelDelta = pLcd->ghostingEnabled? 1 : EVMU_LCD_GHOSTING_FRAMES;

    p = sfr[0x22];
    if(p>=0x83)
        p -= 0x83;
    b = (p>>6);
    p = (p&0x3f)*2;
    for(y=0; y<32; y++) {
        for(x=0; x<48; ) {
            unsigned value = xram[b][p++];
            for(int i = 7; i >= 0; --i) {
                int prevVal = pLcd_->pixelBuffer[y][x];
                if(prevVal == -1) {
                    pLcd_->pixelBuffer[y][x] = 0;
                    pLcd->screenChanged = GBL_TRUE;
                }

                if((value>>(unsigned)i)&0x1) {
                    pLcd_->pixelBuffer[y][x] += pixelDelta;
                    if(pLcd_->pixelBuffer[y][x] > EVMU_LCD_GHOSTING_FRAMES)
                        pLcd_->pixelBuffer[y][x] = EVMU_LCD_GHOSTING_FRAMES;


                } else {
                    pLcd_->pixelBuffer[y][x] -= pixelDelta;
                    if(pLcd_->pixelBuffer[y][x] < 0)
                        pLcd_->pixelBuffer[y][x] = 0;
                }

                if(pLcd_->pixelBuffer[y][x] != prevVal)
                    pLcd->screenChanged = GBL_TRUE;

                x++;
            }

            if((p&0xf)>=12)
                p+=4;
            if(p>=128) {
                b++;
                p-=128;
            }
            if(b==2 && p>=6) {
                b = 0;
                p -= 6;
            }

        }
    }

    EVMU_LCD_ICONS activeIcons = 0;
    FOREACH_ICON_BIT_(bit, index, EVMU_LCD_ICONS_ALL) {
        const GblBool value = !!(pLcd_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+index+1]
                                 & GBL_BIT_MASK(1, bit));
        if(value) activeIcons |= GBL_BIT_MASK(1, index);
    }

    if(activeIcons != pLcd_->icons) {
        pLcd_->icons = activeIcons;
        pLcd->screenChanged = GBL_TRUE;
    }
}

EVMU_EXPORT void EvmuLcd_setPixel(EvmuLcd* pSelf, size_t x, size_t y, GblBool on) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);

    GBL_ASSERT(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT);

    int addr;
    unsigned bank, bit;
    xramBitFromRowCol_(x, y, &bank, &addr, &bit);
    addr -= 0x180;

    int prevVal = pSelf_->pRam->xram[bank][addr] & (0x1<<bit);

    if(on != prevVal) {
        if(on) {
            pSelf_->pRam->xram[bank][addr] |= (0x1<<bit);
        } else {
            pSelf_->pRam->xram[bank][addr] &= ~(0x1<<bit);
        }

        pSelf->screenChanged = GBL_TRUE;
    }
}

EVMU_EXPORT GblBool EvmuLcd_pixel(const EvmuLcd* pSelf, size_t x, size_t y) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    GBL_ASSERT(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT);

    unsigned bank, bit;
    int addr;

    xramBitFromRowCol_(x, y, &bank, &addr, &bit);
    addr -= 0x180;

    return ((pSelf_->pRam->xram[bank][addr]>>bit)&0x1);

}

static float samplePixel_(const EvmuLcd* pSelf, size_t x, size_t y) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    const uint8_t sample = pSelf_->pixelBuffer[y][x];

    int samples = 20;
    float avg = sample*samples;

    // linear filtering
    if(pSelf->filterEnabled) {
        // above
        if(y > 0) {
            // top left
            if(x > 0) avg += pSelf_->pixelBuffer[y-1][x-1];
            else avg += sample;

            // top middle
            avg += pSelf_->pixelBuffer[y-1][x];

            // top right
            if(x < EVMU_LCD_PIXEL_WIDTH-1) avg += pSelf_->pixelBuffer[y-1][x+1];
            else avg += sample;

        } else avg += sample * 3;
        samples += 3;

        // below
        if(y < EVMU_LCD_PIXEL_HEIGHT-1) {
            // bottom left
            if(x > 0) avg += pSelf_->pixelBuffer[y+1][x-1];
            else avg += sample;

            // bottom middle
            avg += pSelf_->pixelBuffer[y+1][x];

            // bottom right
            if(x < EVMU_LCD_PIXEL_WIDTH-1) avg += pSelf_->pixelBuffer[y+1][x+1];
            else avg += sample;

        } else avg += sample * 3;
        samples += 3;

        // left
        if(x > 0) avg += pSelf_->pixelBuffer[y][x-1];
        else avg += sample;
        ++samples;

        // right
        if(x < EVMU_LCD_PIXEL_WIDTH-1) avg += pSelf_->pixelBuffer[y][x+1];
        else avg += sample;
        ++samples;
    }

    avg /= (float)samples;

    return avg;

}

EVMU_EXPORT uint8_t EvmuLcd_decoratedPixel(const EvmuLcd* pSelf, size_t x, size_t y) {
    GBL_ASSERT(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT);

    const uint8_t white = (samplePixel_(pSelf, x, y)/(float)EVMU_LCD_GHOSTING_FRAMES)*255.0f;
    return pSelf->invertColors? white : 255 - white;
}

EVMU_EXPORT GblFlags EvmuLcd_icons(const EvmuLcd* pSelf) {
    EVMU_LCD_ICONS icons = 0;
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);

    FOREACH_ICON_BIT_(bit, index, EVMU_LCD_ICONS_ALL) {
        if(pSelf_->pRam->xram[EVMU_XRAM_BANK_ICON]
                                [EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+index+1] & GBL_BIT_MASK(1, bit))
            icons |= GBL_BIT_MASK(1, index);
    }
    return icons;
}

EVMU_EXPORT void EvmuLcd_setIcons(EvmuLcd* pSelf, EVMU_LCD_ICONS icons) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    GblBool changed = GBL_FALSE;

    FOREACH_ICON_BIT_(bit, icon, icons) {
        const GblBool prevVal = !!(pSelf_->pRam->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1] & (0x1<<bit));
        const GblBool enabled = !!(icons & GBL_BIT_MASK(1, icon));

        if(enabled != prevVal) {
            if(enabled) pSelf_->pRam->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1] |= (0x1<<bit);
            else pSelf_->pRam->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1] &= ~(0x1<<bit);
            changed = GBL_TRUE;
        }
    }

    if(changed) {
        pSelf->screenChanged = GBL_TRUE;
        GblSignal_emit(GBL_INSTANCE(pSelf), "iconsChange", icons);
    }
}

EVMU_EXPORT GblBool EvmuLcd_screenEnabled(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)]>>EVMU_SFR_VCCR_VCCR7_POS);
}

EVMU_EXPORT void EvmuLcd_setScreenEnabled(EvmuLcd* pSelf, GblBool enabled) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);

    const GblBool prevValue =
            !!(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] & EVMU_SFR_VCCR_VCCR7_MASK);

    if(prevValue != enabled) {
        if(enabled)
            pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] |= EVMU_SFR_VCCR_VCCR7_MASK;
        else
            pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] &= ~EVMU_SFR_VCCR_VCCR7_MASK;

        pSelf->screenChanged = GBL_TRUE;

        GblSignal_emit(GBL_INSTANCE(pSelf), "screenToggle", enabled);
    }
}

EVMU_EXPORT GblBool EvmuLcd_refreshEnabled(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)]>>EVMU_SFR_MCR_MCR3_POS);
}

EVMU_EXPORT void EvmuLcd_setRefreshEnabled(EvmuLcd* pSelf, GblBool enabled) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    int wasEnabled =  pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] & EVMU_SFR_MCR_MCR3_MASK;

    if(enabled != wasEnabled) {
        if(enabled) {
            pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] |= EVMU_SFR_MCR_MCR3_MASK;
        } else {
            pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] &= ~EVMU_SFR_MCR_MCR3_MASK;
        }
     // pSelf_->updated = 1;
    }
}

EVMU_EXPORT EVMU_LCD_REFRESH_RATE EvmuLcd_refreshRate(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)]>>EVMU_SFR_MCR_MCR4_POS);
}

EVMU_EXPORT void EvmuLcd_setRefreshRate(EvmuLcd* pSelf, EVMU_LCD_REFRESH_RATE rate) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    if(rate) {
        pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] |= EVMU_SFR_MCR_MCR4_MASK;
    } else {
        pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] &= ~EVMU_SFR_MCR_MCR4_MASK;
    }
}

EVMU_EXPORT EvmuTicks EvmuLcd_refreshRateTicks(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] & EVMU_SFR_MCR_MCR4_MASK)?
                EVMU_LCD_REFRESH_TICKS_83HZ_ : EVMU_LCD_REFRESH_TICKS_166HZ_;
}

EVMU_EXPORT void EvmuLcd_drawLogo(const EvmuLcd* pSelf) {
    const static uint8_t byteArray[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
        0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff,
        0x01, 0x70, 0xff, 0xff, 0xff, 0xfe, 0x07, 0x3e, 0x7f, 0xff, 0xff, 0xfc, 0x0f, 0x3f, 0x3f, 0xff,
        0xff, 0xf8, 0x3e, 0x3f, 0x9f, 0xff, 0xff, 0xf8, 0x7e, 0xbf, 0xdf, 0xff, 0xff, 0xf0, 0x7e, 0x9f,
        0xef, 0xff, 0xff, 0xf0, 0xfa, 0x9f, 0xff, 0xff, 0xff, 0xe0, 0xfa, 0x9f, 0xff, 0xff, 0xff, 0xe1,
        0xfa, 0x9f, 0xff, 0xff, 0xff, 0xe1, 0xf8, 0x9f, 0xff, 0xff, 0xff, 0xe1, 0xf8, 0x9b, 0xff, 0xff,
        0xff, 0xe1, 0xea, 0x09, 0xf7, 0xff, 0xff, 0xe0, 0xea, 0x09, 0xf7, 0xff, 0xff, 0xf0, 0xe2, 0x81,
        0xef, 0xff, 0xff, 0xf0, 0x6a, 0x81, 0xef, 0xff, 0xff, 0xf8, 0x40, 0x80, 0xcf, 0xff, 0xff, 0xf8,
        0x20, 0x00, 0x9f, 0xff, 0xff, 0xfc, 0x18, 0x02, 0x1f, 0xff, 0xff, 0xfe, 0x03, 0x00, 0x3f, 0xff,
        0xff, 0xff, 0x02, 0x00, 0x7f, 0xff, 0xff, 0xff, 0x82, 0x00, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x83,
        0xff, 0xff, 0xff, 0xff, 0xfc, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x9f, 0xff, 0xff, 0xff, 0xff,
        0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff
    };

}

static GBL_RESULT EvmuLcd_refreshScreen_(EvmuLcd* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_CALL(GblSignal_emit(GBL_INSTANCE(pSelf), "screenRefresh"));

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_GblObject_property_(const GblObject* pObject, const GblProperty* pProp, GblVariant* pValue) {
    GBL_CTX_BEGIN(NULL);

    EvmuLcd* pSelf = EVMU_LCD(pObject);

    switch(pProp->id) {
    case EvmuLcd_Property_Id_screenEnabled:
        GblVariant_setBool(pValue, EvmuLcd_screenEnabled(pSelf));
        break;
    case EvmuLcd_Property_Id_refreshEnabled:
        GblVariant_setBool(pValue, EvmuLcd_refreshEnabled(pSelf));
        break;
    case EvmuLcd_Property_Id_refreshRate:
        GblVariant_setEnum(pValue, EvmuLcd_refreshRate(pSelf), GBL_ENUM_TYPE);
        break;
    case EvmuLcd_Property_Id_ghostingEnabled:
        GblVariant_setBool(pValue, pSelf->ghostingEnabled);
        break;
    case EvmuLcd_Property_Id_filterEnabled:
        GblVariant_setBool(pValue, pSelf->filterEnabled);
        break;
    case EvmuLcd_Property_Id_invertColors:
        GblVariant_setBool(pValue, pSelf->invertColors);
        break;
    case EvmuLcd_Property_Id_icons:
        GblVariant_setFlags(pValue, EvmuLcd_icons(pSelf), GBL_FLAGS_TYPE);
        break;
    default:
        return GBL_RESULT_ERROR_INVALID_PROPERTY;
    }


    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_GblObject_setProperty_(GblObject* pObject, const GblProperty* pProp, GblVariant* pValue) {
    GBL_CTX_BEGIN(NULL);

    EvmuLcd* pSelf = EVMU_LCD(pObject);

    switch(pProp->id) {
    case EvmuLcd_Property_Id_screenEnabled:
        EvmuLcd_setScreenEnabled(pSelf, GblVariant_toBool(pValue));
        break;
    case EvmuLcd_Property_Id_refreshEnabled:
        EvmuLcd_setRefreshEnabled(pSelf, GblVariant_toBool(pValue));
        break;
    case EvmuLcd_Property_Id_refreshRate:
        EvmuLcd_setRefreshRate(pSelf, GblVariant_toEnum(pValue));
        break;
    case EvmuLcd_Property_Id_ghostingEnabled:
        pSelf->ghostingEnabled = GblVariant_toBool(pValue);
        break;
    case EvmuLcd_Property_Id_filterEnabled:
        pSelf->filterEnabled = GblVariant_toBool(pValue);
        break;
    case EvmuLcd_Property_Id_invertColors:
        pSelf->invertColors = GblVariant_toBool(pValue);
        break;
    case EvmuLcd_Property_Id_icons:
        EvmuLcd_setIcons(pSelf, GblVariant_toFlags(pValue));
        break;
    default:
        return GBL_RESULT_ERROR_INVALID_PROPERTY;
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_LCD_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_IBehavior_update_(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);

    EvmuLcd*  pLcd   = EVMU_LCD(pSelf);
    EvmuLcd_* pLcd_  = EVMU_LCD_(pLcd);

    pLcd_->refreshElapsed += ticks;

    if(!EvmuLcd_refreshEnabled(pLcd))
        GBL_CTX_DONE();

    EvmuTicks refreshTicks = EvmuLcd_refreshRateTicks(pLcd) * EVMU_LCD_SCREEN_REFRESH_DIVISOR;
    GblBool screenChanged = GBL_FALSE;
    while(pLcd_->refreshElapsed >= refreshTicks) {
        pLcd_->refreshElapsed -= refreshTicks;
        updateLcdBuffer_(pLcd);
        if(pLcd->screenChanged) {
            screenChanged = GBL_TRUE;
        }
    }

    if(screenChanged)
        GBL_VCALL(EvmuLcd, pFnRefreshScreen, pLcd);

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_IBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    EvmuLcd*  pLcd   = EVMU_LCD(pSelf);
    EvmuLcd_* pLcd_  = EVMU_LCD_(pLcd);

    memset(pLcd_->pixelBuffer, -1, sizeof(int)*EVMU_LCD_PIXEL_WIDTH *EVMU_LCD_PIXEL_HEIGHT);
    pLcd->screenChanged = GBL_TRUE;
    pLcd_->icons = EVMU_LCD_ICON_GAME;

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_init_(GblInstance* pInstance) {
    GBL_CTX_BEGIN(NULL);

    EvmuLcd* pSelf = EVMU_LCD(pInstance);

    GblObject_setName(GBL_OBJECT(pInstance), EVMU_LCD_NAME);
    pSelf->screenRefreshDivisor = EVMU_LCD_SCREEN_REFRESH_DIVISOR;

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcdClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);

    if(!GblType_classRefCount(GblClass_typeOf(pClass))) {
        GBL_PROPERTIES_REGISTER(EvmuLcd);

        GBL_CTX_CALL(GblSignal_install(GblClass_typeOf(pClass),
                                       "screenRefresh",
                                       GblMarshal_CClosure_VOID__INSTANCE,
                                       0));

        GBL_CTX_CALL(GblSignal_install(GblClass_typeOf(pClass),
                                       "screenToggle",
                                       GblMarshal_CClosure_VOID__INSTANCE_BOOL,
                                       1,
                                       GBL_BOOL_TYPE));

        GBL_CTX_CALL(GblSignal_install(GblClass_typeOf(pClass),
                                       "iconsChange",
                                       GblMarshal_CClosure_VOID__INSTANCE_FLAGS,
                                       1,
                                       GBL_FLAGS_TYPE));
    }

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed   = EvmuLcd_GblObject_constructed_;
    GBL_OBJECT_CLASS(pClass)    ->pFnProperty      = EvmuLcd_GblObject_property_;
    GBL_OBJECT_CLASS(pClass)    ->pFnSetProperty   = EvmuLcd_GblObject_setProperty_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate        = EvmuLcd_IBehavior_update_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset         = EvmuLcd_IBehavior_reset_;
    EVMU_LCD_CLASS(pClass)      ->pFnRefreshScreen = EvmuLcd_refreshScreen_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuLcd_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuLcdClass),
        .pFnClassInit           = EvmuLcdClass_init_,
        .instanceSize           = sizeof(EvmuLcd),
        .instancePrivateSize    = sizeof(EvmuLcd_),
        .pFnInstanceInit        = EvmuLcd_init_
    };

    if(type == GBL_INVALID_TYPE) {
        type = GblType_register(GblQuark_internStatic("EvmuLcd"),
                                EVMU_PERIPHERAL_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}


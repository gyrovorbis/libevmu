#include "gyro_vmu_device.h"
#include "hw/evmu_lcd_.h"
#include "hw/evmu_device_.h"
#include "hw/evmu_memory_.h"
#include <evmu/hw/evmu_address_space.h>

#define EVMU_LCD_REFRESH_RATE_DIVISOR_  50

#define EVMU_LCD_REFRESH_TICKS_83HZ_     83
#define EVMU_LCD_REFRESH_TICKS_166HZ_    166

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


GBL_INLINE void xramBitFromRowCol_(int x, int y, unsigned* bank, int* addr, unsigned* bit) {
    *bank = y/16;
    unsigned row = y%16;
    unsigned col = x/8;
    *addr = xramAddrLut_[row][col];
    *bit = 7-x%8;
}

static void updateLcdBuffer_(EvmuLcd_* pLcd_) {
    unsigned char *sfr = pLcd_->pMemory->sfr;
    unsigned char (*xram)[0x80] = pLcd_->pMemory->xram;
      int y, x, b=0, p=0;

    const int pixelDelta = pLcd_->ghostingEnabled? 1 : EVMU_LCD_GHOSTING_FRAMES;

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
                    pLcd_->updated = 1;
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
                    pLcd_->updated = 1;

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

    for(int i = 0; i < EVMU_LCD_ICON_COUNT; ++i) {
        uint8_t value = pLcd_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_XRAM_ICN_FILE)+i];
        if(pLcd_->dispIcons[i] != value) {
            pLcd_->updated = 1;
            pLcd_->dispIcons[i] = value;
        }
    }
}

EVMU_EXPORT void EvmuLcd_setPixel(EvmuLcd* pSelf, GblSize x, GblSize y, GblBool on) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);

    GBL_ASSERT(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT);

    int addr;
    unsigned bank, bit;
    xramBitFromRowCol_(x, y, &bank, &addr, &bit);
    addr -= 0x180;

    int prevVal = pSelf_->pMemory->xram[bank][addr] & (0x1<<bit);

    if(on != prevVal) {
        if(on) {
            pSelf_->pMemory->xram[bank][addr] |= (0x1<<bit);
        } else {
            pSelf_->pMemory->xram[bank][addr] &= ~(0x1<<bit);
        }

        pSelf_->updated = 1;
    }
}

EVMU_EXPORT GblBool EvmuLcd_pixel(const EvmuLcd* pSelf, GblSize x, GblSize y) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    GBL_ASSERT(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT);

    unsigned bank, bit;
    int addr;

    xramBitFromRowCol_(x, y, &bank, &addr, &bit);
    addr -= 0x180;

    return ((pSelf_->pMemory->xram[bank][addr]>>bit)&0x1);

}

static float samplePixel_(EvmuLcd_* pSelf_, GblSize x, GblSize y) {
    const uint8_t sample = pSelf_->pixelBuffer[y][x];

    int samples = 15;
    float avg = sample*samples;

    if(pSelf_->filter == EVMU_LCD_FILTER_LINEAR) {
        // above
        if(y > 0) avg += pSelf_->pixelBuffer[y-1][x];
        else avg += sample;
        ++samples;

        // below
        if(y < EVMU_LCD_PIXEL_HEIGHT-1) avg += pSelf_->pixelBuffer[y+1][x];
        else avg += sample;
        ++samples;

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

EVMU_EXPORT uint8_t EvmuLcd_decoratedPixel(const EvmuLcd* pSelf, GblSize x, GblSize y) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    GBL_ASSERT(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT);

    return 255 - (samplePixel_(pSelf_, x, y)/(float)EVMU_LCD_GHOSTING_FRAMES)*255.0f;
}

EVMU_EXPORT GblBool EvmuLcd_iconEnabled(const EvmuLcd* pSelf, EVMU_LCD_ICON icon) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);

    GBL_ASSERT(icon < EVMU_LCD_ICON_COUNT);
    uint8_t bit;

    //0x181-184, icons at bit 6-0
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
    default:
    case EVMU_LCD_ICON_FLASH:
        bit = 0;
        break;
    }

    return (pSelf_->pMemory->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1]>>bit)&0x1;
}


EVMU_EXPORT void EvmuLcd_setIconEnabled(EvmuLcd* pSelf, EVMU_LCD_ICON icon, GblBool enabled) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);

    GBL_ASSERT(icon >= 0 && icon < EVMU_LCD_ICON_COUNT);
    uint8_t bit;

    //0x181-184, icons at bit 6-0
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
    default:
    case EVMU_LCD_ICON_FLASH:
        bit = 0;
        break;
    }

    int prevVal = pSelf_->pMemory->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1] & (0x1<<bit);

    if(enabled != prevVal) {
        if(enabled) pSelf_->pMemory->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1] |= (0x1<<bit);
        else pSelf_->pMemory->xram[EVMU_XRAM_BANK_ICON][EVMU_XRAM_OFFSET(EVMU_ADDRESS_SEGMENT_XRAM_BASE)+icon+1] &= ~(0x1<<bit);
        pSelf_->updated = 1;
    }
}

EVMU_EXPORT GblBool EvmuLcd_displayEnabled(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)]>>EVMU_SFR_VCCR_VCCR7_POS);
}

EVMU_EXPORT void EvmuLcd_setDisplayEnabled(EvmuLcd* pSelf, GblBool enabled) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    if(enabled) {
        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] |= EVMU_SFR_VCCR_VCCR7_MASK;
    } else {
        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] &= ~EVMU_SFR_VCCR_VCCR7_MASK;
    }
    pSelf_->updated = 1;
}

EVMU_EXPORT GblBool EvmuLcd_refreshEnabled(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)]>>EVMU_SFR_MCR_MCR3_POS);
}

EVMU_EXPORT void EvmuLcd_setRefreshEnabled(EvmuLcd* pSelf, GblBool enabled) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    int wasEnabled =  pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] & EVMU_SFR_MCR_MCR3_MASK;

    if(enabled != wasEnabled) {
        if(enabled) {
            pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] |= EVMU_SFR_MCR_MCR3_MASK;
        } else {
            pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] &= ~EVMU_SFR_MCR_MCR3_MASK;
        }
     // pSelf_->updated = 1;
    }
}

EVMU_EXPORT EVMU_LCD_REFRESH_RATE EvmuLcd_refreshRate(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)]>>EVMU_SFR_MCR_MCR4_POS);
}

EVMU_EXPORT void EvmuLcd_setRefreshRate(EvmuLcd* pSelf, EVMU_LCD_REFRESH_RATE rate) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    if(rate) {
        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] |= EVMU_SFR_MCR_MCR4_MASK;
    } else {
        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)] &= ~EVMU_SFR_MCR_MCR4_MASK;
    }
}

EVMU_EXPORT EvmuTicks EvmuLcd_refreshRateTicks(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_MCR)]>>EVMU_SFR_MCR_MCR4_POS)?
                EVMU_LCD_REFRESH_TICKS_83HZ_ : EVMU_LCD_REFRESH_TICKS_166HZ_;
}

EVMU_EXPORT GblBool EvmuLcd_ghostingEnabled(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return pSelf_->ghostingEnabled;
}

EVMU_EXPORT void EvmuLcd_setGhostingEnabled(EvmuLcd* pSelf, GblBool enable) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    pSelf_->ghostingEnabled = enable;
    pSelf_->updated = GBL_TRUE;
}

EVMU_EXPORT EVMU_LCD_FILTER EvmuLcd_filter(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return pSelf_->filter;
}

EVMU_EXPORT void EvmuLcd_setFilter(EvmuLcd* pSelf, EVMU_LCD_FILTER filter) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    pSelf_->filter = filter;
    pSelf_->updated = GBL_TRUE;
}

EVMU_EXPORT GblBool EvmuLcd_updated(const EvmuLcd* pSelf) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    return pSelf_->updated;
}

EVMU_EXPORT void EvmuLcd_setUpdated(EvmuLcd* pSelf, GblBool updated) {
    EvmuLcd_* pSelf_ = EVMU_LCD_(pSelf);
    pSelf_->updated = updated;
}

static GBL_RESULT EvmuLcd_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_LCD_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_IBehavior_update_(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);

    EvmuLcd*  pLcd   = EVMU_LCD(pSelf);
    EvmuLcd_* pLcd_  = EVMU_LCD_(pLcd);

    pLcd_->refreshElapsed += ticks;

    EvmuTicks refreshTicks = EvmuLcd_refreshRateTicks(pLcd) * EVMU_LCD_REFRESH_RATE_DIVISOR_;
    while(pLcd_->refreshElapsed >= refreshTicks) {
        pLcd_->refreshElapsed -= refreshTicks;
        updateLcdBuffer_(pLcd_);
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcd_IBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    EvmuLcd*  pLcd   = EVMU_LCD(pSelf);
    EvmuLcd_* pLcd_  = EVMU_LCD_(pLcd);

    memset(pLcd_->pixelBuffer, -1, sizeof(int)*EVMU_LCD_PIXEL_WIDTH*EVMU_LCD_PIXEL_HEIGHT);

    GBL_CTX_END();
}

static GBL_RESULT EvmuLcdClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuLcd_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuLcd_IBehavior_update_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuLcd_IBehavior_reset_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuLcd_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuLcdClass),
        .pFnClassInit           = EvmuLcdClass_init_,
        .instanceSize           = sizeof(EvmuLcd),
        .instancePrivateSize    = sizeof(EvmuLcd_)
    };

    if(!GblType_verify(type)) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuLcd"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}


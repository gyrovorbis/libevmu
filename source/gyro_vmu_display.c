#include "gyro_vmu_display.h"
#include "gyro_vmu_device.h"
#include <gyro_vmu_sfr.h>
#include <assert.h>

/* THIS IS WHERE LIBGYRO-BASED RENDERING SHIT SHOULD BE!!!! */

// 6 bytes per row (8 bits per byte) = 48 bits per row
// rows are in groups of 2
// after each group of 2, next row starts after 4 bytes
// maps a row and a byte (col/8) to an address
static int _xramAddrLUT[16][6] = {
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

int gyVmuDisplayInit(struct VMUDevice* dev) {
    memset(dev->display.lcdBuffer, -1, sizeof(int)*VMU_DISP_PIXEL_WIDTH*VMU_DISP_PIXEL_HEIGHT);
}

inline static void _xramBitFromRowCol(int x, int y, unsigned* bank, int* addr, unsigned* bit) {
    *bank = y/16;
    unsigned row = y%16;
    unsigned col = x/8;
    *addr = _xramAddrLUT[row][col];
    *bit = 7-x%8;
}

void _updateLcdBuffer(VMUDevice* dev) {
    unsigned char *sfr =     dev->sfr;
    unsigned char (*xram)[0x80] =     dev->xram;
      int y, x, b=0, p=0;

    const int pixelDelta = dev->display.ghostingEnabled? 1 : VMU_DISP_GHOSTING_FRAMES;

    p = sfr[0x22];
    if(p>=0x83)
        p -= 0x83;
    b = (p>>6);
    p = (p&0x3f)*2;
    for(y=0; y<32; y++) {
        for(x=0; x<48; ) {
            unsigned value = xram[b][p++];
            for(int i = 7; i >= 0; --i) {
                int prevVal = dev->display.lcdBuffer[y][x];
                if(prevVal == -1) {
                    dev->display.lcdBuffer[y][x] = 0;
                    dev->display.screenChanged = 1;
                }

                if((value>>(unsigned)i)&0x1) {
                    dev->display.lcdBuffer[y][x] += pixelDelta;
                    if(dev->display.lcdBuffer[y][x] > VMU_DISP_GHOSTING_FRAMES)
                        dev->display.lcdBuffer[y][x] = VMU_DISP_GHOSTING_FRAMES;


                } else {
                    dev->display.lcdBuffer[y][x] -= pixelDelta;
                    if(dev->display.lcdBuffer[y][x] < 0)
                        dev->display.lcdBuffer[y][x] = 0;
                }

                if(dev->display.lcdBuffer[y][x] != prevVal)
                    dev->display.screenChanged = 1;

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

    for(int i = 0; i < VMU_DISP_ICN_COUNT; ++i) {
        uint8_t value = dev->sfr[SFR_OFFSET(SFR_ADDR_XRAM_ICN_FILE)+i];
        if(dev->display.dispIcons[i] != value) {
            dev->display.screenChanged = 1;
            dev->display.dispIcons[i] = value;
        }
    }
}

void gyVmuDisplayPixelSet(struct VMUDevice* dev, int x, int y, int on) {
    assert(x >= 0 && x < VMU_DISP_PIXEL_WIDTH && y >= 0 && y < VMU_DISP_PIXEL_HEIGHT);

    int addr;
    unsigned bank, bit;
    _xramBitFromRowCol(x, y, &bank, &addr, &bit);
    addr -= 0x180;

    int prevVal = dev->xram[bank][addr] & (0x1<<bit);

    if(on != prevVal) {
        if(on) {
            dev->xram[bank][addr] |= (0x1<<bit);
        } else {
            dev->xram[bank][addr] &= ~(0x1<<bit);
        }

        dev->display.screenChanged = 1;
    }
}

int gyVmuDisplayPixelGet(const VMUDevice* dev, int x, int y) {
    assert(x >= 0 && x < VMU_DISP_PIXEL_WIDTH && y >= 0 && y < VMU_DISP_PIXEL_HEIGHT);

    unsigned bank, bit;
    int addr;

    _xramBitFromRowCol(x, y, &bank, &addr, &bit);
    addr -= 0x180;

    return ((dev->xram[bank][addr]>>bit)&0x1);

}

int gyVmuDisplayPixelGhostValue(const struct VMUDevice* dev, int x, int y) {
    assert(x >= 0 && x < VMU_DISP_PIXEL_WIDTH && y >= 0 && y < VMU_DISP_PIXEL_HEIGHT);
    return 255 - ((float)dev->display.lcdBuffer[y][x]/(float)VMU_DISP_GHOSTING_FRAMES)*255.0f;
}

int gyVmuDisplayIconGet(const VMUDevice *dev, VMU_DISP_ICN icn) {
    assert(icn >= 0 && icn < VMU_DISP_ICN_COUNT);
    uint8_t bit;

    //0x181-184, icons at bit 6-0
    switch(icn) {
    case VMU_DISP_ICN_FILE:
        bit = 6;
        break;
    case VMU_DISP_ICN_GAME:
        bit = 4;
        break;
    case VMU_DISP_ICN_CLOCK:
        bit = 2;
        break;
    default:
    case VMU_DISP_ICN_FLASH:
        bit = 0;
        break;
    }

    return (dev->xram[VMU_XRAM_BANK_ICN][XRAM_OFFSET(SFR_ADDR_XRAM_BASE)+icn+1]>>bit)&0x1;
}


void gyVmuDisplayIconSet(VMUDevice *dev, VMU_DISP_ICN icn, int val) {
    assert(icn >= 0 && icn < VMU_DISP_ICN_COUNT);
    uint8_t bit;

    //0x181-184, icons at bit 6-0
    switch(icn) {
    case VMU_DISP_ICN_FILE:
        bit = 6;
        break;
    case VMU_DISP_ICN_GAME:
        bit = 4;
        break;
    case VMU_DISP_ICN_CLOCK:
        bit = 2;
        break;
    default:
    case VMU_DISP_ICN_FLASH:
        bit = 0;
        break;
    }

    int prevVal = dev->xram[VMU_XRAM_BANK_ICN][XRAM_OFFSET(SFR_ADDR_XRAM_BASE)+icn+1] & (0x1<<bit);

    if(val != prevVal) {
        if(val) dev->xram[VMU_XRAM_BANK_ICN][XRAM_OFFSET(SFR_ADDR_XRAM_BASE)+icn+1] |= (0x1<<bit);
        else dev->xram[VMU_XRAM_BANK_ICN][XRAM_OFFSET(SFR_ADDR_XRAM_BASE)+icn+1] &= ~(0x1<<bit);
        dev->display.screenChanged = 1;
    }
}

int gyVmuDisplayEnabled(const VMUDevice* dev) {
    return (dev->sfr[SFR_OFFSET(SFR_ADDR_VCCR)]>>SFR_VCCR_VCCR7_POS);
}

void gyVmuDisplayEnabledSet(VMUDevice* dev, int enabled) {
    if(enabled) {
        dev->sfr[SFR_OFFSET(SFR_ADDR_VCCR)] |= SFR_VCCR_VCCR7_MASK;
    } else {
        dev->sfr[SFR_OFFSET(SFR_ADDR_VCCR)] &= ~SFR_VCCR_VCCR7_MASK;
    }
}

int gyVmuDisplayUpdateEnabled(const VMUDevice* dev) {
    return (dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)]>>SFR_MCR_MCR3_POS);
}

void gyVmuDisplayUpdateEnabledSet(VMUDevice* dev, int enabled) {
    int wasEnabled =  dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)] & SFR_MCR_MCR3_MASK;

    if(enabled != wasEnabled) {
        if(enabled) {
            dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)] |= SFR_MCR_MCR3_MASK;
        } else {
            dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)] &= ~SFR_MCR_MCR3_MASK;
        }
        dev->display.screenChanged = 1;
    }
}

VMU_DISP_REFRESH_RATE gyVmuDisplayRefreshRate(const struct VMUDevice* dev) {
    return (dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)]>>SFR_MCR_MCR4_POS);
}

void gyVmuDisplaySetRefreshRate(struct VMUDevice* dev, VMU_DISP_REFRESH_RATE rate) {
    if(rate) {
        dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)] |= SFR_MCR_MCR4_MASK;
    } else {
        dev->sfr[SFR_OFFSET(SFR_ADDR_MCR)] &= ~SFR_MCR_MCR4_MASK;
    }
}

int gyVmuDisplayUpdate(struct VMUDevice* dev, double deltaTime) {
    double refreshTime = (1.0/(gyVmuDisplayRefreshRate(dev) == VMU_DISP_REFRESH_83HZ?
                82.7 : 165.5));

    dev->display.refreshElapsed += deltaTime;

    if(dev->display.refreshElapsed >= refreshTime) {
        dev->display.refreshElapsed  = 0.0;
            _updateLcdBuffer(dev);
            return 1;
    }

    return 0;

}

int gyVmuDisplayGhostingEnabledGet(const struct VMUDevice* dev) {
    return dev->display.ghostingEnabled;
}

void gyVmuDisplayGhostingEnabledSet(struct VMUDevice* dev, int enable) {
    dev->display.ghostingEnabled = enable;
}


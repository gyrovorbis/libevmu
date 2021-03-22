#ifndef EVMU_LCD_H
#define EVMU_LCD_H

#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

//3 - Video/LCD Memory (XRAM)
#define XRAM_BANK_SIZE          0x80
#define XRAM_BANK_COUNT         3

#define EVMU_LCD_PIXEL_WIDTH        48
#define EVMU_LCD_PIXEL_HEIGHT       32

#define EVMU_LCD_GHOSTING_FRAMES    10

struct VMUDevice;

typedef enum EVMU_LCD_REFRESH_RATE {
    EVMU_LCD_REFRESH_83HZ,
    EVMU_LCD_REFRESH_166HZ
} EVMU_LCD_REFRESH_RATE;

typedef enum VMU_XRAM_BANK {
    VMU_XRAM_BANK_LCD_TOP = 0,
    VMU_XRAM_BANK_LCD_BOT,
    VMU_XRAM_BANK_ICN
} VMU_XRAM_BANK;

typedef enum EVMU_LCD_ICON_TYPE {
    EVMU_LCD_ICON_FILE,
    EVMU_LCD_ICON_GAME,
    EVMU_LCD_ICON_CLOCK,
    EVMU_LCD_ICON_FLASH,
    EVMU_LCD_ICON_COUNT
} EVMU_LCD_ICON_TYPE;


GBL_DECLARE_HANDLE(EvmuLcd);

GBL_DECLARE_ENUM(EVMU_LCD_FILTER_TYPE) {
    EVMU_LCD_FILTER_NONE,
    EVMU_LCD_FILTER_LINEAR,
    EVMU_LCD_FILTER_COUNT
};

GBL_DECLARE_ENUM(EVMU_LCD_PROPERTY) {
    EVMU_LCD_PROPERTY_LCD_ENABLED = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_LCD_PROPERTY_UPDATE_ENABLED,
    EVMU_LCD_PROPERTY_REFRESH_RATE,
    EVMU_LCD_PROPERTY_LCD_CHANGED,
    EVMU_LCD_PROPERTY_GHOSTING_ENABLED,
    EVMU_LCD_PROPERTY_FILTER_TYPE,
    EVMU_LCD_PROPERTY_FILTER_ROW_COUNT,
    EVMU_LCD_PROPERTY_FILTER_COL_COUNT,
    EVMU_LCD_PROPERTY_COUNT
};

EVMU_API EvmuLcdPixel           (const EvmuLcd* pDisplay, uint32_t row, uint32_t col, GblBool*  pValue);
EVMU_API EvmuLcdPixelSet        (EvmuLcd*       pDisplay, uint32_t row, uint32_t col, GblBool   value);
EVMU_API EvmuLcdPixelGhosted    (const EvmuLcd* pDisplay, uint32_t row, uint32_t col, int*      pValue);
EVMU_API EvmuLcdPixelFiltered   (const EvmuLcd* pDisplay, uint32_t row, uint32_t col, int*      pValue);

EVMU_API EvmuLcdIcon            (const EvmuLcd* pDisplay, EVMU_LCD_ICON_TYPE iconType, GblBool* pValue);
EVMU_API EvmuLcdIconSet         (EvmuLcd*       pDisplay, EVMU_LCD_ICON_TYPE iconType, GblBool value);

#if 0
int     gyVmuDisplayEnabled(const struct VMUDevice* dev);
void    gyVmuDisplayEnabledSet(struct VMUDevice* dev, int enabled);

int     gyVmuDisplayUpdateEnabled(const struct VMUDevice* dev);
void    gyVmuDisplayUpdateEnabledSet(struct VMUDevice* dev, int enabled);

VMU_DISP_REFRESH_RATE
    gyVmuDisplayRefreshRate(const struct VMUDevice* dev);
void gyVmuDisplayRefreshRateSet(struct VMUDevice* dev, VMU_DISP_REFRESH_RATE rate);

int     gyVmuDisplayPixelGet(const struct VMUDevice* dev, int x, int y);
void    gyVmuDisplayPixelSet(struct VMUDevice* dev, int x, int y, int on);

int     gyVmuDisplayIconGet(const struct VMUDevice* dev, VMU_DISP_ICN icn);
void    gyVmuDisplayIconSet(struct VMUDevice* dev, VMU_DISP_ICN icn, int on);

int     gyVmuDisplayGhostingEnabledGet(const struct VMUDevice* dev);
void    gyVmuDisplayGhostingEnabledSet(struct VMUDevice* dev, int enable);

int gyVmuDisplayPixelGhostValue(const struct VMUDevice* dev, int x, int y);

int     gyVmuDisplayInit(struct VMUDevice* dev);
int    gyVmuDisplayUpdate(struct VMUDevice* dev, float deltaTime);
#endif



#ifdef __cplusplus
}
#endif



#endif // GYRO_VMU_DISPLAY_H


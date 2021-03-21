#ifndef GYRO_VMU_DISPLAY_H
#define GYRO_VMU_DISPLAY_H

#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

//3 - Video/LCD Memory (XRAM)
#define XRAM_BANK_SIZE          0x80
#define XRAM_BANK_COUNT         3

#define VMU_DISP_PIXEL_WIDTH        48
#define VMU_DISP_PIXEL_HEIGHT       32

#define VMU_DISP_GHOSTING_FRAMES    10

struct VMUDevice;

typedef enum EVMU_DISPLAY_REFRESH_RATE {
    EVMU_DISPLAY_REFRESH_83HZ,
    EVMU_DISPLAY_REFRESH_166HZ
} EVMU_DISPLAY_REFRESH_RATE;

typedef enum VMU_XRAM_BANK {
    VMU_XRAM_BANK_LCD_TOP = 0,
    VMU_XRAM_BANK_LCD_BOT,
    VMU_XRAM_BANK_ICN
} VMU_XRAM_BANK;

typedef enum EVMU_DISPLAY_ICON_TYPE {
    EVMU_DISPLAY_ICON_FILE,
    EVMU_DISPLAY_ICON_GAME,
    EVMU_DISPLAY_ICON_CLOCK,
    EVMU_DISPLAY_ICON_FLASH,
    EVMU_DISPLAY_ICON_COUNT
} EVMU_DISPLAY_ICON_TYPE;


GBL_DEFINE_HANDLE(EvmuDisplay)

GBL_DECLARE_ENUM(EVMU_DISPLAY_FILTER_TYPE) {
    EVMU_DISPLAY_FILTER_NONE,
    EVMU_DISPLAY_FILTER_LINEAR,
    EVMU_DISPLAY_FILTER_COUNT
};

GBL_DECLARE_ENUM(EVMU_DISPLAY_PROPERTY) {
    EVMU_DISPLAY_PROPERTY_LCD_ENABLED = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_DISPLAY_PROPERTY_UPDATE_ENABLED,
    EVMU_DISPLAY_PROPERTY_REFRESH_RATE,
    EVMU_DISPLAY_PROPERTY_LCD_CHANGED,
    EVMU_DISPLAY_PROPERTY_GHOSTING_ENABLED,
    EVMU_DISPLAY_PROPERTY_FILTER_TYPE,
    EVMU_DISPLAY_PROPERTY_FILTER_ROW_COUNT,
    EVMU_DISPLAY_PROPERTY_FILTER_COL_COUNT
    EVMU_DISPLAY_PROPERTY_COUNT
};

EVMU_API evmuDisplayPixel           (const EvmuDisplay* pDisplay, uint32_t row, uint32_t col, GblBool*  pValue);
EVMU_API evmuDisplayPixelSet        (EvmuDisplay*       pDisplay, uint32_t row, uint32_t col, GblBool   value);
EVMU_API evmuDisplayPixelGhosted    (const EvmuDisplay* pDisplay, uint32_t row, uint32_t col, int*      pValue);
EVMU_API evmuDisplayPixelFiltered   (const EvmuDisplay* pDisplay, uint32_t row, uint32_t col, int*      pValue);

EVMU_API evmuDisplayIcon            (const EvmuDisplay* pDisplay, EVMU_DISPLAY_ICON_TYPE iconType, GblBool* pValue);
EVMU_API evmuDisplayIconSet         (EvmuDisplay*       pDisplay, EVMU_DISPLAY_ICON_TYPE iconType, GblBool value);

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


#ifndef GYRO_VMU_DISPLAY_H
#define GYRO_VMU_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#define VMU_DISP_PIXEL_WIDTH        48
#define VMU_DISP_PIXEL_HEIGHT       32

#define VMU_DISP_GHOSTING_FRAMES    10

struct VMUDevice;

typedef enum VMU_DISP_REFRESH_RATE {
    VMU_DISP_REFRESH_83HZ,
    VMU_DISP_REFRESH_166HZ
} VMU_DISP_REFRESH_RATE;

typedef enum VMU_XRAM_BANK {
    VMU_XRAM_BANK_LCD_TOP = 0,
    VMU_XRAM_BANK_LCD_BOT,
    VMU_XRAM_BANK_ICN
} VMU_XRAM_BANK;

typedef enum VMU_DISP_ICN {
    VMU_DISP_ICN_FILE,
    VMU_DISP_ICN_GAME,
    VMU_DISP_ICN_CLOCK,
    VMU_DISP_ICN_FLASH,
    VMU_DISP_ICN_COUNT
} VMU_DISP_ICN;

typedef struct VMUDisplay {
    int     lcdBuffer[VMU_DISP_PIXEL_HEIGHT][VMU_DISP_PIXEL_WIDTH];
    float   refreshElapsed;
    int     ghostingEnabled;
    int     screenChanged;
} VMUDisplay;

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



#ifdef __cplusplus
}
#endif



#endif // GYRO_VMU_DISPLAY_H


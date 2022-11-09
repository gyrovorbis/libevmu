#ifndef GYRO_VMU_LCD_H
#define GYRO_VMU_LCD_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMU_LCD_FILE_EXTENSION      "lcd"
#define VMU_LCD_FILE_HEADER_SIZE    16
#define VMU_LCD_SIGNATURE_SIZE      4
#define VMU_LCD_SIGNATURE           "LCDi"
#define VMU_LCD_FRAME_INFO_SIZE     4
#define VMU_LCD_FRAME_DATA_SIZE     1536
#define VMU_LCD_REPEAT_INFINITE     255
#define VMU_LCD_COPYRIGHT_SIZE      40

#define VMU_LCD_DELAY_SECS          (1.0f/12.0f)

#define VMU_LCD_FILE_PIXEL_ON       0x08
#define VMU_LCD_FILE_PIXEL_OFF      0x00

struct VMUDevice;

typedef struct LCDFileHeader {
    char        signature[VMU_LCD_SIGNATURE_SIZE];   //'LCDi'
    uint16_t    version;        //1 (for Dream Animator 0.5)
    uint16_t    width;          //48
    uint16_t    height;         //32
    uint16_t    bitDepth;       //1
    char        reserved;       //0
    uint8_t     repeatCount;    //0-254, 255 = forever
    uint16_t    frameCount;
} LCDFileHeader;                //16 bytes

typedef struct LCDFrameInfo {
    char        reserved1;      //0
    uint8_t     delay;          //1/12s of a second
    char        reserved2[2];   //0
} LCDFrameInfo;                 //4 bytes

typedef struct LCDCopyright {
    char copyright[VMU_LCD_COPYRIGHT_SIZE];         //'Dream Animator ver.0.50 (C)1999 F.Sahara' (for Dream Animator 0.5)
} LCDCopyright;

typedef enum LCD_FILE_STATE {
    LCD_FILE_STATE_STOPPED,
    LCD_FILE_STATE_PLAYING,
    LCD_FILE_STATE_COMPLETE
} LCD_FILE_STATE;

typedef struct LCDFile {
    LCDFileHeader*  header;
    LCDFrameInfo**  frameInfo;
    uint8_t**       frameData;
    LCDCopyright*   copyright;
    uint8_t*        fileData;
    size_t          fileLength;
    float           elapsedTime;
    int             currentFrame;
    int             loopCount;
    int             state;
} LCDFile;

LCDFile*    gyVmuLcdFileLoad(const char* filePath);
void        gyVmuLcdPrintFileInfo(const LCDFile* lcdFile);
void        gyVmuLcdFileUnload(LCDFile* lcdFile);

void        gyVmuLcdFileUpdate(struct VMUDevice* dev, float deltaTime);
void        gyVmuLcdFileProcessInput(struct VMUDevice* dev);
int         gyVmuLcdFileLoadAndStart(struct VMUDevice* dev, const char* filePath);
void        gyVmuLcdFileFrameStart(struct VMUDevice* dev, uint16_t frameIndex);
void        gyVmuLcdFileStopAndUnload(struct VMUDevice* dev);



#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_LCD_H


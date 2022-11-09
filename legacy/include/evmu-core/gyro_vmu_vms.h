#ifndef GYRO_VMU_VMS_H
#define GYRO_VMU_VMS_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMU_VMS_FILE_INFO_SIZE                          128
#define VMU_VMS_FILE_INFO_VMU_DESC_SIZE                 16
#define VMU_VMS_FILE_INFO_DC_DESC_SIZE                  32
#define VMU_VMS_FILE_INFO_CREATOR_APP_SIZE              16
#define VMU_VMS_FILE_INFO_RESERVED_SIZE                 20

#define VMU_VMS_ICON_PALETTE_BLUE_POS                   0
#define VMU_VMS_ICON_PALETTE_BLUE_MASK                  0x000f
#define VMU_VMS_ICON_PALETTE_GREEN_POS                  4
#define VMU_VMS_ICON_PALETTE_GREEN_MASK                 0x00f0
#define VMU_VMS_ICON_PALETTE_RED_POS                    8
#define VMU_VMS_ICON_PALETTE_RED_MASK                   0x0f00
#define VMU_VMS_ICON_PALETTE_ALPHA_POS                  12
#define VMU_VMS_ICON_PALETTE_ALPHA_MASK                 0xf000

#define VMU_VMS_ICON_PALETTE_SIZE                       16
#define VMU_VMS_ICON_BITMAP_WIDTH                       32
#define VMU_VMS_ICON_BITMAP_HEIGHT                      32
#define VMU_VMS_ICON_BITMAP_SIZE                        512
#define VMU_VMS_ICON_COUNT_MAX                          3

#define VMU_VMS_EYECATCH_BITMAP_WIDTH                   72
#define VMU_VMS_EYECATCH_BITMAP_HEIGHT                  56

#define VMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_PALETTE_256 512
#define VMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_PALETTE_16  32

#define VMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_16BIT        8064
#define VMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_PALETTE_256  4032
#define VMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_PALETTE_16   2016

struct VMUDevice;
struct VMUFlashDirEntry;

typedef enum VMS_EYECATCH_MODE {
    VMS_EYECATCH_NONE,
    VMS_EYECATCH_COLOR_16BIT,
    VMS_EYECATCH_COLOR_PALETTE_256,
    VMS_EYECATCH_COLOR_PALETTE_16,
    VMS_EYECATCH_MAX
} VMS_EYECATCH_MODE;

//Strings are JIS X 0201 encoded (included within Shift-JIS)
// - Normal ASCII for low byte (except backslash becomes yen)
// - Katakana high byte (no Kanji)
typedef struct VMSFileInfo {
    char        vmuDesc[VMU_VMS_FILE_INFO_VMU_DESC_SIZE];
    char        dcDesc[VMU_VMS_FILE_INFO_DC_DESC_SIZE];
    char        creatorApp[VMU_VMS_FILE_INFO_CREATOR_APP_SIZE];
    uint16_t    iconCount;
    uint16_t    animSpeed;
    uint16_t    eyecatchType;
    uint16_t    crc;
    uint32_t    dataBytes;      //Size of actual file data, without header, icons, and shit (ignored for GAME files)
    char        reserved[VMU_VMS_FILE_INFO_RESERVED_SIZE];
    uint16_t    palette[VMU_VMS_ICON_PALETTE_SIZE]; //IS THIS PREMULTIPLIED BY ALPHA!?
} VMSFileInfo;

void        gyVmuVmsHeaderVmuDescGet(const VMSFileInfo* info, char* string);
void        gyVmuVmsHeaderDcDescGet(const VMSFileInfo* info, char* string);
void        gyVmuVmsHeaderCreatorAppGet(const VMSFileInfo* info, char* string);
uint16_t    gyVmuVmsFileInfoHeaderSize(const VMSFileInfo* info); //bytes
uint16_t    gyVmuVMSFileInfoCrcCalc(const unsigned char* buf, size_t size, uint16_t* partialCrc);
void        gyVmuPrintVMSFileInfo(const VMSFileInfo* vms);
void*       gyVmuVMSFileInfoEyecatch(const VMSFileInfo* vms);
int         gyVmuVmsFileInfoType(const void* image);


uint16_t**  gyVmuVMSFileInfoCreateIconsARGB444(const struct VMUDevice* dev, const struct VMUFlashDirEntry* dirEntry);
uint16_t*   gyVmuVMSFileInfoCreateEyeCatchARGB444(const struct VMUDevice* dev, const struct VMUFlashDirEntry* dirEntry);


//int gyVmuVmsFileCreate

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_VMS_H


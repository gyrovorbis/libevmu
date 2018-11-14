#ifndef GYRO_VMU_EXTRA_BG_PVR_H
#define GYRO_VMU_EXTRA_BG_PVR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GYRO_VMU_EXTRA_BG_PVR_FILE_NAME                 "EXTRA.BG.PVR"
#define GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE               4
#define GYRO_VMU_EXTRA_BG_PVR_GLOBAL_INDEX_SEGMENT_ID   "GBIX"
#define GYRO_VMU_EXTRA_BG_GLOBAL_INDEX_SIZE             4
#define GYRO_VMU_EXTRA_BG_PVR_GRAPHIC_SEGMENT_ID        "PVRT"
#define GYRO_VMU_EXTRA_BG_PVR_GRAPHIC_IMAGE_UNUSED_SIZE 2
#define GYRO_VMU_EXTRA_BG_PVR_TEX_FMT                   VMU_PVR_TEX_FMT_RGB565


typedef enum  VMU_PVR_TEX_FMT {
    VMU_PVR_TEX_FMT_ARGB1555    = 0x00, // (bilevel translucent alpha 0,255)
    VMU_PVR_TEX_FMT_RGB565      = 0x01, //(no translucent)
    VMU_PVR_TEX_FMT_ARGB4444    = 0x02, //(translucent alpha 0-255)
    VMU_PVR_TEX_FMT_YUV442      = 0x03,
    VMU_PVR_TEX_FMT_BUMP        = 0x04,
    VMU_PVR_TEX_FMT_4_BIT       = 0x05, //(SEE BELOW)
    VMU_PVR_TEX_FMT_8_BIT       = 0x06 // (SEE BELOW)
} VMU_PVR_TEX_FMT;


typedef enum VMU_PVR_TEX_LAYOUT {
    VMU_PVR_TEX_LAYOUT_SQUARE_TWIDDLED          = 0x01,
    VMU_PVR_TEX_LAYOUT_SQUARE_TWIDDLED_MIPMAP   = 0x02,
    VMU_PVR_TEX_LAYOUT_VQ                       = 0x03,
    VMU_PVR_TEX_LAYOUT_VQ_MIPMAP                = 0x04,
    VMU_PVR_TEX_LAYOUT_8_BIT_CLUT_TWIDDLED      = 0X05,
    VMU_PVR_TEX_LAYOUT_4_BIT_CLUT_TWIDDLED      = 0X06,
    VMU_PVR_TEX_LAYOUT_8_BIT_DIRECT_TWIDDLED    = 0x07,
    VMU_PVR_TEX_LAYOUT_4_BIT_DIRECT_TWIDDLED    = 0X08,
    VMU_PVR_TEX_LAYOUT_RECTANGLE                = 0x09,
    VMU_PVR_TEX_LAYOUT_RECTANGULAR_STRIDE       = 0x0B,
    VMU_PVR_TEX_LAYOUT_RECTANGULAR_TWIDDLED     = 0x0D,
    VMU_PVR_TEX_LAYOUT_SMALL_VQ                 = 0x10,
    VMU_PVR_TEX_LAYOUT_SMALL_VQ_MIPMAP          = 0x11,
    VMU_PVR_TEX_LAYOUT_SQUARE_TWIDDLED_MIPMAP2  = 0x12
} VMU_PVR_TEX_LAYOUT;

typedef struct VmuExtraBgSegmentHeader {
    char        segmentId[GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE];
    uint32_t    additionalLength;
} VmuExtraBgSegmentHeader;


typedef struct VmuExtraBgPvrGlobalIndexHeader {
    VmuExtraBgSegmentHeader header;
    uint8_t                 globalIndex[GYRO_VMU_EXTRA_BG_GLOBAL_INDEX_SIZE];
} VmuExtraBgPvrGlobalIndexHeader;


typedef struct VmuExtraBgPvrGraphicHeader {
    VmuExtraBgSegmentHeader header;
    uint8_t                 texFormat;
    uint8_t                 texLayout;
    uint8_t                 imageUnused[GYRO_VMU_EXTRA_BG_PVR_GRAPHIC_IMAGE_UNUSED_SIZE];
    uint16_t                width;
    uint16_t                height;
} VmuExtraBgPvrGraphicHeader;


typedef struct VmuExtraBgPvrFileInfo {
    VmuExtraBgPvrGlobalIndexHeader*     globalIndexHeader;
    VmuExtraBgPvrGraphicHeader*         graphicHeader;
} VmuExtraBgPvrFileInfo;

struct VMSFileInfo;


void                gyVmuExtraBgPvrFileInfo(const struct VMSFileInfo* vmsHeader, VmuExtraBgPvrFileInfo* info);
void                gyVmuExtraBgPvrFileInfoPrint(const VmuExtraBgPvrFileInfo* info);
const uint8_t*      gyVmuExtraBgPvrImageData(const VmuExtraBgPvrFileInfo* info);
const uint8_t*      gyVmuExtraBgPvrImageRGB888(const VmuExtraBgPvrFileInfo* info);


#if 0
unsigned long int VmuPvrTexUntwiddledTexelPosition(unsigned long int x, unsigned long int y, unsigned long* outX, unsigned long* outY)
{
    unsigned long int pos = 0;

    if(x >= kTwiddleTableSize || y >= kTwiddleTableSize)
    {
        pos = UntwiddleValue(y)  |  UntwiddleValue(x) << 1;
    }
    else
    {
        pos = gTwiddledTable[y]  |  gTwiddledTable[x] << 1;
    }

    return pos;
}

// Twiddle
unsigned long int VmuPvrTexUntwiddleValue(unsigned long int value)
{
    unsigned long int untwiddled = 0;

    for (size_t i = 0; i < 10; i++)
    {
        unsigned long int shift = pow(2, i);
        if (value & shift) untwiddled |= (shift << i);
    }

    return untwiddled;
}


void TexelToRGBA(unsigned short int srcTexel, enum VMU_PVR_TEX_FMT srcFormat, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
    switch( srcFormat )
    {
        case VMU_PVR_TEX_FMT_RGB565:
            *a = 0xFF;
            *r = (srcTexel & 0xF800) >> 8;
            *g = (srcTexel & 0x07E0) >> 3;
            *b = (srcTexel & 0x001F) << 3;
    default:
            break;
    }
}
#endif


#ifdef __cplusplus
}
#endif


#endif // GYRO_VMU_EXTRA_BG_PVR_H

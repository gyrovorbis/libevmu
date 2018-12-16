#include "gyro_vmu_extra_bg_pvr.h"
#include "gyro_vmu_vms.h"
#include <stdlib.h>
#include <libGyro/gyro_system_api.h>
#include <assert.h>
#include <string.h>


void gyVmuExtraBgPvrFileInfo(const struct VMSFileInfo* vmsHeader, VmuExtraBgPvrFileInfo* info) {
    assert(vmsHeader && info);

    const size_t headerSize     = gyVmuVmsFileInfoHeaderSize(vmsHeader);
    const uint8_t* data         = (const uint8_t*)vmsHeader;
    data += headerSize;
    memset(info, 0, sizeof(VmuExtraBgPvrFileInfo));

    const uint8_t* endData = data + vmsHeader->dataBytes;

    _gyLog(GY_DEBUG_VERBOSE, "Parsing .PVR File Header");
    _gyPush();

    do {

        if(memcmp(data, GYRO_VMU_EXTRA_BG_PVR_GLOBAL_INDEX_SEGMENT_ID, GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE) == 0) {
            _gyLog(GY_DEBUG_VERBOSE, "Global Index Header Segment Found!");
            info->globalIndexHeader = (const VmuExtraBgPvrGlobalIndexHeader*)data;
            data += info->globalIndexHeader->header.additionalLength + sizeof(VmuExtraBgSegmentHeader);

        } else if(memcmp(data, GYRO_VMU_EXTRA_BG_PVR_GRAPHIC_SEGMENT_ID, GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE) == 0) {
            _gyLog(GY_DEBUG_VERBOSE, "PVR Graphic Header Found!");
            info->graphicHeader = (const VmuExtraBgPvrGraphicHeader*)data;
            data += info->graphicHeader->header.additionalLength + sizeof(VmuExtraBgSegmentHeader);

        } else {
            char id[GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE+1];
            memcpy(id, data, GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE);
            id[GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE] = '\0';

            _gyLog(GY_DEBUG_VERBOSE, "Unknown PVR Segment Identifier found while parsing EXTRA.PVR.BG file! [Id: %s]", id);
            break;
        }

    } while(data < endData);


    _gyPop(1);
}

const char* gyVmuPvrTextureFormatString(const VMU_PVR_TEX_FMT fmt) {
    switch(fmt) {
        case VMU_PVR_TEX_FMT_ARGB1555:    return "ARGB1555";
        case VMU_PVR_TEX_FMT_RGB565:      return "RGB565";
        case VMU_PVR_TEX_FMT_ARGB4444:    return "ARGB4444";
        case VMU_PVR_TEX_FMT_YUV442:      return "YUV442";
        case VMU_PVR_TEX_FMT_BUMP:        return "BUMP_MAP";
        case VMU_PVR_TEX_FMT_4_BIT:       return "4-BIT PALETTED";
        case VMU_PVR_TEX_FMT_8_BIT:       return "8-BIT PALETTED";
        default:                          return "UNKNOWN";
    }
}


const char* gyVmuPvrTextureLayoutString(const VMU_PVR_TEX_LAYOUT layout) {
    switch(layout) {
        case VMU_PVR_TEX_LAYOUT_SQUARE_TWIDDLED:         return "SQUARE + TWIDDLED";
        case VMU_PVR_TEX_LAYOUT_SQUARE_TWIDDLED_MIPMAP:  return "SQUARE + TWIDDLED + MIPMAP";
        case VMU_PVR_TEX_LAYOUT_VQ:                      return "VQ";
        case VMU_PVR_TEX_LAYOUT_VQ_MIPMAP:               return "VQ + MIPMAP";
        case VMU_PVR_TEX_LAYOUT_8_BIT_CLUT_TWIDDLED:     return "8-BIT CLUT + TWIDDLED";
        case VMU_PVR_TEX_LAYOUT_4_BIT_CLUT_TWIDDLED:     return "4-BIT CLUT + TWIDDLED";
        case VMU_PVR_TEX_LAYOUT_8_BIT_DIRECT_TWIDDLED:   return "8-BIT DIRECT + TWIDDLED";
        case VMU_PVR_TEX_LAYOUT_4_BIT_DIRECT_TWIDDLED:   return "4-BIT DIRECT + TWIDDLED";
        case VMU_PVR_TEX_LAYOUT_RECTANGLE:               return "RECTANGLE";
        case VMU_PVR_TEX_LAYOUT_RECTANGULAR_STRIDE:      return "RECTANGULAR + STRIDE";
        case VMU_PVR_TEX_LAYOUT_RECTANGULAR_TWIDDLED:    return "RECTANGULAR + TWIDDLED";
        case VMU_PVR_TEX_LAYOUT_SMALL_VQ:                return "SMALL_VQ";
        case VMU_PVR_TEX_LAYOUT_SMALL_VQ_MIPMAP:         return "SMALL_VQ + MIPMAP";
        case VMU_PVR_TEX_LAYOUT_SQUARE_TWIDDLED_MIPMAP2: return "SQUARE + TWIDDLED + MIPMAP";
        default:                                         return "UNKNOWN";
    }
}

void gyVmuExtraBgPvrFileInfoPrint(const VmuExtraBgPvrFileInfo* fileInfo) {
    assert(fileInfo && fileInfo->graphicHeader);
    const VmuExtraBgPvrGraphicHeader* info = fileInfo->graphicHeader;

    _gyLog(GY_DEBUG_VERBOSE, "EXTRA_BG PVR File Header Info");
    _gyPush();

    char identString[GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE+1];
    memcpy(identString, info->header.segmentId, GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE);
    identString[GYRO_VMU_EXTRA_BG_SEGMENT_ID_SIZE] = '\0';
    _gyLog(GY_DEBUG_VERBOSE, "%-30s: %40s", "Identifier",            identString);
    _gyLog(GY_DEBUG_VERBOSE, "%-30s: %40u", "Data Size (bytes)",     info->header.additionalLength);
    _gyLog(GY_DEBUG_VERBOSE, "%-30s: %40s", "Texture Format",        gyVmuPvrTextureFormatString((VMU_PVR_TEX_FMT)info->texFormat));
    _gyLog(GY_DEBUG_VERBOSE, "%-30s: %40s", "Texture Layout",        gyVmuPvrTextureLayoutString((VMU_PVR_TEX_LAYOUT)info->texLayout));
    _gyLog(GY_DEBUG_VERBOSE, "%-30s: %40u", "Width",                 info->width);
    _gyLog(GY_DEBUG_VERBOSE, "%-30s: %40u", "Height",                info->height);

    _gyPop(1);

}


const uint8_t* gyVmuExtraBgPvrImageData(const VmuExtraBgPvrFileInfo* info) {
    const uint8_t* data = NULL;
    if(info && info->graphicHeader) {
        data = (const uint8_t*)info->graphicHeader;
        data += sizeof(VmuExtraBgSegmentHeader);
    }
    return data;
}

const uint8_t* gyVmuExtraBgPvrImageRGB888(const VmuExtraBgPvrFileInfo* info) {
    assert(info && info->graphicHeader);
    uint8_t* img = NULL;

    _gyLog(GY_DEBUG_VERBOSE, "Creating Image data for EXTRA.BG.PVR");
    _gyPush();

    const uint8_t* data = gyVmuExtraBgPvrImageData(info);
    if(!data) {
        _gyLog(GY_DEBUG_ERROR, "Failed to retrieve raw image data bytes from file data!");

    } else {
        const VmuExtraBgPvrGraphicHeader* graphicHeader = info->graphicHeader;

        int srcBytesPerPixel = 0;
        switch(graphicHeader->texFormat) {
        case VMU_PVR_TEX_FMT_RGB565:
            srcBytesPerPixel = 2;
            break;
        default:
            _gyLog(GY_DEBUG_ERROR, "Couldn't figure out bpp for the given PVR format!");
            srcBytesPerPixel = 0;
        }

        const int dstBytesPerPixel = 3; //RGB888

        //Going from a 16bpp DC format to a 24bpp Qt format
        img = malloc(dstBytesPerPixel * sizeof(uint8_t) * graphicHeader->width * graphicHeader->height);
        memset(img, 0, dstBytesPerPixel * sizeof(uint8_t) * graphicHeader->width * graphicHeader->height);

        for(int y = 0; y < graphicHeader->height; ++y) {
            for(int x = 0; x < graphicHeader->width; ++x) {
                //determine indices
                int pixelNumber = y * graphicHeader->width + x;
                int srcIndex    = pixelNumber * srcBytesPerPixel;
                int dstIndex    = pixelNumber * dstBytesPerPixel;

                //read source data
                uint32_t srcColor = *(const uint32_t*)&data[srcIndex];

                //extract relevant bits
                uint32_t srcBitMask = 0;
                for(int i = 0; i < srcBytesPerPixel * 8 && srcBytesPerPixel <= 4; ++i) {
                    srcBitMask |= 0x1 << i;
                }
                srcColor &= srcBitMask;

                //Convert source color to destination color
                uint32_t dstColor = srcColor;

                //Write destination color
                img[dstIndex]   = ((srcColor>>11) & 0x1f) << 0x3;
                img[dstIndex+1] = ((srcColor>>5) & 0x3f) << 0x2;
                img[dstIndex+2] = ((srcColor)    & 0x1) << 0x3;
                //*(uint32_t*)&img[dstIndex] |= dstColor;


            }
        }

    }

    _gyPop(1);
    return img;

}

#include "gyro_vmu_icondata.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <libGyro/gyro_system_api.h>
#include "gyro_vmu_vms.h"

static uint8_t _iconDataBiosSecretBitSequence[VMU_ICONDATA_BIOS_SECRET_BYTE_COUNT] = {
    0xDA, 0x69, 0xD0, 0xDA, 0xC7, 0x4E, 0xF8, 0x36, 0x18, 0x92, 0x79, 0x68, 0x2D, 0xB5, 0x30, 0x86
};

int gyVmuIconDataSecretBiosUnlocked(const IconDataFileInfo* icnDat) {
    const uint8_t* bytes = (const uint8_t*)icnDat;
    bytes += VMU_ICONDATA_BIOS_SECRET_OFFSET;
    return (memcmp(bytes, _iconDataBiosSecretBitSequence, VMU_ICONDATA_BIOS_SECRET_BYTE_COUNT) == 0);
}

void gyVmuIconDataSecretBiosSetUnlocked(const IconDataFileInfo* icnDat, int unlocked) {
    const uint8_t* bytes = (const uint8_t*)icnDat;

    if(unlocked) {
        memcpy(&bytes[VMU_ICONDATA_BIOS_SECRET_OFFSET], _iconDataBiosSecretBitSequence, VMU_ICONDATA_BIOS_SECRET_BYTE_COUNT);
    } else {
        memset(&bytes[VMU_ICONDATA_BIOS_SECRET_OFFSET], 0, VMU_ICONDATA_BIOS_SECRET_BYTE_COUNT);
    }
}

const uint8_t* gyVmuIconDataBiosSecretBitSequence(void) {
  return _iconDataBiosSecretBitSequence;
}

size_t gyVmuIconDataSize(const IconDataFileInfo* icnDat) {
    if(!icnDat) return 0;
    size_t size = 0;

    size_t vmuIcnEnd    = icnDat->vmuIconOffset + VMU_ICONDATA_VMU_ICON_BYTES;
    size_t dcIcnEnd     = icnDat->dcIconOffset + VMU_ICONDATA_DC_PALETTE_BYTES + VMU_ICONDATA_DC_ICON_BYTES;

    size = (vmuIcnEnd > dcIcnEnd)? vmuIcnEnd : dcIcnEnd;
    return size;
}

const uint8_t* 	gyVmuIconDataVmuIconData(const IconDataFileInfo* icnDat) {
    assert(icnDat);

    const uint8_t* bytes = (const uint8_t*)icnDat;

    if(icnDat) {
        if(icnDat->vmuIconOffset) {
            return &bytes[icnDat->vmuIconOffset];
        }
    }

    return NULL;
}

const uint16_t* gyVmuIconDataDcPalette(const IconDataFileInfo* icnDat) {
    assert(icnDat);

    const uint8_t* bytes = (uint8_t*)icnDat;

    if(icnDat) {
        if(icnDat->dcIconOffset) {
            return (uint16_t*)&bytes[icnDat->dcIconOffset];
        }
    }

    return NULL;
}

const uint8_t*  gyVmuIconDataDcIconData(const IconDataFileInfo* icnDat) {
    const uint8_t* bytes = gyVmuIconDataDcPalette(icnDat);

    if(bytes) {
        bytes += VMU_ICONDATA_DC_PALETTE_BYTES;
    }

    return bytes;
}



void gyVmuIconDataPrint(const IconDataFileInfo* icnDat) {
    _gyLog(GY_DEBUG_VERBOSE, "ICONDATA.VMS Pseudo-VMS Header");
    _gyPush();


    char string[VMU_VMS_FILE_INFO_VMU_DESC_SIZE+1];
    const VMSFileInfo* vms = (const VMSFileInfo*)icnDat;
    memcpy(string, vms->vmuDesc, sizeof(vms->vmuDesc));
    string[sizeof(vms->vmuDesc)] = 0;
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "VMU Description",          string);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "VMU Icon Offset",          icnDat->vmuIconOffset);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "DC Icon Offset",           icnDat->dcIconOffset);
    int secretBios = gyVmuIconDataSecretBiosUnlocked(icnDat);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Secret BIOS Unlocked",     secretBios? "Yes" : "No");

    _gyPop(1);
}


#if 0
int 			createIconDataFileInfo(IconDataFileInfo* icnDat, QImage vmuIcn, QImage dcIcn) {
    bool success;

    memset(icnDat, 0, sizeof(IconDataFileInfo));

    _gyLog(GY_DEBUG_VERBOSE, "Creating ICONDATA_VMS File Data");
    _gyPush();

    _gyLog(GY_DEBUG_VEROBSE, "Creating VMU Icon."
    _gyPush(1);

    QImage vmuImg = vmuIcn.scaled(VMU_ICONDATA_ICON_WIDTH, VMU_ICONDATA_ICON_HEIGHT);
    vmuImg = vmuImg.convertToFormat(QImage::Format_Mono);
    Q_ASSERT(vmuImg.sizeInBytes() == VMU_ICONDATA_VMU_ICON_BYTES);
    if(!vmuImg.isNull()) {
        memcpy((uint8_t*)(icnDat + sizeof(IconDataFileInfo)), vmuImg.constBits(), sizeof(VMU_ICONDATA_VMU_ICON_BYTES));
        icnDat->vmuIconOffset = sizeof(IconDataFileInfo);
    }

    _gyPop();

    _gyLog(GY_DEBUG_VEROBSE, "Creating DC Icon."
    _gyPush();
    QImage dcImg = dcIcn.scaled(VMU_ICONDATA_ICON_WIDTH, VMU_ICONDATA_ICON_HEIGHT);



    _gyPop(1);



    _gyPop(1);
    return succes;
}
#endif


uint16_t* gyVmuIconDataDcIconImageARGB4444(const IconDataFileInfo* icnDat) {
    uint16_t* imgData = NULL;

    _gyLog(GY_DEBUG_VERBOSE, "Creating ARGB4444 Image Data of ICONDATA_VMS DC Icon");
    _gyPush();

    const uint16_t* palDat = gyVmuIconDataDcPalette(icnDat);
    if(!palDat) {
        _gyLog(GY_DEBUG_ERROR, "Failed to retrieve pointer to palette data!");
    } else {

        const uint8_t* imgBytes = gyVmuIconDataDcIconData(icnDat);

        if(!imgBytes) {
            _gyLog(GY_DEBUG_ERROR, "Failed to retrieve pointer to pixel index data!");
        } else {

            imgData = malloc(sizeof(uint16_t) * VMU_ICONDATA_ICON_WIDTH * VMU_ICONDATA_ICON_HEIGHT);

            //Read each pixel as a nybll
            for(int y = 0; y < VMU_ICONDATA_ICON_HEIGHT; ++y) {
                for(int x = 0; x < VMU_ICONDATA_ICON_WIDTH; ++x) {
                    const int   imgIndex = y * VMU_ICONDATA_ICON_WIDTH + x;
                    int         byteIndex = imgIndex / 2;

                    uint8_t colorIndex = imgBytes[byteIndex];
                    if(!(imgIndex % 2)) {
                        colorIndex >>= 4;
                    }
                    colorIndex &= 0x0f;

                    imgData[imgIndex] = palDat[colorIndex];

                }

            }
        }
    }

    _gyPop(1);

    return imgData;
}

uint16_t* gyVmuIconDataVmuIconImageARGB4444(const IconDataFileInfo* icnDat) {
    uint16_t* imgData = NULL;

    _gyLog(GY_DEBUG_VERBOSE, "Creating ARGB4444 Image Data of ICONDATA_VMS VMU Icon");
    _gyPush();

    const uint8_t* rawData = gyVmuIconDataVmuIconData(icnDat);

    if(!rawData) {
        _gyLog(GY_DEBUG_ERROR, "Failed to retrieve pointer to pixel data!");
    } else {

        imgData = malloc(sizeof(uint16_t) * VMU_ICONDATA_ICON_WIDTH * VMU_ICONDATA_ICON_HEIGHT);
        memset(imgData, 0, sizeof(uint16_t) * VMU_ICONDATA_ICON_WIDTH * VMU_ICONDATA_ICON_HEIGHT);

        for(int y = 0; y < VMU_ICONDATA_ICON_HEIGHT; ++y) {
            for(int x = 0; x < VMU_ICONDATA_ICON_WIDTH; ++x) {
                const int   imgIndex    = y * VMU_ICONDATA_ICON_WIDTH + x;
                int         byteIndex   = imgIndex / 8;
                int         bitIndex    = imgIndex % 8;

                if(rawData[byteIndex] & (0x80 >> bitIndex)) {
                    imgData[imgIndex] = 0xf000;
                } else {
                    imgData[imgIndex] = 0xffff;
                }

            }
        }
    }

    _gyPop(1);

    return imgData;

}

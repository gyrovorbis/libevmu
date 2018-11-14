#ifndef GYRO_VMU_ICONDATA_H
#define GYRO_VMU_ICONDATA_H

#include <stdint.h>
#include <stdlib.h>
//Set icondata icon if there is no game 
//data present instead of the ES logo/screensaver

#define VMU_ICONDATA_VMS_FILE_NAME          "ICONDATA_VMS"

#define VMU_ICONDATA_DESC_SIZE				16
#define VMU_ICONDATA_ICON_WIDTH				32
#define VMU_ICONDATA_ICON_HEIGHT			32
#define VMU_ICONDATA_VMU_ICON_BYTES			128
#define VMU_ICONDATA_DC_PALETTE_SIZE		16
#define VMU_ICONDATA_DC_PALETTE_BYTES		32
#define VMU_ICONDATA_DC_ICON_BYTES			512

#define VMU_ICONDATA_BIOS_SECRET_OFFSET     0x2c0
#define VMU_ICONDATA_BIOS_SECRET_BYTE_COUNT 16

#define ICONDATA_PALETTE_ENTRY_A(c) ((c >> 12) & 0x000f)
#define ICONDATA_PALETTE_ENTRY_R(c) ((c >> 8 ) & 0x000f)
#define ICONDATA_PALETTE_ENTRY_G(c) ((c >> 4 ) & 0x000f)
#define ICONDATA_PALETTE_ENTRY_B(c) ((c      ) & 0x000f)

#ifdef __cplusplus
extern "C" {
#endif

struct VMUDevice;

typedef struct IconDataFileInfo {
	char 		desc[VMU_ICONDATA_DESC_SIZE];
	uint32_t	vmuIconOffset;
	uint32_t	dcIconOffset;
} IconDataFileInfo;

//Qt's QImage::Format_Mono/Format_MonoLSB should do it.
const uint8_t* 	gyVmuIconDataVmuIconData(const IconDataFileInfo* icnDat);
const uint16_t* gyVmuIconDataDcPalette(const IconDataFileInfo* icnDat);
const uint8_t*  gyVmuIconDataDcIconData(const IconDataFileInfo* icnDat);
uint16_t*       gyVmuIconDataDcIconImageARGB4444(const IconDataFileInfo* icnDat);
uint16_t*       gyVmuIconDataVmuIconImageARGB4444(const IconDataFileInfo* icnDat);
int             gyVmuIconDataSecretBiosUnlocked(const IconDataFileInfo* icnDat);
void            gyVmuIconDataSecretBiosSetUnlocked(const IconDataFileInfo* icnDat, int unlocked);

size_t          gyVmuIconDataSize(const IconDataFileInfo* icnDat);
void            gyVmuIconDataPrint(const IconDataFileInfo* icnDat);

const uint8_t*  gyVmuIconDataBiosSecretBitSequence(void);






#ifdef __cplusplus
}
#endif

#endif

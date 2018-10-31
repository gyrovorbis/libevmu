#ifndef GYRO_VMU_ICONDATA_H
#define GYRO_VMU_ICONDATA_H

#include <stdint.h>
//Set icondata icon if there is no game 
//data present instead of the ES logo/screensaver

#define VMU_ICONDATA_DESC_SIZE				16
#define VMU_ICONDATA_ICON_WIDTH				32
#define VMU_ICONDATA_ICON_HEIGHT			32
#define VMU_ICONDATA_VMU_ICON_BYTES			128
#define VMU_ICONDATA_DC_PALETTE_SIZE		16
#define VMU_ICONDATA_DC_PALETTE_BYTES		32
#define VMU_ICONDATA_DC_ICON_BYTES			512

#define ICONDATA_PALETTE_ENTRY_R(c) ((c & 0xf000) >> 12)
#define ICONDATA_PALETTE_ENTRY_G(c) ((c & 0x0f00) >> 8)
#define ICONDATA_PALETTE_ENTRY_B(c) ((c & 0x00f0) >> 4)
#define ICONDATA_PALETTE_ENTRY_A(c) ((c & 0x000f))

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


QImage iconDataVmuIconImage(const VMUDevice* dev, const FlashDirEntry* dirEntry) {
	assert(dev && dirEntry);

	//blah blah get file;
	const IconDataFileInfo* icnDat; //= gyVmuFlashFileReadBytes(dev, dirEntry, blah);

	QVector<QRgb> colorTable;
	uint8_t* palDat = gyVmuIconDataDcPalette(dev, icnDat);
	for(int p = 0; p < VMU_ICONDATA_DC_PALETTE_SIZE; ++p) {
		uint16_t palEntry = palDat[p];
		colorTable.push_back(QColor::fromRgb(
			ICONDATA_PALETTE_ENTRY_R(palEntry) * 2,
			ICONDATA_PALETTE_ENTRY_G(palEntry) * 2,
			ICONDATA_PALETTE_ENTRY_B(palEntry) * 2));
	}

	uint8_t* imgBytes = new uint8_t[VMU_ICONDATA_DC_ICON_BYTES];
	memcpy(imgBytes, gyVmuIconDataDcIconData(icnDat), VMU_ICONDATA_DC_ICON_BYTES);
	QImage img(imgBytes, 
			VMU_ICONDATA_ICON_WIDTH,
			VMU_ICONDATA_ICON_HEIGHT, 
			QImage::Format_Indexed8,
			[](void* imgData) {
				delete[] imgData;
			},
			imgBytes);
	img.setColorTable(colorTable);

	return img;
}




#ifdef __cplusplus
}
#endif

#endif

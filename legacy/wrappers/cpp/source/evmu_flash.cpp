#include <evmu-core-cpp/evmu_flash.hpp>
#include <evmu-core/formats/gyro_vmu_icondata.h>
#include <evmu-core/formats/gyro_vmu_pso_img.h>
#include <gimbal/preprocessor/gimbal_macro_utils.h>

namespace evmu {

bool VmuFlashDirEntry::isValid(void) const {
    if(isNull()) return false;

    if(!isIconDataVms() && !(getFileType() == VMU_FLASH_FILE_TYPE_GAME || getFileType() == VMU_FLASH_FILE_TYPE_DATA)) return false;

    return true;
}

bool VmuFlashDirEntry::isIconDataVms(void) const {
    return (getFileName() == VMU_ICONDATA_VMS_FILE_NAME);
}

bool VmuFlashDirEntry::isExtraBgPvr(void) const {
    return (getFileName() == GYRO_VMU_EXTRA_BG_PVR_FILE_NAME);
}

bool VmuFlashDirEntry::isPsoImg(void) const {
    return (getFileName() == VMU_PSO_IMG_FILE_NAME);
}

std::string VmuFlashDirEntry::getFileTypeStr(void) const {
    switch(getFileType()) {
    case VMU_FLASH_FILE_TYPE_DATA: return "DATA";
    case VMU_FLASH_FILE_TYPE_GAME: return "GAME";
    case VMU_FLASH_FILE_TYPE_NONE: return "NONE";
    default:                       return "INVALID";
    }
}

std::string VmuFlashDirEntry::getCreationDateStr(void) const {

    std::string str;
    str += std::to_string(GBL_BCD_BYTE_UNPACK(_dirEntry->timeStamp[VMU_FLASH_DIRECTORY_DATE_MONTH]));
    str += "/";
    str += std::to_string(GBL_BCD_BYTE_UNPACK(_dirEntry->timeStamp[VMU_FLASH_DIRECTORY_DATE_DAY]));
    str += "/";
    str += std::to_string(GBL_BCD_BYTE_UNPACK(_dirEntry->timeStamp[VMU_FLASH_DIRECTORY_DATE_YEAR]));
    str += " ";
    str += std::to_string(GBL_BCD_BYTE_UNPACK(_dirEntry->timeStamp[VMU_FLASH_DIRECTORY_DATE_HOUR]));
    str += ":";
    str += std::to_string(GBL_BCD_BYTE_UNPACK(_dirEntry->timeStamp[VMU_FLASH_DIRECTORY_DATE_MINUTE]));

   return str;
}

bool VmuFlashDirEntry::readFile(uint8_t* buffer) const {
    return gyVmuFlashFileRead(_dev, _dirEntry, buffer, 1);
}

std::string VmuIconDataVmsFileInfo::getVmuDescription(void) const {
    char buffer[VMU_ICONDATA_DESC_SIZE+1] = { '\0' };
    memcpy(buffer, _info->desc, VMU_ICONDATA_DESC_SIZE);
    return buffer;
}

bool VmuIconDataVmsFileInfo::isSecretBiosUnlocked(void) const {
    return gyVmuIconDataSecretBiosUnlocked(_info);
}
void VmuIconDataVmsFileInfo::setSecretBiosUnlocked(bool unlocked) const {
    gyVmuIconDataSecretBiosSetUnlocked(_info, unlocked);
}

const uint8_t* VmuIconDataVmsFileInfo::getVmuIconData(void) const {
    return gyVmuIconDataVmuIconData(_info);
}
const uint8_t* VmuIconDataVmsFileInfo::getDcIconData(void) const {
    return gyVmuIconDataDcIconData(_info);
}
const uint16_t* VmuIconDataVmsFileInfo::getDcPalette(void) const {
    return gyVmuIconDataDcPalette(_info);
}

uint16_t* VmuIconDataVmsFileInfo::createVmuIconImageArgb4444(void) const {
    return gyVmuIconDataVmuIconImageARGB4444(_info);
}
uint16_t* VmuIconDataVmsFileInfo::createDcIconImageArgb4444(void) const {
    return gyVmuIconDataDcIconImageARGB4444(_info);
}

size_t VmuIconDataVmsFileInfo::getVmuIconOffset(void) const {
    return _info->vmuIconOffset;
}
size_t VmuIconDataVmsFileInfo::getDcIconOffset(void) const {
    return _info->dcIconOffset;
}

}

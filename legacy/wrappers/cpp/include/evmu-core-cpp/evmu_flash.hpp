#ifndef EVMU_FLASH_HPP
#define EVMU_FLASH_HPP

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <evmu-core/gyro_vmu_flash.h>
#include <evmu-core/formats/gyro_vmu_extra_bg_pvr.h>
#include <evmu-core/formats/gyro_vmu_vms.h>
#include <cassert>
#include <cstring>


class   EVmuDevice;
struct  VMUFlashDirEntry;
struct  VMSFileInfo;
struct  IconDataFileInfo;
struct  ExtraBgPvrFileInfo;
struct  VmuExtraBgPvrFileInfo;


namespace evmu {

class VmuIconDataVmsFileInfo {
protected:
    IconDataFileInfo*   _info = nullptr;

public:
                    VmuIconDataVmsFileInfo(IconDataFileInfo* info);
    bool            isNull(void) const;

    std::string     getVmuDescription(void) const;

    bool            isSecretBiosUnlocked(void) const;
    void            setSecretBiosUnlocked(bool unlocked) const;

    size_t          getVmuIconOffset(void) const;
    size_t          getDcIconOffset(void) const;

    const uint8_t*  getVmuIconData(void) const;
    const uint8_t*  getDcIconData(void) const;
    const uint16_t* getDcPalette(void) const;

    uint16_t*       createVmuIconImageArgb4444(void) const;
    uint16_t*       createDcIconImageArgb4444(void) const;

    operator        IconDataFileInfo*(void) const;
    operator        bool(void) const;


};

class VmuExtraBgPvrFileInfo {
public:
    ::VmuExtraBgPvrFileInfo   _info;

                            VmuExtraBgPvrFileInfo(const VMSFileInfo* info);
    bool                    isNull(void) const;

    VMU_PVR_TEX_FMT         getTextureFormat(void) const;
    VMU_PVR_TEX_LAYOUT      getTextureLayout(void) const;
    uint16_t                getWidth(void) const;
    uint16_t                getHeight(void) const;

    const uint8_t*          getTextureData(void) const;
    const uint8_t*          createTextureImageRgb888(void) const;

    operator                ::VmuExtraBgPvrFileInfo(void) const;
    operator                bool(void) const;

};

class VmuVmsHeader {
protected:
    VMSFileInfo*        _info = nullptr;

public:
                        VmuVmsHeader(VMSFileInfo* info);
     bool               isNull(void) const;

    size_t              getHeaderSize(void) const;
    size_t              getPayloadSize(void) const;

    bool                copyPayloadData(uint8_t* buffer) const;

    //Properties
    std::string         getVmuDescription(void) const;
    void                setVmuDescription(std::string desc) const;

    std::string         getDcDescription(void) const;
    void                setDcDescription(std::string desc) const;

    std::string         getCreatorApp(void) const;
    void                setCreatorApp(std::string app);

    uint16_t            getCrc(void) const;
    void                setCrc(uint16_t value) const;

    //Icons / Eyecatch
    uint16_t            getIconCount(void) const;
    uint16_t            getAnimSpeed(void) const;
    bool                hasEyeCatch(void) const;
    VMS_EYECATCH_MODE   getEyeCatchType(void) const;

    /* This shit requires the data to be stored consecutively, so we
     * would have to self-extract, and fuck that for now
     * uint16_t            getPaletteEntry(unsigned index) const;
     * uint16_t**          createIconsImagesArgb444(void) const;
     * uint16_t*           createEyeCatchArgb444(void) const;
     */

    operator            VMSFileInfo*(void) const;
    operator            bool(void) const;

};






class VmuFlashDirEntry {
protected:
    VMUDevice*              _dev 		= nullptr;
    VMUFlashDirEntry*       _dirEntry 	= nullptr;
public:
                            VmuFlashDirEntry(VMUDevice* dev, VMUFlashDirEntry* dirEntry);

    bool                    isNull(void) const;
    bool                    isValid(void) const;

    bool					isIconDataVms(void) const;
    bool					isExtraBgPvr(void) const;
    bool                    isPsoImg(void) const;
    bool					isGame(void) const;
    bool					isData(void) const;

    VmuVmsHeader			getVmsHeader(void) const;
    VmuIconDataVmsFileInfo	getIconDataFileInfo(void) const;
    VmuExtraBgPvrFileInfo	getExtraBgPvrFileInfo(void) const;

    void                    setCopyProtected(bool enabled) const;

    size_t					getFileSizeBytes(void) const;
    bool					isCrcValid(void) const;
    uint16_t				calculateCrc(void) const;
    void					fixCrc(void) const;

    //DirectoryEntry
    unsigned				getFileIndex(void) const;
    VMU_FLASH_FILE_TYPE		getFileType(void) const;
    std::string             getFileTypeStr(void) const;
    std::string 			getFileName(void) const;
    std::string             getCreationDateStr(void) const;
    bool					isCopyProtected(void) const;
    uint16_t				getFirstBlock(void) const;
    uint16_t				getFileSizeBlocks(void) const;
    uint16_t				getHeaderBlockOffset(void) const;
    bool					deleteFile(void);


    bool                    readFile(uint8_t* buffer) const;
    size_t					readFileBytes(uint8_t* buffer, size_t bytes, size_t offset, bool includeHeader=true) const;
    uint8_t					readFileByte(uint32_t offset);
    bool					writeFileByte(uint32_t offset, uint8_t value);


    //Type-compatible with evmu-core C API
    operator                    VMUFlashDirEntry*(void) const;
    operator                    bool(void) const;
    const VmuFlashDirEntry&    operator=(VMUFlashDirEntry* dirEntry);
};


inline bool VmuFlashDirEntry::isNull(void) const {
    return !_dev || !_dirEntry || _dirEntry->fileType == VMU_FLASH_FILE_TYPE_NONE;
}

inline VmuFlashDirEntry::VmuFlashDirEntry(VMUDevice* dev, VMUFlashDirEntry* entry):
    _dev(dev),
    _dirEntry(entry)
{
    assert(static_cast<VMUDevice*>(dev));
}

inline VmuFlashDirEntry::operator VMUFlashDirEntry *(void) const {
    return _dirEntry;
}

inline VmuFlashDirEntry::operator bool(void) const {
    return !isNull();
}

inline std::string VmuFlashDirEntry::getFileName(void) const {
    char buffer[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE+1] = { '\0' };
    memcpy(buffer, _dirEntry->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);
    std::string str = buffer;
    return str;
}

inline VMU_FLASH_FILE_TYPE VmuFlashDirEntry::getFileType(void) const {
    return static_cast<VMU_FLASH_FILE_TYPE>(_dirEntry->fileType);
}

inline bool VmuFlashDirEntry::isCopyProtected(void) const {
    return (_dirEntry->copyProtection == VMU_FLASH_COPY_PROTECTION_COPY_PROTECTED);
}

inline void VmuFlashDirEntry::setCopyProtected(bool value) const {
    _dirEntry->copyProtection = value? VMU_FLASH_COPY_PROTECTION_COPY_PROTECTED :
                                       VMU_FLASH_COPY_PROTECTION_COPY_OK;
}

inline uint16_t VmuFlashDirEntry::getFirstBlock(void) const {
    return _dirEntry->firstBlock;
}

inline uint16_t VmuFlashDirEntry::getFileSizeBlocks(void) const {
    return _dirEntry->fileSize;
}
inline uint16_t VmuFlashDirEntry::getHeaderBlockOffset(void) const {
    return _dirEntry->headerOffset;
}

inline bool VmuFlashDirEntry::isGame(void) const {
    return _dirEntry->fileType == VMU_FLASH_FILE_TYPE_GAME;
}

inline bool	VmuFlashDirEntry::isData(void) const {
    return _dirEntry->fileType == VMU_FLASH_FILE_TYPE_DATA;
}

inline VmuVmsHeader VmuFlashDirEntry::getVmsHeader(void) const {
    switch(getFileType()) {
    case VMU_FLASH_FILE_TYPE_GAME:
    case VMU_FLASH_FILE_TYPE_DATA:
        return isIconDataVms()? nullptr : gyVmuFlashDirEntryVmsHeader(_dev, _dirEntry);
    default:
        return nullptr;
    }
}

inline size_t VmuFlashDirEntry::getFileSizeBytes(void) const {
    switch(getFileType()) {
    case VMU_FLASH_FILE_TYPE_GAME:
        return getFileSizeBlocks() * VMU_FLASH_BLOCK_SIZE;
    case VMU_FLASH_FILE_TYPE_DATA: {
        auto hdr = getVmsHeader();
        return hdr.getHeaderSize() + hdr.getPayloadSize();
    }
    default:
        return 0;
    }
}

inline bool	VmuFlashDirEntry::isCrcValid(void) const {
    auto vms = getVmsHeader();
    if(!vms.isNull()) {
        return vms.getCrc() == gyVmuFlashFileCalculateCRC(_dev, _dirEntry);
    } else return false;
}

inline uint16_t	VmuFlashDirEntry::calculateCrc(void) const {
    return gyVmuFlashFileCalculateCRC(_dev, _dirEntry);
}

inline void	VmuFlashDirEntry::fixCrc(void) const {
    auto vms = getVmsHeader();
    if(!vms.isNull()) {
        vms.setCrc(gyVmuFlashFileCalculateCRC(_dev, _dirEntry));
    }
}

inline unsigned VmuFlashDirEntry::getFileIndex(void) const {
   return gyVmuFlashDirEntryIndex(_dev, _dirEntry);
}

inline bool VmuFlashDirEntry::deleteFile(void) {
    bool success = gyVmuFlashFileDelete(_dev, _dirEntry);
    _dirEntry = nullptr;
    return success;
}

inline VmuIconDataVmsFileInfo	VmuFlashDirEntry::getIconDataFileInfo(void) const {
    if(isIconDataVms()) {
        return VmuIconDataVmsFileInfo(reinterpret_cast<IconDataFileInfo*>(gyVmuFlashDirEntryVmsHeader(_dev, _dirEntry)));
    } else return VmuIconDataVmsFileInfo(nullptr);
}

inline VmuExtraBgPvrFileInfo VmuFlashDirEntry::getExtraBgPvrFileInfo(void) const {
    if(isExtraBgPvr()) {
        return VmuExtraBgPvrFileInfo(getVmsHeader());
    } else return VmuExtraBgPvrFileInfo(nullptr);
}



//===========   ICONDATA_VMS ===========
inline VmuIconDataVmsFileInfo::VmuIconDataVmsFileInfo(IconDataFileInfo* info):
    _info(info)
{}
inline bool VmuIconDataVmsFileInfo::isNull(void) const {
    return !_info;
}
inline VmuIconDataVmsFileInfo::operator bool(void) const {
    return !isNull();
}

inline VmuIconDataVmsFileInfo::operator IconDataFileInfo*(void) const {
    return _info;
}


//============= EXTRA BG PVR ==============

inline VmuExtraBgPvrFileInfo::VmuExtraBgPvrFileInfo(const VMSFileInfo* vmsHeader) {
    if(vmsHeader) gyVmuExtraBgPvrFileInfo(vmsHeader, &_info);
    else memset(&_info, 0, sizeof(VmuExtraBgPvrFileInfo));
}


inline bool VmuExtraBgPvrFileInfo::isNull(void) const {
    return !_info.graphicHeader;
}

inline VMU_PVR_TEX_FMT VmuExtraBgPvrFileInfo::getTextureFormat(void) const {
    return static_cast<VMU_PVR_TEX_FMT>(_info.graphicHeader->texFormat);
}

inline VMU_PVR_TEX_LAYOUT VmuExtraBgPvrFileInfo::getTextureLayout(void) const {
    return static_cast<VMU_PVR_TEX_LAYOUT>(_info.graphicHeader->texLayout);
}
inline uint16_t VmuExtraBgPvrFileInfo::getWidth(void) const {
    return _info.graphicHeader->width;
}
inline uint16_t VmuExtraBgPvrFileInfo::getHeight(void) const {
    return _info.graphicHeader->height;
}

inline VmuExtraBgPvrFileInfo::operator ::VmuExtraBgPvrFileInfo(void) const {
    return _info;
};

inline VmuExtraBgPvrFileInfo::operator bool(void) const {
    return !isNull();
}

inline const uint8_t* VmuExtraBgPvrFileInfo::getTextureData(void) const {
    return gyVmuExtraBgPvrImageData(&_info);
}
inline const uint8_t* VmuExtraBgPvrFileInfo::createTextureImageRgb888(void) const {
    return gyVmuExtraBgPvrImageRGB888(&_info);
}



//============== VMS HEADER =============


inline VmuVmsHeader::VmuVmsHeader(VMSFileInfo* info):
    _info(info) {}

inline bool VmuVmsHeader::isNull(void) const {
    return !_info;
}

inline size_t VmuVmsHeader::getHeaderSize(void) const {
    return gyVmuVmsFileInfoHeaderSize(_info);
}
inline size_t VmuVmsHeader::getPayloadSize(void) const {
    return _info->dataBytes;
}

//Properties
inline std::string VmuVmsHeader::getVmuDescription(void) const {
    char buffer[VMU_VMS_FILE_INFO_VMU_DESC_SIZE+1];
    gyVmuVmsHeaderVmuDescGet(_info, buffer);
    return buffer;
}

inline void VmuVmsHeader::setVmuDescription(std::string desc) const {
    strncpy(_info->vmuDesc, desc.c_str(), VMU_VMS_FILE_INFO_VMU_DESC_SIZE);
}

inline std::string VmuVmsHeader::getDcDescription(void) const {
    char buffer[VMU_VMS_FILE_INFO_DC_DESC_SIZE+1];
    gyVmuVmsHeaderDcDescGet(_info, buffer);
    return buffer;
}

inline void VmuVmsHeader::setDcDescription(std::string desc) const {
    strncpy(_info->vmuDesc, desc.c_str(), VMU_VMS_FILE_INFO_DC_DESC_SIZE);
}

inline std::string VmuVmsHeader::getCreatorApp(void) const {
    char buffer[VMU_VMS_FILE_INFO_CREATOR_APP_SIZE+1];
    gyVmuVmsHeaderCreatorAppGet(_info, buffer);
    return buffer;
}
inline void VmuVmsHeader::setCreatorApp(std::string app) {
    strncpy(_info->vmuDesc, app.c_str(), VMU_VMS_FILE_INFO_CREATOR_APP_SIZE);
}

inline uint16_t VmuVmsHeader::getCrc(void) const {
    return _info->crc;
}
inline void VmuVmsHeader::setCrc(uint16_t value) const {
    _info->crc = value;
}

inline uint16_t VmuVmsHeader::getIconCount(void) const {
    return _info->iconCount;
}
inline uint16_t VmuVmsHeader::getAnimSpeed(void) const {
    return _info->animSpeed;
}
inline bool VmuVmsHeader::hasEyeCatch(void) const {
    return getEyeCatchType() != VMS_EYECATCH_NONE;
}
inline VMS_EYECATCH_MODE VmuVmsHeader::getEyeCatchType(void) const {
    return static_cast<VMS_EYECATCH_MODE>(_info->eyecatchType);
}

inline VmuVmsHeader::operator VMSFileInfo*(void) const {
    return _info;
}

inline VmuVmsHeader::operator bool(void) const {
    return !isNull();
}

}

#endif // GYRO_ELYSIAN_VMU_FLASH_HPP

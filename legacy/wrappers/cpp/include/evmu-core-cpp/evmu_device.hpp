#ifndef EVMU_DEVICE_HPP
#define EVMU_DEVICE_HPP

#include <cstdint>
#include <cassert>
#include <limits>
#include <evmu-core-cpp/evmu_flash.hpp>

#include <evmu/types/evmu_ibehavior.h>
#include <evmu/hw/evmu_device.h>

#include "fs/evmu_fat_.h"

#define ELYSIAN_VMU_READ_INVALID_VALUE              0xaa
#define ELYSIAN_VMU_DEVICE_BUILTIN_ICON_RSRC_DIR    ":/volume_icons/"

struct LCDFile;

namespace evmu {

class VmuDevice {
protected:
    EvmuDevice*			pDev_   = nullptr;
    bool                _halted = false;

public:

                        VmuDevice(void) = default;
                        VmuDevice(EvmuDevice* pDev);

    bool                update(double deltaTime) const;
    bool                isNull(void) const;
    bool                saveState(std::string path) const;
    bool                loadState(std::string path) const;

    uint8_t             viewMemoryByte(EvmuAddress address) const;
    uint8_t 			readMemoryByte(uint16_t address) const;
    bool				writeMemoryByte(uint16_t address, uint8_t value) const;

    bool				isRunning(void) const;
    bool                isHalted(void) const;
    void                setHalted(bool value);
    bool				isBiosLoaded(void) const;
    void                skipBiosSetup(bool enable);
//VMU_BIOS_MODE			getCurrentBiosMode(void) const;
    void				reset(void) const;
    uint16_t            getProgramCounter(void) const;
    bool                setProgramCounter(uint16_t address) const;
    LCDFile*            getLcdFile(void) const;

    bool                hasCustomVolumeColor(void) const;
    uint16_t            getVolumeIconShape(void) const;
    void                setVolumeIconShape(uint16_t index);

    //====== LCD SCREEN DISPLAY ==========
    bool                getDisplayPixelValue(unsigned x, unsigned y) const;
    bool                setDisplayPixelValue(unsigned x, unsigned y, bool value) const;
    EVMU_LCD_ICONS      displayIconsEnabled(void) const;
    void                setDisplayIconsEnabled(EVMU_LCD_ICONS icn) const;
    bool                isDisplayPixelGhostingEnabled(void) const;
    bool                isDisplayLinearFilteringEnabled(void) const;
    void                setDisplayLinearFilteringEnabled(bool value) const;
    void                setDisplayPixelGhostingEnabled(bool enabled=true) const;
    int                 getDisplayPixelGhostValue(unsigned x, unsigned y) const;
    bool                isDisplayEnabled(void) const;
    void                setDisplayEnabled(bool enabled=true) const;
    bool                isDisplayUpdateEnabled(void) const;
    void                setDisplayUpdateEnabled(bool enabled=true) const;
    bool                hasDisplayChanged(void) const;
    void                setDisplayChanged(bool value) const;

    //========== GENERAL FLASH ==========

    uint8_t 			readFlashByte(uint32_t address) const;
    bool				writeFlashByte(uint32_t address, uint8_t value) const;

    bool                isSleeping(void) const;
    bool				isFlashFormatted(void) const;
    bool				formatFlash(void) const;
    EvmuFlashUsage	    getFlashMemoryUsage(void) const;

    EvmuRootBlock*      getFlashRootBlock(void) const;

    uint16_t            getFatEntry(uint16_t block) const;
    uint8_t*            getFlashBlockData(uint16_t block) const;

    //===== FILESYSTEM =======
    unsigned			getFlashFileCount(void) const;
    VmuFlashDirEntry	getGameFlashDirEntry(void) const;
    VmuFlashDirEntry 	getFileFlashDirEntry(int index) const;
    VmuFlashDirEntry 	findFlashDirEntry(const char* name) const;

    //========== OTHER ==========

    //Type-compatible with evmu-core C API
                        operator EvmuDevice*(void) const;
                        operator bool(void) const;
    const VmuDevice&    operator =(EvmuDevice* dev);
    bool                operator ==(const VmuDevice& rhs) const;
    bool                operator !=(const VmuDevice& rhs) const;


    //===== IMPLEMENT ME!!!======
    unsigned			getExtraBlockCount(void) const;
    bool				setExtraBlocksEnabled(bool unlocked=true);
    uint8_t 			readWramByte(uint16_t address) const;
    bool				writeWramByte(uint16_t address, uint8_t value) const;
    // battery high/low, battery monitor on/off
    // buzzer shit
    // save/load state
    // export



};



template<typename Derived>
class VirtualByteArray {
public:
    size_t getSize(void) const {
        return static_cast<const Derived*>(this)->_getSize();
    }

    bool setByte(size_t offset, uint8_t value) {
        return static_cast<Derived*>(this)->_setByte(offset, value);
    }

    uint8_t getByte(size_t offset) const {
            return static_cast<const Derived*>(this)->_getByte(offset);
    }

    template<typename T>
    bool extract(size_t offset, T* data) {
        return memcpy(data, offset, sizeof(T));
    }

    bool memcpy(void* data, size_t offset, size_t size) {
        if(offset + size > getSize()) return false;

        for(size_t i = 0; i < size; ++i) {
            *((static_cast<uint8_t*>(data))+i) = getByte(offset + i);
        }

        return true;
    }

    uint8_t     operator[](size_t offset) { return getByte(offset); }

};

class VmuFlashByteArray: public VirtualByteArray<VmuFlashByteArray> {
    VmuDevice          _dev;
    VmuFlashDirEntry   _entry;
    size_t             _size;

    uint16_t _blockFatList[EVMU_FAT_BLOCK_COUNT_DEFAULT];
public:
    uint8_t _getByte(size_t address) const {
        size_t addr = getFlashAddrFromFileOffset(address);
        if(addr < getSize()) {
            return _dev.readFlashByte(addr);
        } return 0;
    }

    size_t _getSize(void) const {
        return _size;
    }

    size_t getFlashAddrFromFileOffset(size_t offset) const {
        size_t block        = _blockFatList[offset / EVMU_FAT_BLOCK_SIZE];
        if(block == EVMU_FAT_BLOCK_FAT_UNALLOCATED) return std::numeric_limits<size_t>::max();
        size_t blockOffset  = offset % EVMU_FAT_BLOCK_SIZE;
        return block * EVMU_FAT_BLOCK_SIZE + blockOffset;
    }

    bool _setByte(size_t address, uint8_t value) {
        size_t addr = getFlashAddrFromFileOffset(address);
        if(addr < getSize()) {
            return _dev.writeFlashByte(addr, value);
        } return false;
    }

    VmuFlashByteArray(EvmuDevice* dev, EvmuDirEntry* entry):
        _dev(dev),
        _entry(dev, entry)
    {
        for(std::size_t i = 0; i < EVMU_FAT_BLOCK_COUNT_DEFAULT; ++i)
            _blockFatList[i] = EVMU_FAT_BLOCK_FAT_UNALLOCATED;

        _size = _entry.getFileSizeBytes();

        EvmuBlock fatBlock = _entry.getFirstBlock();
        std::size_t idx = 0;

        while(fatBlock != EVMU_FAT_BLOCK_FAT_LAST_IN_FILE) {
            _blockFatList[idx++] = fatBlock;
            fatBlock = EvmuFat_blockNext(dev->pFat, fatBlock);
        }

    }
};




//======== INLINEZ ============
inline VmuDevice::VmuDevice(EvmuDevice* dev):
    pDev_(dev)
{}

inline bool VmuDevice::operator ==(const VmuDevice& rhs) const {
    return pDev_ == rhs.pDev_;
}
inline bool VmuDevice::operator !=(const VmuDevice& rhs) const {
    return !(*this == rhs);
}

inline bool VmuDevice::isNull(void) const {
    return !pDev_;
}

inline bool VmuDevice::update(double deltaTime) const {
    return EvmuIBehavior_update(EVMU_IBEHAVIOR(pDev_), deltaTime*1000000000.0);//gyVmuDeviceUpdate(_dev, deltaTime);
}

inline const VmuDevice& VmuDevice::operator =(EvmuDevice* dev) {
    pDev_ = dev;
    return *this;
}

inline VmuDevice::operator bool(void) const {
    return !isNull();
}

inline VmuDevice::operator EvmuDevice*(void) const {
    return pDev_;
}

inline uint8_t VmuDevice::viewMemoryByte(EvmuAddress address) const {
    return EvmuMemory_viewData(pDev_->pMemory, address);
}

inline uint8_t VmuDevice::readMemoryByte(uint16_t address) const {
    return EvmuMemory_readData(pDev_->pMemory, address);
}

inline bool VmuDevice::writeMemoryByte(uint16_t address, uint8_t value) const {
    return GBL_RESULT_SUCCESS(EvmuMemory_writeData(pDev_->pMemory, address, value));
}

inline bool VmuDevice::isBiosLoaded(void) const {
    return EvmuRom_biosType(pDev_->pRom) != EVMU_BIOS_TYPE_EMULATED;
}

inline void VmuDevice::skipBiosSetup(bool enable) {
    EvmuRom_skipBiosSetup(pDev_->pRom, enable);
}

inline uint16_t VmuDevice::getProgramCounter(void) const {
    return EvmuCpu_pc(pDev_->pCpu);
}

inline bool VmuDevice::setProgramCounter(uint16_t address) const {
    EvmuCpu_setPc(pDev_->pCpu, address);
    return true;
}

inline void VmuDevice::reset(void) const {
    EvmuIBehavior_reset(EVMU_IBEHAVIOR(pDev_));
}

inline uint8_t VmuDevice::readFlashByte(uint32_t address) const {
    return EvmuMemory_readFlash(pDev_->pMemory, address);
}

inline bool	VmuDevice::writeFlashByte(uint32_t address, uint8_t value) const {
    return GBL_RESULT_SUCCESS(EvmuMemory_writeFlash(pDev_->pMemory, address, value));
}

inline bool VmuDevice::getDisplayPixelValue(unsigned x, unsigned y) const {
    return EvmuLcd_decoratedPixel(pDev_->pLcd, x, y);
}

inline bool VmuDevice::setDisplayPixelValue(unsigned x, unsigned y, bool value) const {
    EvmuLcd_setPixel(pDev_->pLcd, x, y, value);
    return true;
}

inline bool VmuDevice::isDisplayEnabled(void) const {
    return EvmuLcd_screenEnabled(pDev_->pLcd);
}

inline void VmuDevice::setDisplayEnabled(bool enabled) const {
    EvmuLcd_setScreenEnabled(pDev_->pLcd, enabled);
}

inline bool VmuDevice::isDisplayUpdateEnabled(void) const {
    return EvmuLcd_refreshEnabled(pDev_->pLcd);
}

inline void VmuDevice::setDisplayUpdateEnabled(bool enabled) const {
    return EvmuLcd_setRefreshEnabled(pDev_->pLcd, enabled);
}

inline EVMU_LCD_ICONS VmuDevice::displayIconsEnabled(void) const {
    return EvmuLcd_icons(pDev_->pLcd);
}

inline void VmuDevice::setDisplayIconsEnabled(EVMU_LCD_ICONS icons) const {
    EvmuLcd_setIcons(pDev_->pLcd, icons);
}

inline bool VmuDevice::isDisplayPixelGhostingEnabled(void) const {
    return !!pDev_->pLcd->ghostingEnabled;
}

inline void VmuDevice::setDisplayPixelGhostingEnabled(bool enabled) const {
    pDev_->pLcd->ghostingEnabled = enabled;
}

inline bool VmuDevice::isDisplayLinearFilteringEnabled(void) const {
    return pDev_->pLcd->filterEnabled;
}

inline void VmuDevice::setDisplayLinearFilteringEnabled(bool enabled) const {
    pDev_->pLcd->filterEnabled = enabled;
}

inline int VmuDevice::getDisplayPixelGhostValue(unsigned x, unsigned y) const {
    return EvmuLcd_decoratedPixel(pDev_->pLcd, x, y);
}

inline unsigned	VmuDevice::getFlashFileCount(void) const {
    return EvmuFileManager_count(pDev_->pFileMgr);
}

inline VmuFlashDirEntry VmuDevice::getGameFlashDirEntry(void) const {
    return { pDev_, EvmuFileManager_game(pDev_->pFileMgr) };
}

inline VmuFlashDirEntry VmuDevice::getFileFlashDirEntry(int index) const {
    return { pDev_, EvmuFat_dirEntry(pDev_->pFat, index) };
}

inline VmuFlashDirEntry VmuDevice::findFlashDirEntry(const char* name) const {
    return { pDev_, EvmuFileManager_find(pDev_->pFileMgr, name) };
}

inline bool	VmuDevice::isFlashFormatted(void) const {
    return EvmuFat_isFormatted(pDev_->pFat);
}

inline bool	VmuDevice::formatFlash(void) const {
    return EvmuFat_format(pDev_->pFat, nullptr);
}

inline EvmuFlashUsage VmuDevice::getFlashMemoryUsage(void) const {
    EvmuFlashUsage usage{};

    EvmuFat_usage(pDev_->pFat, &usage);
    return usage;
}
inline bool	VmuDevice::setExtraBlocksEnabled(bool unlocked) {
    return false;
}

inline EvmuRootBlock* VmuDevice::getFlashRootBlock(void) const {
    return EvmuFat_root(pDev_->pFat);
}

inline uint16_t VmuDevice::getFatEntry(uint16_t block) const {
    return EvmuFat_blockNext(pDev_->pFat, block);
}

inline uint8_t* VmuDevice::getFlashBlockData(uint16_t block) const {
    return reinterpret_cast<uint8_t*>(EvmuFat_blockData(pDev_->pFat, block));
}

inline bool VmuDevice::isRunning(void) const {
    return (EvmuFileManager_game(pDev_->pFileMgr) || isBiosLoaded());
}

inline bool VmuDevice::isHalted(void) const {
    return _halted;
}

inline void VmuDevice::setHalted(bool value) {
    _halted = value;
}

inline LCDFile* VmuDevice::getLcdFile(void) const {
    return nullptr;
}

inline bool VmuDevice::hasDisplayChanged(void) const {
    return pDev_->pLcd->screenChanged;
}

inline void VmuDevice::setDisplayChanged(bool val) const {
    pDev_->pLcd->screenChanged = val;
}


}

#endif // GYRO_ELYSIAN_VMU_DEVICE_HPP

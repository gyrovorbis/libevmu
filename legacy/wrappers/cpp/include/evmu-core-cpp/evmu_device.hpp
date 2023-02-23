#ifndef EVMU_DEVICE_HPP
#define EVMU_DEVICE_HPP

#include <cstdint>
#include <cassert>
#include <limits>
#include <evmu-core/gyro_vmu_flash.h>
#include <evmu-core/gyro_vmu_device.h>
#include <evmu-core-cpp/evmu_flash.hpp>

#include <evmu/types/evmu_ibehavior.h>
#include "hw/evmu_device_.h"
#include "hw/evmu_memory_.h"

#define ELYSIAN_VMU_READ_INVALID_VALUE              0xaa
#define ELYSIAN_VMU_DEVICE_BUILTIN_ICON_RSRC_DIR    ":/volume_icons/"

struct LCDFile;

namespace evmu {

class VmuDevice {
protected:
    VMUDevice*			_dev    = nullptr;
    bool                _halted = false;

public:

                        VmuDevice(void) = default;
                        VmuDevice(VMUDevice* dev);

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
//VMU_BIOS_MODE			getCurrentBiosMode(void) const;
    void				reset(void) const;
    uint16_t            getProgramCounter(void) const;
    bool                setProgramCounter(uint16_t address) const;
    LCDFile*            getLcdFile(void) const;

    bool                hasCustomVolumeColor(void) const;
    bool                hasCustomVolumeIcon(void) const;
    uint16_t            getVolumeIconShape(void) const;
    void                setVolumeIconShape(uint16_t index);

    //====== LCD SCREEN DISPLAY ==========
    bool                getDisplayPixelValue(unsigned x, unsigned y) const;
    bool                setDisplayPixelValue(unsigned x, unsigned y, bool value) const;
    bool                isDisplayModeIconEnabled(EVMU_LCD_ICON icn) const;
    bool                setDisplayModeIconEnabled(EVMU_LCD_ICON icn, bool enabled=true) const;
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
    bool				defragmentFlash(void) const;
    VMUFlashMemUsage	getFlashMemoryUsage(void) const;

    VMUFlashRootBlock*	getFlashRootBlock(void) const;

    uint16_t            getFatEntry(uint16_t block) const;
    uint8_t*            getFlashBlockData(uint16_t block) const;

    //===== FILESYSTEM =======
    unsigned			getFlashFileCount(void) const;
    VmuFlashDirEntry	getGameFlashDirEntry(void) const;
    VmuFlashDirEntry	getIconDataVmsFlashDirEntry(void) const;
    VmuFlashDirEntry 	getExtraBgPvrFlashDirEntry(void) const;
    VmuFlashDirEntry 	getFileFlashDirEntry(int index) const;
    VmuFlashDirEntry 	findFlashDirEntry(const char* name) const;

    //========== OTHER ==========

    //Type-compatible with evmu-core C API
                        operator VMUDevice*(void) const;
                        operator bool(void) const;
    const VmuDevice&    operator =(VMUDevice* dev);
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
    size_t              _size;

    uint16_t _blockFatList[VMU_FLASH_BLOCK_COUNT_DEFAULT];
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
        size_t block        = _blockFatList[offset / VMU_FLASH_BLOCK_SIZE];
        if(block == VMU_FLASH_BLOCK_FAT_UNALLOCATED) return std::numeric_limits<size_t>::max();
        size_t blockOffset  = offset % VMU_FLASH_BLOCK_SIZE;
        return block * VMU_FLASH_BLOCK_SIZE + blockOffset;
    }

    bool _setByte(size_t address, uint8_t value) {
        size_t addr = getFlashAddrFromFileOffset(address);
        if(addr < getSize()) {
            return _dev.writeFlashByte(addr, value);
        } return false;
    }

    VmuFlashByteArray(VMUDevice* dev, VMUFlashDirEntry* entry):
        _dev(dev),
        _entry(dev, entry)
    {
        for(int i = 0; i < VMU_FLASH_BLOCK_COUNT_DEFAULT; ++i)
            _blockFatList[i] = VMU_FLASH_BLOCK_FAT_UNALLOCATED;

        _size = _entry.getFileSizeBytes();

        uint16_t fatBlock = _entry.getFirstBlock();
        int idx = 0;

        while(fatBlock != VMU_FLASH_BLOCK_FAT_LAST_IN_FILE) {
            _blockFatList[idx++] = fatBlock;
            fatBlock = gyVmuFlashBlockNext(dev, fatBlock);
        }

    }
};




//======== INLINEZ ============
inline VmuDevice::VmuDevice(VMUDevice* dev):
    _dev(dev)
{}

inline bool VmuDevice::operator ==(const VmuDevice& rhs) const {
    return _dev == rhs._dev;
}
inline bool VmuDevice::operator !=(const VmuDevice& rhs) const {
    return !(*this == rhs);
}

inline bool VmuDevice::isNull(void) const {
    return !_dev;
}

inline bool VmuDevice::update(double deltaTime) const {
    return EvmuIBehavior_update(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)), deltaTime*1000000000.0);//gyVmuDeviceUpdate(_dev, deltaTime);
}

inline const VmuDevice& VmuDevice::operator =(VMUDevice* dev) {
    _dev = dev;
    return *this;
}

inline VmuDevice::operator bool(void) const {
    return !isNull();
}

inline VmuDevice::operator VMUDevice*(void) const {
    return _dev;
}

inline uint8_t VmuDevice::viewMemoryByte(EvmuAddress address) const {
    return address < VMU_MEM_ADDR_SPACE_RANGE?
                EvmuMemory_viewInt(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pMemory, address):
                ELYSIAN_VMU_READ_INVALID_VALUE;
}

inline uint8_t VmuDevice::readMemoryByte(uint16_t address) const {
    return address < VMU_MEM_ADDR_SPACE_RANGE?
                EvmuMemory_readInt(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pMemory, address):
                ELYSIAN_VMU_READ_INVALID_VALUE;
}

inline bool VmuDevice::writeMemoryByte(uint16_t address, uint8_t value) const {
    if(address < VMU_MEM_ADDR_SPACE_RANGE) {
        EvmuMemory_writeInt(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pMemory, address, value);
        return true;
    } else return false;
}

inline bool VmuDevice::isBiosLoaded(void) const {
    return EvmuRom_biosLoaded(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pRom);
}

inline uint16_t VmuDevice::getProgramCounter(void) const {
    return EvmuCpu_pc(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pCpu);
}

inline bool VmuDevice::setProgramCounter(uint16_t address) const {
    EvmuCpu_setPc(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pCpu, address);
    return true;
}

inline void VmuDevice::reset(void) const {
    EvmuIBehavior_reset(EVMU_IBEHAVIOR(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)));
}

inline uint8_t VmuDevice::readFlashByte(uint32_t address) const {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(_dev);
    return address < EVMU_FLASH_SIZE? pDevice_->pMemory->flash[address] : ELYSIAN_VMU_READ_INVALID_VALUE;
}

inline bool	VmuDevice::writeFlashByte(uint32_t address, uint8_t value) const {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_PRISTINE(_dev);
    if(address < EVMU_FLASH_SIZE) {
        pDevice_->pMemory->flash[address] = value;
        return true;
    } else return false;
}

inline bool VmuDevice::getDisplayPixelValue(unsigned x, unsigned y) const {
    return (x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT)?
                EvmuLcd_decoratedPixel(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, x, y) :
                ELYSIAN_VMU_READ_INVALID_VALUE;
}

inline bool VmuDevice::setDisplayPixelValue(unsigned x, unsigned y, bool value) const {
    if(x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT) {
        EvmuLcd_setPixel(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, x, y, value);
        return true;
    } else return false;
}


inline bool VmuDevice::isDisplayEnabled(void) const {
    return EvmuLcd_displayEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd);
}
inline void VmuDevice::setDisplayEnabled(bool enabled) const {
    EvmuLcd_setDisplayEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, enabled);
}
inline bool VmuDevice::isDisplayUpdateEnabled(void) const {
    return EvmuLcd_refreshEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd);
}
inline void VmuDevice::setDisplayUpdateEnabled(bool enabled) const {
    return EvmuLcd_setRefreshEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, enabled);
}

inline bool VmuDevice::isDisplayModeIconEnabled(EVMU_LCD_ICON icn) const {
    return icn < EVMU_LCD_ICON_COUNT?
                EvmuLcd_iconEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, icn) : 0;
}

inline bool VmuDevice::setDisplayModeIconEnabled(EVMU_LCD_ICON icn, bool enabled) const {
    if(icn < EVMU_LCD_ICON_COUNT) {
        EvmuLcd_setIconEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, icn, enabled);
        return true;
    } else return false;
}

inline bool VmuDevice::isDisplayPixelGhostingEnabled(void) const {
    return EvmuLcd_ghostingEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd);
}
inline void VmuDevice::setDisplayPixelGhostingEnabled(bool enabled) const {
    EvmuLcd_setGhostingEnabled(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, enabled);
}

inline bool VmuDevice::isDisplayLinearFilteringEnabled(void) const {
    return EvmuLcd_filter(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd) == EVMU_LCD_FILTER_LINEAR;
}
inline void VmuDevice::setDisplayLinearFilteringEnabled(bool enabled) const {
    EvmuLcd_setFilter(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd,
                      enabled? EVMU_LCD_FILTER_LINEAR : EVMU_LCD_FILTER_NONE);
}

inline int VmuDevice::getDisplayPixelGhostValue(unsigned x, unsigned y) const {
    return (x < EVMU_LCD_PIXEL_WIDTH && y < EVMU_LCD_PIXEL_HEIGHT)?
                EvmuLcd_decoratedPixel(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, x, y) :
                0;
}

inline unsigned	VmuDevice::getFlashFileCount(void) const {
    return gyVmuFlashFileCount(_dev);
}
inline VmuFlashDirEntry VmuDevice::getGameFlashDirEntry(void) const {
    return VmuFlashDirEntry(_dev, gyVmuFlashDirEntryGame(_dev));
}
inline VmuFlashDirEntry VmuDevice::getIconDataVmsFlashDirEntry(void) const {
    return VmuFlashDirEntry(_dev, gyVmuFlashDirEntryIconData(_dev));
}
inline VmuFlashDirEntry VmuDevice::getExtraBgPvrFlashDirEntry(void) const {
    return VmuFlashDirEntry(_dev, gyVmuFlashDirEntryExtraBgPvr(_dev));
}
inline VmuFlashDirEntry VmuDevice::getFileFlashDirEntry(int index) const {
    return VmuFlashDirEntry(_dev, gyVmuFlashFileAtIndex(_dev, index));
}
inline VmuFlashDirEntry VmuDevice::findFlashDirEntry(const char* name) const {
    return VmuFlashDirEntry(_dev, gyVmuFlashDirEntryFind(_dev, name));
}

inline bool	VmuDevice::isFlashFormatted(void) const {
    return gyVmuFlashCheckFormatted(_dev);
}
inline bool	VmuDevice::formatFlash(void) const {
    return gyVmuFlashFormatDefault(_dev);
}
inline bool	VmuDevice::defragmentFlash(void) const {
    return gyVmuFlashDefragment(_dev, -1);
}
inline VMUFlashMemUsage	VmuDevice::getFlashMemoryUsage(void) const {
    return gyVmuFlashMemUsage(_dev);
}
inline bool	VmuDevice::setExtraBlocksEnabled(bool unlocked) {

}
inline VMUFlashRootBlock* VmuDevice::getFlashRootBlock(void) const {
    return gyVmuFlashBlockRoot(_dev);
}

inline uint16_t VmuDevice::getFatEntry(uint16_t block) const {
    uint16_t* entry = gyVmuFlashBlockFATEntry(_dev, block);
    assert(entry);
    return *entry;
}

inline uint8_t* VmuDevice::getFlashBlockData(uint16_t block) const {
    return gyVmuFlashBlock(_dev, block);
}

inline bool VmuDevice::isRunning(void) const {
    return (gyVmuFlashDirEntryGame(_dev) || isBiosLoaded());
}

inline bool VmuDevice::isHalted(void) const {
    return _halted;
}

inline void VmuDevice::setHalted(bool value) {
    _halted = value;
}

inline LCDFile* VmuDevice::getLcdFile(void) const {
    return _dev->lcdFile;
}

inline bool VmuDevice::hasDisplayChanged(void) const {
    return EvmuLcd_updated(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd);
}

inline void VmuDevice::setDisplayChanged(bool val) const {
    EvmuLcd_setUpdated(EVMU_DEVICE_PRISTINE_PUBLIC(_dev)->pLcd, val);
}


}

#endif // GYRO_ELYSIAN_VMU_DEVICE_HPP

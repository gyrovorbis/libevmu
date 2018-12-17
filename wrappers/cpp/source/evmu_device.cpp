#include <evmu-core-cpp/evmu_device.hpp>

namespace evmu {
//===== INSTANCED ======


bool VmuDevice::saveState(std::string path) const {
    return gyVmuDeviceSaveState(_dev, path.c_str());
}

bool VmuDevice::loadState(std::string path) const {
    return gyVmuDeviceLoadState(_dev, path.c_str());
}

bool VmuDevice::hasCustomVolumeColor(void) const {
    return getFlashRootBlock()->volumeLabel.vmu.customColor;
}

bool VmuDevice::hasCustomVolumeIcon(void) const {
    auto iconDataEntry = getIconDataVmsFlashDirEntry();
    if(iconDataEntry.isNull()) return false;
    if(iconDataEntry.getIconDataFileInfo().isNull()) return false;
    return iconDataEntry.getIconDataFileInfo().getDcIconData();
}

uint16_t VmuDevice::getVolumeIconShape(void) const {
    VMUFlashRootBlock* root = getFlashRootBlock();
    return root->iconShape;
}

void VmuDevice::setVolumeIconShape(uint16_t index) {
    VMUFlashRootBlock* root = getFlashRootBlock();
    root->iconShape = index;
}



}

#include <evmu-core-cpp/evmu_device.hpp>

namespace evmu {
//===== INSTANCED ======


bool VmuDevice::saveState(std::string path) const {
    return false;
}

bool VmuDevice::loadState(std::string path) const {
    return false;
}

bool VmuDevice::hasCustomVolumeColor(void) const {
    return getFlashRootBlock()->volumeLabel.vmu.customColor;
}

uint16_t VmuDevice::getVolumeIconShape(void) const {
    return getFlashRootBlock()->iconShape;
}

void VmuDevice::setVolumeIconShape(uint16_t index) {
    getFlashRootBlock()->iconShape = index;
}



}

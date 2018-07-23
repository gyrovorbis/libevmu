#include "gyro_vmu_maple.h"
#include "gyro_vmu_version.h"
#include <gyro_visual_memory_api.h>


VMUMapleDeviceInfo _evmuMapleDevInfo = {
    MAPLE_FUNC_MEMCARD|MAPLE_FUNC_LCD|MAPLE_FUNC_CLOCK,
    { 1, 2, 3 },
    0,
    0,
    "ElysianVMU v"GYRO_VMU_VERSION_STRING,
    "Copyright 2018 Elysian Shadows",
    0x7c,
    0x82
};

const VMUMapleDeviceInfo* VMUMapleDeviceInfoGet(const struct VMUDevice* dev) {
    (void)dev;
    return &_evmuMapleDevInfo;
}

#include "gyro_vmu_port7.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_sfr.h"
#include <gyro_system_api.h>

void gyVmuPort7Connect(struct VMUDevice* dev, VMU_PORT7_CONNECTION_TYPE type) {

    switch(type) {
    case VMU_PORT7_CONNECTION_NONE:
        _gyLog(GY_DEBUG_VERBOSE, "Disconnecting VMU external serial pins.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~(SFR_P7_P70_MASK|SFR_P7_P72_MASK|SFR_P7_P73_MASK); //Clear all external pins
        break;
    case VMU_PORT7_CONNECTION_VMU:
        _gyLog(GY_DEBUG_VERBOSE, "Configuring VMU external serial pins for VMU to VMU connection.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~(SFR_P7_P70_MASK|SFR_P7_P72_MASK); //Clear DC pins
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= SFR_P7_P73_MASK;   //Set VMU pin
        break;

    case VMU_PORT7_CONNECTION_DC:
        _gyLog(GY_DEBUG_VERBOSE, "Configuring VMU external serial pins for DC to VMU connection.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~SFR_P7_P73_MASK; //Clear VMU/VMU connection pin
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= (SFR_P7_P70_MASK|SFR_P7_P72_MASK); //set external voltage and DC controller pins
        break;
    }
}

void gyVmuPort7Disconnect(struct VMUDevice* dev) {
    gyVmuPort7Connect(dev, VMU_PORT7_CONNECTION_NONE);
}

#include <evmu/hw/evmu_port7.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_device_.h"
#if 0
void gyVmuPort7Connect(VmuDevice dev, EVMU_PORT7_CONNECTION_TYPE type) {

    switch(type) {
    case EVMU_PORT7_CONNECTION_NONE:
        _gyLog(GY_DEBUG_VERBOSE, "Disconnecting VMU external serial pins.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~(SFR_P7_P70_MASK|SFR_P7_P72_MASK|SFR_P7_P73_MASK); //Clear all external pins
        break;
    case EVMU_PORT7_CONNECTION_VMU:
        _gyLog(GY_DEBUG_VERBOSE, "Configuring VMU external serial pins for VMU to VMU connection.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~(SFR_P7_P70_MASK|SFR_P7_P72_MASK); //Clear DC pins
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= SFR_P7_P73_MASK;   //Set VMU pin
        break;

    case EVMU_PORT7_CONNECTION_DC:
        _gyLog(GY_DEBUG_VERBOSE, "Configuring VMU external serial pins for DC to VMU connection.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~SFR_P7_P73_MASK; //Clear VMU/VMU connection pin
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= (SFR_P7_P70_MASK|SFR_P7_P72_MASK); //set external voltage and DC controller pins
        break;
    }
}

void gyVmuPort7Disconnect(EvmuDevice dev) {
    gyVmuPort7Connect(dev, VMU_PORT7_CONNECTION_NONE);
}
#endif

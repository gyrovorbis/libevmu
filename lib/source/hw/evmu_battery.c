#include "evmu_device_.h"
#include <evmu/hw/evmu_sfr.h>
#if 0
int gyVmuBatteryLow(const struct VMUDevice* dev) {
    return !((dev->sfr[SFR_OFFSET(SFR_ADDR_P7)]&SFR_P7_P71_MASK) >> SFR_P7_P71_POS);
}

void gyVmuBatteryLowSet(struct VMUDevice* dev, int low) {
    if(low) dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~SFR_P7_P71_MASK;
    else    dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= SFR_P7_P71_MASK;
}

int gyVmuBatteryMonitorEnabled(const struct VMUDevice* dev) {
    return dev->ram[0][RAM_ADDR_BATTERY_CHECK]? 0 : 1;
}

void gyVmuBatteryMonitorEnabledSet(struct VMUDevice* dev, int enabled) {
    dev->ram[0][RAM_ADDR_BATTERY_CHECK] = enabled? 0 : 0xff;
}
#endif

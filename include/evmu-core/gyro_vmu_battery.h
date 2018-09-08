#ifndef GYRO_VMU_BATTERY_H
#define GYRO_VMU_BATTERY_H

#ifdef __cplusplus
extern "C" {
#endif

struct  VMUDevice;

int     gyVmuBatteryLow(const struct VMUDevice* dev);
void    gyVmuBatteryLowSet(struct VMUDevice* dev, int low);
int     gyVmuBatteryMonitorEnabled(const struct VMUDevice* dev);
void    gyVmuBatteryMonitorEnabledSet(struct VMUDevice* dev, int enabled);

#ifdef __cplusplus
}
#endif


#endif // GYRO_VMU_BATTERY_H


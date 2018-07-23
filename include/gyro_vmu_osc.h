#ifndef GYRO_VMU_OSC_H
#define GYRO_VMU_OSC_H

#ifdef __cplusplus
extern "C" {
#endif

#define VMU_OSC_QUARTZ_FREQ_HZ  32768
#define VMU_OSC_RC_FREQ_HZ      879236

struct VMUDevice;

typedef enum VMU_OSC_SRC {
    VMU_OSC_SRC_QUARTZ,
    VMU_OSC_SRC_RC,
    VMU_OSC_SRC_DC
} VMU_OSC_SRC;

typedef enum VMU_OSC_DIV {
    VMU_OSC_DIV_12,
    VMU_OSC_DIV_6,
} VMU_OSC_DIV;

typedef enum VMU_OSC_MODE {
    VMU_OSC_MODE_879KHZ,

} VMU_OSC_MODE;


unsigned gyVmuOscCyclesPerSec(struct VMUDevice* dev);
float gyVmuOscSecPerCycle(struct VMUDevice* dev);

//battery-life calculations
//set/gets

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_OSCILLATOR_H


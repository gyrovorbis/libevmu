#ifndef GYRO_VMU_PORT7_H
#define GYRO_VMU_PORT7_H

#ifdef __cplusplus
extern "C" {
#endif

struct VMUDevice;

typedef enum VMU_PORT7_CONNECTION_TYPE {
    VMU_PORT7_CONNECTION_NONE   = 0x00,
    VMU_PORT7_CONNECTION_DC     = 0x44, //'D' ASCII
    VMU_PORT7_CONNECTION_VMU    = 0x56  //'V' ASCII
} VMU_PORT7_CONNECTION_TYPE;


void gyVmuPort7Connect(struct VMUDevice* dev, VMU_PORT7_CONNECTION_TYPE type);
void gyVmuPort7Disconnect(struct VMUDevice* dev);


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_PORT7_H

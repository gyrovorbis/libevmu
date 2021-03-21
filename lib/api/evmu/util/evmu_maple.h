#ifndef GYRO_VMU_MAPLE_H
#define GYRO_VMU_MAPLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GYRO_VMU_MAPLE_DEVICE_INFO_FUNCTION_DATA_SIZE   3
#define GYRO_VMU_MAPLE_DEVICE_INFO_PRODUCT_NAME_SIZE    30
#define GYRO_VMU_MAPLE_DEVICE_INFO_PRODUCT_LICENSE_SIZE 60

struct VMUDevice;

typedef struct VMUMapleDeviceInfo {
    int32_t     func;                                                           //function codes supported by peripheral (or-d together) (big endian)
    int32_t     functionData[GYRO_VMU_MAPLE_DEVICE_INFO_FUNCTION_DATA_SIZE];    //additional info for supported function codes
    int8_t      areaCode;                                                       //regional code of peripheral
    int8_t      connectorDirection;                                             //physical orientation of BUS connection
    char        productName[GYRO_VMU_MAPLE_DEVICE_INFO_PRODUCT_NAME_SIZE];
    char        productLicense[GYRO_VMU_MAPLE_DEVICE_INFO_PRODUCT_LICENSE_SIZE];
    int16_t     standbyPower;                                                   //standby power consumption (little endian)
    int16_t     maxPower;                                                       //maximum power consumption (little endian
} VMUMapleDeviceInfo;


const VMUMapleDeviceInfo* VMUMapleDeviceInfoGet(const struct VMUDevice* dev);


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_MAPLE_H

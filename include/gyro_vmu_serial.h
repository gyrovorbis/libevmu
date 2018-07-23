#ifndef GYRO_VMU_SERIAL_H
#define GYRO_VMU_SERIAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VMU_SERIAL_IFACE_COUNT      2
#define VMU_SERIAL_IFACE_RECV_POS   0
#define VMU_SERIAL_IFACE_RECV_MASK  0x1
#define VMU_SERIAL_IFACE_SEND_POS   1
#define VMU_SERIAL_IFACE_SEND_MASK  0x2

struct VMUDevice;

typedef enum VMU_SERIAL_CLK_STATE {
    VMU_SERIAL_CLK_STATE1,
    VMU_SERIAL_CLK_STATE2
} VMU_SERIAL_CLK_STATE;

typedef enum VMU_SERIAL_SIGNAL {
    VMU_SERIAL_SO0,
    VMU_SERIAL_SI0,
    VMU_SERIAL_SCK0,
    VMU_SERIAL_SO1,
    VMU_SERIAL_SI1,
    VMU_SERIAL_SCK1,
    VMU_SERIAL_SIGNAL_COUNT
} VMU_SERIAL_SIGNAL;

typedef struct VMUSerialIface {
    int     sbufCurPos;
    int     sentFirstBit;
    int     sconReg;
    int     sbufReg;
    uint8_t intReq;
    uint8_t soPin;
    uint8_t siPin;
    uint8_t sckPin;
    uint8_t sbufVal;
    uint8_t ovWaiting;
} VMUSerialIface;

typedef struct VMUSerial {
    VMUSerialIface  ifaces[VMU_SERIAL_IFACE_COUNT];
    float           timeElapsed;
    float           tsbr;
    uint8_t         sbrState;
} VMUSerial;


void gyVmuSerialInit(struct VMUDevice* dev);
void gyVmuSerialUpdate(struct VMUDevice* dev, float deltaTime);
void gyVmuSerialMemorySink(struct VMUDevice* dev, int addr, uint8_t value);
void gyVmuSerialP1Recv(struct VMUDevice* dev, uint8_t pins, uint8_t changedMask);

int gyVmuSerialIfaceConfigMask(const struct VMUDevice* dev, int ifaceIndex);



#ifdef __cplusplus
}
#endif





#endif // GYRO_VMU_SERIAL_H

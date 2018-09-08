#ifndef GYRO_VMU_PORT1_H
#define GYRO_VMU_PORT1_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VMUDevice;

typedef enum PORT1_PIN {
    PORT1_PIN0,
    PORT1_PIN1,
    PORT1_PIN2,
    PORT1_PIN3,
    PORT1_PIN4,
    PORT1_PIN5,
    PORT1_PIN6,
    PORT1_PIN7,
    PORT1_SIZE
} PORT1_PIN;

typedef enum PORT1_PIN_SRC {
    PORT1_PIN_DATA,
    PORT1_PIN_SERIAL
} PORT1_PIN_SRC;

typedef enum PORT1_PIN_DIR {
    PORT1_PIN_IN,
    PORT1_PIN_OUT
} PORT1_PIN_DIR;

typedef int (*Port1ExtReadyReadCb)(void* context);
typedef int (*Port1ExtReadCb)(void* context, uint8_t* buff, size_t bytes);
typedef int (*Port1ExtWriteCb)(void* context, const uint8_t* buff, size_t bytes);

typedef struct VMUPort1Ext {
    void*               context;
    Port1ExtReadyReadCb readyRead;
    Port1ExtReadCb      read;
    Port1ExtWriteCb     write;
} VMUPort1Ext;

typedef struct VMUPort1 {
    VMUPort1Ext*    ext;
    uint8_t         pins;
    uint8_t         serialInSignals;
    uint8_t         extSignals;
    uint8_t         pendingMask;
    uint8_t         writing;
} VMUPort1;

void gyVmuPort1MemorySink(struct VMUDevice* dev, int addr, uint8_t val);
void gyVmuPort1PollRecv(struct VMUDevice* dev);

int gyVmuPort1Pin(const struct VMUDevice* dev, uint8_t pin);
int gyVmuPort1Pins(const struct VMUDevice* dev);

void gyVmuPort1PinsWriteBegin(struct VMUDevice* dev);
void gyVmuPort1PinsWriteEnd(struct VMUDevice* dev);
void gyVmuPort1PinSerialWrite(struct VMUDevice* dev, uint8_t pin, uint8_t value);
void gyVmuPort1PinsSerialWrite(struct VMUDevice* dev, uint8_t value);

PORT1_PIN_SRC gyVmuPort1PinSrc(const struct VMUDevice* dev, uint8_t pin);
PORT1_PIN_DIR gyVmuPort1PinDir(const struct VMUDevice* dev, uint8_t pin);

void gyVmuPort1ExtConnect(struct VMUDevice* dev, VMUPort1Ext* ext);



#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_PORT1_H

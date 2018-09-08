#ifndef GYRO_VMU_GAMEPAD_H
#define GYRO_VMU_GAMEPAD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VMUDevice;
struct GYKeyboard;
struct GYController;

typedef enum VMU_GAMEPAD_BUTTON {
    VMU_GAMEPAD_UP,
    VMU_GAMEPAD_DOWN,
    VMU_GAMEPAD_LEFT,
    VMU_GAMEPAD_RIGHT,
    VMU_GAMEPAD_A,
    VMU_GAMEPAD_B,
    VMU_GAMEPAD_MODE,
    VMU_GAMEPAD_SLEEP,
    VMU_GAMEPAD_MAX
} VMU_GAMEPAD_BUTTON;

typedef struct VMUGamepad {
    struct GYKeyboard*      kbd;
    struct GYController*    cont;
    uint8_t                 turboB;
    uint8_t                 turboA;
    uint8_t                 u;
    uint8_t                 d;
    uint8_t                 l;
    uint8_t                 r;
    uint8_t                 a;
    uint8_t                 b;
    uint8_t                 ta;
    uint8_t                 tb;
    uint8_t                 m;
    uint8_t                 s;
    uint8_t                 lt;
    uint8_t                 rt;
    bool                    kbdAttached;
    bool                    contAttached;
} VMUGamepad;

int     gyVmuButtonStateGet(const struct VMUDevice* dev, VMU_GAMEPAD_BUTTON but);
void    gyVmuButtonStateSet(struct VMUDevice* dev, VMU_GAMEPAD_BUTTON but, int down);
int     gyVmuGamepadPoll(struct VMUDevice* dev);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_GAMEPAD_H


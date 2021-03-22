#ifndef EVMU_GAMEPAD_H
#define EVMU_GAMEPAD_H

#include <stdbool.h>
#include <stdint.h>
#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

// THIS SHIT SHOULD BE MERGED WITH PORT3 LOGIC/DATA AND SHIT!!111

struct VMUDevice;
struct GYKeyboard;
struct GYController;

typedef enum EVMU_GAMEPAD_BUTTON {
    EVMU_GAMEPAD_BUTTON_UP,
    EVMU_GAMEPAD_BUTTON_DOWN,
    EVMU_GAMEPAD_BUTTON_LEFT,
    EVMU_GAMEPAD_BUTTON_RIGHT,
    EVMU_GAMEPAD_BUTTON_A,
    EVMU_GAMEPAD_BUTTON_B,
    EVMU_GAMEPAD_BUTTON_MODE,
    EVMU_GAMEPAD_BUTTON_SLEEP,
    EVMU_GAMEPAD_BUTTON_COUNT
} EVMU_GAMEPAD_BUTTON;

typedef struct EvmuGamepad {
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
} EvmuGamepad;

GBL_DECLARE_HANDLE(EvmuGamePad);

GBL_DECLARE_ENUM(EVMU_GAME_PAD_PROPERTY) {
    EVMU_GAME_PAD_PROPERTY_BUTTON_UP = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_GAME_PAD_PROPERTY_BUTTON_DOWN,
    EVMU_GAME_PAD_PROPERTY_BUTTON_LEFT,
    EVMU_GAME_PAD_PROPERTY_BUTTON_RIGHT,
    EVMU_GAME_PAD_PROPERTY_BUTTON_A,
    EVMU_GAME_PAD_PROPERTY_BUTTON_B,
    EVMU_GAME_PAD_PROPERTY_BUTTON_MODE,
    EVMU_GAME_PAD_PROPERTY_BUTTON_SLEEP,
    EVMU_GAME_PAD_PROPERTY_BUTTON_COUNT
};


int     gyVmuButtonStateGet(const struct VMUDevice* dev, EVMU_GAMEPAD_BUTTON but);
void    gyVmuButtonStateSet(struct VMUDevice* dev, EVMU_GAMEPAD_BUTTON but, int down);
int     gyVmuGamepadPoll(struct VMUDevice* dev);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_GAMEPAD_H


#ifndef EVMU_GAMEPAD__H
#define EVMU_GAMEPAD__H

#include <evmu/hw/evmu_gamepad.h>
#include <gyro_input_api.h>

#define EVMU_GAMEPAD_(instance)     ((EvmuGamepad_*)GBL_INSTANCE_PRIVATE(instance, EVMU_GAMEPAD_TYPE))
#define EVMU_GAMEPAD_PUBLIC_(priv)  ((EvmuGamepad*)GBL_INSTANCE_PUBLIC(priv, EVMU_GAMEPAD_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_STRUCT(EvmuGamepad_) {
    EvmuMemory_*            pMemory;
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
    GblBool                 kbdAttached;
    GblBool                 contAttached;
};

GBL_DECLS_END

#endif // EVMU_GAMEPAD__H

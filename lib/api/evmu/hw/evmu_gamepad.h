#ifndef EVMU_GAMEPAD_H
#define EVMU_GAMEPAD_H

#include "../types/evmu_peripheral.h"

#define EVMU_GAMEPAD_TYPE               (GBL_TYPEOF(EvmuGamepad))
#define EVMU_GAMEPAD_NAME               "gamepad"

#define EVMU_GAMEPAD(instance)          (GBL_INSTANCE_CAST(instance, EvmuGamepad))
#define EVMU_GAMEPAD_CLASS(klass)       (GBL_CLASS_CAST(klass, EvmuGamepad))
#define EVMU_GAMEPAD_GET_CLASS(instance)(GBL_INSTANCE_GET_CLASS(instance, EvmuGamepad))

#define GBL_SELF_TYPE EvmuGamepad

GBL_DECLS_BEGIN

/* Supposed to only poll for input when latch is set/reset
 * reading from pin should not return latch value
 */
GBL_DECLARE_ENUM(EVMU_GAMEPAD_BUTTON) {
    EVMU_GAMEPAD_BUTTON_UP,
    EVMU_GAMEPAD_BUTTON_DOWN,
    EVMU_GAMEPAD_BUTTON_LEFT,
    EVMU_GAMEPAD_BUTTON_RIGHT,
    EVMU_GAMEPAD_BUTTON_A,
    EVMU_GAMEPAD_BUTTON_B,
    EVMU_GAMEPAD_BUTTON_MODE,
    EVMU_GAMEPAD_BUTTON_SLEEP,
    EVMU_GAMEPAD_BUTTON_STANDARD_COUNT,
    EVMU_GAMEPAD_BUTTON_TURBO_A = EVMU_GAMEPAD_BUTTON_STANDARD_COUNT,
    EVMU_GAMEPAD_BUTTON_TURBO_B,
    EVMU_GAMEPAD_BUTTON_FAST_FORWARD,
    EVMU_GAMEPAD_BUTTON_REWIND,
    EVMU_GAMEPAD_BUTTON_COUNT
};

GBL_CLASS_DERIVE_EMPTY   (EvmuGamepad, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuGamepad, EvmuPeripheral)

GBL_PROPERTIES(EvmuGamepad,
    (up,          GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (down,        GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (left,        GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (right,       GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (a,           GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (b,           GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (mode,        GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (sleep,       GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (turboA,      GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (turboB,      GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (fastForward, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (rewind,      GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE)
)

EVMU_EXPORT GblType     EvmuGamepad_type             (void)                       GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuGamepad_poll             (GBL_SELF)                   GBL_NOEXCEPT;

EVMU_EXPORT void        EvmuGamepad_setButtonPressed (GBL_SELF,
                                                      EVMU_GAMEPAD_BUTTON button,
                                                      GblBool pressed)            GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuGamepad_buttonPressed    (GBL_CSELF,
                                                      EVMU_GAMEPAD_BUTTON button) GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuGamepad_buttonTapped     (GBL_CSELF,
                                                      EVMU_GAMEPAD_BUTTON button) GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuGamepad_buttonReleased   (GBL_CSELF,
                                                      EVMU_GAMEPAD_BUTTON button) GBL_NOEXCEPT;


GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_GAMEPAD_H


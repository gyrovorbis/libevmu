/*! \file
 *  \brief Port 3 button states + back-end polling hooks
 *  \ingroup peripherals
 *
 *  \author Falco Girgis
 */

#ifndef EVMU_GAMEPAD_H
#define EVMU_GAMEPAD_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_GAMEPAD_TYPE               (GBL_TYPEOF(EvmuGamepad))
#define EVMU_GAMEPAD_NAME               "gamepad"

#define EVMU_GAMEPAD(instance)          (GBL_INSTANCE_CAST(instance, EvmuGamepad))
#define EVMU_GAMEPAD_CLASS(klass)       (GBL_CLASS_CAST(klass, EvmuGamepad))
#define EVMU_GAMEPAD_GET_CLASS(instance)(GBL_INSTANCE_GET_CLASS(instance, EvmuGamepad))

#define GBL_SELF_TYPE EvmuGamepad

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuGamepad);

GBL_CLASS_DERIVE(EvmuGamepad, EvmuPeripheral)
    EVMU_RESULT (*pFnPollButtons)(GBL_SELF);
GBL_CLASS_END

GBL_INSTANCE_DERIVE(EvmuGamepad, EvmuPeripheral)
    // physical buttons (input)
    uint16_t up          : 1;
    uint16_t down        : 1;
    uint16_t left        : 1;
    uint16_t right       : 1;
    uint16_t a           : 1;
    uint16_t b           : 1;
    uint16_t mode        : 1;
    uint16_t sleep       : 1;
    // additional virtual buttons (input) (WIP)
    uint16_t turboA      : 1;
    uint16_t turboB      : 1;
    uint16_t fastForward : 1;
    uint16_t slowMotion  : 1;
GBL_INSTANCE_END

GBL_PROPERTIES(EvmuGamepad,
    (configured,  GBL_GENERIC, (READ),        GBL_BOOL_TYPE),
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

GBL_SIGNALS(EvmuGamepad,
    (updatingButtons, (GBL_INSTANCE_TYPE, receiver))
)

EVMU_EXPORT GblType EvmuGamepad_type         (void)      GBL_NOEXCEPT;
EVMU_EXPORT GblBool EvmuGamepad_isConfigured (GBL_CSELF) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_GAMEPAD_H


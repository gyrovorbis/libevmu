/*! \file
 *  \brief EvmuGamepad and Port 3 Button Management
 *  \ingroup peripherals
 *
 *  This file provides an API for accessing and setting
 *  the button states of a Gamepad. This is done via the
 *  EvmuGamepad instance.
 *
 *  To implement a new input back-end, you may simply set
 *  the current state variables based on the source input
 *  device; however, a more sophisticated method would be
 *  to either reimplement EvmuGamepadClass::pFnPollButtons
 *  with this logic, or to connect a closure to the
 *  "updatingButtons" signal, which will synchronize external
 *  polling and internal update logic.
 *
 *  \copyright 2023 Falco Girgis
 */

#ifndef EVMU_GAMEPAD_H
#define EVMU_GAMEPAD_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_GAMEPAD_TYPE            (GBL_TYPEID(EvmuGamepad))            //!< GblType UUID for EvmuGamepad
#define EVMU_GAMEPAD(self)           (GBL_CAST(EvmuGamepad, self))        //!< Cast GblInstance to EvmuGamepad
#define EVMU_GAMEPAD_CLASS(klass)    (GBL_CLASS_CAST(EvmuGamepad, klass)) //!< Cast GblClass to EvmuGamepadClass
#define EVMU_GAMEPAD_GET_CLASS(self) (GBL_CLASSOF(EvmuGamepad, self))     //!< Function-style class accessor
//! @}

#define EVMU_GAMEPAD_NAME               "gamepad"   //!< Gamepad GblObject name

#define GBL_SELF_TYPE EvmuGamepad

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuGamepad);

/*! \struct  EvmuGamepadClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass VTable structure for EvmuGamepad
 *
 *  EvmuGamepadClass is the virtual table/class structure
 *  for the EvmuGamepad peripheral instance.
 *
 *  \sa EvmuGamepad
 */
GBL_CLASS_DERIVE(EvmuGamepad, EvmuPeripheral)
    //! Invoked when time to update input (default fires "updatingButtons" signal)
    EVMU_RESULT (*pFnPollButtons)(GBL_SELF);
GBL_CLASS_END

/*! \struct  EvmuGamepad
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   Gamepad peripheral for managing button states
 *
 *  The EvmuGamepad peripheral provides both an API for accessing
 *  current Port 3 button states and for implementing an input
 *  back-end to drive these button states via virtual override or signal.
 *
 *  \sa EvmuGamepadClass
 */
GBL_INSTANCE_DERIVE(EvmuGamepad, EvmuPeripheral)
    // physical buttons (input)
    uint16_t up          : 1; //!< Dpad Up button state      (1: pressed, 0: released)
    uint16_t down        : 1; //!< Dpad Down button state    (1: pressed, 0: released)
    uint16_t left        : 1; //!< Dpad Left button state    (1: pressed, 0: released)
    uint16_t right       : 1; //!< Dpad Right button state   (1: pressed, 0: released)
    uint16_t a           : 1; //!< A button state            (1: pressed, 0: released)
    uint16_t b           : 1; //!< B button state            (1: pressed, 0: released)
    uint16_t mode        : 1; //!< Mode button state         (1: pressed, 0: released)
    uint16_t sleep       : 1; //!< Sleep button state        (1: pressed, 0: released)
    // additional virtual buttons (input) (WIP)
    uint16_t turboA      : 1; //!< TurboA button state       (1: pressed, 0: released)
    uint16_t turboB      : 1; //!< TurboB button state       (1: pressed, 0: released)
    uint16_t fastForward : 1; //!< Fast-Forward button state (1: pressed, 0: released)
    uint16_t slowMotion  : 1; //!< Rewind button state       (1: preseed, 0: released)
GBL_INSTANCE_END

//!\cond
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
    (slowMotion,  GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE)
)

GBL_SIGNALS(EvmuGamepad,
    (updatingButtons, (GBL_INSTANCE_TYPE, receiver))
)
//!\endcond

//! Returns the GblType UUID associated with EvmuGamepad
EVMU_EXPORT GblType EvmuGamepad_type (void) GBL_NOEXCEPT;

/*! \name Configuration
 *  \brief Method(s) for querying configuration state
 *  \relatesalso EvmuGamepad
 *  @{
 */
//! Returns GBL_TRUE if the port 3 DDR pins have been configured properly for button input
EVMU_EXPORT GblBool EvmuGamepad_isConfigured (GBL_CSELF) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_GAMEPAD_H


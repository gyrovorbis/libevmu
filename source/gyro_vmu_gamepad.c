#include "gyro_vmu_gamepad.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_sfr.h"
#include <assert.h>
#include <gyro_input_api.h>
#include <gyro_system_api.h>

static inline int _turboButtonStateNext(int cur, int prev) {
    switch(cur) {
    case GY_BUT_STATE_TAPPED:
        return GY_BUT_STATE_TAPPED;
        break;
    case GY_BUT_STATE_DOWN:
        return (prev == GY_BUT_STATE_TAPPED)? GY_BUT_STATE_RELEASED : GY_BUT_STATE_TAPPED;
        break;
    case GY_BUT_STATE_RELEASED:
        return (prev == GY_BUT_STATE_RELEASED)? GY_BUT_STATE_UP : GY_BUT_STATE_RELEASED;
        break;
    default:
    case GY_BUT_STATE_UP:
        return GY_BUT_STATE_UP;
        break;
    }
}

int gyVmuGamepadPoll(struct VMUDevice* dev) {
    bool            hasKbd          = gyKeyboardPoll(0, &dev->gamepad.kbd);
#ifndef VMU_CHILD_APP
    gyInputPollEvents();
#endif
    bool            hasCont         = gyControllerPoll(0, &dev->gamepad.cont);
    GYKeyboard*     kbd             = dev->gamepad.kbd;
    GYController*   cont            = dev->gamepad.cont;
    bool            deviceRemoved   = false;
    int             deviceCount     = 0;
    uint8_t u                       = 0;
    uint8_t d                       = 0;
    uint8_t l                       = 0;
    uint8_t r                       = 0;
    uint8_t a                       = 0;
    uint8_t b                       = 0;
    uint8_t ta                      = 0;
    uint8_t tb                      = 0;
    uint8_t m                       = 0;
    uint8_t s                       = 0;
    uint8_t lt                      = 0;
    uint8_t rt                      = 0;

    //Check keyboard state
    if(hasKbd) {
        ++deviceCount;
    } else if(dev->gamepad.kbdAttached) {
        deviceRemoved = true;
    }

    //Check controller state
    if(hasCont) {
        ++deviceCount;
    } else if(dev->gamepad.contAttached) {
        deviceRemoved = true;
    }

#if 1
    //Have to clear gamepad buttons if we just removed the last input device while buttons were still down!
    if(deviceRemoved && !deviceCount) {
        u   = dev->gamepad.u    << 1;
        d   = dev->gamepad.d    << 1;
        l   = dev->gamepad.l    << 1;
        r   = dev->gamepad.r    << 1;
        a   = dev->gamepad.a    << 1;
        b   = dev->gamepad.b    << 1;
        ta  = dev->gamepad.ta   << 1;
        tb  = dev->gamepad.tb   << 1;
        m   = dev->gamepad.m    << 1;
        s   = dev->gamepad.s    << 1;
        lt  = dev->gamepad.lt   << 1;
        rt  = dev->gamepad.rt   << 1;
    } else {

        //Keyboard input
        if(hasKbd && kbd) {
            u   |= kbd->keys[GY_KEY_DPAD_U];
            d   |= kbd->keys[GY_KEY_DPAD_D];
            l   |= kbd->keys[GY_KEY_DPAD_L];
            r   |= kbd->keys[GY_KEY_DPAD_R];
            a   |= kbd->keys[GY_KEY_A];
            b   |= kbd->keys[GY_KEY_B];
            ta  |= kbd->keys[GY_KEY_X];
            tb  |= kbd->keys[GY_KEY_Y];
            m   |= kbd->keys[GY_KEY_M];
            s   |= kbd->keys[GY_KEY_S];
            lt  |= kbd->keys[GY_KEY_L];
            rt  |= kbd->keys[GY_KEY_R];
        }

        //Combine with Controller input
        if(hasCont && cont) {
            //Combine Dpad input with Analog input
            u   |= cont->buttons[GY_CONTROLLER_BUT_DPAD_UP]       | cont->buttons[GY_CONTROLLER_BUT_ANALOG1_UP];
            d   |= cont->buttons[GY_CONTROLLER_BUT_DPAD_DOWN]     | cont->buttons[GY_CONTROLLER_BUT_ANALOG1_DOWN];
            l   |= cont->buttons[GY_CONTROLLER_BUT_DPAD_LEFT]     | cont->buttons[GY_CONTROLLER_BUT_ANALOG1_LEFT];
            r   |= cont->buttons[GY_CONTROLLER_BUT_DPAD_RIGHT]    | cont->buttons[GY_CONTROLLER_BUT_ANALOG1_RIGHT];
            a   |= cont->buttons[GY_CONTROLLER_BUT_CIRCLE];
            b   |= cont->buttons[GY_CONTROLLER_BUT_X];
            ta  |= cont->buttons[GY_CONTROLLER_BUT_TRIANGLE];
            tb  |= cont->buttons[GY_CONTROLLER_BUT_SQUARE];
            m   |= cont->buttons[GY_CONTROLLER_BUT_SELECT];
            s   |= cont->buttons[GY_CONTROLLER_BUT_START];
            lt  |= cont->buttons[GY_CONTROLLER_BUT_LSHOULDER];
            rt  |= cont->buttons[GY_CONTROLLER_BUT_RSHOULDER];
        }

    }

    //Update turbo buttons
    dev->gamepad.turboA = _turboButtonStateNext(ta, dev->gamepad.turboA);
    dev->gamepad.turboB = _turboButtonStateNext(tb, dev->gamepad.turboB);
    a |= dev->gamepad.turboA;
    b |= dev->gamepad.turboB;
#else
    u   = kbd->keys[GY_KEY_DPAD_U];
    d   = kbd->keys[GY_KEY_DPAD_D];
    l   = kbd->keys[GY_KEY_DPAD_L];
    r   = kbd->keys[GY_KEY_DPAD_R];
    a   = kbd->keys[GY_KEY_A];
    b   = kbd->keys[GY_KEY_B];
#endif

    //Update VMU hardware pin states
    if(u == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_UP, 1);
    else if(u == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_UP, 0);
    if(d == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_DOWN, 1);
    else if(d == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_DOWN, 0);
    if(l == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_LEFT, 1);
    else if(l == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_LEFT, 0);
    if(r == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_RIGHT, 1);
    else if(r == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_RIGHT, 0);

    if(a == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_A, 1);
    else if(a == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_A, 0);
    if(b == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_B, 1);
    else if(b == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_B, 0);
    if(s == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_SLEEP, 1);
    else if(s == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_SLEEP, 0);

    if(dev->biosLoaded) {
        if(m == GY_BUT_STATE_TAPPED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_MODE, 1);
        else if(m == GY_BUT_STATE_RELEASED) gyVmuButtonStateSet(dev, VMU_GAMEPAD_MODE, 0);
    }

    //Update internal input state
    dev->gamepad.kbdAttached    = hasKbd;
    dev->gamepad.contAttached   = hasCont;
    dev->gamepad.u              = u;
    dev->gamepad.d              = d;
    dev->gamepad.l              = l;
    dev->gamepad.r              = r;
    dev->gamepad.a              = a;
    dev->gamepad.b              = b;
    dev->gamepad.ta             = ta;
    dev->gamepad.tb             = tb;
    dev->gamepad.m              = m;
    dev->gamepad.s              = s;
    dev->gamepad.lt             = lt;
    dev->gamepad.rt             = rt;

    return deviceCount > 0;
}

int gyVmuButtonStateGet(const struct VMUDevice* dev, VMU_GAMEPAD_BUTTON but) {
    assert(but >= 0 && but < VMU_GAMEPAD_MAX);
    return dev->sfr[SFR_OFFSET(SFR_ADDR_P3)]&(1u<<but);
}

void gyVmuButtonStateSet(struct VMUDevice* dev, VMU_GAMEPAD_BUTTON but, int down) {
    assert(but >= 0 && but < VMU_GAMEPAD_MAX);

    const unsigned char mask = 1u<<but;
    if(down)
        dev->sfr[SFR_OFFSET(SFR_ADDR_P3)] &= ~mask;
    else
        dev->sfr[SFR_OFFSET(SFR_ADDR_P3)] |= mask;

    if(dev->sfr[SFR_OFFSET(SFR_ADDR_P3INT)] & SFR_P3INT_P32INT_MASK) {
        dev->sfr[SFR_OFFSET(SFR_ADDR_P3INT)] |= SFR_P3INT_P31INT_MASK;
    }

}

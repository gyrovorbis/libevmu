#include <evmu/hw/evmu_gamepad.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_device_.h"
#include "evmu_gamepad_.h"
#include "evmu_memory_.h"
#include "../types/evmu_peripheral_.h"
#include <stdbool.h>

EVMU_INLINE uint8_t turboButtonStateNext_(uint8_t cur, uint8_t prev) {
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

EVMU_EXPORT EVMU_RESULT EvmuGamepad_poll(EvmuGamepad* pSelf) {
    GBL_CTX_BEGIN(NULL);
    EvmuGamepad_* pSelf_ = EVMU_GAMEPAD_(pSelf);

    bool            hasKbd          = gyKeyboardPoll(0, &pSelf_->kbd);
#ifndef VMU_CHILD_APP
    gyInputPollEvents();
#endif
    bool            hasCont         = gyControllerPoll(0, &pSelf_->cont);
    GYKeyboard*     kbd             = pSelf_->kbd;
    GYController*   cont            = pSelf_->cont;
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
    } else if(pSelf_->kbdAttached) {
        deviceRemoved = true;
    }

    //Check controller state
    if(hasCont) {
        ++deviceCount;
    } else if(pSelf_->contAttached) {
        deviceRemoved = true;
    }

#if 1
    //Have to clear gamepad buttons if we just removed the last input device while buttons were still down!
    if(deviceRemoved && !deviceCount) {
        u   = pSelf_->u    << 1;
        d   = pSelf_->d    << 1;
        l   = pSelf_->l    << 1;
        r   = pSelf_->r    << 1;
        a   = pSelf_->a    << 1;
        b   = pSelf_->b    << 1;
        ta  = pSelf_->ta   << 1;
        tb  = pSelf_->tb   << 1;
        m   = pSelf_->m    << 1;
        s   = pSelf_->s    << 1;
        lt  = pSelf_->lt   << 1;
        rt  = pSelf_->rt   << 1;
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
    pSelf_->turboA = turboButtonStateNext_(ta, pSelf_->turboA);
    pSelf_->turboB = turboButtonStateNext_(tb, pSelf_->turboB);
    a |= pSelf_->turboA;
    b |= pSelf_->turboB;
#else
    u   = kbd->keys[GY_KEY_DPAD_U];
    d   = kbd->keys[GY_KEY_DPAD_D];
    l   = kbd->keys[GY_KEY_DPAD_L];
    r   = kbd->keys[GY_KEY_DPAD_R];
    a   = kbd->keys[GY_KEY_A];
    b   = kbd->keys[GY_KEY_B];
#endif

    //Update VMU hardware pin states
    if(u == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_UP, GBL_TRUE);
    else if(u == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_UP, GBL_FALSE);
    if(d == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_DOWN, GBL_TRUE);
    else if(d == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_DOWN, GBL_FALSE);
    if(l == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_LEFT, GBL_TRUE);
    else if(l == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_LEFT, GBL_FALSE);
    if(r == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_RIGHT, GBL_TRUE);
    else if(r == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_RIGHT, GBL_FALSE);

    if(a == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_A, GBL_TRUE);
    else if(a == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_A, GBL_FALSE);
    if(b == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_B, GBL_TRUE);
    else if(b == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_B, GBL_FALSE);
    if(s == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_SLEEP, GBL_TRUE);
    else if(s == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_SLEEP, GBL_FALSE);

    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    if(EvmuRom_biosLoaded(pDevice->pRom)) {
        if(m == GY_BUT_STATE_TAPPED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_MODE, GBL_TRUE);
        else if(m == GY_BUT_STATE_RELEASED) EvmuGamepad_setButtonPressed(pSelf, EVMU_GAMEPAD_BUTTON_MODE, GBL_FALSE);
    }

    //Update internal input state
    pSelf_->kbdAttached    = hasKbd;
    pSelf_->contAttached   = hasCont;
    pSelf_->u              = u;
    pSelf_->d              = d;
    pSelf_->l              = l;
    pSelf_->r              = r;
    pSelf_->a              = a;
    pSelf_->b              = b;
    pSelf_->ta             = ta;
    pSelf_->tb             = tb;
    pSelf_->m              = m;
    pSelf_->s              = s;
    pSelf_->lt             = lt;
    pSelf_->rt             = rt;

    GBL_CTX_END();
}

EVMU_EXPORT void EvmuGamepad_setButtonPressed(EvmuGamepad* pSelf, EVMU_GAMEPAD_BUTTON but, GblBool down) {
    EvmuGamepad_* pSelf_ = EVMU_GAMEPAD_(pSelf);
    GBL_ASSERT(but >= 0 && but < EVMU_GAMEPAD_BUTTON_STANDARD_COUNT);

    const uint8_t mask   = 1u << but;
    const EvmuWord p3    = pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)];
    const EvmuWord p3Int = pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)];
    const EvmuWord p3Ddr = pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3DDR)];

    if(down /*&& (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)] & mask
            & ~pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3DDR)])*/) {
        if(pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)] & EVMU_SFR_P3INT_P32INT_MASK)
            pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)] |= EVMU_SFR_P3INT_P31INT_MASK;

        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)] &= ~mask;

        if(pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)] & EVMU_SFR_P3INT_P32INT_MASK) {
            pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PCON)] &= ~EVMU_SFR_PCON_HOLD_MASK;
        }
    } else {
        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)] |= mask;
        pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)] &= ~EVMU_SFR_P3INT_P31INT_MASK;
    }
}

EVMU_INLINE uint8_t buttonState_(EvmuGamepad_* pSelf_, EVMU_GAMEPAD_BUTTON but) {
    switch(but) {
    case EVMU_GAMEPAD_BUTTON_A:            return pSelf_->a;
    case EVMU_GAMEPAD_BUTTON_B:            return pSelf_->b;
    case EVMU_GAMEPAD_BUTTON_MODE:         return pSelf_->m;
    case EVMU_GAMEPAD_BUTTON_SLEEP:        return pSelf_->s;
    case EVMU_GAMEPAD_BUTTON_UP:           return pSelf_->u;
    case EVMU_GAMEPAD_BUTTON_DOWN:         return pSelf_->d;
    case EVMU_GAMEPAD_BUTTON_LEFT:         return pSelf_->l;
    case EVMU_GAMEPAD_BUTTON_RIGHT:        return pSelf_->r;
    case EVMU_GAMEPAD_BUTTON_FAST_FORWARD: return pSelf_->rt;
    case EVMU_GAMEPAD_BUTTON_REWIND:       return pSelf_->lt;
    case EVMU_GAMEPAD_BUTTON_TURBO_A:      return pSelf_->ta;
    case EVMU_GAMEPAD_BUTTON_TURBO_B:      return pSelf_->tb;
    default: GBL_ASSERT(GBL_FALSE);        return 0;
    }

}

EVMU_EXPORT GblBool EvmuGamepad_buttonPressed(const EvmuGamepad* pSelf, EVMU_GAMEPAD_BUTTON but) {
#if 0
    EvmuGamepad_* pSelf_ = EVMU_GAMEPAD_(pSelf);
    GBL_ASSERT(but >= 0 && but < EVMU_GAMEPAD_BUTTON_STANDARD_COUNT);
    return pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)] & (1u<<but);
#else
    return buttonState_(EVMU_GAMEPAD_(pSelf), but)? GBL_TRUE : GBL_FALSE;
#endif
}

EVMU_EXPORT GblBool EvmuGamepad_buttonTapped(const EvmuGamepad* pSelf, EVMU_GAMEPAD_BUTTON but) {
    return buttonState_(EVMU_GAMEPAD_(pSelf), but) == GY_BUT_STATE_TAPPED;
}

EVMU_EXPORT GblBool EvmuGamepad_buttonReleased(const EvmuGamepad* pSelf, EVMU_GAMEPAD_BUTTON but) {
    return buttonState_(EVMU_GAMEPAD_(pSelf), but) == GY_BUT_STATE_RELEASED;
}

static GBL_RESULT EvmuGamepad_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_GAMEPAD_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuGamepadClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_OBJECT_CLASS(pClass)->pFnConstructed = EvmuGamepad_GblObject_constructed_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuGamepad_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuGamepadClass),
        .pFnClassInit           = EvmuGamepadClass_init_,
        .instanceSize           = sizeof(EvmuGamepad),
        .instancePrivateSize    = sizeof(EvmuGamepad_)
    };

    if(!GblType_verify(type)) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuGamepad"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}


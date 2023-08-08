#include <evmu/hw/evmu_gamepad.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_device_.h"
#include "evmu_gamepad_.h"
#include "evmu_ram_.h"
#include "../types/evmu_peripheral_.h"
#include <gimbal/meta/signals/gimbal_marshal.h>


EVMU_EXPORT GblBool EvmuGamepad_isConfigured(const EvmuGamepad* pSelf) {
    return ~EVMU_GAMEPAD_(pSelf)->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3DDR)];
}

EvmuWord EvmuGamepad__port3Value_(const EvmuGamepad_* pSelf_) {
    EvmuGamepad* pSelf = EVMU_GAMEPAD_PUBLIC_(pSelf_);
    // P3DDR | P3Latch | ~P3Port
    return pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3DDR)] |
           (uint8_t)~pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)] |
           ((!pSelf->up    << EVMU_SFR_P3_UP_POS)    |
            (!pSelf->down  << EVMU_SFR_P3_DOWN_POS)  |
            (!pSelf->left  << EVMU_SFR_P3_LEFT_POS)  |
            (!pSelf->right << EVMU_SFR_P3_RIGHT_POS) |
            (!pSelf->a     << EVMU_SFR_P3_A_POS)     |
            (!pSelf->b     << EVMU_SFR_P3_B_POS)     |
            (!pSelf->mode  << EVMU_SFR_P3_MODE_POS)  |
            (!pSelf->sleep << EVMU_SFR_P3_SLEEP_POS));
}

static EVMU_RESULT EvmuGamepad_pollButtons_(EvmuGamepad* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EvmuGamepad_* pSelf_ = EVMU_GAMEPAD_(pSelf);
    const EvmuWord p3    = EvmuGamepad__port3Value_(pSelf_);
    const EvmuWord p3Int = pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)];

    // Check if any buttons are active
    if((uint8_t)~p3) {
        // Check whether interrupt generation is enabled
        if(p3Int & EVMU_SFR_P3INT_P32INT_MASK) {
            // Set interrupt source to P3
            pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3INT)] |= EVMU_SFR_P3INT_P31INT_MASK;
            // Break out of HOLD mode
            pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PCON)] &= ~EVMU_SFR_PCON_HOLD_MASK;
            // Check if interrupt should be handled
            if(p3Int & EVMU_SFR_P3INT_P30INT_MASK) {
                // Submit IRQ to PIC
                EvmuPic_raiseIrq(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf))->pPic, EVMU_IRQ_P3);
            }
        }
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuGamepad_EvmuIBehavior_update_(EvmuIBehavior* pIBehav, EvmuTicks ticks) {
    GBL_UNUSED(ticks);
    GBL_CTX_BEGIN(NULL);

    EvmuGamepad* pSelf = EVMU_GAMEPAD(pIBehav);

    // Only update button states if P3DDR has any pins configured as input
    if(EvmuGamepad_isConfigured(pSelf)) {
        // Fire signal to any attached slots which are implementing input back-ends
        GBL_CTX_VERIFY_CALL(GblSignal_emit(GBL_INSTANCE(pSelf), "updatingButtons"));
        // Call virtual method for subclass to process state
        GBL_VCALL(EvmuGamepad, pFnPollButtons, pSelf);
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuGamepad_EvmuIBehavior_reset_(EvmuIBehavior* pIBehav) {
    GBL_CTX_BEGIN(NULL);

    EvmuGamepad* pSelf = EVMU_GAMEPAD(pIBehav);

    pSelf->up          = 0;
    pSelf->down        = 0;
    pSelf->left        = 0;
    pSelf->right       = 0;
    pSelf->a           = 0;
    pSelf->b           = 0;
    pSelf->mode        = 0;
    pSelf->sleep       = 0;
    pSelf->turboA      = 0;
    pSelf->turboB      = 0;
    pSelf->fastForward = 0;
    pSelf->slowMotion  = 0;

    GBL_CTX_END();
}

static GBL_RESULT EvmuGamepad_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_GAMEPAD_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuGamepadClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);

    GBL_CTX_BEGIN(NULL);

    if(!GblType_classRefCount(GBL_CLASS_TYPEOF(pClass))) {
        GBL_PROPERTIES_REGISTER(EvmuGamepad);

        GblSignal_install(EVMU_GAMEPAD_TYPE,
                          "updatingButtons",
                          GblMarshal_CClosure_VOID__INSTANCE,
                          0);
    }

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuGamepad_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuGamepad_EvmuIBehavior_update_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuGamepad_EvmuIBehavior_reset_;
    EVMU_GAMEPAD_CLASS(pClass)  ->pFnPollButtons = EvmuGamepad_pollButtons_;

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
        type = GblType_register(GblQuark_internStatic("EvmuGamepad"),
                                EVMU_PERIPHERAL_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}

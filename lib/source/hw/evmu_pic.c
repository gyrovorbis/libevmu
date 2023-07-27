#include <evmu/hw/evmu_address_space.h>
#include "evmu_pic_.h"
#include "evmu_ram_.h"
#include "evmu_device_.h"
#include "../types/evmu_peripheral_.h"

const static EvmuAddress isrAddrLut_[EVMU_IRQ_COUNT] = {
    EVMU_ISR_ADDR_RESET,
    EVMU_ISR_ADDR_EXT_INT0,
    EVMU_ISR_ADDR_EXT_INT1,
    EVMU_ISR_ADDR_EXT_INT2_T0L,
    EVMU_ISR_ADDR_EXT_INT3_TBASE,
    EVMU_ISR_ADDR_T0H,
    EVMU_ISR_ADDR_T1,
    EVMU_ISR_ADDR_SIO0,
    EVMU_ISR_ADDR_SIO1,
    EVMU_ISR_ADDR_RFB,
    EVMU_ISR_ADDR_P3,
    EVMU_ISR_ADDR_11,
    EVMU_ISR_ADDR_12,
    EVMU_ISR_ADDR_13,
    EVMU_ISR_ADDR_14,
    EVMU_ISR_ADDR_15
};

EVMU_EXPORT EvmuAddress EvmuPic_isrAddress(EVMU_IRQ irq) {
    GBL_ASSERT(irq < EVMU_IRQ_COUNT);
    return isrAddrLut_[irq];
}

EVMU_EXPORT void EvmuPic_raiseIrq(EvmuPic* pSelf, EVMU_IRQ irq) {
    EvmuPic_* pSelf_ = EVMU_PIC_(pSelf);
    pSelf_->intReq |= (1u << irq);
}

EVMU_EXPORT size_t EvmuPic_irqsActiveDepth(const EvmuPic* pSelf) {
    EvmuPic_* pSelf_ = EVMU_PIC_(pSelf);
    int depth = 0;
    for(int p = EVMU_IRQ_PRIORITY_HIGHEST; p >= EVMU_IRQ_PRIORITY_LOW; --p) {
        if(pSelf_->intStack[p]) {
            ++depth;
        }
    }
    return depth;
}

EVMU_EXPORT EVMU_IRQ_PRIORITY EvmuPic_irqPriority(const EvmuPic* pSelf, EVMU_IRQ irq) {
    for(int p = EVMU_IRQ_PRIORITY_HIGHEST; p >= EVMU_IRQ_PRIORITY_LOW; --p) {
        uint16_t priorityMask = EvmuPic_irqsEnabledByPriority(pSelf, (EVMU_IRQ_PRIORITY)p);
        if(priorityMask & irq) {
            return (EVMU_IRQ_PRIORITY)p;
        }
    }
    return EVMU_IRQ_PRIORITY_NONE;
}

EVMU_EXPORT EvmuIrqMask EvmuPic_irqsEnabledByPriority(const EvmuPic* pSelf, EVMU_IRQ_PRIORITY priority) {
#define INT_MASK_FROM_IP_BIT_(bit, priority, interrupt) \
        ((((pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IP)] & bit##_MASK) >> bit##_POS) ^ !(priority))) << interrupt;

    EvmuPic_*    pSelf_  = EVMU_PIC_(pSelf);
    EvmuRam_* pRam = pSelf_->pRam;

    uint16_t mask = 0;

    if(priority == EVMU_IRQ_PRIORITY_HIGHEST) {
        if(!(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] & EVMU_SFR_IE_IE0_MASK)) {
            mask |= 1 << EVMU_IRQ_EXT_INT0;
        }
        if(!(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] & EVMU_SFR_IE_IE1_MASK) &&
                !(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] & EVMU_SFR_IE_IE0_MASK)) {
            mask |= 1 << EVMU_IRQ_EXT_INT1;
        }
    } else {
        //Only check for high and low priority interrupts if bit IE7 of the Interrupt Enable SFR is set (unmasking them)
        if(priority == EVMU_IRQ_PRIORITY_HIGH ||
                (pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] & EVMU_SFR_IE_IE7_MASK)) {
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_P3,   priority, EVMU_IRQ_P3);
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_SIO1, priority, EVMU_IRQ_SIO1);
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_SIO0, priority, EVMU_IRQ_SIO0);
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_T1,   priority, EVMU_IRQ_T1);
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_T0H,  priority, EVMU_IRQ_T0H);
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_INT3, priority, EVMU_IRQ_EXT_INT3_TBASE);
            mask |= INT_MASK_FROM_IP_BIT_(EVMU_SFR_IP_INT2, priority, EVMU_IRQ_EXT_INT2_T0L);

            if(priority == EVMU_IRQ_PRIORITY_LOW) {
                //Both interrupts(IE0+IE1) are set to low if IE0 is set to low.
                if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] & EVMU_SFR_IE_IE0_MASK) {
                    mask |= 1 << EVMU_IRQ_EXT_INT0;
                    mask |= 1 << EVMU_IRQ_EXT_INT1;
                }

                if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] & EVMU_SFR_IE_IE1_MASK) {
                    mask |= 1 << EVMU_IRQ_EXT_INT1;
                }
            }
        }
    }
    return mask;
#undef INT_MASK_FROM_PRIORITY_BIT_
}

EVMU_EXPORT EvmuIrqMask EvmuPic_irqsActive(const EvmuPic* pSelf) {
    EvmuPic_* pSelf_ = EVMU_PIC_(pSelf);
    EvmuIrqMask activeMask = 0;
    for(int p = EVMU_IRQ_PRIORITY_HIGHEST; p >= EVMU_IRQ_PRIORITY_LOW; --p) {
        activeMask |= pSelf_->intStack[p];
    }
    return activeMask;
}

GblBool EvmuPic__retiInstruction(EvmuPic_* pSelf_) {
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(EVMU_PIC_PUBLIC_(pSelf_)));
    EvmuRam* pRam = EVMU_RAM_PUBLIC_(pSelf_->pRam);

    EvmuAddress r = EvmuRam_popStack(pRam) << 8u;
    r |= EvmuRam_popStack(pRam);
    EvmuCpu_setPc(pDevice->pCpu, r);
    pSelf_->processThisInstr = 0;
    for(int p = EVMU_IRQ_PRIORITY_HIGHEST; p >= EVMU_IRQ_PRIORITY_LOW; --p) {
        if(pSelf_->intStack[p]) {
            pSelf_->prevIntPriority = p;
            pSelf_->intStack[p] = 0;
            return 1;
        }
    }
    return 0;
}


static int EvmuPic__checkInterrupt_(EvmuPic_* pSelf_, EVMU_IRQ_PRIORITY p) {
    EvmuRam_* pRam_ = pSelf_->pRam;
    EvmuRam*  pRam  = EVMU_RAM_PUBLIC_(pRam_);
    EvmuDevice*  pDevice  = EvmuPeripheral_device(EVMU_PERIPHERAL(EVMU_PIC_PUBLIC_(pSelf_)));

    uint16_t priorityMask = EvmuPic_irqsEnabledByPriority(EVMU_PIC_PUBLIC_(pSelf_), (EVMU_IRQ_PRIORITY)p);

    for(uint16_t i = 0; i < EVMU_IRQ_COUNT; ++i) {
        uint16_t interrupt = (1 << i);
        // Check if request should be accepted
        if(priorityMask & interrupt & pSelf_->intReq) {
            pSelf_->intReq &= ~interrupt;          //clear request
            pSelf_->intStack[p] = interrupt;
            EvmuRam_pushStack(pRam,  EvmuCpu_pc(pDevice->pCpu) & 0xff);
            EvmuRam_pushStack(pRam, (EvmuCpu_pc(pDevice->pCpu) & 0xff00) >> 8);   //push return address
            EvmuRam_writeData(pRam,
                                EVMU_ADDRESS_SFR_PCON,
                                EvmuRam_readData(pRam,
                                                   EVMU_ADDRESS_SFR_PCON) & ~EVMU_SFR_PCON_HALT_MASK);
            EvmuCpu_setPc(pDevice->pCpu, EvmuPic_isrAddress((EVMU_IRQ)i));   //jump to ISR address
            return 1;
        }

    }
    return 0;
}


EVMU_EXPORT GblBool EvmuPic_update(EvmuPic* pSelf) {
    EvmuPic_*    pSelf_  = EVMU_PIC_(pSelf);
    EvmuRam_* pRam = pSelf_->pRam;

    if(!pSelf_->processThisInstr) {
        pSelf_->processThisInstr = GBL_TRUE;
        return GBL_FALSE;
    }

    if(!EvmuPic_irqsActive(pSelf)) {
        for(int p = pSelf_->prevIntPriority - 1; p >= EVMU_IRQ_PRIORITY_LOW; --p) {
            if(EvmuPic__checkInterrupt_(pSelf_, (EVMU_IRQ_PRIORITY)p)) return GBL_TRUE;
        }

        for(int p = EVMU_IRQ_PRIORITY_HIGH; p >= pSelf_->prevIntPriority; --p) {
            if(EvmuPic__checkInterrupt_(pSelf_, (EVMU_IRQ_PRIORITY)p)) return GBL_TRUE;
        }
    } else {
        for(int p = EVMU_IRQ_PRIORITY_HIGHEST; p >= EVMU_IRQ_PRIORITY_LOW; --p) {
            if(pSelf_->intStack[p]) {
                break;
            }
            if(EvmuPic__checkInterrupt_(pSelf_, (EVMU_IRQ_PRIORITY)p)) return GBL_TRUE;

        }
    }
    return GBL_FALSE;
}

static GBL_RESULT EvmuPic_IBehavior_update_(EvmuIBehavior* pIBehavior, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pIBehavior, ticks);

    EvmuPic* pSelf = EVMU_PIC(pIBehavior);
    EvmuPic_* pSelf_ = EVMU_PIC_(pSelf);

    GBL_UNUSED(pSelf_);

    GBL_CTX_END();
}

static GBL_RESULT EvmuPic_IBehavior_reset_(EvmuIBehavior* pIBehavior) {
    GBL_CTX_BEGIN(NULL);

    EvmuPic* pSelf   = EVMU_PIC(pIBehavior);
    EvmuPic_* pSelf_ = EVMU_PIC_(pSelf);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pIBehavior);

    EvmuRam_* pMem = pSelf_->pRam;
    memset(pSelf_, 0, sizeof(EvmuPic_));
    pSelf_->processThisInstr = 1;
    pSelf_->pRam = pMem;

    GBL_CTX_END();
}

static GBL_RESULT EvmuPic_GblObject_constructed_(GblObject* pObject) {
    GBL_CTX_BEGIN(NULL);

    EvmuPic* pSelf   = EVMU_PIC(pObject);
    EvmuPic_* pSelf_ = EVMU_PIC_(pSelf);

    GBL_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pObject);
    GblObject_setName(pObject, EVMU_PIC_NAME);

    pSelf_->processThisInstr = 1;

    GBL_CTX_END();
}

static GBL_RESULT EvmuPicClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuPic_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuPic_IBehavior_reset_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuPic_IBehavior_update_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuPic_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuPicClass),
        .pFnClassInit           = EvmuPicClass_init_,
        .instanceSize           = sizeof(EvmuPic),
        .instancePrivateSize    = sizeof(EvmuPic_)
    };

    if(!GblType_verify(type)) {
        type = GblType_register(GblQuark_internStringStatic("EvmuPic"),
                                EVMU_PERIPHERAL_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}




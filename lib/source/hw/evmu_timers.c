#include "evmu_timers_.h"
#include "evmu_memory_.h"
#include <gyro_vmu_cpu.h>
#include "evmu_device_.h"
#include <gyro_vmu_device.h>

static void EvmuTimers_updateBaseTimer_(EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_  = EVMU_TIMERS_(pSelf);
    EvmuMemory_* pMemory = pSelf_->pMemory;
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    VMUDevice*   dev     = EVMU_DEVICE_REEST(pDevice);

    EvmuWord btcr = EvmuMemory_readInt(pDevice->pMemory, EVMU_ADDRESS_SFR_BTCR);

    if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] & EVMU_SFR_BTCR_OP_CTRL_MASK) {
#if 1
        //hard-coded to generate interrupt every 0.5s by VMU
        const double tCyc = EvmuCpu_secsPerInstruction(pDevice->pCpu);

        pSelf_->baseTimer.tBaseDeltaTime += tCyc;
        pSelf_->baseTimer.tBase1DeltaTime += tCyc;
        if(pSelf_->baseTimer.tBase1DeltaTime >= 0.1f) { //call this many cycles 0.1s...
            pSelf_->baseTimer.tBase1DeltaTime -= 0.1f;
            pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] |= EVMU_SFR_BTCR_INT1_SRC_MASK;
            if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] & EVMU_SFR_BTCR_INT1_REQ_EN_MASK)
                EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_EXT_INT3_TBASE);
        }

        if(pSelf_->baseTimer.tBaseDeltaTime >= 0.5f) { //call this many cycles 0.5s...
            pSelf_->baseTimer.tBaseDeltaTime -= 0.5f;
            pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] |= EVMU_SFR_BTCR_INT0_SRC_MASK;
            if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] & EVMU_SFR_BTCR_INT0_REQ_EN_MASK)
                EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_EXT_INT3_TBASE);
        }
#else
     const EvmuCycles cycles =  EvmuCpu_cyclesPerInstruction(pDevice->pCpu);

     if(btcr & EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK)
         pSelf_->baseTimer.th += cycles;
     else if(pSelf_->baseTimer.tl & 0x100)
         pSelf_->baseTimer.th += cycles;


#endif
    }
}

static void EvmuTimers_updateTimer0_(EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_ = EVMU_TIMERS_(pSelf);
    EvmuMemory_* pMemory = pSelf_->pMemory;
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    VMUDevice*   dev = EVMU_DEVICE_REEST(pDevice);

    int cy = EvmuCpu_cyclesPerInstruction(pDevice->pCpu);

    /* Timer 0 */
    //T0H overflow or interrupts enabled
       // if(sfr[0x10] & 0xc0) {
    if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&(EVMU_SFR_T0CNT_P0HRUN_MASK|EVMU_SFR_T0CNT_P0LRUN_MASK)) {
        int c0=0;

        //find out how many times greater t0base is than t0scale
        if((pSelf_->timer0.tbase += cy) >= pSelf_->timer0.tscale)
            do c0++;
            while((pSelf_->timer0.tbase -= pSelf_->timer0.tscale) >= pSelf_->timer0.tscale);

    //c0*= 4;
            //only update if t0base > t0scale
        if(c0)  {
            //16-bit counter and both T0L and T0H are in run state
            if((pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&(EVMU_SFR_T0CNT_P0LONG_MASK|EVMU_SFR_T0CNT_P0LRUN_MASK|EVMU_SFR_T0CNT_P0HRUN_MASK))
                    == (EVMU_SFR_T0CNT_P0LONG_MASK|EVMU_SFR_T0CNT_P0LRUN_MASK|EVMU_SFR_T0CNT_P0HRUN_MASK))
            {
                pSelf_->timer0.base.tl += c0;
                if(pSelf_->timer0.base.tl >= 256) {
                    pSelf_->timer0.base.tl -= 256;
                    if(++pSelf_->timer0.base.th >= 256) {
                        pSelf_->timer0.base.th -= 256;
                        if((pSelf_->timer0.base.tl += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)] )>= 256) {
                            pSelf_->timer0.base.tl -= 256;
                            if((pSelf_->timer0.base.th += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)]) >= 256) {
                                pSelf_->timer0.base.tl = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)];
                                pSelf_->timer0.base.th = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)];
                            }
                        }
                        //set overflow flags for both T0L and T0H
                        pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] |= EVMU_SFR_T0CNT_P0HOVF_MASK|EVMU_SFR_T0CNT_T0LOVF_MASK;
                        //if T0H interrupts are enabled
                        if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_T0HIE_MASK)
                            EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_T0H);
                    }
                }

            } else {
                //Update T0L as 8-bit
                if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] & EVMU_SFR_T0CNT_P0LRUN_MASK) {
                    pSelf_->timer0.base.tl += c0;
                    if(pSelf_->timer0.base.tl >= 256) {
                        pSelf_->timer0.base.tl -= 256;
                        if((pSelf_->timer0.base.tl += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)]) >= 256)
                            pSelf_->timer0.base.tl = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)];
                        pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] |= EVMU_SFR_T0CNT_T0LOVF_MASK;
                        if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_T0LIE_MASK)
                            EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_EXT_INT2_T0L);
                    }
                }

                //Update T0H as 8-bit
                if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] & EVMU_SFR_T0CNT_P0HRUN_MASK) {
                    pSelf_->timer0.base.th += c0;
                    if(pSelf_->timer0.base.th >= 256) {
                        pSelf_->timer0.base.th -= 256;
                        if((pSelf_->timer0.base.th += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)]) >= 256)
                            pSelf_->timer0.base.th = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)];
                        pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] |= EVMU_SFR_T0CNT_P0HOVF_MASK;
                        if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_T0HIE_MASK)
                            EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_T0H);
                    }
                }
            }
        }
    }
}

static void EvmuTimers_updateTimer1_(EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_ = EVMU_TIMERS_(pSelf);
    EvmuMemory_* pMemory = pSelf_->pMemory;
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    VMUDevice*   dev = EVMU_DEVICE_REEST(pDevice);

    int cy = EvmuCpu_cyclesPerInstruction(pDevice->pCpu);

    //Interrupts enabled for T1H or overflow on T1H
    if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & (EVMU_SFR_T1CNT_T1HRUN_MASK|EVMU_SFR_T1CNT_T1LRUN_MASK)) {

        //Both T1H and T1L running, T1 set to 16-bit mode
        if((pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & (EVMU_SFR_T1CNT_T1LONG_MASK|EVMU_SFR_T1CNT_T1HRUN_MASK|EVMU_SFR_T1CNT_T1LRUN_MASK)) ==
                (EVMU_SFR_T1CNT_T1LONG_MASK|EVMU_SFR_T1CNT_T1HRUN_MASK|EVMU_SFR_T1CNT_T1LRUN_MASK))
        {
            pSelf_->timer1.base.tl += cy;
            if(pSelf_->timer1.base.tl >= 256) {
                pSelf_->timer1.base.tl -= 256;
                if(++pSelf_->timer1.base.th >= 256) {
                    pSelf_->timer1.base.th -= 256;
                    if((pSelf_->timer1.base.tl += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]) >= 256) {
                        pSelf_->timer1.base.tl -= 256;
                        if((pSelf_->timer1.base.th += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)]) >= 256) {
                            pSelf_->timer1.base.tl = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)];
                            pSelf_->timer1.base.th = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)];
                        }
                    }
                    pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= (EVMU_SFR_T1CNT_T1HOVF_MASK|EVMU_SFR_T1CNT_T1LONG_MASK);
                    if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1HIE_MASK)
                        EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_T1);
                }
            }
        } else {
            //If T1L is running as 8-bit timer
            if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LRUN_MASK) {
                pSelf_->timer1.base.tl += cy;
                if(pSelf_->timer1.base.tl >= 256) {
                    pSelf_->timer1.base.tl -= 256;
                    if((pSelf_->timer1.base.tl += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]) >= 256)
                        pSelf_->timer1.base.tl = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)];
                    pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= EVMU_SFR_T1CNT_T1LOVF_MASK;
                    if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LIE_MASK)
                        EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_T1);
                }
            }
            //If T1H is running as 8-bit timer
            if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1HRUN_MASK) {
                pSelf_->timer1.base.th += cy;
                if(pSelf_->timer1.base.th >= 256) {
                    pSelf_->timer1.base.th -= 256;
                    if((pSelf_->timer1.base.th += pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)]) >= 256)
                        pSelf_->timer1.base.th = pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)];
                    pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= EVMU_SFR_T1CNT_T1HOVF_MASK;
                    if(pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1HIE_MASK)
                        EvmuPic_raiseIrq(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pPic, EVMU_IRQ_T1);
                }
            }
        }
    }
}

EVMU_EXPORT void EvmuTimers_update(EvmuTimers* pSelf) {
    EvmuTimers_updateBaseTimer_(pSelf);
    EvmuTimers_updateTimer0_(pSelf);
    EvmuTimers_updateTimer1_(pSelf);
}

EVMU_EXPORT EVMU_TIMER1_MODE EvmuTimers_timer1Mode(const EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_ = EVMU_TIMERS_(pSelf);
    return (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)]
            &(EVMU_SFR_T1CNT_T1LONG_MASK|EVMU_SFR_T1CNT_ELDT1C_MASK))>>EVMU_SFR_T1CNT_ELDT1C_POS;
}

static GBL_RESULT EvmuTimers_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_TIMERS_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuTimers_IBehavior_update_(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);

    EvmuTimers*  pTimers   = EVMU_TIMERS(pSelf);
    EvmuTimers_* pTimers_  = EVMU_TIMERS_(pTimers);
    GBL_UNUSED(pTimers_);

    GBL_CTX_END();
}

static GBL_RESULT EvmuTimers_IBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    EvmuTimers*  pTimers   = EVMU_TIMERS(pSelf);
    EvmuTimers_* pTimers_  = EVMU_TIMERS_(pTimers);

    memset(&pTimers_->baseTimer, 0, sizeof(EvmuBaseTimer));
    memset(&pTimers_->timer0, 0, sizeof(EvmuTimer0));
    memset(&pTimers_->timer1, 0, sizeof(EvmuTimer1));

    //pTimers_->timer0.tscale = 256;

    GBL_CTX_END();
}

static GBL_RESULT EvmuTimersClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuTimers_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuTimers_IBehavior_update_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuTimers_IBehavior_reset_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuTimers_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuTimersClass),
        .pFnClassInit           = EvmuTimersClass_init_,
        .instanceSize           = sizeof(EvmuTimers),
        .instancePrivateSize    = sizeof(EvmuTimers_)
    };

    if(!GblType_verify(type)) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuTimers"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}




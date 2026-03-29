#include "evmu_timers_.h"
#include "evmu_ram_.h"
#include "evmu_device_.h"
#include "evmu_buzzer_.h"

static unsigned advanceReloadCounter(unsigned* pCounter,
                                      unsigned  ticks,
                                      unsigned  modulus,
                                      unsigned  reload)
{
    const uint64_t start = *pCounter % modulus;

    if(!ticks) {
        *pCounter = (unsigned)start;
        return 0;
    }

    const uint64_t ticksToOverflow = modulus - start;
    if(ticks < ticksToOverflow) {
        *pCounter = (unsigned)(start + ticks);
        return 0;
    }

    ticks -= (unsigned)ticksToOverflow;

    const uint64_t period = modulus - reload;
    const uint64_t overflowCount = 1u + (ticks / period);
    const uint64_t remainder = ticks % period;

    *pCounter = (unsigned)(reload + remainder);

    return (unsigned)overflowCount;
}

static unsigned advanceReloadCounter8(int* pCounter,
                                       unsigned ticks,
                                       EvmuWord reload)
{
    unsigned value = (unsigned)(*pCounter & 0xff);
    const unsigned overflowCount =
        advanceReloadCounter(&value, ticks, 0x100u, reload);

    *pCounter = (int)value;

    return overflowCount;
}

static unsigned advanceReloadCounter16(EvmuTimer* pTimer,
                                        unsigned   ticks,
                                        EvmuWord   reloadLow,
                                        EvmuWord   reloadHigh)
{
    unsigned value =
        ((unsigned)(pTimer->th & 0xff) << 8u) |
         (unsigned)(pTimer->tl & 0xff);
    const unsigned reload =
        ((unsigned)reloadHigh << 8u) | (unsigned)reloadLow;
    const unsigned overflowCount =
        advanceReloadCounter(&value, ticks, 0x10000u, reload);

    pTimer->tl = (int)(value & 0xff);
    pTimer->th = (int)((value >> 8u) & 0xff);

    return overflowCount;
}

static unsigned consumeStartDelay_(unsigned* pDelay, unsigned ticks) {
    const unsigned skipped = (*pDelay < ticks)? *pDelay : ticks;
    *pDelay -= skipped;
    return ticks - skipped;
}

static unsigned advanceChainedReloadCounter16_(EvmuTimer* pTimer,
                                                unsigned   ticks,
                                                EvmuWord   reloadLow,
                                                EvmuWord   reloadHigh,
                                                unsigned*  pLowOverflowCount)
{
    const unsigned lowOverflowCount =
        advanceReloadCounter8(&pTimer->tl, ticks, reloadLow);
    const unsigned highOverflowCount =
        advanceReloadCounter8(&pTimer->th, lowOverflowCount, reloadHigh);

    if(pLowOverflowCount)
        *pLowOverflowCount = lowOverflowCount;

    return highOverflowCount;
}

static void EvmuTimers_updateBaseTimer_(EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_  = EVMU_TIMERS_(pSelf);
    EvmuRam_*    pRam    = pSelf_->pRam;
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));

    EvmuWord btcr = EvmuRam_readData(pDevice->pRam, EVMU_ADDRESS_SFR_BTCR);

    if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] & EVMU_SFR_BTCR_OP_CTRL_MASK) {
#if 1
        //hard-coded to generate interrupt every 0.5s by VMU
        const double tCyc = EvmuCpu_secs(pDevice->pCpu);

        pSelf_->baseTimer.tBaseDeltaTime += tCyc;
        pSelf_->baseTimer.tBase1DeltaTime += tCyc;
        if(pSelf_->baseTimer.tBase1DeltaTime >= 0.1f) { //call this many cycles 0.1s...
            pSelf_->baseTimer.tBase1DeltaTime -= 0.1f;
            pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] |= EVMU_SFR_BTCR_INT1_SRC_MASK;
            if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] & EVMU_SFR_BTCR_INT1_REQ_EN_MASK)
                EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_EXT_INT3_TBASE);
        }

        if(pSelf_->baseTimer.tBaseDeltaTime >= 0.5f) { //call this many cycles 0.5s...
            pSelf_->baseTimer.tBaseDeltaTime -= 0.5f;
            pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] |= EVMU_SFR_BTCR_INT0_SRC_MASK;
            if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] & EVMU_SFR_BTCR_INT0_REQ_EN_MASK)
                EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_EXT_INT3_TBASE);
        }
#else
     const EvmuCycles cycles =  EvmuCpu_cycles(pDevice->pCpu);

     if(btcr & EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK)
         pSelf_->baseTimer.th += cycles;
     else if(pSelf_->baseTimer.tl & 0x100)
         pSelf_->baseTimer.th += cycles;


#endif
    }
}

static void EvmuTimers_updateTimer0_(EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_  = EVMU_TIMERS_(pSelf);
    EvmuRam_* pRam = pSelf_->pRam;
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));

    int cy = EvmuCpu_cycles(pDevice->pCpu);

    /* Timer 0 */
    //T0H overflow or interrupts enabled
       // if(sfr[0x10] & 0xc0) {
    if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&(EVMU_SFR_T0CNT_P0HRUN_MASK|EVMU_SFR_T0CNT_P0LRUN_MASK)) {
        int c0=0;
        const unsigned gatedCycles =
            consumeStartDelay_(&pSelf_->timer0.startDelayCycles,
                               (unsigned)cy);

        //find out how many times greater t0base is than t0scale
        if((pSelf_->timer0.tbase += (int)gatedCycles) >= pSelf_->timer0.tscale)
            do c0++;
            while((pSelf_->timer0.tbase -= pSelf_->timer0.tscale) >= pSelf_->timer0.tscale);

    //c0*= 4;
            //only update if t0base > t0scale
        if(c0)  {
            //16-bit counter and both T0L and T0H are in run state
            if((pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&(EVMU_SFR_T0CNT_P0LONG_MASK|EVMU_SFR_T0CNT_P0LRUN_MASK|EVMU_SFR_T0CNT_P0HRUN_MASK))
                    == (EVMU_SFR_T0CNT_P0LONG_MASK|EVMU_SFR_T0CNT_P0LRUN_MASK|EVMU_SFR_T0CNT_P0HRUN_MASK))
            {
                if(advanceReloadCounter16(&pSelf_->timer0.base,
                                           (unsigned)c0,
                                           pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)],
                                           pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)]))
                {
                    //set overflow flags for both T0L and T0H
                    pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] |= EVMU_SFR_T0CNT_P0HOVF_MASK|EVMU_SFR_T0CNT_T0LOVF_MASK;
                    //if T0H interrupts are enabled
                    if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_T0HIE_MASK)
                        EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_T0H);
                }
            } else {
                //Update T0L as 8-bit
                if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] & EVMU_SFR_T0CNT_P0LRUN_MASK) {
                    if(advanceReloadCounter8(&pSelf_->timer0.base.tl,
                                              (unsigned)c0,
                                              pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)]))
                    {
                        pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] |= EVMU_SFR_T0CNT_T0LOVF_MASK;
                        if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_T0LIE_MASK)
                            EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_EXT_INT2_T0L);
                    }
                }

                //Update T0H as 8-bit
                if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] & EVMU_SFR_T0CNT_P0HRUN_MASK) {
                    if(advanceReloadCounter8(&pSelf_->timer0.base.th,
                                              (unsigned)c0,
                                              pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)]))
                    {
                        pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)] |= EVMU_SFR_T0CNT_P0HOVF_MASK;
                        if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_T0HIE_MASK)
                            EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_T0H);
                    }
                }
            }
        }
    }
}

static void EvmuTimers_updateTimer1_(EvmuTimers* pSelf) {
    EvmuTimers_* pSelf_ = EVMU_TIMERS_(pSelf);
    EvmuRam_* pRam = pSelf_->pRam;
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));

    const unsigned cy =
        consumeStartDelay_(&pSelf_->timer1.startDelayCycles,
                           (unsigned)EvmuCpu_cycles(pDevice->pCpu));

    //Interrupts enabled for T1H or overflow on T1H
    if(cy &&
       (pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] &
        (EVMU_SFR_T1CNT_T1HRUN_MASK|EVMU_SFR_T1CNT_T1LRUN_MASK)))
    {

        //Both T1H and T1L running, T1 set to 16-bit mode
        if((pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & (EVMU_SFR_T1CNT_T1LONG_MASK|EVMU_SFR_T1CNT_T1HRUN_MASK|EVMU_SFR_T1CNT_T1LRUN_MASK)) ==
                (EVMU_SFR_T1CNT_T1LONG_MASK|EVMU_SFR_T1CNT_T1HRUN_MASK|EVMU_SFR_T1CNT_T1LRUN_MASK))
        {
            unsigned lowOverflowCount = 0;
            if(advanceChainedReloadCounter16_(&pSelf_->timer1.base,
                                              cy,
                                              pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)],
                                              pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)],
                                              &lowOverflowCount))
            {
                pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= EVMU_SFR_T1CNT_T1HOVF_MASK;
                if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1HIE_MASK)
                    EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_T1);
            }

            if(lowOverflowCount)
                pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= EVMU_SFR_T1CNT_T1LOVF_MASK;
        } else {
            //If T1L is running as 8-bit timer
            if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LRUN_MASK) {
                if(advanceReloadCounter8(&pSelf_->timer1.base.tl,
                                          (unsigned)cy,
                                          pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]))
                {
                    pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= EVMU_SFR_T1CNT_T1LOVF_MASK;

                    if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LONG_MASK)
                        EvmuBuzzer__timer1Mode1Reload_(pSelf_->pBuzzer);

                    if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LIE_MASK)
                        EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_T1);
                }
            }
            //If T1H is running as 8-bit timer
            if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1HRUN_MASK) {
                if(advanceReloadCounter8(&pSelf_->timer1.base.th,
                                          (unsigned)cy,
                                          pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)]))
                {
                    pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] |= EVMU_SFR_T1CNT_T1HOVF_MASK;
                    if(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1HIE_MASK)
                        EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_T1);
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

    return (pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)]
            &(EVMU_SFR_T1CNT_T1LONG_MASK|EVMU_SFR_T1CNT_ELDT1C_MASK))>>EVMU_SFR_T1CNT_ELDT1C_POS;
}

EVMU_EXPORT EVMU_TIMER0_MODE EvmuTimers_timer0Mode(const EvmuTimers* pSelf) {
    EvmuRam* pRam = EVMU_RAM_PUBLIC_(EVMU_TIMERS_(pSelf)->pRam);

    const EvmuWord value = EvmuRam_viewData(pRam, EVMU_ADDRESS_SFR_T0CNT);
    return (value & (EVMU_SFR_T0CNT_P0LONG_MASK |
                     EVMU_SFR_T0CNT_P0LEXT_MASK))
            >> EVMU_SFR_T0CNT_P0LEXT_POS;
}

static GBL_RESULT EvmuTimers_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_TIMERS_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuTimers_IBehavior_update_(EvmuIBehavior* pSelf, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelf, ticks);

    EvmuTimers*  pTimers   = EVMU_TIMERS(pSelf);
    EvmuTimers_* pTimers_  = EVMU_TIMERS_(pTimers);
    GBL_UNUSED(pTimers_);

    GBL_CTX_END();
}

static GBL_RESULT EvmuTimers_IBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    EvmuTimers*  pTimers   = EVMU_TIMERS(pSelf);
    EvmuTimers_* pTimers_  = EVMU_TIMERS_(pTimers);

    memset(&pTimers_->baseTimer, 0, sizeof(EvmuBaseTimer));
    memset(&pTimers_->timer0, 0, sizeof(EvmuTimer0));
    memset(&pTimers_->timer1, 0, sizeof(EvmuTimer1));

    pTimers_->timer0.tscale = 256;

    GBL_CTX_END();
}

static GBL_RESULT EvmuTimersClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);

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

    if GBL_UNLIKELY(!GblType_verify(type)) {
        type = GblType_register(GblQuark_internStatic("EvmuTimers"),
                                EVMU_PERIPHERAL_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}

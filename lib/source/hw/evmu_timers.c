#include "evmu_timers_.h"
#include "evmu_ram_.h"
#include "evmu_device_.h"
#include "evmu_buzzer_.h"
#include <evmu/hw/evmu_clock.h>

#define EVMU_BASE_TIMER_COUNTER_BITS_  14u
#define EVMU_BASE_TIMER_COUNTER_MAX_   (1u << EVMU_BASE_TIMER_COUNTER_BITS_)
#define EVMU_BASE_TIMER_COUNTER_MASK_  (EVMU_BASE_TIMER_COUNTER_MAX_ - 1u)

typedef enum EVMU_BASE_TIMER_CLOCK_ {
    EVMU_BASE_TIMER_CLOCK__QUARTZ_,
    EVMU_BASE_TIMER_CLOCK__CYCLE_,
    EVMU_BASE_TIMER_CLOCK__T0_PRESCALER_
} EVMU_BASE_TIMER_CLOCK_;

static EVMU_BASE_TIMER_CLOCK_ baseTimerClock_(const EvmuRam_* pRam) {
    switch((pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ISL)] >> 4u) & 0x3u) {
    case 0x1u:
        return EVMU_BASE_TIMER_CLOCK__CYCLE_;
    case 0x3u:
        return EVMU_BASE_TIMER_CLOCK__T0_PRESCALER_;
    default:
        return EVMU_BASE_TIMER_CLOCK__QUARTZ_;
    }
}

static uint64_t oscillatorHz_(EVMU_OSCILLATOR oscillator) {
    switch(oscillator) {
    case EVMU_OSCILLATOR_CF:
        return EVMU_CLOCK_OSC_CF_FREQ;
    case EVMU_OSCILLATOR_QUARTZ:
        return EVMU_CLOCK_OSC_QUARTZ_FREQ;
    default:
        return EVMU_CLOCK_OSC_RC_FREQ;
    }
}

static unsigned dividerFactor_(EVMU_CLOCK_DIVIDER divider) {
    switch(divider) {
    case EVMU_CLOCK_DIVIDER_6:
        return 6u;
    case EVMU_CLOCK_DIVIDER_12:
        return 12u;
    default:
        return 1u;
    }
}

static unsigned consumeStartDelay_(unsigned* pDelay, unsigned ticks) {
    const unsigned skipped = (*pDelay < ticks)? *pDelay : ticks;
    *pDelay -= skipped;
    return ticks - skipped;
}

static unsigned baseTimerInt0Rate_(EvmuWord btcr) {
    return (btcr & EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK)? 0x40u : 0x4000u;
}

static unsigned baseTimerInt1Rate_(EvmuWord btcr) {
    switch(btcr & (EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK |
                   EVMU_SFR_BTCR_INT1_CYCLE_CTRL_MASK))
    {
    case 0x00u:
    case 0x80u:
        return 0x20u;
    case 0x10u:
    case 0x90u:
        return 0x80u;
    case 0x20u:
        return 0x200u;
    case 0x30u:
        return 0x800u;
    case 0xa0u:
        return 0x2u;
    case 0xb0u:
        return 0x8u;
    }

    return 0u;
}

static unsigned baseTimerTicksElapsed_(EvmuTimers* pSelf,
                                       EvmuDevice* pDevice,
                                       unsigned    cpuCycles)
{
    EvmuTimers_* pSelf_ = EVMU_TIMERS_(pSelf);
    EvmuRam_* pRam = pSelf_->pRam;

    switch(baseTimerClock_(pRam)) {
    case EVMU_BASE_TIMER_CLOCK__CYCLE_:
        return consumeStartDelay_(&pSelf_->baseTimer.startDelayCycles, cpuCycles);
    case EVMU_BASE_TIMER_CLOCK__T0_PRESCALER_:
    {
        pSelf_->baseTimer.startDelayCycles = 0;
        const uint64_t scaledNumerator =
            pSelf_->baseTimer.tickRemainder +
            (uint64_t)cpuCycles;
        const unsigned elapsed = (unsigned)(scaledNumerator / pSelf_->timer0.tscale);

        pSelf_->baseTimer.tickRemainder = scaledNumerator % pSelf_->timer0.tscale;

        return elapsed;
    }
    case EVMU_BASE_TIMER_CLOCK__QUARTZ_: {
        pSelf_->baseTimer.startDelayCycles = 0;
        EVMU_OSCILLATOR source = EVMU_OSCILLATOR_QUARTZ;
        EVMU_CLOCK_DIVIDER divider = EVMU_CLOCK_DIVIDER_12;

        EvmuClock_systemConfig(pDevice->pClock, &source, &divider);

        const uint64_t systemHz = oscillatorHz_(source);
        const uint64_t scaledNumerator =
            pSelf_->baseTimer.tickRemainder +
            (uint64_t)cpuCycles *
            EVMU_CLOCK_OSC_QUARTZ_FREQ *
            dividerFactor_(divider);
        const unsigned elapsed = (unsigned)(scaledNumerator / systemHz);

        pSelf_->baseTimer.tickRemainder = scaledNumerator % systemHz;

        return elapsed;
    }
    }
}

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
    const unsigned cpuCycles = (unsigned)EvmuCpu_cycles(pDevice->pCpu);
    const unsigned elapsed = baseTimerTicksElapsed_(pSelf, pDevice, cpuCycles);

    if(!(btcr & EVMU_SFR_BTCR_OP_CTRL_MASK))
        return;

    if(elapsed) {
        const unsigned currentCounter = pSelf_->baseTimer.counter;
        const unsigned nextCounter = currentCounter + elapsed;
        const unsigned int0Rate = baseTimerInt0Rate_(btcr);
        const unsigned int1Rate = baseTimerInt1Rate_(btcr);

        if(int1Rate && (currentCounter / int1Rate) < (nextCounter / int1Rate)) {
            pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] |= EVMU_SFR_BTCR_INT1_SRC_MASK;
            if(btcr & EVMU_SFR_BTCR_INT1_REQ_EN_MASK)
                EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_EXT_INT3_TBASE);
        }

        if((currentCounter / int0Rate) < (nextCounter / int0Rate)) {
            pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_BTCR)] |= EVMU_SFR_BTCR_INT0_SRC_MASK;
            if(btcr & EVMU_SFR_BTCR_INT0_REQ_EN_MASK)
                EvmuPic_raiseIrq(pDevice->pPic, EVMU_IRQ_EXT_INT3_TBASE);
        }

        pSelf_->baseTimer.counter = (uint16_t)(nextCounter & EVMU_BASE_TIMER_COUNTER_MASK_);
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

                    if(!(pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LONG_MASK))
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

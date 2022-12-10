#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_wave.h>
#include <evmu/hw/evmu_address_space.h>
#include "evmu/events/evmu_memory_event.h"
#include "evmu_device_.h"
#include "evmu_memory_.h"
#include "evmu_clock_.h"

#define EVMU_CLOCK_OSC_QUARTZ_STABILIZATION_TIME
#if 0
GBL_EXPORT EVMU_RESULT EvmuClock_systemConfig(const EvmuClock* pSelf, EVMU_OSCILLATOR* pSource, EVMU_CLOCK_DIVIDER* pDivider)

static void EvmuClock_updateSignal_(EvmuClock* pSelf, EVMU_CLOCK_SIGNAL signal) {
    EvmuClock_* pSelf_ = EVMU_CLOCK_(pSelf);
    EvmuMemory_* pMem = pSelf_->pMemory;
    EvmuClockSignal_* pSignal = &pSelf_->signals[signal];

    switch(signal) {
    case EVMU_CLOCK_SIGNAL_OSCILLATOR_QUARTZ: {
        pSignal->hz = 32768;

        pSignal->halfCycleTime =
        pSignal->active = EvmuClock_oscillatorActive(pSelf, EVMU_OSCILLATOR_QUARTZ);
        break;
    }
    case EVMU_CLOCK_SIGNAL_OSCILLATOR_RC:
        break;
    case EVMU_CLOCK_SIGNAL_OSCILLATOR_CF:
        break;
    case EVMU_CLOCK_SIGNAL_SYSTEM_1:
        break;
    case EVMU_CLOCK_SIGNAL_SYSTEM_2:
        break;
    default: GBL_ASSERT(GBL_FALSE, "Inavlid clock signal!");

    }
}
#endif
static void EvmuClockSignal_init_(EvmuClockSignal_* pSelf, EvmuCycles hz, EvmuTicks cycleTime, EvmuTicks stabilizationTime) {
    memset(pSelf, 0, sizeof(EvmuClockSignal_));
    pSelf->hz = hz;
    pSelf->halfCycleTime = (cycleTime >> 1);
    pSelf->stabilizationHalfCycles = hz / stabilizationTime * 2;
    EvmuWave_reset(&pSelf->wave);
}

static EvmuCycles EvmuClockSignal_update_(EvmuClockSignal_* pSelf, EvmuTicks deltaTime) {
    EvmuCycles prevHalfCycles = pSelf->halfCyclesTotal;
    EvmuTicks timeLeft = deltaTime + pSelf->timeRemainder;
    while(timeLeft > pSelf->halfCycleTime) {
        ++pSelf->halfCyclesTotal;
        if(!pSelf->active) {
            EvmuWave_update(&pSelf->wave, EVMU_LOGIC_Z);
        } else {
            if(pSelf->halfCyclesTotal < pSelf->stabilizationHalfCycles) {
                EvmuWave_update(&pSelf->wave, EVMU_LOGIC_X);
            } else {
                if(pSelf->halfCyclesTotal % 2) {
                    EvmuWave_update(&pSelf->wave, EVMU_LOGIC_1);
                } else {
                    EvmuWave_update(&pSelf->wave, EVMU_LOGIC_0);
                }
            }
        }

        timeLeft -= pSelf->halfCycleTime;
    }
    pSelf->timeRemainder = timeLeft;
    return pSelf->halfCyclesTotal - prevHalfCycles;
}


static GBL_RESULT EvmuClock_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(pSelf);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuClock_update_(EvmuIBehavior* pSelfBehav, EvmuTicks ticks) {
    GBL_CTX_BEGIN(pSelfBehav);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pSelfBehav, ticks);

    EvmuClock*  pSelf       = EVMU_CLOCK(pSelfBehav);
    EvmuClock_* pPrivate    = EVMU_CLOCK_(pSelf);
    EvmuTicks   deltaTime   = ticks;

    while(deltaTime > 0) {
        EvmuTicks timeStep = EvmuClock_systemTicksPerCycle(pSelf);
        timeStep = deltaTime < timeStep? deltaTime : timeStep;

        for(unsigned c = 0; c < EVMU_CLOCK_SIGNAL_COUNT; ++c) {
            EvmuClockSignal_* pSignal = &pPrivate->signals[c];
            EvmuCycles deltaCycles = EvmuClockSignal_update_(pSignal, timeStep);

            if(deltaCycles && EvmuWave_hasChanged(&pSignal->wave)) {
                GBL_CTX_CALL(GblBox_construct(GBL_BOX(&pPrivate->event), EVMU_CLOCK_EVENT_TYPE));
                pPrivate->event.signal = c;
                pPrivate->event.wave = pSignal->wave;
                GBL_CTX_EVENT(&pPrivate->event);
            }
        }
        deltaTime -= timeStep;
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuClock_memoryEvent_(EvmuPeripheral* pSelf, EvmuMemoryEvent* pEvent) {
    GBL_CTX_BEGIN(pSelf);

    switch(pEvent->op) {
    case EVMU_MEMORY_EVENT_OP_WRITE:
       switch(pEvent->address) {
       case EVMU_ADDRESS_SFR_PCON:
           break;
       case EVMU_ADDRESS_SFR_OCR:
           break;
       default: break;
       }

    default: break;
    }

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuClock_oscillatorSpecs(const EvmuClock* pSelf, EVMU_OSCILLATOR oscillator, EvmuOscillatorSpecs* pSpecs) {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_ARG(oscillator < EVMU_OSCILLATOR_COUNT);
    GBL_CTX_VERIFY_POINTER(pSpecs);
    switch(oscillator) {
    case EVMU_OSCILLATOR_QUARTZ:
        pSpecs->currentMicroAmps    = 610;
        pSpecs->hzReference         =
        pSpecs->hzToleranceHigh     =
        pSpecs->hzToleranceLow      = 32768;
        pSpecs->stabilizationTicks  = 200;
        break;
    case EVMU_OSCILLATOR_RC:
        pSpecs->currentMicroAmps    = 2600;
        pSpecs->hzReference         = 879236;
        pSpecs->hzToleranceLow      = 600000;
        pSpecs->hzToleranceHigh     = 1200000;
        pSpecs->stabilizationTicks  = 0;
        break;
    case EVMU_OSCILLATOR_CF:
        pSpecs->currentMicroAmps    = 0;
        pSpecs->hzReference         =
        pSpecs->hzToleranceHigh     =
        pSpecs->hzToleranceLow      = 5000000;
        pSpecs->stabilizationTicks  = 0;
        break;
    }
    GBL_CTX_END();
}

GBL_EXPORT GblBool EvmuClock_oscillatorActive(const EvmuClock* pSelf, EVMU_OSCILLATOR oscillator) {
#if 0
    GblBool active = GBL_FALSE;
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_ARG(oscillator < EVMU_OSCILLATOR_COUNT);

    switch(oscillator) {
    case EVMU_OSCILLATOR_QUARTZ:
        /*THIS IS ABSOLUTELY WRONG, VMU after reset has Quartz disabled, then BIOS enables it.
         * There must be a control register somewhere for it. Perhaps OCR.2? */
        active = GBL_TRUE;
        break;
    case EVMU_OSCILLATOR_RC:
        active = !EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                           EVMU_ADDRESS_SFR_OCR,
                                           EVMU_SFR_OCR_OCR1_MASK);
        break;
    case EVMU_OSCILLATOR_CF:
        active = !EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                           EVMU_ADDRESS_SFR_OCR,
                                           EVMU_SFR_OCR_OCR0_MASK);
        break;
    }

    GBL_CTX_END_BLOCK();
    return active;
#endif
}

GBL_EXPORT GBL_RESULT EvmuClock_setOscillatorActive(const EvmuClock* pSelf, EVMU_OSCILLATOR oscillator, GblBool active) {
    GBL_CTX_BEGIN(pSelf);
#if 0
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_ARG(oscillator < EVMU_OSCILLATOR_COUNT);

    switch(oscillator) {
    case EVMU_OSCILLATOR_QUARTZ:
        /*THIS IS ABSOLUTELY WRONG, VMU after reset has Quartz disabled, then BIOS enables it.
         * There must be a control register somewhere for it. Perhaps OCR.2? */
        EVMU_PERIPHERAL_WARNING("Cannot activate/deactivate Quartz oscillator! (automatic in bios)");
        break;
    case EVMU_OSCILLATOR_RC:
        if(active)
            EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR1_MASK);
        else
            EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR1_MASK);
        break;
    case EVMU_OSCILLATOR_CF:
        if(active)
            EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR0_MASK);
        else
            EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR0_MASK);
        break;
    }
#endif
    GBL_CTX_END();
}

GBL_EXPORT EVMU_CLOCK_SYSTEM_STATE EvmuClock_systemState(const EvmuClock* pSelf) GBL_NOEXCEPT {
    EVMU_CLOCK_SYSTEM_STATE state = EVMU_CLOCK_SYSTEM_STATE_UNKNOWN;
    GBL_CTX_BEGIN(pSelf);
#if 0
    GBL_CTX_VERIFY_POINTER(pSelf);

    if(EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HOLD_MASK))
    {
        state = EVMU_CLOCK_SYSTEM_STATE_HOLD;
    } else if(EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                       EVMU_ADDRESS_SFR_PCON,
                                       EVMU_SFR_PCON_HALT_MASK))
    {
        state = EVMU_CLOCK_SYSTEM_STATE_HALT;
    } else {
        state = EVMU_CLOCK_SYSTEM_STATE_RUNNING;
    }
#endif
    GBL_CTX_END_BLOCK();
    return state;
}


GBL_EXPORT EVMU_RESULT EvmuClock_setSystemState(const EvmuClock* pSelf, EVMU_CLOCK_SYSTEM_STATE state) GBL_NOEXCEPT {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_ARG(state != EVMU_CLOCK_SYSTEM_STATE_UNKNOWN &&
                       state < EVMU_CLOCK_SYSTEM_STATE_COUNT);
#if 0

    if(state == EVMU_CLOCK_SYSTEM_STATE_HOLD) {
        EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HOLD_MASK);
        EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HALT_MASK);

    } else if(state == EVMU_CLOCK_SYSTEM_STATE_HALT) {
        EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                  EVMU_ADDRESS_SFR_PCON,
                                  EVMU_SFR_PCON_HOLD_MASK);
        EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HALT_MASK);

    } else {
        EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                  EVMU_ADDRESS_SFR_PCON,
                                  EVMU_SFR_PCON_HOLD_MASK);
        EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                  EVMU_ADDRESS_SFR_PCON,
                                  EVMU_SFR_PCON_HALT_MASK);
    }
#endif
    GBL_CTX_END();
}

GBL_EXPORT EVMU_RESULT EvmuClock_systemConfig(const EvmuClock* pSelf, EVMU_OSCILLATOR* pSource, EVMU_CLOCK_DIVIDER* pDivider) GBL_NOEXCEPT {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_POINTER(pSource);
    GBL_CTX_VERIFY_POINTER(pDivider);

#if 0

    if(EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                EVMU_ADDRESS_SFR_OCR,
                                EVMU_SFR_OCR_OCR4_MASK))  {
        *pSource    = EVMU_OSCILLATOR_CF;
        *pDivider   = EVMU_CLOCK_DIVIDER_1;

    } else if(EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                       EVMU_ADDRESS_SFR_OCR,
                                       EVMU_SFR_OCR_OCR5_MASK)) {
        *pSource = EVMU_OSCILLATOR_QUARTZ;
        *pDivider = EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                             EVMU_ADDRESS_SFR_OCR,
                                             EVMU_SFR_OCR_OCR7_MASK) ?
                        EVMU_CLOCK_DIVIDER_6 : EVMU_CLOCK_DIVIDER_12;
    } else {
        *pSource = EVMU_OSCILLATOR_RC;
        *pDivider = EvmuMemory__sfrMaskTest_(EVMU_CLOCK_(pSelf)->pMemory,
                                             EVMU_ADDRESS_SFR_OCR,
                                             EVMU_SFR_OCR_OCR7_MASK) ?
                        EVMU_CLOCK_DIVIDER_6 : EVMU_CLOCK_DIVIDER_12;

    }
#endif
    GBL_CTX_END();
}

GBL_EXPORT EVMU_RESULT EvmuClock_setSystemConfig(const EvmuClock* pSelf, EVMU_OSCILLATOR source, EVMU_CLOCK_DIVIDER divider) GBL_NOEXCEPT {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY_ARG(source < EVMU_OSCILLATOR_COUNT);
    GBL_CTX_VERIFY_ARG(divider < EVMU_CLOCK_DIVIDER_COUNT);

#if 0

    if(source == EVMU_OSCILLATOR_CF) {
        GBL_CTX_VERIFY_ARG(divider == EVMU_CLOCK_DIVIDER_1,
                           "Cannot set clock divider with CF oscillator!");
        GBL_CTX_CALL(EvmuClock_setOscillatorActive(pSelf, EVMU_OSCILLATOR_CF, GBL_TRUE));
        EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                EVMU_ADDRESS_SFR_OCR,
                                EVMU_SFR_OCR_OCR4_MASK);
    } else {
        GBL_CTX_VERIFY_ARG(divider != EVMU_CLOCK_DIVIDER_1,
                           "Only valid dividers for RC/Quartz osillators are 1/6 and 1/12!");

        if(divider == EVMU_CLOCK_DIVIDER_12)
            EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR7_MASK);
        else
            EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR7_MASK);

        GBL_CTX_CALL(EvmuClock_setOscillatorActive(pSelf, source, GBL_TRUE));

        if(source == EVMU_OSCILLATOR_RC)
            EvmuMemory__sfrMaskClear_(EVMU_CLOCK_(pSelf)->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR5_MASK);
        else // source == Quartz
            EvmuMemory__sfrMaskSet_(EVMU_CLOCK_(pSelf)->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR5_MASK);

    }
#endif
    GBL_CTX_END();
}

GBL_EXPORT EvmuTicks EvmuClock_systemTicksPerCycle(const EvmuClock* pSelf) {
    const EvmuWord ocr = EVMU_CLOCK_(pSelf)->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_OCR)];
    EvmuTicks ticks = 0;

    if(ocr & EVMU_SFR_OCR_OCR5_MASK) {
        ticks = (ocr & EVMU_SFR_OCR_OCR7_MASK)?
                    EVMU_CLOCK_OSC_QUARTZ_TCYC_1_6: EVMU_CLOCK_OSC_QUARTZ_TCYC_1_12;
    } else {
        ticks = (ocr & EVMU_SFR_OCR_OCR7_MASK)?
                    EVMU_CLOCK_OSC_RC_TCYC_1_6: EVMU_CLOCK_OSC_RC_TCYC_1_12;
    }

    ticks *= 1000; //msec to nsec

    return ticks;
}

EVMU_EXPORT uint64_t EvmuClock_systemCyclesPerSec(const EvmuClock* pSelf) {
    //unsigned char pcon = dev->sfr[EVMU_SFR_OFFSET(SFR_ADDR_PCON)];
    unsigned char ocr = EVMU_CLOCK_(pSelf)->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_OCR)];
    double val;

    //INACCURATE, THERE IS A REMAINDER FROM THESE DIVISIONS!!!!
    if(ocr&EVMU_SFR_OCR_OCR5_MASK) {
        val = ((double)EVMU_CLOCK_OSC_QUARTZ_FREQ)/((ocr&EVMU_SFR_OCR_OCR7_MASK)? 6.0 : 12.0);
    } else {
        val =  ((double)EVMU_CLOCK_OSC_RC_FREQ)/((ocr&EVMU_SFR_OCR_OCR7_MASK)? 6.0 : 12.0);
    }

    return val;
}

EVMU_EXPORT double EvmuClock_systemSecsPerCycle(const EvmuClock* pSelf) {
    double val = 1.0/(double)(EvmuClock_systemCyclesPerSec(pSelf));
    return val;
}


static GBL_RESULT EvmuClock_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuClock_* pSelf_ = EVMU_CLOCK_(pSelf);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructor, pSelf);

    GblObject_setName(pSelf, EVMU_CLOCK_NAME);

    GBL_CTX_VERIFY_CALL(GblEvent_construct((GblEvent*)&pSelf_->event, EVMU_CLOCK_EVENT_TYPE));

    for(GblSize s = 0; s < EVMU_CLOCK_SIGNAL_COUNT; ++s) {
     //   EvmuClockSignal_init_(&pSelf_->signals[s], );
    }

    GBL_CTX_END();
}


static GBL_RESULT EvmuClock_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    GBL_BOX_UNREF((&EVMU_CLOCK_(EVMU_CLOCK(pSelf))->event));

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pSelf);
    GBL_CTX_END();
}


static GBL_RESULT EvmuClockClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset          = EvmuClock_reset_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate         = EvmuClock_update_;
    EVMU_PERIPHERAL_CLASS(pClass)->pFnMemoryEvent   = EvmuClock_memoryEvent_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor        = EvmuClock_constructor_;
    GBL_BOX_CLASS(pClass)->pFnDestructor            = EvmuClock_destructor_;

    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuClock_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .pFnClassInit        = EvmuClockClass_init_,
        .classSize           = sizeof(EvmuClockClass),
        .instanceSize        = sizeof(EvmuClock),
        .instancePrivateSize = sizeof(EvmuClock_)
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuClock"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}


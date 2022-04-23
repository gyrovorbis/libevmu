#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_wave.h>
#include <evmu/hw/evmu_address_space.h>
#include "evmu_device_.h"
#include "evmu_memory_.h"


#define EVMU_CLOCK_OSC_QUARTZ_HZ            32768
#define EVMU_CLOCK_OSC_RC_HZ                879236

#define EVMU_CLOCK_OSC_QUARTZ_TCYC_1_12     366210
#define EVMU_CLOCK_OSC_QUARTZ_TCYC_1_6      183105

#define EVMU_CLOCK_OSC_RC_TCYC_1_12         12568
#define EVMU_CLOCK_OSC_RC_TCYC_1_6          6284

#define EVMU_CLOCK_OSC_QUARTZ_CURRENT       2600 //uA
#define EVMU_CLOCK_OSC_RC_CURRENT           610 //uA

static GBL_RESULT EvmuClock_constructor_(EvmuClock* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_PERIPHERAL_TYPE,
                             EvmuPeripheralClass,
                             base.base.pFnConstructor, (GblObject*)pSelf);

    pSelf->pPrivate = &DEV_CLOCK_(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf)));
    pSelf->pPrivate->pPublic = pSelf;


    GBL_API_END();
}


static GBL_RESULT EvmuClock_destructor_(EvmuClock* pSelf) {
    GBL_API_BEGIN(NULL);

    GBL_API_FREE(pSelf->pPrivate);

    GBL_INSTANCE_VCALL_SUPER(EVMU_PERIPHERAL_TYPE,
                             EvmuPeripheralClass,
                             base.base.pFnDestructor, (GblObject*)pSelf);
    GBL_API_END();
}


static GBL_RESULT EvmuClock_reset_(EvmuClock* pSelf) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                             pFnReset, (EvmuEntity*)pSelf);





    GBL_API_END();
}

static GBL_RESULT EvmuClock_update_(EvmuClock* pSelf, EvmuTicks ticks) {
    GBL_API_BEGIN(NULL);
    GBL_INSTANCE_VCALL_SUPER(EVMU_ENTITY_TYPE, EvmuEntityClass,
                             pFnUpdate, (void*)pSelf, ticks);

    EvmuClock_* pPrivate = pSelf->pPrivate;
    EvmuTicks deltaTime = ticks;

    while(deltaTime > 0) {
        EvmuTicks timeStep = EvmuClock_timestepTicks(pSelf);
        timeStep = deltaTime < timeStep? deltaTime : timeStep;
        for(unsigned c = 0; c < EVMU_CLOCK_SIGNAL_COUNT; ++c) {

            if(c == EVMU_CLOCK_SIGNAL_CYCLE) {
                if(EvmuClock_systemState(pSelf) == EVMU_CLOCK_SYSTEM_STATE_RUNNING) {
                    EvmuClockSignal_update_(&pPrivate->signals[c], timeStep);
                    if(EvmuWave_hasChangedEdgeRising(&pPrivate->signals[c].wave)) {
                        //EvmuCpu__runCycle_(EvmuDevice_cpu(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf)))->pPrivate);
                    }
                }
            }
        }
        deltaTime -= timeStep;
    }


    GBL_API_END();
}

static GBL_RESULT EvmuClockClass_init_(EvmuClockClass* pClass, void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_API_BEGIN(pCtx);
    pClass->base.base.pFnReset               = (void*)EvmuClock_reset_;
    pClass->base.base.pFnUpdate              = (void*)EvmuClock_update_;
    pClass->base.base.base.pFnConstructor    = (void*)EvmuClock_constructor_;
    pClass->base.base.base.pFnDestructor     = (void*)EvmuClock_destructor_;
    GBL_API_END();
}


GBL_EXPORT GblType EvmuClock_type(void) {
    static GblType type = GBL_TYPE_INVALID;
    if(type == GBL_TYPE_INVALID) {
        type = gblTypeRegisterStatic(EVMU_ENTITY_TYPE,
                                     "EvmuClock",
                                     &((const GblTypeInfo) {
                                         .pFnClassInit  = (GblTypeClassInitFn)EvmuClockClass_init_,
                                         .classSize     = sizeof(EvmuClockClass),
                                         .classAlign    = GBL_ALIGNOF(EvmuClockClass),
                                         .instanceSize  = sizeof(EvmuClock),
                                         .instanceAlign = GBL_ALIGNOF(EvmuClock)
                                     }),
                                     GBL_TYPE_FLAGS_NONE);

    }
    return type;
}

EVMU_API EvmuClock_oscillatorSpecs(const EvmuClock* pSelf, EVMU_OSCILLATOR oscillator, EvmuOscillatorSpecs* pSpecs) {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_ARG(oscillator < EVMU_OSCILLATOR_COUNT);
    GBL_API_VERIFY_POINTER(pSpecs);
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
    GBL_API_END();
}

GBL_EXPORT GblBool EvmUClock_oscillatorActive(const EvmuClock* pSelf, EVMU_OSCILLATOR oscillator) {
    GblBool active = GBL_FALSE;
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_ARG(oscillator < EVMU_OSCILLATOR_COUNT);
    switch(oscillator) {
    case EVMU_OSCILLATOR_QUARTZ:
        /*THIS IS ABSOLUTELY WRONG, VMU after reset has Quartz disabled, then BIOS enables it.
         * There must be a control register somewhere for it. Perhaps OCR.2? */
        active = GBL_TRUE;
        break;
    case EVMU_OSCILLATOR_RC:
        active = !EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR1_MASK);
        break;
    case EVMU_OSCILLATOR_CF:
        active = !EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR0_MASK);
        break;
    }
    GBL_API_END_BLOCK();
    return active;
}

GBL_EXPORT GBL_RESULT EvmUClock_oscillatorActiveSet(const EvmuClock* pSelf, EVMU_OSCILLATOR oscillator, GblBool active) {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_ARG(oscillator < EVMU_OSCILLATOR_COUNT);
    switch(oscillator) {
    case EVMU_OSCILLATOR_QUARTZ:
        /*THIS IS ABSOLUTELY WRONG, VMU after reset has Quartz disabled, then BIOS enables it.
         * There must be a control register somewhere for it. Perhaps OCR.2? */
        EVMU_PERIPHERAL_WARNING("Cannot activate/deactivate Quartz oscillator! (automatic in bios)");
        break;
    case EVMU_OSCILLATOR_RC:
        if(active)
            EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR1_MASK);
        else
            EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR1_MASK);
        break;
    case EVMU_OSCILLATOR_CF:
        if(active)
            EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR0_MASK);
        else
            EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR0_MASK);
        break;
    }
    GBL_API_END();
}

GBL_EXPORT EVMU_CLOCK_SYSTEM_STATE EvmuClock_systemState(const EvmuClock* pSelf) GBL_NOEXCEPT {
    EVMU_CLOCK_SYSTEM_STATE state = EVMU_CLOCK_SYSTEM_STATE_UNKNOWN;
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);

    if(EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HOLD_MASK))
    {
        state = EVMU_CLOCK_SYSTEM_STATE_HOLD;
    } else if(EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                       EVMU_ADDRESS_SFR_PCON,
                                       EVMU_SFR_PCON_HALT_MASK))
    {
        state = EVMU_CLOCK_SYSTEM_STATE_HALT;
    } else {
        state = EVMU_CLOCK_SYSTEM_STATE_RUNNING;
    }

    GBL_API_END_BLOCK();
    return state;
}


GBL_EXPORT EVMU_RESULT EvmuClock_systemStateSet(const EvmuClock* pSelf, EVMU_CLOCK_SYSTEM_STATE state) GBL_NOEXCEPT {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_ARG(state != EVMU_CLOCK_SYSTEM_STATE_UNKNOWN &&
                       state < EVMU_CLOCK_SYSTEM_STATE_COUNT);


    if(state == EVMU_CLOCK_SYSTEM_STATE_HOLD) {
        EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HOLD_MASK);
        EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HALT_MASK);

    } else if(state == EVMU_CLOCK_SYSTEM_STATE_HALT) {
        EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                  EVMU_ADDRESS_SFR_PCON,
                                  EVMU_SFR_PCON_HOLD_MASK);
        EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                EVMU_ADDRESS_SFR_PCON,
                                EVMU_SFR_PCON_HALT_MASK);

    } else {
        EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                  EVMU_ADDRESS_SFR_PCON,
                                  EVMU_SFR_PCON_HOLD_MASK);
        EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                  EVMU_ADDRESS_SFR_PCON,
                                  EVMU_SFR_PCON_HALT_MASK);
    }

    GBL_API_END();
}

GBL_EXPORT EVMU_RESULT EvmuClock_systemConfig(const EvmuClock* pSelf, EVMU_OSCILLATOR* pSource, EVMU_CLOCK_DIVIDER* pDivider) GBL_NOEXCEPT {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_POINTER(pSource);
    GBL_API_VERIFY_POINTER(pDivider);

    if(EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                EVMU_ADDRESS_SFR_OCR,
                                EVMU_SFR_OCR_OCR4_MASK))  {
        *pSource    = EVMU_OSCILLATOR_CF;
        *pDivider   = EVMU_CLOCK_DIVIDER_1;

    } else if(EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                       EVMU_ADDRESS_SFR_OCR,
                                       EVMU_SFR_OCR_OCR5_MASK)) {
        *pSource = EVMU_OSCILLATOR_QUARTZ;
        *pDivider = EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                             EVMU_ADDRESS_SFR_OCR,
                                             EVMU_SFR_OCR_OCR7_MASK) ?
                        EVMU_CLOCK_DIVIDER_6 : EVMU_CLOCK_DIVIDER_12;
    } else {
        *pSource = EVMU_OSCILLATOR_RC;
        *pDivider = EvmuMemory__sfrMaskTest_(pSelf->pPrivate->pMemory,
                                             EVMU_ADDRESS_SFR_OCR,
                                             EVMU_SFR_OCR_OCR7_MASK) ?
                        EVMU_CLOCK_DIVIDER_6 : EVMU_CLOCK_DIVIDER_12;

    }
    GBL_API_END();
}

GBL_EXPORT EVMU_RESULT EvmuClock_systemConfigSet(const EvmuClock* pSelf, EVMU_OSCILLATOR source, EVMU_CLOCK_DIVIDER divider) GBL_NOEXCEPT {
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    GBL_API_VERIFY_ARG(source < EVMU_OSCILLATOR_COUNT);
    GBL_API_VERIFY_ARG(divider < EVMU_CLOCK_DIVIDER_COUNT);

    if(source == EVMU_OSCILLATOR_CF) {
        GBL_API_VERIFY_ARG(divider == EVMU_CLOCK_DIVIDER_1,
                           "Cannot set clock divider with CF oscillator!");
        GBL_API_CALL(EvmuClock_oscillatorActiveSet(pSelf, EVMU_OSCILLATOR_CF, GBL_TRUE));
        EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                EVMU_ADDRESS_SFR_OCR,
                                EVMU_SFR_OCR_OCR4_MASK);
    } else {
        GBL_API_VERIFY_ARG(divider != EVMU_CLOCK_DIVIDER_1,
                           "Only valid dividers for RC/Quartz osillators are 1/6 and 1/12!");

        if(divider == EVMU_CLOCK_DIVIDER_12)
            EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                      EVMU_ADDRESS_SFR_OCR,
                                      EVMU_SFR_OCR_OCR7_MASK);
        else
            EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR7_MASK);

        GBL_API_CALL(EvmuClock_oscillatorActiveSet(pSelf, source, GBL_TRUE));

        if(source == EVMU_OSCILLATOR_RC)
            EvmuMemory__sfrMaskClear_(pSelf->pPrivate->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR5_MASK);
        else // source == Quartz
            EvmuMemory__sfrMaskSet_(pSelf->pPrivate->pMemory,
                                    EVMU_ADDRESS_SFR_OCR,
                                    EVMU_SFR_OCR_OCR5_MASK);

    }
    GBL_API_END();
}

GBL_EXPORT EvmuTicks EvmuClock_timeStepTicks(const EvmuClock* pSelf) {
    EvmuTicks smallest = 0;
    GBL_API_BEGIN(NULL);
    GBL_API_VERIFY_POINTER(pSelf);
    for(unsigned c = 0; c < EVMU_CLOCK_SIGNAL_COUNT; ++c) {
        if(pSelf->pPrivate->signals[c].halfCycleTime < smallest)
            smallest = pSelf->pPrivate->signals[c].halfCycleTime;
    }
    GBL_API_END_BLOCK();
    return smallest;
}




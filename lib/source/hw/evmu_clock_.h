#ifndef EVMU_CLOCK__H
#define EVMU_CLOCK__H

#include <evmu/hw/evmu_clock.h>
#include "evmu_peripheral_.h"

#ifdef __cplusplus
extern "C" {
#endif

// MAIN DREAMCAST 5HZ CLOCK CALLED the CERAMIC "CF" CLOCK!!!

typedef struct EVMU_OSCILLATOR_STATE {
    EVMU_OSCILLATOR_STATE_DISABLED,
    EVMU_OSCILLATOR_STATE_STARTING,
    EVMU_OSCILLATOR_STATE_STEADY,
    EVMU_OSCILLATOR_STATE_ENDING,
    EVMU_OSCILLATOR_STATE_COUNT
} EVMU_OSCILLATOR_STATE;

typedef struct EvmuOscillator {
    EvmuTicks               hz;
    EvmuTicks               cycleTime;
    EvmuTicks               cycleElapsed;
    EvmuTicks               tolerance;
    EvmuTicks               stabilizationTime;
    EvmuTicks               stabilizationElapsed;
    EVMU_OSCILLATOR_STATE   state;
    GYWave                  waveForm;
} EvmuOscillator;

typedef struct EvmuClock_ {
    EvmuPeripheral peripheral;
    EvmuOscillator quartzOsc;
    EvmuOscillator rcOsc;
    EvmuOscillator cfOsc;




// these signals need to have a corresponding stable/valid stream for when clock isn't stable.
    //MCLK;
    //SCLK;

} EvmuClock_;

EvmuClock clk;
EvmuClock stepper;
EVMU_CLOCK_TICK_EVENT event;



evmuClockTicksStepper(clk, stepper, ticks);
while((event = evmuClockTickStep(clk, stepper)) != EVMU_CLOCK_TICK_END) {
    // probably make this a bitmask so they can be combined
    switch(event) {
    case EVMU_CLOCK_TICK_EVENT_RC_OSCILLATOR:
    case EVMU_CLOCK_TICK_EVENT_QUARTZ_OSCILLATOR:
    case EVMU_CLOCK_TICK_EVENT_CF_OSCILLATOR:
    case EVMU_CLOCK_TICK_EVENT_SYSTEM_1:
    case EVMU_CLOCK_TICK_EVENT_SYSTEM_2:
    default: assert(0);
    }
}





#ifdef __cplusplus
}
#endif

#endif // EVMU_CLOCK__H

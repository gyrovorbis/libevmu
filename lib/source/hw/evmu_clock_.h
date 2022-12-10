#ifndef EVMU_CLOCK__H
#define EVMU_CLOCK__H

#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_wave.h>
#include <evmu/events/evmu_clock_event.h>

#define EVMU_CLOCK_(instance)    ((EvmuClock_*)GBL_INSTANCE_PRIVATE(instance, EVMU_CLOCK_TYPE))
#define EVMU_CLOCK_PUBLIC_(priv)  ((EvmuClock*)GBL_INSTANCE_PUBLIC(priv, EVMU_CLOCK_TYPE))

#define GBL_SELF_TYPE EvmuClock_

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

typedef struct EvmuClockSignal_ {
    EvmuCycles              hz;
    EvmuTicks               halfCycleTime;
    EvmuTicks               stabilizationHalfCycles;

    GblBool                 active;
    EvmuCycles              halfCyclesTotal;
    EvmuTicks               timeRemainder;

    EvmuWave                wave;
} EvmuClockSignal_;

typedef struct EvmuClock_ {
    EvmuMemory_*        pMemory;

    EvmuClockEvent      event;
    EvmuClockSignal_    signals[EVMU_CLOCK_SIGNAL_COUNT];
} EvmuClock_;


#if 0

When switching the system clock to the stopped quartz oscillator, a wait period is required to allow the
oscillator to stabilize. For the quartz oscillator in the VMU (32.678 kHz), this wait period is approx. 200 ms.

// these signals need to have a corresponding stable/valid stream for when clock isn't stable.
    //MCLK;
    //SCLK;

//const EvmuPeripheralDriver* evmuCpuDriver_(void) {
    static const EvmuPeripheralDriver evmuClockDriver_ = {
        EVMU_PERIPHERAL_CLOCK,
        "Clock Subystem",
        "Clocks!!!",
    };

    // set system clock
    // halt/resume not handled here?


    // clock info
    // frequency
    // TCyc in ms
    // tolerance
    // isStable

    // convert deltaTime to cycles
    // convert cycles to Ticks

    // Return 0 if disabled or still return shit anyway?

    // oscillator tolerance/variability
    // oscillator restabilizing cycle period (200mhz or some shit for swapping to RC)


    // YOU CANNOT READ/WRITE TO FLASH WITHOUT BEING IN A PARTICULAR CLOCK MODE!
    // MAKE SURE TO ERROR CHECK

    // if system2, puts into halt
    // if quartz or system1, shits
    // MAKE SURE TO EMIT ALL OF THE EVENTS WHEN SHIT CHANGES

    GBL_DECLARE_ENUM(EVMU_CLOCK_GENERATOR_PROPERTY) {
        EVMU_CLOCK_GENERATOR_PROPERTY_HALTED = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
        EVMU_CLOCK_GENERATOR_PROPERTY_SYSTEM_CLOCK1_ENABLED,
        EVMU_CLOCK_GENERATOR_PROPERTY_SYSTEM_CLOCK2_ENABLED,
        EVMU_CLOCK_GENERATOR_PROPERTY_OSCILLATOR_RC_ENABLED,
        EVMU_CLOCK_GENERATOR_PROPERTY_OSCILLATOR_CF_ENABLED,
        EVMU_CLOCK_GENERATOR_PROPERTY_SYSTEM_CLOCK_DIVISOR,
        EVMU_CLOCK_GENERATOR_PROPERTY_SYSTEM_CLOCK_OSCILLATOR,
        EVMU_CLOCK_GENERATOR_PROPERTY_SYSTEM_CLOCK_FREQUENCY,
        EVMU_CLOCK_GENERATOR_PROPERTY_COUNT
    };


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
#endif

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CLOCK__H

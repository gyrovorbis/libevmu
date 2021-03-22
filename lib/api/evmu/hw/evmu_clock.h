#ifndef EVMU_CLOCK_H
#define EVMU_CLOCK_H

#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

// YOU CANNOT READ/WRITE TO FLASH WITHOUT BEING IN A PARTICULAR CLOCK MODE!
// MAKE SURE TO ERROR CHECK

GBL_DECLARE_HANDLE(EvmuClock);

GBL_DECLARE_ENUM(EVMU_CLOCK_SOURCE) {
    EVMU_CLOCK_OSCILLATOR_QUARTZ,
    EVMU_CLOCK_OSCILLATOR_RC,
    EVMU_CLOCK_OSCILLATOR_CF, //Ceramic, Dreamcast
    EVMU_CLOCK_SYSTEM_1,
    EVMU_CLOCK_SYSTEM_2,
    EVMU_CLOCK_COUNT
};

GBL_DECLARE_ENUM(EVMU_CLOCK_SYSTEM_STATE) {
    EVMU_CLOCK_SYSTEM_RUNNING,
    EVMU_CLOCK_SYSTEM_HALT,
    EVMU_CLOCK_SYSTEM_HOLD
};

typedef struct EvmuClockStats {
    EvmuBool    stable;
    EvmuTicks   cycleTime;
    EvmuTicks   cycleFrequency;
    uint32_t    tolerance;
    uint32_t    currentConsumption; //microAmps
} EvmuClockStats;


GBL_DECLARE_ENUM(EVMU_CLOCK_DIVIDER) {
    EVMU_CLOCK_DIVIDER_12,
    EVMU_CLOCK_DIVIDER_6,
};

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


EVMU_API evmuClockSourceStats(EvmuClock hClock, EVMU_CLOCK_SOURCE source, EvmuClockStats* pStatus);

// if system2, puts into halt
// if quartz or system1, shits
// MAKE SURE TO EMIT ALL OF THE EVENTS WHEN SHIT CHANGES
EVMU_API evmuClockSourceActive(EvmuClock hClock, EVMU_CLOCK_SOURCE source, EvmuBool* pActive);
EVMU_API evmuClockSourceActiveSet(EvmuClock hClock, EVMU_CLOCK_SOURCE source, EvmuBool active);

EVMU_API evmuClockSystemConfig(EvmuClock hClock, EVMU_CLOCK_SOURCE* pSource, EVMU_CLOCK_DIVIDER* pDivider);
EVMU_API evmuClockSystemConfigSet(EvmuClock hClock, EVMU_CLOCK_SOURCE oscillator, EVMU_CLOCK_DIVIDER divider);

EVMU_API evmuClockSystemState(EvmuClock hClock, EVMU_CLOCK_SYSTEM_STATE* pState);
EVMU_API evmuClockSystemStateSet(EvmuClock hClock, EVMU_CLOCK_SYSTEM_STATE pState);

EVMU_API evmuClockTicksToCycles(EvmuClock hClock, EVMU_CLOCK_SOURCE source, EvmuTicks ticks, EvmuCycles* pCycles);
EVMU_API evmuClockCyclesToTicks(EvmuClock hClock, EVMU_CLOCK_SOURCE source, EvmuCycles cycles, EvmuTicks* pTicks);

EVMU_API evmuClockTimestepTicks(EvmuClock hClock, EvmuTicks* pTicks);

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

#ifdef __cplusplus
}
#endif


#endif // EVMU_CLOCK_H

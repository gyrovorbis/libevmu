#ifndef EVMU_CLOCK_H
#define EVMU_CLOCK_H

#include "../types/evmu_peripheral.h"
#include "evmu_wave.h"

#define EVMU_CLOCK_TYPE                 (GBL_TYPEOF(EvmuClock))
#define EVMU_CLOCK_NAME                 "clock"

#define EVMU_CLOCK(instance)            (GBL_INSTANCE_CAST(instance, EvmuClock))
#define EVMU_CLOCK_CLASS(klass)         (GBL_CLASS_CAST(klass, EvmuClock))
#define EVMU_CLOCK_GET_CLASS(instance)  (GBL_INSTANCE_GET_CLASS(instance, EvmuClock))

#define EVMU_CLOCK_OSC_QUARTZ_FREQ      32768    //hz
#define EVMU_CLOCK_OSC_RC_FREQ          879236   //hz

#define EVMU_CLOCK_OSC_QUARTZ_TCYC_1_12 366210   //ns per cycle
#define EVMU_CLOCK_OSC_QUARTZ_TCYC_1_6  183105   //ns per cycle

#define EVMU_CLOCK_OSC_RC_TCYC_1_12     12568    //ns per cycle
#define EVMU_CLOCK_OSC_RC_TCYC_1_6      6284     //ns per cycle

#define EVMU_CLOCK_OSC_QUARTZ_CURRENT   2600     //uA
#define EVMU_CLOCK_OSC_RC_CURRENT       610      //uA

#define GBL_SELF_TYPE EvmuClock

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_OSCILLATOR) {
    EVMU_OSCILLATOR_QUARTZ,
    EVMU_OSCILLATOR_RC,
    EVMU_OSCILLATOR_CF,
    EVMU_OSCILLATOR_COUNT
};

GBL_DECLARE_ENUM(EVMU_CLOCK_SIGNAL) {
//    EVMU_CLOCK_SIGNAL_OSCILLATOR_QUARTZ,
//    EVMU_CLOCK_SIGNAL_OSCILLATOR_RC,
//    EVMU_CLOCK_SIGNAL_OSCILLATOR_CF,
    EVMU_CLOCK_SIGNAL_CYCLE,
    EVMU_CLOCK_SIGNAL_SYSTEM_1 = EVMU_CLOCK_SIGNAL_CYCLE,
    EVMU_CLOCK_SIGNAL_SYSTEM_2,
    EVMU_CLOCK_SIGNAL_COUNT
};

GBL_DECLARE_ENUM(EVMU_CLOCK_SYSTEM_STATE) {
    EVMU_CLOCK_SYSTEM_STATE_UNKNOWN,
    EVMU_CLOCK_SYSTEM_STATE_RUNNING,
    EVMU_CLOCK_SYSTEM_STATE_HALT,
    EVMU_CLOCK_SYSTEM_STATE_HOLD,
    EVMU_CLOCK_SYSTEM_STATE_COUNT
};

GBL_DECLARE_ENUM(EVMU_CLOCK_DIVIDER) {
    EVMU_CLOCK_DIVIDER_1,
    EVMU_CLOCK_DIVIDER_12,
    EVMU_CLOCK_DIVIDER_6,
    EVMU_CLOCK_DIVIDER_COUNT
};

typedef struct EvmuOscillatorSpecs {
    EvmuCycles      hzReference;
    EvmuCycles      hzToleranceLow;
    EvmuCycles      hzToleranceHigh;
    EvmuCycles      stabilizationTicks;
    unsigned        currentMicroAmps;
} EvmuOscillatorSpecs;

typedef struct EvmuClockStats {
    GblBool     stable;
    EvmuTicks   cycleTime;
    EvmuTicks   cycleFrequency;
} EvmuClockStats;

GBL_CLASS_DERIVE_EMPTY   (EvmuClock, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuClock, EvmuPeripheral)

GBL_PROPERTIES(EvmuClock,
    (quartzState,       GBL_GENERIC, (READ),                    GBL_ENUM_TYPE),
    (quartzStable,      GBL_GENERIC, (READ),                    GBL_BOOL_TYPE),
    (rcState,           GBL_GENERIC, (READ),                    GBL_ENUM_TYPE),
    (rcStable,          GBL_GENERIC, (READ),                    GBL_BOOL_TYPE),
    (cfState,           GBL_GENERIC, (READ),                    GBL_ENUM_TYPE),
    (cfStable,          GBL_GENERIC, (READ),                    GBL_BOOL_TYPE),
    (cycleState,        GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_ENUM_TYPE),
    (cycleOscillator,   GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_ENUM_TYPE),
    (cycleFrequency,    GBL_GENERIC, (READ),                    GBL_UINT32_TYPE),
    (system2State,      GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_ENUM_TYPE),
    (system2Oscillator, GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_ENUM_TYPE),
    (system2Frequency,  GBL_GENERIC, (READ),                    GBL_UINT32_TYPE)
)

EVMU_EXPORT GblType     EvmuClock_type                (void)                                                               GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuClock_oscillatorSpecs     (GBL_CSELF, EVMU_OSCILLATOR oscillator, EvmuOscillatorSpecs* pSpecs) GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuClock_oscillatorActive    (GBL_CSELF, EVMU_OSCILLATOR oscillator)                              GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuClock_setOscillatorActive (GBL_CSELF, EVMU_OSCILLATOR oscillator, GblBool active)              GBL_NOEXCEPT;

EVMU_EXPORT EVMU_CLOCK_SYSTEM_STATE
                        EvmuClock_systemState         (GBL_CSELF)                                                          GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuClock_setSystemState      (GBL_CSELF, EVMU_CLOCK_SYSTEM_STATE state)                           GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuClock_systemConfig        (GBL_CSELF, EVMU_OSCILLATOR* pSource, EVMU_CLOCK_DIVIDER* pDivider)  GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuClock_setSystemConfig     (GBL_CSELF, EVMU_OSCILLATOR source, EVMU_CLOCK_DIVIDER divider)      GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuClock_signalStats         (GBL_CSELF, EVMU_CLOCK_SIGNAL signal, EvmuClockStats* pStatus)       GBL_NOEXCEPT;
EVMU_EXPORT EvmuWave    EvmuClock_signalWave          (GBL_CSELF, EVMU_CLOCK_SIGNAL signal)                                GBL_NOEXCEPT;
EVMU_EXPORT EvmuCycles  EvmuClock_signalTicksToCycles (GBL_CSELF, EVMU_CLOCK_SIGNAL signal, EvmuTicks ticks)               GBL_NOEXCEPT;
EVMU_EXPORT EvmuTicks   EvmuClock_signalCyclesToTicks (GBL_CSELF, EVMU_CLOCK_SIGNAL signal, EvmuCycles cycles)             GBL_NOEXCEPT;

EVMU_EXPORT uint64_t    EvmuClock_systemCyclesPerSec  (GBL_CSELF)                                                          GBL_NOEXCEPT;
EVMU_EXPORT double      EvmuClock_systemSecsPerCycle  (GBL_CSELF)                                                          GBL_NOEXCEPT;

EVMU_EXPORT EvmuTicks   EvmuClock_systemTicksPerCycle  (GBL_CSELF)                                                         GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CLOCK_H

#ifndef EVMU_CLOCK_H
#define EVMU_CLOCK_H

#include "../types/evmu_peripheral.h"

#define EVMU_CLOCK_TYPE                     (EvmuClock_type())
#define EVMU_CLOCK_STRUCT                   EvmuClock
#define EVMU_CLOCK_CLASS_STRUCT             EvmuClockClass
#define EVMU_CLOCK(inst)                    (GBL_INSTANCE_CAST_PREFIX  (inst,  EVMU_CLOCK))
#define EVMU_CLOCK_CHECK(inst)              (GBL_INSTANCE_CHECK_PREFIX (inst,  EVMU_CLOCK))
#define EVMU_CLOCK_CLASS(klass)             (GBL_CLASS_CAST_PREFIX     (klass, EVMU_CLOCK))
#define EVMU_CLOCK_CLASS_CHECK(klass)       (GBL_CLASS_CHECK_PREFIX    (klass, EVMU_CLOCK))
#define EVMU_CLOCK_GET_CLASS(inst)          (GBL_INSTANCE_CAST_CLASS_PREFIX (inst,  EVMU_CLOCK))

#define SELF    EvmuClock* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuClock_);

GBL_DECLARE_ENUM(EVMU_OSCILLATOR) {
    EVMU_OSCILLATOR_QUARTZ,
    EVMU_OSCILLATOR_RC,
    EVMU_OSCILLATOR_CF,
    EVMU_OSCILLATOR_COUNT
};

GBL_DECLARE_ENUM(EVMU_CLOCK_SIGNAL) {
    EVMU_CLOCK_SIGNAL_OSCILLATOR_QUARTZ,
    EVMU_CLOCK_SIGNAL_OSCILLATOR_RC,
    EVMU_CLOCK_SIGNAL_OSCILLATOR_CF,
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
    GblUint         currentMicroAmps;
} EvmuOscillatorSpecs;

typedef struct EvmuClockStats {
    GblBool     stable;
    EvmuTicks   cycleTime;
    EvmuTicks   cycleFrequency;
} EvmuClockStats;

typedef struct EvmuClockClass {
    EvmuPeripheralClass     base;
} EvmuClockClass;

typedef struct EvmuClock {
    union {
        EvmuClockClass*     pClass;
        EvmuPeripheral      base;
    };
    EvmuClock_*             pPrivate;
} EvmuClock;

GBL_EXPORT GblType      EvmuClock_type                  (void)                                                          GBL_NOEXCEPT;

EVMU_API                EvmuClock_oscillatorSpecs       (CSELF, EVMU_OSCILLATOR oscillator, EvmuOscillatorSpecs* pSpecs)GBL_NOEXCEPT;
GBL_EXPORT GblBool      EvmuClock_oscillatorActive      (CSELF, EVMU_OSCILLATOR oscillator)                             GBL_NOEXCEPT;
EVMU_API                EvmuClock_oscillatorActiveSet   (CSELF, EVMU_OSCILLATOR oscillator, GblBool active)             GBL_NOEXCEPT;

GBL_EXPORT EVMU_CLOCK_SYSTEM_STATE
                        EvmuClock_systemState           (CSELF)                                                         GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT  EvmuClock_systemStateSet        (CSELF, EVMU_CLOCK_SYSTEM_STATE state)                          GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT  EvmuClock_systemConfig          (CSELF, EVMU_OSCILLATOR* pSource, EVMU_CLOCK_DIVIDER* pDivider) GBL_NOEXCEPT;
GBL_EXPORT EVMU_RESULT  EvmuClock_systemConfigSet       (CSELF, EVMU_OSCILLATOR source, EVMU_CLOCK_DIVIDER divider)     GBL_NOEXCEPT;

GBL_EXPORT EVMU_RESULT  EvmuClock_signalStats           (CSELF, EVMU_CLOCK_SIGNAL signal, EvmuClockStats* pStatus)      GBL_NOEXCEPT;
GBL_EXPORT EvmuWave     EvmuClock_signalWave            (CSELF, EVMU_CLOCK_SIGNAL signal)                               GBL_NOEXCEPT;
GBL_EXPORT EvmuCycles   EvmuClock_signalTicksToCycles   (CSELF, EVMU_CLOCK_SIGNAL signal, EvmuTicks ticks)              GBL_NOEXCEPT;
GBL_EXPORT EvmuTicks    EvmuClock_signalCyclesToTicks   (CSELF, EVMU_CLOCK_SIGNAL signal, EvmuCycles cycles)            GBL_NOEXCEPT;

GBL_EXPORT EvmuTicks    EvmuClock_timestepTicks         (CSELF)                                                         GBL_NOEXCEPT;

GBL_DECLS_END


#undef CSELF
#undef SELF


#endif // EVMU_CLOCK_H

#ifndef EVMU_TIMERS__H
#define EVMU_TIMERS__H

#include <evmu/hw/evmu_timers.h>

#define EVMU_TIMERS_(instance)      ((EvmuTimers_*)GBL_INSTANCE_PRIVATE(instance, EVMU_TIMERS_TYPE))
#define EVMU_TIMERS_PUBLIC_(priv)   ((EvmuTimers*)GBL_INSTANCE_PUBLIC(priv, EVMU_TIMERS_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_STRUCT(EvmuTimer) {
    int         tl;
    int         th;
};

GBL_DECLARE_STRUCT(EvmuTimer0) {
    EvmuTimer base;
    int       tbase;
    int       tscale;
};

GBL_DECLARE_STRUCT(EvmuTimer1) {
    EvmuTimer base;
};

GBL_DECLARE_STRUCT(EvmuBaseTimer) {
    float tBaseDeltaTime;
    float tBase1DeltaTime;
    uint8_t tl;
    uint8_t th;
};

GBL_DECLARE_STRUCT(EvmuTimers_) {
    EvmuMemory_*   pMemory;

    EvmuTimer0    timer0;
    EvmuTimer1    timer1;
    EvmuBaseTimer baseTimer;
};

GBL_DECLS_END

#endif // EVMU_TIMERS__H

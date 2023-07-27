#ifndef EVMU_TIMERS__H
#define EVMU_TIMERS__H

#include <evmu/hw/evmu_timers.h>

#define EVMU_TIMERS_(instance)      (GBL_PRIVATE(EvmuTimers, instance))
#define EVMU_TIMERS_PUBLIC_(priv)   (GBL_PUBLIC(EvmuTimers, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);
GBL_FORWARD_DECLARE_STRUCT(EvmuBuzzer_);

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
    EvmuRam_*     pRam;
    EvmuBuzzer_*  pBuzzer;
    EvmuTimer0    timer0;
    EvmuTimer1    timer1;
    EvmuBaseTimer baseTimer;
};

GBL_DECLS_END

#endif // EVMU_TIMERS__H

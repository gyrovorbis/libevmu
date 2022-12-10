#ifndef EVMU_BUZZER__H
#define EVMU_BUZZER__H

#include <evmu/hw/evmu_buzzer.h>

#define EVMU_BUZZER_WAV_BUFFER_SIZE_    256

#define EVMU_BUZZER_(instance)      ((EvmuBuzzer_*)GBL_INSTANCE_PRIVATE(instance, EVMU_BUZZER_TYPE))
#define EVMU_BUZZER_PUBLIC(priv)    ((EvmuBuzzer*)GBL_INSTANCE_PUBLIC(priv, EVMU_BUZZER_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_STRUCT(EvmuBuzzer_) {
    uint8_t                 wavBuffer[EVMU_BUZZER_WAV_BUFFER_SIZE_];
    struct GYAudioSource*   audioSrc;
    struct GYAudBuffer*     audioBuff;
    EvmuMemory_*            pMemory;
    float                   noteDuration;
    float                   noteElapsed;
    GblBool                 enabled;
    GblBool                 playing;
    int                     pulseWidth;
    int                     period;
    unsigned                bpm;
    GblBool                 changed;
};

void EvmuBuzzer__memorySink(EvmuBuzzer_* pSelf_, EvmuAddress address, EvmuWord value);

GBL_DECLS_END

#endif // EVMU_DEVICE_BUZZER__H

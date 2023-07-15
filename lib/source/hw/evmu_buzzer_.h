#ifndef EVMU_BUZZER__H
#define EVMU_BUZZER__H

#include <evmu/hw/evmu_buzzer.h>

#define EVMU_BUZZER_(instance)      ((EvmuBuzzer_*)GBL_INSTANCE_PRIVATE(instance, EVMU_BUZZER_TYPE))
#define EVMU_BUZZER_PUBLIC_(priv)   ((EvmuBuzzer*)GBL_INSTANCE_PUBLIC(priv, EVMU_BUZZER_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuBuzzer_) {
    uint8_t      pcmBuffer[EVMU_BUZZER_PCM_BUFFER_SIZE];
    EvmuRam_*    pRam;
    GblBool      enabled;
    GblBool      active;
    uint16_t     tonePeriod;
    uint8_t      toneInvPulseLength;
    size_t       pcmSamples;
    size_t       pcmFrequency;
};

void EvmuBuzzer__memorySink_        (EvmuBuzzer_* pSelf_, EvmuAddress address, EvmuWord value);
void EvmuBuzzer__timer1Mode1Reload_ (EvmuBuzzer_* pSelf_);

GBL_DECLS_END

#endif // EVMU_DEVICE_BUZZER__H

#ifndef EVMU_BUZZER_H
#define EVMU_BUZZER_H

#include "../types/evmu_peripheral.h"

#define EVMU_BUZZER_TYPE                (GBL_TYPEOF(EvmuBuzzer))
#define EVMU_BUZZER_NAME                "buzzer"

#define EVMU_BUZZER(instance)           (GBL_INSTANCE_CAST(instance, EvmuBuzzer))
#define EVMU_BUZZER_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuBuzzer))
#define EVMU_BUZZER_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuBuzzer))

#define GBL_SELF_TYPE EvmuBuzzer

GBL_DECLS_BEGIN

/*  I THINK MAYBE BASE TIMER IS DRIVING THIS BITCH IN MAPLE MODE!
 *  THAT WOULD EXPLAIN THE OUTPUT FEEDING INTO BUZZER AND THE 16384 DIVIDER
 *  MAYBE THE 2 MAPLE VALUES FEED DIRECTLY INTO BASE TIMER CONFIG REG
 */

GBL_DECLARE_ENUM(EVMU_BUZZER_NOTE) {
    EVMU_BUZZER_NOTE_C3,
    EVMU_BUZZER_NOTE_D3,
    EVMU_BUZZER_NOTE_E3,
    EVMU_BUZZER_NOTE_F3,
    EVMU_BUZZER_NOTE_G3,
    EVMU_BUZZER_NOTE_A3,
    EVMU_BUZZER_NOTE_B3,
    EVMU_BUZZER_NOTE_C4,
    EVMU_BUZZER_NOTE_D4,
    EVMU_BUZZER_NOTE_E4,
    EVMU_BUZZER_NOTE_F4,
    EVMU_BUZZER_NOTE_G4,
    EVMU_BUZZER_NOTE_A4,
    EVMU_BUZZER_NOTE_B4,
    EVMU_BUZZER_NOTE_COUNT
};

GBL_DECLARE_ENUM(EVMU_BUZZER_NOTE_TIME) {
    EVMU_BUZZER_NOTE_WHOLE,
    EVMU_BUZZER_NOTE_HALF,
    EVMU_BUZZER_NOTE_QUARTER,
    EVMU_BUZZER_NOTE_EIGHTH,
    EVMU_BUZZER_NOTE_SIXTEENTH
};

GBL_CLASS_DERIVE_EMPTY   (EvmuBuzzer, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY(EvmuBuzzer, EvmuPeripheral)

GBL_PROPERTIES(EvmuBuzzer,
    (enabled,           GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (active,            GBL_GENERIC, (READ),        GBL_BOOL_TYPE),
    (userMode,          GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (period,            GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (pulseTime,         GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (waveSamples,       GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (waveActiveSamples, GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (freqResponse,      GBL_GENERIC, (READ),        GBL_UINT8_TYPE)
)

EVMU_EXPORT GblType     EvmuBuzzer_type           (void)                       GBL_NOEXCEPT;
EVMU_EXPORT uint16_t    EvmuBuzzer_notePeriod     (EVMU_BUZZER_NOTE note)      GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuBuzzer_enabled        (GBL_CSELF)                  GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuBuzzer_setEnabled     (GBL_SELF, GblBool enabled)  GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuBuzzer_waveChanged    (GBL_CSELF)                  GBL_NOEXCEPT;
EVMU_EXPORT void        EvmuBuzzer_setWaveChanged (GBL_SELF, GblBool changed)  GBL_NOEXCEPT;

EVMU_EXPORT const void* EvmuBuzzer_waveBuffer     (GBL_CSELF)                  GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuBuzzer_waveSamples    (GBL_CSELF)                  GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuBuzzer_active         (GBL_CSELF)                  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_play           (GBL_SELF,
                                                   uint16_t period,
                                                   uint8_t pulseWidth)         GBL_NOEXCEPT;

EVMU_EXPORT void        EvmuBuzzer_stop           (GBL_SELF)                   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playNote       (GBL_SELF,
                                                   EVMU_BUZZER_NOTE note,
                                                   EVMU_BUZZER_NOTE_TIME time) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BUZZER_H

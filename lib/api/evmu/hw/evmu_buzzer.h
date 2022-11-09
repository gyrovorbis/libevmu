#ifndef EVMU_BUZZER_H
#define EVMU_BUZZER_H

#include "../types/evmu_peripheral.h"

#define EVMU_BUZZER_TYPE                (GBL_TYPEOF(EvmuBuzzer))

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

EVMU_EXPORT GblType     EvmuBuzzer_type        (void)                       GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuBuzzer_enabled     (GBL_CSELF)                  GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuBuzzer_active      (GBL_CSELF)                  GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuBuzzer_userMode    (GBL_CSELF)                  GBL_NOEXCEPT;

EVMU_EXPORT void        EvmuBuzzer_setUserMode (GBL_CSELF,
                                                GblBool enabled)            GBL_NOEXCEPT;

EVMU_EXPORT const void* EvmuBuzzer_waveBuffer  (GBL_CSELF)                  GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuBuzzer_waveSamples (GBL_CSELF)                  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_play        (GBL_CSELF,
                                                uint8_t period,
                                                uint8_t pulseWidth)         GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playNote    (GBL_CSELF,
                                                EVMU_BUZZER_NOTE note,
                                                EVMU_BUZZER_NOTE_TIME time) GBL_NOEXCEPT;


GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // GYRO_VMU_SOUND_H

//============PRIVATE INTERNAL===================
#if 0
typedef struct EVMUBuzzer {
    struct GYAudioSource*   _audioSrc;
    struct GYAudBuffer*     _audioBuff;
    float                   _noteDuration;
    float                   _noteElapsed;
    int                     _enabled;
    int                     _soundPlaying;
    int                     _pulseWidth;
    int                     _period;
    unsigned                _bpm;
} EvmuBuzzer;



int     gyVmuBuzzerInit(struct VMUDevice* dev);
int     gyVmuBuzzerUninit(struct VMUDevice* dev);
void    gyVmuBuzzerReset(struct VMUDevice* dev);
void    gyVmuBuzzerMemorySink(struct VMUDevice* dev, int addr, uint8_t value);
void    gyVmuBuzzerUpdate(struct VMUDevice* dev, float deltaTime);

//EVMU_RESULT evmuBuzzerMode(const EvmuDevice* pDevice, EVMU_BUZZER_MODE* pMode);
//EVMU_RESULT evmuBuzzerModeSet(EvmuDevice* pDevice, EVMU_BUZZER_MODE mode);

EVMU_RESULT evmuBuzzerState(const EvmuDevice* pDevice, EVMU_BUZZER_STATE* pState);
EVMU_RESULT evmuBuzzerStateSet(EvmuDevice* pDevice, EVMU_BUZZER_STATE pState);



void    gyVmuBuzzerSoundPlay(struct VMUDevice* dev, int period, int pulseWidth, int db);
void    gyVmuBuzzerSoundPlayNote(struct VMUDevice* dev, EVMU_BUZZER_NOTE note, EVMU_BUZZER_NOTE_DURATION duration);


void    gyVmuBuzzerSoundBeatsPerMinuteSet(struct VMUDevice* dev, unsigned bpm);
void    gyVmuBuzzerSoundStop(struct VMUDevice* dev);
int     gyVmuBuzzerIsSoundPlaying(struct VMUDevice* dev);
int     gyVmuBuzzerEnabled(const struct VMUDevice* dev);
void    gyVmuBuzzerEnabledSet(struct VMUDevice* dev, int value);
uint8_t gyVmuBuzzerT1LRValueFromNote(EVMU_BUZZER_NOTE note);
#endif


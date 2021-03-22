#ifndef EVMU_BUZZER_H
#define EVMU_BUZZER_H

#include <stdint.h>
#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  I THINK MAYBE BASE TIMER IS DRIVING THIS BITCH IN MAPLE MODE!
 *  THAT WOULD EXPLAIN THE OUTPUT FEEDING INTO BUZZER AND THE 16384 DIVIDER
 *  MAYBE THE 2 MAPLE VALUES FEED DIRECTLY INTO BASE TIMER CONFIG REG
 */


struct GYAudioSource;
struct GYAudBuffer;
struct VMUDevice;

typedef enum EVMU_BUZZER_STATE {
    EVMU_BUZZER_IDLE        = 0x0,    // Enabled but not playing anything
    EVMU_BUZZER_ACTIVE      = 0x1,    // Enabled and playing from ROM
} EVMU_BUZZER_STATE;

//differentiate between buzzer being disabled by so

typedef enum EVMU_BUZZER_EMULATION_MODE {
    EVMU_BUZZER_EMULATION_MODE_DISABLED,
    EVMU_BUZZER_EMULATION_MODE_EMULATOR,
    EVMU_BUZZER_EULATION_MODE_DIRECT,
} EVMU_BUZZER_EMULATION_MODE;

typedef enum EVMU_BUZZER_NOTE {
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
} EVMU_BUZZER_NOTE;

typedef enum EVMU_BUZZER_NOTE_DURATION {
    EVMU_BUZZER_NOTE_WHOLE,
    EVMU_BUZZER_NOTE_HALF,
    EVMU_BUZZER_NOTE_QUARTER,
    EVMU_BUZZER_NOTE_EIGHTH,
    EVMU_BUZZER_NOTE_SIXTEENTH
} EVMU_BUZZER_NOTE_DURATION;

GBL_DECLARE_HANDLE(EvmuBuzzer);

GBL_DECLARE_ENUM(EVMU_BUZZER_PROPERTY) {
    EVMU_BUZZER_PROPERTY_NOTE_DURATION = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_BUZZER_PROPERTY_NOTE_ELAPSED,
    EVMU_BUZZER_PROPERTY_NOTE_STATE, //idle, playing
    EVMU_BUZZER_PROPERTY_PULSE_WIDTH,
    EVMU_BUZZER_PROPERTY_PERIOD,
    EVMU_BUZZER_PROPERTY_BPM,
    EVMU_BUZZER_PROPERTY_COUNT
};


//EVMU_API evmuBuzzerCaptureBegin();
//EVMU_API evmuBuzzerCaptureEnd();


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
#endif
// ================== PUBLIC API ===============

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


#ifdef __cplusplus
}
#endif



#endif // GYRO_VMU_SOUND_H


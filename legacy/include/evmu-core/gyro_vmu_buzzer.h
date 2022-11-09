#ifndef GYRO_VMU_BUZZER_H
#define GYRO_VMU_BUZZER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GYAudioSource;
struct GYAudBuffer;
struct VMUDevice;

typedef enum GY_VMU_BUZZER_NOTE {
    GY_VMU_BUZZER_NOTE_C3,
    GY_VMU_BUZZER_NOTE_D3,
    GY_VMU_BUZZER_NOTE_E3,
    GY_VMU_BUZZER_NOTE_F3,
    GY_VMU_BUZZER_NOTE_G3,
    GY_VMU_BUZZER_NOTE_A3,
    GY_VMU_BUZZER_NOTE_B3,
    GY_VMU_BUZZER_NOTE_C4,
    GY_VMU_BUZZER_NOTE_D4,
    GY_VMU_BUZZER_NOTE_E4,
    GY_VMU_BUZZER_NOTE_F4,
    GY_VMU_BUZZER_NOTE_G4,
    GY_VMU_BUZZER_NOTE_A4,
    GY_VMU_BUZZER_NOTE_B4,
    GY_VMU_BUZZER_NOTE_COUNT
} GY_VMU_BUZZER_NOTE;

typedef enum GY_VMU_BUZZER_NOTE_DURATION {
    GY_VMU_BUZZER_NOTE_WHOLE,
    GY_VMU_BUZZER_NOTE_HALF,
    GY_VMU_BUZZER_NOTE_QUARTER,
    GY_VMU_BUZZER_NOTE_EIGHTH,
    GY_VMU_BUZZER_NOTE_SIZTEENTH
} GY_VMU_BUZZER_NOTE_DURATION;

typedef struct VMUBuzzer {
    struct GYAudioSource*   _audioSrc;
    struct GYAudBuffer*     _audioBuff;
    float                   _noteDuration;
    float                   _noteElapsed;
    int                     _enabled;
    int                     _soundPlaying;
    int                     _pulseWidth;
    int                     _period;
    unsigned                _bpm;
} VMUBuzzer;

int     gyVmuBuzzerInit(struct VMUDevice* dev);
int     gyVmuBuzzerUninit(struct VMUDevice* dev);
void    gyVmuBuzzerReset(struct VMUDevice* dev);

void    gyVmuBuzzerMemorySink(struct VMUDevice* dev, int addr, uint8_t value);
void    gyVmuBuzzerSoundPlay(struct VMUDevice* dev, int period, int pulseWidth, int db);
void    gyVmuBuzzerSoundPlayNote(struct VMUDevice* dev, GY_VMU_BUZZER_NOTE note, GY_VMU_BUZZER_NOTE_DURATION duration);
void    gyVmuBuzzerSoundBeatsPerMinuteSet(struct VMUDevice* dev, unsigned bpm);
void    gyVmuBuzzerSoundStop(struct VMUDevice* dev);
int     gyVmuBuzzerIsSoundPlaying(struct VMUDevice* dev);
int     gyVmuBuzzerEnabled(const struct VMUDevice* dev);
void    gyVmuBuzzerEnabledSet(struct VMUDevice* dev, int value);
uint8_t gyVmuBuzzerT1LRValueFromNote(GY_VMU_BUZZER_NOTE note);
void    gyVmuBuzzerUpdate(struct VMUDevice* dev, float deltaTime);


#ifdef __cplusplus
}
#endif



#endif // GYRO_VMU_SOUND_H


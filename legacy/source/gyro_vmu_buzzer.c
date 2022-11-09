#include "gyro_vmu_buzzer.h"
#include "gyro_vmu_sfr.h"
#include <gyro_system_api.h>
#include <gyro_matrix_api.h>
#include <gyro_audio_api.h>
#include <gyro_vmu_device.h>
#include <al/gyro_al.h>
#include <assert.h>
#include <string.h>

static unsigned char _freqResponse[0xff] = {
    [0xe0 ... 0xe6] = 62,
    [0xe7 ... 0xe8] = 63,
    [0xe9]          = 64,
    [0xea ... 0xec] = 63,
    [0xed ... 0xef] = 64,
    [0xf0]          = 66,
    [0xf1]          = 65,
    [0xf2]          = 66,
    [0xf3 ... 0xf6] = 64,
    [0xf7]          = 65,
    [0xf8]          = 66,
    [0xf9]          = 69,
    [0xfa]          = 71,
    [0xfb]          = 72,
    [0xfc]          = 70,
    [0xfd]          = 69,
    [0xfe]          = 66
};

static uint8_t _noteTL1RLut[GY_VMU_BUZZER_NOTE_COUNT] = {
    [GY_VMU_BUZZER_NOTE_C4] = 0xeb, //261.63Hz actual (260.213Hz VMU)
    [GY_VMU_BUZZER_NOTE_D4] = 0xed, //293.66Hz actual (287.604Hz VMU)
    [GY_VMU_BUZZER_NOTE_E4] = 0xef, //329.63Hz actual (321.220Hz VMU)
    [GY_VMU_BUZZER_NOTE_F4] = 0xf0, //349.23Hz actual (341.530Hz VMU)
    [GY_VMU_BUZZER_NOTE_G4] = 0xf2, //392.00Hz actual (390.320Hz VMU)
    [GY_VMU_BUZZER_NOTE_A4] = 0xf4, //440.00Hz actual (455.373Hz VMU)
    [GY_VMU_BUZZER_NOTE_B4] = 0xf5, //493.88Hz actual (496.771Hz VMU)
};


uint8_t gyVmuBuzzerT1LRValueFromNote(GY_VMU_BUZZER_NOTE note) {
    return _noteTL1RLut[note];
}

void gyVmuBuzzerSoundPlayNote(struct VMUDevice* dev, GY_VMU_BUZZER_NOTE note, GY_VMU_BUZZER_NOTE_DURATION duration) {
    uint8_t period = 256 - gyVmuBuzzerT1LRValueFromNote(note);
    gyVmuBuzzerSoundPlay(dev, period, period/2, 1);
}


void gyVmuBuzzerMemorySink(VMUDevice *dev, int addr, uint8_t value) {
    (void)value;

    //Check for buzzer state change
    if(addr == SFR_ADDR_T1CNT || addr == SFR_ADDR_T1LR || addr == SFR_ADDR_T1LC) {
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)]&SFR_T1CNT_T1LRUN_MASK) {
            if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)]&SFR_T1CNT_ELDT1C_MASK) { //This bit must be set for buzzer output changes to be applied
                //if(mode == VMU_TIMER1_MODE_TIMER8_PULSE8) - Yes, this really should be correct. Must be in Mode #1.
                int pulseWidth = (dev->sfr[SFR_OFFSET(SFR_ADDR_T1LC)] - dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)]);
                int cycleLength = (256-dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)]);
                //183us with 32khz clock
                if((dev->sfr[SFR_OFFSET(SFR_ADDR_P1DDR)]&SFR_P1DDR_P17DDR_MASK)) {
                    if(dev->sfr[SFR_OFFSET(SFR_ADDR_P1FCR)]&SFR_P1FCR_P17FCR_MASK) {
                        gyVmuBuzzerSoundPlay(dev, cycleLength, pulseWidth, 0);
                    }
                }
            }
        } else {
            gyVmuBuzzerSoundStop(dev); //Stop sound output
        }
    }
}

int gyVmuBuzzerIsSoundPlaying(VMUDevice* dev) {
    return dev->buzzer._soundPlaying;
}

void gyVmuBuzzerSoundPlay(VMUDevice* dev, int period, int pulseWidth, int db) {
    if(db == -1) {
        db = (pulseWidth == period*0.5f && db == -1)? _freqResponse[256-period] : 0;
    }

    if(!dev->buzzer._enabled) return;

    //Check start or stop
    if(period > 0) {

        //ensure sane sound wave
        if(pulseWidth >= 0 && pulseWidth <= period) {

            //Check to see if wave is different from what's in the audio buffer
            if(dev->buzzer._period != period || dev->buzzer._pulseWidth != pulseWidth) {
                float   periodSample  = 0.000183f;
                float   freqSample    = roundf(1.0f/periodSample);
                int     freqSamplei   = freqSample;
                float   periodSecs    = period*periodSample;
                float   samples       = freqSample*periodSecs;
                int     sampleSize    = roundf(samples);
                float   invDutyCycle  = (float)pulseWidth/(float)period;
                int     activeCycle   = roundf(invDutyCycle*(float)sampleSize);

                unsigned char buff[sampleSize];

                memset(buff, 0x7f, activeCycle);
                memset(&buff[activeCycle], 0xff, sampleSize-activeCycle);

                //Stop any other sound we're playing
                gyVmuBuzzerSoundStop(dev);
                //Upload buffer data to sound card
                alSourcei(*(ALuint*)dev->buzzer._audioSrc, AL_BUFFER, 0);
                gyAudBufferData(dev->buzzer._audioBuff, buff, AL_FORMAT_MONO8, sampleSize, freqSamplei);
                //cache wave data for buffer
                dev->buzzer._period         = period;
                dev->buzzer._pulseWidth     = pulseWidth;
            }

            //If we aren't already playing the sound wave, start playing it.
            if(!dev->buzzer._soundPlaying) {
                gyAudSourcePlayBuffer(dev->buzzer._audioSrc, dev->buzzer._audioBuff);
                dev->buzzer._soundPlaying = 1;
            }
        } else {
            _gyLog(GY_DEBUG_WARNING, "Attempting to play malformed sound wave [period: %d, pulseWidth: %d]", period, pulseWidth);
            //assert(0); //malformed sound wave!
        }


    } else { //stop audio if currently playing
        gyVmuBuzzerSoundStop(dev);
    }

}

void gyVmuBuzzerSoundStop(VMUDevice* dev) {
    if(dev->buzzer._soundPlaying) {
        gyAudSourceStop(dev->buzzer._audioSrc);
        dev->buzzer._soundPlaying = 0;
    }
}

int gyVmuBuzzerInit(VMUDevice* dev) {
    dev->buzzer._audioSrc       = gyAudSourceCreate();
    dev->buzzer._audioBuff      = gyAudBufferCreate();
    dev->buzzer._enabled        = 1;
    dev->buzzer._soundPlaying   = 0;
    dev->buzzer._period         = -1;
    dev->buzzer._pulseWidth     = -1;
    dev->buzzer._bpm            = 100;
    dev->buzzer._noteDuration   = -1.0f;
    dev->buzzer._noteElapsed    = 0.0f;

    GYVector3 pos = { 0.0f, 0.0f, 0.0f };
    gyAudListenerSetPos(&pos);
    gyAudSetReferenceDistance(0);
    gyAudSourceSetfv(dev->buzzer._audioSrc, GY_SOURCE_POS, &pos);
    gyAudSourceSetfv(dev->buzzer._audioSrc, GY_SOURCE_VEL, &pos);
    gyAudSourceSetf(dev->buzzer._audioSrc, GY_SOURCE_ROLLOFF_FACTOR, 0.0f);
    gyAudSourceSetf(dev->buzzer._audioSrc, GY_SOURCE_GAIN, 1.0f);
    gyAudSourceSetf(dev->buzzer._audioSrc, GY_SOURCE_PITCH, 1.0f);
    gyAudSourceSeti(dev->buzzer._audioSrc, GY_SOURCE_LOOPING, 1);
    return 1;
}

int gyVmuBuzzerUninit(VMUDevice* dev) {
    gyAudSourceDestroy(dev->buzzer._audioSrc);
    gyAudBufferDestroy(dev->buzzer._audioBuff);
    return 1;
}

void gyVmuBuzzerReset(VMUDevice* dev) {
    gyVmuBuzzerSoundStop(dev);
}

int gyVmuBuzzerEnabled(const VMUDevice* dev) {
    return dev->buzzer._enabled;
}

void gyVmuBuzzerEnabledSet(VMUDevice* dev, int value) {
    dev->buzzer._enabled = value;
}

void gyVmuBuzzerUpdate(struct VMUDevice* dev, float deltaTime) {
    if(dev->buzzer._noteDuration != -1.0f) {
        dev->buzzer._noteElapsed += deltaTime;
        if(dev->buzzer._noteElapsed >= deltaTime) {
            gyVmuBuzzerSoundStop(dev);
        }
    }

}

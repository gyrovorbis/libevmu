#include <evmu/hw/evmu_buzzer.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_device_.h"
#include "evmu_buzzer_.h"
#include "evmu_memory_.h"
#include <string.h>
#include <gyro_audio_api.h>
#include <gyro_matrix_api.h>

#ifdef EVMU_ENABLE_TESTS
#     define EVMU_BUZZER_DISABLED_
#endif

static unsigned char freqResponse_[0xff] = {
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

static uint8_t noteTL1RLut_[EVMU_BUZZER_NOTE_COUNT] = {
    [EVMU_BUZZER_NOTE_C4] = 0xeb, //261.63Hz actual (260.213Hz VMU)
    [EVMU_BUZZER_NOTE_D4] = 0xed, //293.66Hz actual (287.604Hz VMU)
    [EVMU_BUZZER_NOTE_E4] = 0xef, //329.63Hz actual (321.220Hz VMU)
    [EVMU_BUZZER_NOTE_F4] = 0xf0, //349.23Hz actual (341.530Hz VMU)
    [EVMU_BUZZER_NOTE_G4] = 0xf2, //392.00Hz actual (390.320Hz VMU)
    [EVMU_BUZZER_NOTE_A4] = 0xf4, //440.00Hz actual (455.373Hz VMU)
    [EVMU_BUZZER_NOTE_B4] = 0xf5, //493.88Hz actual (496.771Hz VMU)
};


EVMU_EXPORT uint16_t EvmuBuzzer_notePeriod(EVMU_BUZZER_NOTE note) {
    return 256 - noteTL1RLut_[note];
}

void EvmuBuzzer__memorySink(EvmuBuzzer_* pSelf_, EvmuAddress addr, EvmuWord value) {
    GBL_UNUSED(value);

    //Check for buzzer state change
    if(addr == EVMU_ADDRESS_SFR_T1CNT || addr == EVMU_ADDRESS_SFR_T1LR || addr == EVMU_ADDRESS_SFR_T1LC) {
        if(pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LRUN_MASK) {
            if(pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_ELDT1C_MASK) { //This bit must be set for buzzer output changes to be applied
                //if(mode == VMU_TIMER1_MODE_TIMER8_PULSE8) - Yes, this really should be correct. Must be in Mode #1.
                int pulseWidth = (pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LC)] -
                                  pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]);
                int cycleLength = (256 - pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]);

                //183us with 32khz clock
                if((pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P1DDR)] & EVMU_SFR_P1DDR_P17DDR_MASK)) {
                    if(pSelf_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P1FCR)] & EVMU_SFR_P1FCR_P17FCR_MASK) {
                        EvmuBuzzer_play(EVMU_BUZZER_PUBLIC(pSelf_), cycleLength, pulseWidth);
                    }
                }
            }
        } else {
            EvmuBuzzer_stop(EVMU_BUZZER_PUBLIC(pSelf_)); //Stop sound output
        }
    }
}

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playNote(EvmuBuzzer* pSelf, EVMU_BUZZER_NOTE note, EVMU_BUZZER_NOTE_TIME duration) {
    GBL_UNUSED(duration);
    GBL_CTX_BEGIN(NULL);
    const uint16_t period = EvmuBuzzer_notePeriod(note);
    EvmuBuzzer_play(pSelf, period, period/2);
    GBL_CTX_END();
}


EVMU_EXPORT GblBool EvmuBuzzer_active(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->playing;
}

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_play(EvmuBuzzer* pSelf, uint16_t period, uint8_t pulseWidth) {
    GBL_CTX_BEGIN(NULL);

    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    GBL_UNUSED(freqResponse_);
    /*
    if(db == -1) {
        db = (pulseWidth == period*0.5f && db == -1)? freqResponse_[256-period] : 0;
    }
*/
    if(!pSelf_->enabled) GBL_CTX_DONE();

    //Check start or stop
    if(period > 0) {

        //ensure sane sound wave
        if(pulseWidth >= 0 && pulseWidth <= period) {

            //Check to see if wave is different from what's in the audio buffer
            if(pSelf_->period != period || pSelf_->pulseWidth != pulseWidth) {
                float   periodSample  = 0.000183f;
                float   freqSample    = roundf(1.0f/periodSample);
                int     freqSamplei   = freqSample;
                float   periodSecs    = period*periodSample;
                float   samples       = freqSample*periodSecs;
                int     sampleSize    = roundf(samples);
                float   invDutyCycle  = (float)pulseWidth/(float)period;
                int     activeCycle   = roundf(invDutyCycle*(float)sampleSize);

                GBL_ASSERT(sampleSize <= (int)sizeof(pSelf_->wavBuffer), "Wave buffer is too small!");

                memset(pSelf_->wavBuffer, 0x7f, activeCycle);
                memset(&pSelf_->wavBuffer[activeCycle], 0xff, sampleSize-activeCycle);

                //Stop any other sound we're playing
                EvmuBuzzer_stop(pSelf);
                //Upload buffer data to sound card
#ifndef EVMU_BUZZER_DISABLED_
                alSourcei(*(ALuint*)pSelf_->audioSrc, AL_BUFFER, 0);
                gyAudBufferData(pSelf_->audioBuff, pSelf_->wavBuffer, AL_FORMAT_MONO8, sampleSize, freqSamplei);
#endif
                //cache wave data for buffer
                pSelf_->period         = period;
                pSelf_->pulseWidth     = pulseWidth;
                pSelf_->changed        = GBL_TRUE;
            }

            //If we aren't already playing the sound wave, start playing it.
            if(!pSelf_->playing) {
#ifndef EVMU_BUZZER_DISABLED_
                gyAudSourcePlayBuffer(pSelf_->audioSrc, pSelf_->audioBuff);
#endif
                pSelf_->playing = GBL_TRUE;
            }

        } else {
            _gyLog(GY_DEBUG_WARNING, "Attempting to play malformed sound wave [period: %d, pulseWidth: %d]", period, pulseWidth);
            //assert(0); //malformed sound wave!
        }


    } else { //stop audio if currently playing
        EvmuBuzzer_stop(pSelf);
    }

    GBL_CTX_END();
}

EVMU_EXPORT void EvmuBuzzer_stop(EvmuBuzzer* pSelf) {
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);
    if(pSelf_->playing) {
#ifndef EVMU_BUZZER_DISABLED_
        gyAudSourceStop(pSelf_->audioSrc);
#endif
        pSelf_->playing = GBL_FALSE;
        pSelf_->changed = GBL_TRUE;
    }
}

EVMU_EXPORT GblBool EvmuBuzzer_enabled(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->enabled;
}

EVMU_EXPORT void EvmuBuzzer_setEnabled(EvmuBuzzer* pSelf, GblBool enabled) {
    EVMU_BUZZER_(pSelf)->enabled = enabled;
}

EVMU_EXPORT GblBool EvmuBuzzer_waveChanged(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->changed;
}

EVMU_EXPORT void EvmuBuzzer_setWaveChanged(EvmuBuzzer* pSelf, GblBool changed) {
    EVMU_BUZZER_(pSelf)->changed = changed;
}

static GBL_RESULT EvmuBuzzer_IBehavior_update_(EvmuIBehavior* pIBehavior, EvmuTicks ticks) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnUpdate, pIBehavior, ticks);

    EvmuBuzzer* pSelf = EVMU_BUZZER(pIBehavior);
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    const float deltaTime = 1.0f/(float)ticks;

    if(pSelf_->noteDuration != -1.0f) {
        pSelf_->noteElapsed += deltaTime;
        if(pSelf_->noteElapsed >= deltaTime) {
            EvmuBuzzer_stop(pSelf);
        }
    }

    GBL_CTX_END();
}


static GBL_RESULT EvmuBuzzer_IBehavior_reset_(EvmuIBehavior* pIBehavior) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pIBehavior);

    EvmuBuzzer_stop(EVMU_BUZZER(pIBehavior));

    GBL_CTX_END();
}


static GBL_RESULT EvmuBuzzer_GblObject_constructed_(GblObject* pObject) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pObject);

    EvmuBuzzer*  pSelf  = EVMU_BUZZER(pObject);
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    GblObject_setName(pObject, EVMU_BUZZER_NAME);

    pSelf_->enabled        = 1;
    pSelf_->playing        = 0;
    pSelf_->period         = -1;
    pSelf_->pulseWidth     = -1;
    pSelf_->bpm            = 100;
    pSelf_->noteDuration   = -1.0f;
    pSelf_->noteElapsed    = 0.0f;

#ifndef EVMU_BUZZER_DISABLED_
    pSelf_->audioSrc       = gyAudSourceCreate();
    pSelf_->audioBuff      = gyAudBufferCreate();    
    GYVector3 pos = { 0.0f, 0.0f, 0.0f };
    gyAudListenerSetPos(&pos);
    gyAudSetReferenceDistance(0);
    gyAudSourceSetfv(pSelf_->audioSrc, GY_SOURCE_POS, &pos);
    gyAudSourceSetfv(pSelf_->audioSrc, GY_SOURCE_VEL, &pos);
    gyAudSourceSetf(pSelf_->audioSrc, GY_SOURCE_ROLLOFF_FACTOR, 0.0f);
    gyAudSourceSetf(pSelf_->audioSrc, GY_SOURCE_GAIN, 1.0f);
    gyAudSourceSetf(pSelf_->audioSrc, GY_SOURCE_PITCH, 1.0f);
    gyAudSourceSeti(pSelf_->audioSrc, GY_SOURCE_LOOPING, 1);
#endif

    GBL_CTX_END();
}

static GBL_RESULT EvmuBuzzer_GblBox_destructor_(GblBox* pBox) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(GblBox, pFnDestructor, pBox);

    EvmuBuzzer*  pSelf  = EVMU_BUZZER(pBox);
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    gyAudSourceDestroy(pSelf_->audioSrc);
    gyAudBufferDestroy(pSelf_->audioBuff);

    GBL_CTX_END();
}

static GBL_RESULT EvmuBuzzerClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_BOX_CLASS(pClass)       ->pFnDestructor  = EvmuBuzzer_GblBox_destructor_;
    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuBuzzer_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuBuzzer_IBehavior_reset_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnUpdate      = EvmuBuzzer_IBehavior_update_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuBuzzer_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuBuzzerClass),
        .pFnClassInit           = EvmuBuzzerClass_init_,
        .instanceSize           = sizeof(EvmuBuzzer),
        .instancePrivateSize    = sizeof(EvmuBuzzer_)
    };

    if(!GblType_verify(type)) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuBuzzer"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}




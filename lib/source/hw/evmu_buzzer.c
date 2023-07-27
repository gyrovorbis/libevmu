#include <evmu/hw/evmu_buzzer.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_device_.h"
#include "evmu_buzzer_.h"
#include "evmu_ram_.h"

#include <string.h>

#include "../types/evmu_marshal_.h"

#define EVMU_BUZZER_FREQ_RESP_BASE_OFFSET_   0xe0
#define EVMU_BUZZER_FREQ_RESP_DEFAULT_VALUE_ 30
#define EVMU_BUZZER_FREQ_RESP_MAX_VALUE_     72

static uint8_t freqResponse_[0x1f] = {
    [0x00] = 62,
    [0x01] = 62,
    [0x02] = 62,
    [0x03] = 62,
    [0x04] = 62,
    [0x05] = 62,
    [0x06] = 62,
    [0x07] = 63,
    [0x08] = 63,
    [0x09] = 64,
    [0x0a] = 63,
    [0x0b] = 63,
    [0x0c] = 63,
    [0x0d] = 64,
    [0x0f] = 64,
    [0x10] = 66,
    [0x11] = 65,
    [0x12] = 66,
    [0x13] = 64,
    [0x14] = 64,
    [0x15] = 64,
    [0x16] = 64,
    [0x17] = 65,
    [0x18] = 66,
    [0x19] = 69,
    [0x1a] = 71,
    [0x1b] = 72,
    [0x1c] = 70,
    [0x1d] = 69,
    [0x1e] = 66
};

static void EvmuBuzzer_updateTone_(EvmuBuzzer* pSelf) {
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);
    const uint16_t period =
            (256 - pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]);

    const uint8_t invPulseLength =
            (pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LC)] -
             pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)]);

    EvmuBuzzer_setTone(pSelf, period, invPulseLength);
}

void EvmuBuzzer__timer1Mode1Reload_(EvmuBuzzer_* pSelf_) {
    EvmuBuzzer* pSelf = EVMU_BUZZER_PUBLIC_(pSelf_);

    if(EvmuBuzzer_isConfigured(pSelf)) {
        EvmuBuzzer_updateTone_(pSelf);
    }
}

void EvmuBuzzer__memorySink_(EvmuBuzzer_* pSelf_, EvmuAddress address, EvmuWord value) {
    GBL_UNUSED(value);
    EvmuBuzzer* pSelf = EVMU_BUZZER_PUBLIC_(pSelf_);

    switch(address) {
        default: break;
        case EVMU_ADDRESS_SFR_T1LR:
        case EVMU_ADDRESS_SFR_T1LC:
        case EVMU_ADDRESS_SFR_T1CNT:
        case EVMU_ADDRESS_SFR_P1DDR:
        case EVMU_ADDRESS_SFR_P1FCR:
        case EVMU_ADDRESS_SFR_P1:
            if(!EvmuBuzzer_isConfigured(pSelf) ||
                !(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LRUN_MASK))
            {
                EvmuBuzzer_stopTone(pSelf);
            } else {
                if(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_ELDT1C_MASK)
                    EvmuBuzzer_updateTone_(pSelf);
                if(!pSelf_->active)
                    EvmuBuzzer_playTone(pSelf);
            }

        break;
    }
}

EVMU_EXPORT GblBool EvmuBuzzer_isConfigured(const EvmuBuzzer* pSelf) {
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    // port direction configured (outpout)
    if(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P1DDR)] & EVMU_SFR_P1DDR_P17DDR_MASK)
        // port function configured (PWM output)
        if(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P1FCR)] & EVMU_SFR_P1FCR_P17FCR_MASK)
            // port latch (low)
            if(!(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P1)] & EVMU_SFR_P1_P17_MASK))
                // timer 1 configurated (8-bit counter mode)
                if(!(pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)] & EVMU_SFR_T1CNT_T1LONG_MASK))
                    return GBL_TRUE;

    return GBL_FALSE;
}

EVMU_EXPORT GblBool EvmuBuzzer_isEnable(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->enabled;
}

EVMU_EXPORT void EvmuBuzzer_setEnabled(EvmuBuzzer* pSelf, GblBool enabled) {
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    if(!enabled && pSelf_->enabled) {
        EvmuBuzzer_stopTone(pSelf);
    }

    pSelf_->enabled = enabled;
}

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playTone(EvmuBuzzer* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    if(pSelf_->enabled && !pSelf_->active) {
        GBL_VCALL(EvmuBuzzer, pFnPlayPcm, pSelf);
        pSelf_->active = GBL_TRUE;
    }

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_stopTone(EvmuBuzzer* pSelf) {
    GBL_CTX_BEGIN(pSelf);

    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    if(pSelf_->active) {
        EvmuBuzzer_setTone(pSelf, 255, 255);
    }

    GBL_CTX_END();
}

EVMU_EXPORT void EvmuBuzzer_tone(const EvmuBuzzer* pSelf, uint16_t* pPeriod, uint8_t* pInvPulseLength) {
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);
    *pPeriod = pSelf_->tonePeriod;
    *pInvPulseLength = pSelf_->toneInvPulseLength;
}

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_setTone(EvmuBuzzer* pSelf,
                                           uint16_t tonePeriod,
                                           uint8_t toneInvPulseLength) {
    GBL_CTX_BEGIN(pSelf);

    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    if(!pSelf_->enabled) GBL_CTX_DONE();

    //Check to see if wave is different from what's in the audio buffer
    if(pSelf_->tonePeriod != tonePeriod || pSelf_->toneInvPulseLength != toneInvPulseLength) {
        // Prevent crazy numbers from blowing up
        if(tonePeriod < toneInvPulseLength)
            tonePeriod = toneInvPulseLength;

        float   periodSample = 0.000183f;
        float   freqSample   = roundf(1.0f/periodSample);
        int     freqSamplei  = freqSample;
        float   periodSecs   = tonePeriod * periodSample;
        float   samples      = freqSample * periodSecs;
        int     sampleSize   = roundf(samples);
        float   invDutyCycle = (float)toneInvPulseLength/(float)tonePeriod;
        int     activeCycle  = roundf(invDutyCycle*(float)sampleSize);

        GBL_ASSERT(sampleSize <= (int)sizeof(pSelf_->pcmBuffer), "PCM buffer is too small!");

        memset(pSelf_->pcmBuffer, 0x7f, activeCycle);
        memset(&pSelf_->pcmBuffer[activeCycle], 0xff, sampleSize-activeCycle);

        //cache wave data for buffer
        pSelf_->tonePeriod         = tonePeriod;
        pSelf_->toneInvPulseLength = toneInvPulseLength;
        pSelf_->pcmSamples         = sampleSize;
        pSelf_->pcmFrequency       = freqSamplei;
        pSelf->pcmChanged          = GBL_TRUE;

        const GblBool flat = (!activeCycle || sampleSize == activeCycle);

        if(pSelf_->active)
            GBL_VCALL(EvmuBuzzer, pFnStopPcm, pSelf);

        //Upload buffer data to sound card
        GBL_VCALL(EvmuBuzzer, pFnBufferPcm, pSelf);

        if(pSelf_->active) {
            if(flat) pSelf_->active = GBL_FALSE;
            else GBL_VCALL(EvmuBuzzer, pFnPlayPcm, pSelf);
        }
    }

    GBL_CTX_END();
}

EVMU_EXPORT GblBool EvmuBuzzer_isActive(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->active;
}

EVMU_EXPORT const void* EvmuBuzzer_pcmBuffer(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->pcmBuffer;
}

EVMU_EXPORT size_t EvmuBuzzer_pcmSamples(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->pcmSamples;
}


EVMU_EXPORT size_t EvmuBuzzer_pcmFrequency(const EvmuBuzzer* pSelf) {
    return EVMU_BUZZER_(pSelf)->pcmFrequency;
}


EVMU_EXPORT float EvmuBuzzer_pcmGain(const EvmuBuzzer* pSelf) {
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);
/* Cannot work when update is called before play! Mutes next tone!
    if(!pSelf_->active) {
        return 0.0f;
    } else
*/
    if(pSelf->enableFreqResp) {
        if(pSelf_->tonePeriod < EVMU_BUZZER_FREQ_RESP_BASE_OFFSET_ ||
           pSelf_->tonePeriod > EVMU_BUZZER_FREQ_RESP_BASE_OFFSET_ +
                                GBL_COUNT_OF(freqResponse_))
        {
            return (float)EVMU_BUZZER_FREQ_RESP_DEFAULT_VALUE_ / (float)EVMU_BUZZER_FREQ_RESP_MAX_VALUE_;
        } else {
            return (float)freqResponse_[pSelf_->tonePeriod -
                                        EVMU_BUZZER_FREQ_RESP_BASE_OFFSET_] /
                   (float)EVMU_BUZZER_FREQ_RESP_MAX_VALUE_;
        }
    } else {
        return 1.0f;
    }
}

static EVMU_RESULT EvmuBuzzer_playPcm_(EvmuBuzzer* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_VERIFY_CALL(GblSignal_emit(GBL_INSTANCE(pSelf), "toneStart"));
    GBL_CTX_END();
}

static EVMU_RESULT EvmuBuzzer_stopPcm_(EvmuBuzzer* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_VERIFY_CALL(GblSignal_emit(GBL_INSTANCE(pSelf), "toneStop"));
    GBL_CTX_END();
}

static EVMU_RESULT EvmuBuzzer_bufferPcm_(EvmuBuzzer* pSelf) {
    uint16_t tonePeriod;
    uint8_t toneInvPulseLength;

    GBL_CTX_BEGIN(NULL);
    EvmuBuzzer_tone(pSelf, &tonePeriod, &toneInvPulseLength);
    GBL_CTX_VERIFY_CALL(GblSignal_emit(GBL_INSTANCE(pSelf),
                                       "toneUpdate",
                                       tonePeriod,
                                       toneInvPulseLength));
    GBL_CTX_END();
}

static GBL_RESULT EvmuBuzzer_IBehavior_reset_(EvmuIBehavior* pIBehavior) {
    GBL_CTX_BEGIN(NULL);

    EvmuBuzzer* pSelf   = EVMU_BUZZER(pIBehavior);
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pIBehavior);

    EvmuBuzzer_stopTone(EVMU_BUZZER(pIBehavior));

    pSelf->pcmChanged          = GBL_TRUE;
    pSelf_->active             = GBL_FALSE;
    pSelf_->tonePeriod         = 0;
    pSelf_->toneInvPulseLength = 0;
    pSelf_->pcmSamples         = 0;
    pSelf_->pcmFrequency       = 0;

    GBL_CTX_END();
}

static GBL_RESULT EvmuBuzzer_GblObject_constructed_(GblObject* pObject) {
    GBL_CTX_BEGIN(NULL);

    GBL_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pObject);

    EvmuBuzzer*  pSelf  = EVMU_BUZZER(pObject);
    EvmuBuzzer_* pSelf_ = EVMU_BUZZER_(pSelf);

    GblObject_setName(pObject, EVMU_BUZZER_NAME);

    pSelf->pcmChanged = GBL_TRUE;
    pSelf_->enabled   = GBL_TRUE;

    GBL_CTX_END();
}

static GBL_RESULT EvmuBuzzerClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(NULL);

    if(!GblType_classRefCount(GBL_CLASS_TYPEOF(pClass))) {
        GBL_PROPERTIES_REGISTER(EvmuBuzzer);

        GblSignal_install(EVMU_BUZZER_TYPE,
                          "toneStart",
                          GblMarshal_CClosure_VOID__INSTANCE,
                          0);

        GblSignal_install(EVMU_BUZZER_TYPE,
                          "toneStop",
                          GblMarshal_CClosure_VOID__INSTANCE,
                          0);

        GblSignal_install(EVMU_BUZZER_TYPE,
                          "toneUpdate",
                          GblMarshal_CClosure_VOID__INSTANCE_UINT16_UINT8,
                          2,
                          GBL_UINT16_TYPE,
                          GBL_UINT8_TYPE);
    }

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuBuzzer_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuBuzzer_IBehavior_reset_;
    EVMU_BUZZER_CLASS(pClass)   ->pFnPlayPcm     = EvmuBuzzer_playPcm_;
    EVMU_BUZZER_CLASS(pClass)   ->pFnStopPcm     = EvmuBuzzer_stopPcm_;
    EVMU_BUZZER_CLASS(pClass)   ->pFnBufferPcm   = EvmuBuzzer_bufferPcm_;

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
        type = GblType_register(GblQuark_internStringStatic("EvmuBuzzer"),
                                EVMU_PERIPHERAL_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}


# if 0

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


static uint8_t noteTL1RLut_[EVMU_BUZZER_NOTE_COUNT] = {
    [EVMU_BUZZER_NOTE_C4] = 0xeb, //261.63Hz actual (260.213Hz VMU)
    [EVMU_BUZZER_NOTE_D4] = 0xed, //293.66Hz actual (287.604Hz VMU)
    [EVMU_BUZZER_NOTE_E4] = 0xef, //329.63Hz actual (321.220Hz VMU)
    [EVMU_BUZZER_NOTE_F4] = 0xf0, //349.23Hz actual (341.530Hz VMU)
    [EVMU_BUZZER_NOTE_G4] = 0xf2, //392.00Hz actual (390.320Hz VMU)
    [EVMU_BUZZER_NOTE_A4] = 0xf4, //440.00Hz actual (455.373Hz VMU)
    [EVMU_BUZZER_NOTE_B4] = 0xf5, //493.88Hz actual (496.771Hz VMU)
};

#endif

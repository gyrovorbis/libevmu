/*! \file
 *  \brief Piezoelectric buzzer, PWM tone generation
 *  \ingroup Peripherals
 *
 *  \todo
 *      - Stop playback when emulation halts
 */

#ifndef EVMU_BUZZER_H
#define EVMU_BUZZER_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_BUZZER_TYPE                (GBL_TYPEOF(EvmuBuzzer))
#define EVMU_BUZZER_NAME                "buzzer"
#define EVMU_BUZZER(instance)           (GBL_INSTANCE_CAST(instance, EvmuBuzzer))
#define EVMU_BUZZER_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuBuzzer))
#define EVMU_BUZZER_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuBuzzer))

#define EVMU_BUZZER_PCM_BUFFER_SIZE     256

#define GBL_SELF_TYPE EvmuBuzzer

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuBuzzer);

GBL_CLASS_DERIVE(EvmuBuzzer, EvmuPeripheral)
    EVMU_RESULT (*pFnPlayPcm)  (GBL_SELF);
    EVMU_RESULT (*pFnStopPcm)  (GBL_SELF);
    EVMU_RESULT (*pFnBufferPcm)(GBL_SELF);
GBL_CLASS_END

GBL_INSTANCE_DERIVE(EvmuBuzzer, EvmuPeripheral)
    GblBool pcmChanged;     ///< User-toggle for polling updates
    GblBool enableFreqResp; ///< Enables per-tone gain/volume emulation
GBL_INSTANCE_END

GBL_PROPERTIES(EvmuBuzzer,
    (enabled,        GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (configured,     GBL_GENERIC, (READ),        GBL_BOOL_TYPE),
    (active,         GBL_GENERIC, (READ),        GBL_BOOL_TYPE),
    (period,         GBL_GENERIC, (READ),        GBL_UINT16_TYPE),
    (invPulseLength, GBL_GENERIC, (READ),        GBL_UINT8_TYPE),
    (frequency,      GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (gain,           GBL_GENERIC, (READ),        GBL_FLOAT_TYPE)
)

GBL_SIGNALS(EvmuBuzzer,
    (toneStart,  (GBL_INSTANCE_TYPE, pReceiver)),
    (toneStop,   (GBL_INSTANCE_TYPE, pReceiver)),
    (toneUpdate, (GBL_INSTANCE_TYPE, pReceiver),
                 (GBL_UINT16_TYPE,   period),
                 (GBL_UINT8_TYPE,    invPulseLength))
)

EVMU_EXPORT GblType     EvmuBuzzer_type         (void)                  GBL_NOEXCEPT;

EVMU_EXPORT GblBool     EvmuBuzzer_isActive     (GBL_CSELF)             GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuBuzzer_isConfigured (GBL_CSELF)             GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuBuzzer_isEnabled    (GBL_CSELF)             GBL_NOEXCEPT;

EVMU_EXPORT void        EvmuBuzzer_setEnabled   (GBL_SELF,
                                                 GblBool enabled)       GBL_NOEXCEPT;

EVMU_EXPORT void        EvmuBuzzer_tone         (GBL_CSELF,
                                                 uint16_t* pPeriod,
                                                 uint8_t* pInvPulseLen) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_setTone      (GBL_SELF,
                                                 uint16_t period,
                                                 uint8_t invPulseLen)   GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playTone     (GBL_SELF)              GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_stopTone     (GBL_SELF)              GBL_NOEXCEPT;

EVMU_EXPORT const void* EvmuBuzzer_pcmBuffer    (GBL_CSELF)             GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuBuzzer_pcmSamples   (GBL_CSELF)             GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuBuzzer_pcmFrequency (GBL_CSELF)             GBL_NOEXCEPT;
EVMU_EXPORT float       EvmuBuzzer_pcmGain      (GBL_CSELF)             GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BUZZER_H

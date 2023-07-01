/*! \file
 *  \brief Piezoelectric buzzer, PWM tone generation
 *  \ingroup peripherals
 *
 *  The EvmuBuzzer API encompasses everything pertaining to signal
 *  generation and driving the piezoelectric buzzer. It provides
 *  a front-end API for generating and playing tones, as well as a
 *  polymorphic event-driven back-end for buffering and playing the
 *  generated tones with an audio back-end.
 *
 *  \todo
 *      - Stop playback when emulation halts
 *      - Have to emulate Timer1 mode 3 buzzer output
 *      - Have to emulate base timer PWM output mode
 *      - Document emulated modes
 *      - Document how to implement a new back-end
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_BUZZER_H
#define EVMU_BUZZER_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

#define EVMU_BUZZER_TYPE                (GBL_TYPEOF(EvmuBuzzer))                        //!< GblType UUID for GblBuzzer
#define EVMU_BUZZER(instance)           (GBL_INSTANCE_CAST(instance, EvmuBuzzer))       //!< Function-style GblInstance cast
#define EVMU_BUZZER_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuBuzzer))             //!< Function-style GblClass cast
#define EVMU_BUZZER_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuBuzzer))  //!< Get EvmuBuzzerClass from GblInstance

#define EVMU_BUZZER_NAME                "buzzer"                                        //!< EvmuBuzzer GblObject name
#define EVMU_BUZZER_PCM_BUFFER_SIZE     256                                             //!< Size of internal PCM buffer

#define GBL_SELF_TYPE EvmuBuzzer

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuBuzzer);

/*! \struct  EvmuBuzzerClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass structure for EvmuBuzzer
 *
 * EvmuBuzzerClass provides a virtual table of overridable function
 * pointers for implementing PCM audio playback functionality.
 *
 * \note
 * The default implementations simply emit the corresponding signals
 * for each event.
 *
 * \sa EvmuBuzzer
 */
GBL_CLASS_DERIVE(EvmuBuzzer, EvmuPeripheral)
    EVMU_RESULT (*pFnPlayPcm)  (GBL_SELF);  //!< Called when a sample is ready to be played
    EVMU_RESULT (*pFnStopPcm)  (GBL_SELF);  //!< Called when the playing sample should be stopped
    EVMU_RESULT (*pFnBufferPcm)(GBL_SELF);  //!< Called when a sample should be changed or reloaded
GBL_CLASS_END

/*! \struct  EvmuBuzzer
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   Instance structure for Buzzer Peripheral
 *
 *  EvmuBuzzer is the GblInstance structure representing the
 *  piezoelectric buzzer peripheral and associated logic.
 *
 *  \sa EvmuBuzzerClass
 */
GBL_INSTANCE_DERIVE(EvmuBuzzer, EvmuPeripheral)
    GblBool pcmChanged;                     //!< User-toggle for polling updates
    GblBool enableFreqResp;                 //!< Enables per-tone gain/volume emulation
GBL_INSTANCE_END

///\cond
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
///\endcond

/*! Returns the UUID corresponding to the EvmuBuzzer type
 *  \relatesalso EvmuBuzzer
 *  \static
 *
 *  \returns     GblType UUID
 */
EVMU_EXPORT GblType     EvmuBuzzer_type         (void)                  GBL_NOEXCEPT;

/*! Returns whether a tone is currently being played
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     GBL_TRUE if a tone is being played
 *
 *  \sa EvmuBuzzer_isConfigured, EvmuBuzzer_isEnabled
 */
EVMU_EXPORT GblBool     EvmuBuzzer_isActive     (GBL_CSELF)             GBL_NOEXCEPT;

/*! Returns whether the buzzer is properly configured for output
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     GBL_TRUE if buzzer is configured
 *
 *  \sa EvmuBuzzer_isActive, EvmuBuzzer_isEnabled
 */
EVMU_EXPORT GblBool     EvmuBuzzer_isConfigured (GBL_CSELF)             GBL_NOEXCEPT;

/*! Returns whether the buzzer is enabled (in software)
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     GBL_TRUE if buzzer is enabled
 *
 *  \sa EvmuBuzzer_setEnabled, EvmuBuzzer_isActive, EvmuBuzzer_isConfigured
 */
EVMU_EXPORT GblBool     EvmuBuzzer_isEnabled    (GBL_CSELF)             GBL_NOEXCEPT;

/*! Enables or disables the buzzer (in software)
 *  \relatesalso EvmuBuzzer
 *
 *  \param       enabled    GBL_TRUE to enable, GBL_FALSE to disable
 *
 *  \sa EvmuBuzzer_isEnabled
 */
EVMU_EXPORT void        EvmuBuzzer_setEnabled   (GBL_SELF,
                                                 GblBool enabled)       GBL_NOEXCEPT;

/*! Retrieve the waveform for the active tone
 *  \relatesalso EvmuBuzzer
 *
 *  \param[out]  pPeriod       Period of the square wave
 *  \param[out]  pInvPulseLen  Inverse pulse length of the square wave
 *
 *  \sa EvmuBuzzer_setTone
 */
EVMU_EXPORT void        EvmuBuzzer_tone         (GBL_CSELF,
                                                 uint16_t* pPeriod,
                                                 uint8_t* pInvPulseLen) GBL_NOEXCEPT;

/*! Sets the waveform for the current tone to the given square wave
 *  \relatesalso EvmuBuzzer
 *
 *  \param       period       Period of the square wave
 *  \param       invPulseLen  Inverse pulse length of the square wave
 *  \returns     GBL_SUCCESS or error code upon failure or invalid waveform
 *
 *  \sa          EvmuBuzzer_tone
 */
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_setTone      (GBL_SELF,
                                                 uint16_t period,
                                                 uint8_t invPulseLen)   GBL_NOEXCEPT;

/*! Plays the current tone
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     GBL_SUCCESS or error code upon failure or invalid waveform
 *
 *  \sa          EvmuBuzzer_setTone, EvmuBuzzer_stopTone
 */
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playTone     (GBL_SELF)              GBL_NOEXCEPT;

/*! Stops the current tone
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     GBL_SUCCESS or error code upon failure
 *
 *  \sa          EvmuBuzzer_playTone
 */
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_stopTone     (GBL_SELF)              GBL_NOEXCEPT;

/*! Returns a pointer to the current PCM buffer
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     pointer to the current PCM buffer
 *
 *  \sa          EvmuBuzzer_pcmSamples, EvmuBuzzer_pcmFrequency
 */
EVMU_EXPORT const void* EvmuBuzzer_pcmBuffer    (GBL_CSELF)             GBL_NOEXCEPT;

/*! Returns number of samples within the PCM buffer
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     sample count for PCM buffer
 *
 *  \sa          EvmuBuzzer_pcmBuffer, EvmuBuzzer_pcmFrequency
 */
EVMU_EXPORT size_t      EvmuBuzzer_pcmSamples   (GBL_CSELF)             GBL_NOEXCEPT;

/*! Returns the frequency corresponding to the PCM buffer
 *  \relatesalso EvmuBuzzer
 *
 *  \returns     frequency for PCM buffer
 *
 *  \sa          EvmuBuzzer_pcmBuffer, EvmuBuzzer_pcmSamples
 */
EVMU_EXPORT size_t      EvmuBuzzer_pcmFrequency (GBL_CSELF)             GBL_NOEXCEPT;

/*! Returns the gain for the given PCM buffer
 *  \relatesalso EvmuBuzzer
 *
 *  Returns the volume or gain (0.0-1.0) in dB for the given sample.
 *
 *  \note
 *  Unless frequency response emulation has been enabled, this will always
 *  return 1.0. Enable frequency response emulation to emulate volume
 *  characteristics of the piezoelectric buzzer.
 *
 *  \returns     gain (0.0-1.0) for PCM buffer
 *
 *  \sa          EvmuBuzzer::enableFreqResp
 */
EVMU_EXPORT float       EvmuBuzzer_pcmGain      (GBL_CSELF)             GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BUZZER_H

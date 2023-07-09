/*! \file
 *  \brief   EvmuBuzzer: Piezoelectric buzzer, PWM tone generation
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
 *      - Represent PCM buffer as GblByteArray, resizable samples
 *
 *  \test
 *      - PCMs generated from setting tones under different timer1 configs
 *      - signals for tone generation
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_BUZZER_H
#define EVMU_BUZZER_H

#include "../types/evmu_peripheral.h"
#include <gimbal/meta/signals/gimbal_signal.h>

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_BUZZER_TYPE                (GBL_TYPEOF(EvmuBuzzer))                        //!< GblType UUID for GblBuzzer
#define EVMU_BUZZER(instance)           (GBL_INSTANCE_CAST(instance, EvmuBuzzer))       //!< Function-style GblInstance cast
#define EVMU_BUZZER_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuBuzzer))             //!< Function-style GblClass cast
#define EVMU_BUZZER_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuBuzzer))  //!< Get EvmuBuzzerClass from GblInstance
//! @}

#define EVMU_BUZZER_NAME                "buzzer"    //!< EvmuBuzzer GblObject name
#define EVMU_BUZZER_PCM_BUFFER_SIZE     256         //!< Size of internal PCM buffer

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
    GblBool pcmChanged;         //!< User-toggle for polling updates
    GblBool enableFreqResp;     //!< Enables per-tone gain/volume emulation
GBL_INSTANCE_END

//! \cond
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
//! \endcond

//! EvmuBuzzer's type ID
EVMU_EXPORT GblType EvmuBuzzer_type (void) GBL_NOEXCEPT;

/*! \name Configuration
 *  \brief Methods for managing buzzer configuration
 *  \relatesalso EvmuBuzzer
 *  @{
 */
//! Returns whether playback is active or not
EVMU_EXPORT GblBool EvmuBuzzer_isActive     (GBL_CSELF)  GBL_NOEXCEPT;
//! Returns whether the buzzer has been configured for playback
EVMU_EXPORT GblBool EvmuBuzzer_isConfigured (GBL_CSELF)  GBL_NOEXCEPT;
//! Returns whether software has enabled or disabled the buzzer
EVMU_EXPORT GblBool EvmuBuzzer_isEnabled    (GBL_CSELF)  GBL_NOEXCEPT;
//! Allows software to enable or disable buzzer emulation
EVMU_EXPORT void    EvmuBuzzer_setEnabled   (GBL_SELF,
                                             GblBool en) GBL_NOEXCEPT;
//! @}

/*! \name  Tone Playback
 *  \brief Methods for managing tone generation and playback
 *  \relatesalso EvmuBuzzer
 *  @{
 */
//! Returns the current tone's square wave
EVMU_EXPORT void        EvmuBuzzer_tone     (GBL_CSELF,
                                             uint16_t* pPeriod,
                                             uint8_t*  pInvPulseLen) GBL_NOEXCEPT;
//! Sets the square wave of the current tone without impacting playback
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_setTone  (GBL_SELF,
                                             uint16_t period,
                                             uint8_t  invPulseLen)   GBL_NOEXCEPT;
//! Plays the current tone's square wave
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_playTone (GBL_SELF)               GBL_NOEXCEPT;
//! Stops playback of the current tone
EVMU_EXPORT EVMU_RESULT EvmuBuzzer_stopTone (GBL_SELF)               GBL_NOEXCEPT;
//! @}

/*! \name  PCM Back-End
 *  \brief Methods used to implement an audio driver back-end
 *  \relatesalso EvmuBuzzer
 *  @{
 */
//! Returns the current PCM buffer to the audio driver
EVMU_EXPORT const void* EvmuBuzzer_pcmBuffer    (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the PCM buffer's sample size to the audio driver
EVMU_EXPORT size_t      EvmuBuzzer_pcmSamples   (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the PCM buffer's sample frequency to the audio driver
EVMU_EXPORT size_t      EvmuBuzzer_pcmFrequency (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the PCM buffer's gain to the audio driver
EVMU_EXPORT float       EvmuBuzzer_pcmGain      (GBL_CSELF) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_BUZZER_H

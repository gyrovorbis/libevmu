/*! \file
 *  \brief EvmuPic programmable interrupt controller peripheral
 *  \ingroup peripherals
 *
 *  This file models the programmable interrupt controller of the VMU.
 *
 *  \todo
 *  - shouldn't update be private?
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_PIC_H
#define EVMU_PIC_H

#include <stdint.h>
#include "../types/evmu_peripheral.h"

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_PIC_TYPE                   (GBL_TYPEOF(EvmuPic))                       //!< Type UUID for EvmuPic
#define EVMU_PIC(instance)              (GBL_INSTANCE_CAST(instance, EvmuPic))      //!< Function-style GblInstance cast
#define EVMU_PIC_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuPic))            //!< Function-style GblClass cast
#define EVMU_PIC_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuPic)) //!< Get EvmuPicClass from GblInstance
//! @}

#define EVMU_PIC_NAME                   "pic"   //!< EvmuPic GblObject name

/*! \defgroup isrs Interrupt Service Routines
 *  \brief    Entry addresses for each interrupt
 *  @{
 */
#define EVMU_ISR_ADDR_RESET              0x00   //!< Reset ISR address
#define EVMU_ISR_ADDR_EXT_INT0           0x03   //!< INT0 interrupt (external) ISR address
#define EVMU_ISR_ADDR_EXT_INT1           0x0b   //!< INT1 interrupt (external) ISR address
#define EVMU_ISR_ADDR_EXT_INT2_T0L       0x13   //!< INT2 interrupt (external) or T0L overflow ISR address
#define EVMU_ISR_ADDR_EXT_INT3_TBASE     0x1b   //!< INT3 interrupt (external) or Base Timer overflow ISR address
#define EVMU_ISR_ADDR_T0H                0x23   //!< T0H overflow ISR address
#define EVMU_ISR_ADDR_T1                 0x2b   //!< T1H or T1L overflow ISR address
#define EVMU_ISR_ADDR_SIO0               0x33   //!< SIO0 ISR address
#define EVMU_ISR_ADDR_SIO1               0x3b   //!< SI01 ISR address
#define EVMU_ISR_ADDR_RFB                0x43   //!< RFB interrupt (VMU<->VMU receive/detect) ISR address
#define EVMU_ISR_ADDR_P3                 0x4b   //!< P3 interrupt ISR address
#define EVMU_ISR_ADDR_11                 0x4f   //!< ISR 11 Address (undocumented/unused?)
#define EVMU_ISR_ADDR_12                 0x52   //!< ISR 12 Address (undocumented/unused?)
#define EVMU_ISR_ADDR_13                 0x55   //!< ISR 12 Address (undocumented/unused?)
#define EVMU_ISR_ADDR_14                 0x5a   //!< ISR 12 Address (undocumented/unused?)
#define EVMU_ISR_ADDR_15                 0x5d   //!< ISR 12 Address (undocumented/unused?)
//! @}

#define GBL_SELF_TYPE EvmuPic

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_IRQ) {
    EVMU_IRQ_RESET,
    EVMU_IRQ_EXT_INT0,
    EVMU_IRQ_EXT_INT1,
    EVMU_IRQ_EXT_INT2_T0L,
    EVMU_IRQ_EXT_INT3_TBASE,
    EVMU_IRQ_T0H,
    EVMU_IRQ_T1,
    EVMU_IRQ_SIO0,
    EVMU_IRQ_SIO1,
    EVMU_IRQ_RFB,   //Maple interrupt?
    EVMU_IRQ_P3,
    EVMU_IRQ_11,
    EVMU_IRQ_12,
    EVMU_IRQ_13,
    EVMU_IRQ_14,
    EVMU_IRQ_15,
    EVMU_IRQ_COUNT
};

GBL_DECLARE_ENUM(EVMU_IRQ_PRIORITY) {
    EVMU_IRQ_PRIORITY_LOW,
    EVMU_IRQ_PRIORITY_HIGH,
    EVMU_IRQ_PRIORITY_HIGHEST,
    EVMU_IRQ_PRIORITY_COUNT,
    EVMU_IRQ_PRIORITY_NONE
};

//! Mask of EVMU_IRQ values shifted and OR'd into a single mask
typedef uint16_t EvmuIrqMask;

/*! \struct  EvmuPicClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass structure for EvmuPic
 *
 *  Contains no public members.
 *
 *  \sa EvmuPic
 */
// Service ISR routine
// Check for ISRs
GBL_CLASS_DERIVE_EMPTY    (EvmuPic, EvmuPeripheral)

/*! \struct  EvmuPic
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   GblInstance structure for EvmuPic
 *
 *  Contains no public members.
 *
 *  \sa EvmuPicClass
 */
GBL_INSTANCE_DERIVE_EMPTY (EvmuPic, EvmuPeripheral)

//! \cond
GBL_PROPERTIES(EvmuPic,
    (irqEnabledMask,        GBL_GENERIC, (READ), GBL_UINT16_TYPE),
    (irqPendingMask,        GBL_GENERIC, (READ), GBL_UINT16_TYPE),
    (irqActiveMask,         GBL_GENERIC, (READ), GBL_UINT16_TYPE),
    (irqActiveTop,          GBL_GENERIC, (READ), GBL_ENUM_TYPE),
    (irqActiveTopPriority,  GBL_GENERIC, (READ), GBL_UINT8_TYPE),
    (irqActiveDepth,        GBL_GENERIC, (READ), GBL_UINT8_TYPE),
    (processInstruction,    GBL_GENERIC, (READ), GBL_BOOL_TYPE)
)
//! \endcond

EVMU_EXPORT GblType           EvmuPic_type                  (void)                                  GBL_NOEXCEPT;
EVMU_INLINE EvmuAddress       EvmuPic_isrAddress            (EVMU_IRQ irq)                          GBL_NOEXCEPT;

EVMU_EXPORT void              EvmuPic_raiseIrq              (GBL_SELF, EVMU_IRQ irq)                GBL_NOEXCEPT;
EVMU_EXPORT size_t            EvmuPic_irqsActiveDepth       (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT EVMU_IRQ_PRIORITY EvmuPic_irqPriority           (GBL_CSELF, EVMU_IRQ irq)               GBL_NOEXCEPT;
EVMU_EXPORT EvmuIrqMask       EvmuPic_irqsEnabledByPriority (GBL_CSELF, EVMU_IRQ_PRIORITY priority) GBL_NOEXCEPT;
EVMU_EXPORT EvmuIrqMask       EvmuPic_irqsActive            (GBL_CSELF)                             GBL_NOEXCEPT;

EVMU_EXPORT GblBool           EvmuPic_update                (GBL_SELF)                              GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_PIC_H

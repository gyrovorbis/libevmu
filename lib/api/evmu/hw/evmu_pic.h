#ifndef EVMU_PIC_H
#define EVMU_PIC_H

#include <stdint.h>
#include "../types/evmu_peripheral.h"

#define EVMU_PIC_TYPE                   (GBL_TYPEOF(EvmuPic))
#define EVMU_PIC_NAME                   "pic"

#define EVMU_PIC(instance)              (GBL_INSTANCE_CAST(instance, EvmuPic))
#define EVMU_PIC_CLASS(klass)           (GBL_CLASS_CAST(klass, EvmuPic))
#define EVMU_PIC_GET_CLASS(instance)    (GBL_INSTANCE_GET_CLASS(instance, EvmuPic))

// Interrupt Service Routine Addresses
#define EVMU_ISR_ADDR_RESET              0x00    //Reset Interrupt (not really an interrupt? The fuck?
#define EVMU_ISR_ADDR_EXT_INT0           0x03    //INT0 interrupt (external)
#define EVMU_ISR_ADDR_EXT_INT1           0x0b    //INT1 interrupt (external)
#define EVMU_ISR_ADDR_EXT_INT2_T0L       0x13    //INT2 interrupt (external) or T0L overflow
#define EVMU_ISR_ADDR_EXT_INT3_TBASE     0x1b    //INT3 interrupt (external) or Base Timer overflow
#define EVMU_ISR_ADDR_T0H                0x23    //T0H overflow
#define EVMU_ISR_ADDR_T1                 0x2b    //T1H or T1L overflow
#define EVMU_ISR_ADDR_SIO0               0x33    //SIO0 interrupt
#define EVMU_ISR_ADDR_SIO1               0x3b    //SI01 interrupt
#define EVMU_ISR_ADDR_RFB                0x43    //RFB interrupt (VMU<->VMU receive/detect)
#define EVMU_ISR_ADDR_P3                 0x4b    //P3 interrupt
#define EVMU_ISR_ADDR_11                 0x4f
#define EVMU_ISR_ADDR_12                 0x52
#define EVMU_ISR_ADDR_13                 0x55
#define EVMU_ISR_ADDR_14                 0x5a
#define EVMU_ISR_ADDR_15                 0x5d

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

typedef uint16_t EvmuIrqMask;

GBL_CLASS_DERIVE_EMPTY    (EvmuPic, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY (EvmuPic, EvmuPeripheral)

GBL_PROPERTIES(EvmuPic,
    (irqEnabledMask,        GBL_GENERIC, (READ), GBL_UINT16_TYPE),
    (irqPendingMask,        GBL_GENERIC, (READ), GBL_UINT16_TYPE),
    (irqActiveMask,         GBL_GENERIC, (READ), GBL_UINT16_TYPE),
    (irqActiveTop,          GBL_GENERIC, (READ), GBL_ENUM_TYPE),
    (irqActiveTopPriority,  GBL_GENERIC, (READ), GBL_UINT8_TYPE),
    (irqActiveDepth,        GBL_GENERIC, (READ), GBL_UINT8_TYPE),
    (processInstruction,    GBL_GENERIC, (READ), GBL_BOOL_TYPE)
)

EVMU_EXPORT GblType           EvmuPic_type                  (void)                                  GBL_NOEXCEPT;
EVMU_INLINE EvmuAddress       EvmuPic_isrAddress            (EVMU_IRQ irq)                          GBL_NOEXCEPT;

EVMU_EXPORT void              EvmuPic_raiseIrq              (GBL_SELF, EVMU_IRQ irq)                GBL_NOEXCEPT;
EVMU_EXPORT GblSize           EvmuPic_irqsActiveDepth       (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT EVMU_IRQ_PRIORITY EvmuPic_irqPriority           (GBL_CSELF, EVMU_IRQ irq)               GBL_NOEXCEPT;
EVMU_EXPORT EvmuIrqMask       EvmuPic_irqsEnabledByPriority (GBL_CSELF, EVMU_IRQ_PRIORITY priority) GBL_NOEXCEPT;
EVMU_EXPORT EvmuIrqMask       EvmuPic_irqsActive            (GBL_CSELF)                             GBL_NOEXCEPT;

EVMU_EXPORT GblBool           EvmuPic_update                (GBL_SELF)                              GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_PIC_H

#if 0

// COUNT = ALL
EVMU_EXPORT EvmuIrqMask       EvmuPic_irqsActiveByPriority  (GBL_CSELF, EVMU_IRQ_PRIORITY priority)     GBL_NOEXCEPT;
EVMU_EXPORT EvmuIrqMask       EvmuPic_irqsPending           (GBL_CSELF)                                 GBL_NOEXCEPT;

EVMU_EXPORT GblBool           EvmuPic_irqEnabled            (GBL_CSELF, EVMU_IRQ irq)                   GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT       EvmuPic_setIrqEnabled         (GBL_CSELF, EVMU_IRQ irq, GblBool enabled)  GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT       EvmuPic_setIrqPriority        (GBL_CSELF,
                                                             EVMU_IRQ irq,

void                gyVmuInterruptControllerInit(struct VMUDevice* dev);
int                 gyVmuInterruptControllerUpdate(struct VMUDevice* dev);


void                gyVmuInterruptSignal(struct VMUDevice* dev, VMU_INT interrupt);
uint16_t            gyVmuInterruptPriorityMask(const struct VMUDevice* dev, VMU_INT_PRIORITY priority);
int                 gyVmuInterruptDepth(const struct VMUDevice* dev);
int                 gyVmuInterruptsActive(const struct VMUDevice* dev);
int                 gyVmuInterruptCurrent(const struct VMUDevice* dev);
VMU_INT_PRIORITY    gyVmuInterruptPriority(const struct VMUDevice* dev, VMU_INT interrupt);


inline static int gyVmuInterruptAddr(VMU_INT intType) {
    const static unsigned char lut[VMU_INT_COUNT] = {
        ISR_ADDR_RESET,
        ISR_ADDR_EXT_INT0,
        ISR_ADDR_EXT_INT1,
        ISR_ADDR_EXT_INT2_T0L,
        ISR_ADDR_EXT_INT3_TBASE,
        ISR_ADDR_T0H,
        ISR_ADDR_T1,
        ISR_ADDR_SIO0,
        ISR_ADDR_SIO1,
        ISR_ADDR_RFB,
        ISR_P3_ADDR,
        ISR_11_ADDR,
        ISR_12_ADDR,
        ISR_13_ADDR,
        ISR_14_ADDR,
        ISR_15_ADDR
    };
    return lut[intType];
}

#if 0

// High-level property-driven API for enabling, disabling, quering interrupts + modifying priorities
#define EVMU_PIC_IRQ_PROPERTIES(Name) \
    EVMU_PIC_##Name##_ENABLED,        \
    EVMU_PIC_##Name##_PENDING,        \
    EVMU_PIC_##Name##_ACTIVE,         \
    EVMU_PIC_##Name##_PRIORITY


GBL_DECLARE_ENUM(EVMU_PIC_PROPERTY) {
    EVMU_PIC_PROPERTY_IRQ_ENABLED_MASK = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_PIC_PROPERTY_IRQ_PENDING_MASK,
    EVMU_PIC_PROPERTY_IRQ_ACTIVE_MASK,
    EVMU_PIC_PROPERTY_IRQ_ACTIVE_TOP,
    EVMU_PIC_PROPERTY_IRQ_ACTIVE_DEPTH,
    EVMU_PIC_PROPERTY_IRQ_PREVIOUS_PRIORITY,
    EVMU_PIC_PROPERTY_PROCESS_THIS_INSTRUCTION, //polling/accepting state?

    EVMU_PIC_IRQ_PROPERTIES(RESET),
    EVMU_PIC_IRQ_PROPERTIES(EXT_INT0),
    EVMU_PIC_IRQ_PROPERTIES(EXT_INT1),
    EVMU_PIC_IRQ_PROPERTIES(EXT_INT2_T0L),
    EVMU_PIC_IRQ_PROPERTIES(EXT_INT3_TBASE),
    EVMU_PIC_IRQ_PROPERTIES(T0H),
    EVMU_PIC_IRQ_PROPERTIES(T1),
    EVMU_PIC_IRQ_PROPERTIES(SIO0),
    EVMU_PIC_IRQ_PROPERTIES(SIO1),
    EVMU_PIC_IRQ_PROPERTIES(RFB),
    EVMU_PIC_IRQ_PROPERTIES(P3),
    EVMU_PIC_IRQ_PROPERTIES(11),
    EVMU_PIC_IRQ_PROPERTIES(12),
    EVMU_PIC_IRQ_PROPERTIES(13),
    EVMU_PIC_IRQ_PROPERTIES(14),
    EVMU_PIC_IRQ_PROPERTIES(15),
    EVMU_PIC_PROPERTY_COUNT
};

#undef EVMU_PIC_IRQ_PROPERTIES

These are interrupt vectors used by the processor.

0x0000 reset/start
    Contains a jump to the start of code.
    Only 0x001b and 0x004b are used by the football program; all others simply jump to an RETI.

0x0003 external interrupt 0?

0x000b timer/counter 0 interrupt

0x0013 external interrupt 1?

0x001b timer/counter 1 interrupt. This timer is used to run the time-of-day clock, so control should be passed to the OS code (see vector below). The user can do other things with this interrupt, too.

0x0023 divider circuit/port 1/port 3 interrupt?

0x002b interrupt - unknown

0x0033 interrupt - unknown

0x003b interrupt - unknown

0x0043 interrupt - unknown

0x004b interrupt - unknown, but used by football program. Possibly a port 3 interrupt used to wait on a button push.

0x004f interrupt - BIOS firmware does a "CLR1 IO1CR, 1" and a RETI

0x0052 interrupt - BIOS firmware does a "CLR1 IO1CR, 5" and a RETI

0x0055 interrupt - BIOS firmware does a "CLR1 T0CON, 1" and a "CLR1 I23CR, 5", then a RETI.

0x005a interrupt - BIOS firmware does a "CLR1 T0CON, 3" and a RETI

0x005d interrupt - BIOS firmware does a "CLR1 T1CNT, 1" and a "CLR1 T1CNT, 3", then a RETI.

#endif
#endif



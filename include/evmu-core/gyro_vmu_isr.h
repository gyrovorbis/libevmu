#ifndef GYRO_VMU_ISR_H
#define GYRO_VMU_ISR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Interrupt Service Routine Addresses (guess there is no vector table)
#define ISR_ADDR_RESET              0x00    //Reset Interrupt (not really an interrupt? The fuck?
#define ISR_ADDR_EXT_INT0           0x03    //INT0 interrupt (external)
#define ISR_ADDR_EXT_INT1           0x0b    //INT1 interrupt (external)
#define ISR_ADDR_EXT_INT2_T0L       0x13    //INT2 interrupt (external) or T0L overflow
#define ISR_ADDR_EXT_INT3_TBASE     0x1b    //INT3 interrupt (external) or Base Timer overflow
#define ISR_ADDR_T0H                0x23    //T0H overflow
#define ISR_ADDR_T1                 0x2b    //T1H or T1L overflow
#define ISR_ADDR_SIO0               0x33    //SIO0 interrupt
#define ISR_ADDR_SIO1               0x3b    //SI01 interrupt
#define ISR_ADDR_RFB                0x43    //RFB interrupt (VMU<->VMU receive/detect)
#define ISR_P3_ADDR                 0x4b    //P3 interrupt
#define ISR_11_ADDR                 0x4f    //MAPLE slave mode interrupts... ?
#define ISR_12_ADDR                 0x52
#define ISR_13_ADDR                 0x55
#define ISR_14_ADDR                 0x5a
#define ISR_15_ADDR                 0x5d

struct VMUDevice;

typedef enum VMU_INT {
    VMU_INT_RESET,
    VMU_INT_EXT_INT0,
    VMU_INT_EXT_INT1,
    VMU_INT_EXT_INT2_T0L,
    VMU_INT_EXT_INT3_TBASE,
    VMU_INT_T0H,
    VMU_INT_T1,
    VMU_INT_SIO0,
    VMU_INT_SIO1,
    VMU_INT_RFB,
    VMU_INT_P3,
    VMU_INT_11,
    VMU_INT_12,
    VMU_INT_13,
    VMU_INT_14,
    VMU_INT_15,
    VMU_INT_COUNT
} VMU_INT;

typedef enum VMU_INT_PRIORITY {
    VMU_INT_PRIORITY_LOW,
    VMU_INT_PRIORITY_HIGH,
    VMU_INT_PRIORITY_HIGHEST,
    VMU_INT_PRIORITY_COUNT,
    VMU_INT_PRIORITY_NONE
} VMU_INT_PRIORITY;


typedef struct VMUInterruptController {
    uint16_t    intReq;
    uint16_t    intStack[VMU_INT_PRIORITY_COUNT];
    uint8_t     processThisInstr;
    uint8_t     prevIntPriority;
} VMUInterruptController;


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

//0x004b interrupt –  unknown, but used by football program. Possibly a port 3 interrupt used to wait on a button push.

0x004f interrupt - BIOS firmware does a "CLR1 IO1CR, 1" and a RETI

0x0052 interrupt - BIOS firmware does a "CLR1 IO1CR, 5" and a RETI

0x0055 interrupt - BIOS firmware does a "CLR1 T0CON, 1" and a "CLR1 I23CR, 5", then a RETI.

0x005a interrupt - BIOS firmware does a "CLR1 T0CON, 3" and a RETI

0x005d interrupt - BIOS firmware does a "CLR1 T1CNT, 1" and a "CLR1 T1CNT, 3", then a RETI.

#endif



#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_ISR_H


#ifndef EVMU_PIC__H
#define EVMU_PIC__H

#include <evmu/hw/evmu_pic.h>
#include <evmu/evmu_types.h>
#include <evmu/hw/evmu_peripheral.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EvmuPic_ {
    EvmuPeripheral peripheral;

    IrqMask irqPending;
    IrqMask active;
    EVMU_INTERRUPT_PRIORITY haltModePriority; // only an interrupt of higher priority can end halt mode!

    IrqMask    intReq;
    uint16_t    intStack[EVMU_INTERRUPT_PRIORITY_COUNT];
    EvmuBool    processThisInstr;
    uint8_t     prevIntPriority;
} EvmuPic_;

#ifdef __cplusplus
}
#endif


#endif // EVMU_PIC__H

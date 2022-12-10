#ifndef EVMU_PIC__H
#define EVMU_PIC__H

#include <evmu/hw/evmu_pic.h>

#define EVMU_PIC_(instance)     ((EvmuPic_*)GBL_INSTANCE_PRIVATE(instance, EVMU_PIC_TYPE))
#define EVMU_PIC_PUBLIC_(priv)  ((EvmuPic*)GBL_INSTANCE_PUBLIC(priv, EVMU_PIC_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory_);

GBL_DECLARE_STRUCT(EvmuPic_) {
    EvmuMemory_* pMemory;
/*
    EvmuIrqMask       irqPending;
    EvmuIrqMask       active;
    EVMU_IRQ_PRIORITY haltModePriority; // only an interrupt of higher priority can end halt mode!
*/
    EvmuIrqMask       intReq;
    uint16_t          intStack[EVMU_IRQ_PRIORITY_COUNT];
    GblBool           processThisInstr;
    uint8_t           prevIntPriority;
};

GblBool EvmuPic__retiInstruction(EvmuPic_* pSelf_) GBL_NOEXCEPT;

GBL_DECLS_END

#endif // EVMU_PIC__H

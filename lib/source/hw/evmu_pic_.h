#ifndef EVMU_PIC__H
#define EVMU_PIC__H

#include <evmu/hw/evmu_pic.h>

/* EvmuDevice* pDevice = GBL_NEW(EvmuDevice);
 * GblObject* pObject = GBL_CAST(GblObject, pObject);
 * EvmuIBehavior* pBehav = GBL_CLASSOF(EvmuIBehavior, pObject);
 * EvmuDevice_* pDev = GBL_PRIVATE(EvmuDevice, pDevice);
 */

#define EVMU_PIC_(instance)     (GBL_PRIVATE(EvmuPic, instance))
#define EVMU_PIC_PUBLIC_(priv)  (GBL_PUBLIC(EvmuPic, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuPic_) {
    EvmuRam_*         pRam;
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

#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_memory_.h"
#include "evmu_device_.h"

static GBL_RESULT EvmuMemory_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructor, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuMemory_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuMemory_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    memset(pSelf_->ram, 0, EVMU_ADDRESS_SEGMENT_RAM_BANKS*EVMU_ADDRESS_SEGMENT_RAM_SIZE);
    memset(pSelf_->sfr, 0, EVMU_ADDRESS_SEGMENT_SFR_SIZE);
    memset(pSelf_->xram, 0, EVMU_ADDRESS_SEGMENT_XRAM_BANKS*EVMU_ADDRESS_SEGMENT_XRAM_SIZE);
    memset(pSelf_->wram, 0, EVMU_WRAM_SIZE);

    pSelf_->pIntMap[EVMU_MEMORY__INT_SEGMENT_SFR_]  = pSelf_->sfr;
    pSelf_->pIntMap[EVMU_MEMORY__INT_SEGMENT_XRAM_] = pSelf_->xram[0];

    if(EvmuMemory__biosLoaded_(pSelf_)) {
        pSelf_->pIntMap[EVMU_MEMORY__INT_SEGMENT_GP1_]     = pSelf_->ram[0];
        pSelf_->pIntMap[EVMU_MEMORY__INT_SEGMENT_GP2_]     = &pSelf_->ram[0][EVMU_MEMORY__INT_SEGMENT_SIZE_];
        pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 0;
        pSelf_->pExt                                       = pSelf_->rom;

    } else {
        pSelf_->pIntMap[EVMU_MEMORY__INT_SEGMENT_GP1_]     = pSelf_->ram[1];
        pSelf_->pIntMap[EVMU_MEMORY__INT_SEGMENT_GP2_]     = &pSelf_->ram[1][EVMU_MEMORY__INT_SEGMENT_SIZE_];
        pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 1;
        pSelf_->pExt                                       = pSelf_->flash;
    }

    // why ar ewe doing direct manipulation?
    pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]  = EVMU_ADDRESS_SEGMENT_STACK_BASE-1;
    pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)] = EVMU_SFR_PSW_RAMBK0_MASK;
    pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)]  = 0xff;

    //SFR values initialized by BIOS (from Sega Documentation + reverse engineering)
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_P1FCR, 0xbf);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_P3INT, 0xfd);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_ISL,   0xc0);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_VSEL,  0xfc);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_BTCR,  0x40);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_IE,    0xff);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_IP,    0x00);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_OCR,   EVMU_SFR_OCR_OCR7_MASK|EVMU_SFR_OCR_OCR0_MASK); //stop main clock, divide active clock by 6
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_P7,    EVMU_SFR_P7_P71_MASK);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_VCCR,  EVMU_SFR_VCCR_VCCR7_MASK);    //turn on LCD
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_MCR,   EVMU_SFR_MCR_MCR3_MASK);      //enable LCD update
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_PCON,  0);                           //Disable HALT/HOLD modes, run CPU normally.

/* ENABLE GAME ICON
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_ICN);
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_XRAM_ICN_GAME, 0x10);             //Enable Game Icon
    EvmuMemory__writeSfr_(pSelf_, EVMU_ADDRESS_SFR_XBNK, VMU_XRAM_BANK_LCD_TOP); //Change back to default bank
*/

    GBL_CTX_END();
}

static GBL_RESULT EvmuMemoryClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset    = EvmuMemory_reset_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor = EvmuMemory_constructor_;
    GBL_BOX_CLASS(pClass)->pFnDestructor     = EvmuMemory_destructor_;

    GBL_CTX_END();
}


GBL_EXPORT GblType EvmuMemory_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuMemory"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &((const GblTypeInfo) {
                                          .pFnClassInit          = EvmuMemoryClass_init_,
                                          .classSize             = sizeof(EvmuMemoryClass),
                                          .instanceSize          = sizeof(EvmuMemory),
                                          .instancePrivateSize   = sizeof(EvmuMemory_)
                                      }),
                                      GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}


#include <evmu/hw/evmu_ram.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_ram_.h"
#include "evmu_device_.h"
#include "evmu_buzzer_.h"
#include "evmu_timers_.h"
#include "evmu_gamepad_.h"
#include "evmu_rom_.h"
#include <gimbal/utils/gimbal_date_time.h>

EVMU_EXPORT EvmuAddress EvmuRam_indirectAddress(const EvmuRam* pSelf, size_t mode) {
    EvmuAddress value = 0;
    GBL_CTX_BEGIN(pSelf);

    GBL_CTX_VERIFY(mode <= 3, GBL_RESULT_ERROR_OUT_OF_RANGE, "Invalid indirection mode: [%x]", mode);
    value = (EvmuRam_readData(pSelf,
                mode |
                ((EvmuRam_viewData(pSelf, EVMU_ADDRESS_SFR_PSW) &
                  (EVMU_SFR_PSW_IRBK0_MASK|EVMU_SFR_PSW_IRBK1_MASK)) >> 0x1u)) //Bits 2-3 come from PSW
   | (mode&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction

    GBL_CTX_END_BLOCK();
    return value;
}

/* On the VMU, all read-modify-write instructions read from
 * latches, rather than the actual pin/port output to keep
 * data coherent. Otherwise instructions would have unintended
 * side-effects since latch and port values can be diffferent,
 * meaning a SET1 could be setting more than just 1 bit.
 */
EVMU_EXPORT EvmuWord EvmuRam_readDataLatch(const EvmuRam* pSelf, EvmuAddress addr) {
    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    switch(addr) {
    case EVMU_ADDRESS_SFR_T1L:
    case EVMU_ADDRESS_SFR_T1H:
    case EVMU_ADDRESS_SFR_P1:
    case EVMU_ADDRESS_SFR_P3:
    case EVMU_ADDRESS_SFR_P7:
        return pSelf_->pIntMap[addr/EVMU_RAM__INT_SEGMENT_SIZE_][addr%EVMU_RAM__INT_SEGMENT_SIZE_];
    default:
        return EvmuRam_readData(pSelf, addr); //fall through to memory for non-latch data
    }
}

EVMU_EXPORT EvmuWord EvmuRam_readData(const EvmuRam* pSelf, EvmuAddress addr) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);

    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    EvmuDevice*  pDev   = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDev_  = EVMU_DEVICE_(pDev);

    //Write out other DNE SFR register bits to return 1s for H regions.

    //CHECK IF WRITE-ONLY REGISTER:
    switch(addr) {
    case EVMU_ADDRESS_SFR_P1DDR:
    case EVMU_ADDRESS_SFR_P1FCR:
    case EVMU_ADDRESS_SFR_P3DDR:
    case EVMU_ADDRESS_SFR_MCR:
    case EVMU_ADDRESS_SFR_VCCR:
        //_gyLog(GY_DEBUG_WARNING, "MEMORY[%x]: READ from WRITE-ONLY register!", addr);
        value = 0xff;    //Return (hopefully hardware-accurate) bullshit.
        GBL_CTX_DONE();
    }

    switch(addr) {
    case EVMU_ADDRESS_SFR_VTRBF: { //Reading from separate working memory
        value = EvmuWram_readByte(pDev->pWram,
                                  EvmuWram_accessAddress(pDev->pWram));
        //must auto-increment pointer if VSEL_INCE is set
        if(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VSEL)] & EVMU_SFR_VSEL_INCE_MASK) {
#if 0
            const uint16_t vrmad = ((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8) |
                                    pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)]) + 1;

            pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)] = vrmad & 0xff;
            pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] = (vrmad >> 8) & 0x1;

#else
            //check for 8-bit overflow after incrementing VRMAD1
            if(!++pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])
                //carry 9th bit to VRMAD2
                pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] ^= 1;
        #endif
        }
        GBL_CTX_DONE();
    }
    case EVMU_ADDRESS_SFR_T0L:
        value = pDev_->pTimers->timer0.base.tl; GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_T0H:
        value = pDev_->pTimers->timer0.base.th; GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_T1L:
        value = pDev_->pTimers->timer1.base.tl; GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_T1H:
        value = pDev_->pTimers->timer1.base.th; GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_VRMAD2:
        value = 0xfe|(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]&0x1);
        GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_P1:
        value = 0; GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_P3:
        value = EvmuGamepad__port3Value_(pDev_->pGamepad);
        GBL_CTX_DONE();
    case EVMU_ADDRESS_SFR_P7:
        value = 0xf0|(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)]);
        GBL_CTX_DONE();
    default: {
        GBL_CTX_VERIFY(addr/EVMU_RAM__INT_SEGMENT_SIZE_ < EVMU_RAM__INT_SEGMENT_COUNT_,
                       GBL_RESULT_ERROR_OUT_OF_RANGE,
                       "Out-of-range read attempted: [%x]", addr);
        value = pSelf_->pIntMap[addr/EVMU_RAM__INT_SEGMENT_SIZE_][addr%EVMU_RAM__INT_SEGMENT_SIZE_];
    }
        GBL_CTX_END_BLOCK();
    }
    return value;
}

EVMU_EXPORT EvmuWord EvmuRam_viewData(const EvmuRam* pSelf, EvmuAddress address) {
    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);

    EvmuWord value = 0;
    if(address == EVMU_ADDRESS_SFR_VTRBF) {
        EvmuWram* pWram = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf))->pWram;
        value = EvmuWram_readByte(pWram, EvmuWram_accessAddress(pWram));
    } else {
        value = EvmuRam_readData(pSelf, address);
    }

    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuRam_writeData(EvmuRam* pSelf, EvmuAddress addr, EvmuWord val) {
    GBL_CTX_BEGIN(pSelf);

    EvmuRam_* pSelf_  = EVMU_RAM_(pSelf);
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDev_   = EVMU_DEVICE_(pDevice);

    //Check for SFRs with side-effects
    switch(addr) {
    case EVMU_ADDRESS_SFR_ACC: {
        if(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)] != val) {
            //pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)] &= ~SFR_PSW_P_MASK|getParity(val);
            pSelf_->sfr[0x01] = (pSelf_->sfr[0x01]&0xfe)|gblParity(val);
        }
        break;
    }
    case EVMU_ADDRESS_SFR_VTRBF: //Writing to separate working memory
        EvmuWram_writeByte(pDevice->pWram,
                           EvmuWram_accessAddress(pDevice->pWram),
                           val);
        //must auto-increment pointer if VSEL_INCE is set
        if(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VSEL)]&EVMU_SFR_VSEL_INCE_MASK) {
        #if 1
            //check for 8-bit overflow after incrementing VRMAD1
            if(!++pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])
                //carry 9th bit to VRMAD2
                pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] ^= 1;
#else
            const uint16_t vrmad = ((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8) |
                                    pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)]) + 1;

            pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)] = vrmad & 0xff;
            pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] = (vrmad >> 8) & 0x1;

#endif
        }
        GBL_CTX_DONE();
        break;
    case EVMU_ADDRESS_SFR_EXT: {
#if 1
        //changing CPU mode (change imem between BIOS in rom and APP in flash)
        const EvmuWord mode = val & 0x1;
        if((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)]&0x1) != mode) {
            const EvmuAddress pc = EvmuCpu_pc(pDevice->pCpu);

            //next instr must be JMPF, do it now, since imem is changing
            //if(pc > 0xfffd || pSelf_->pExt[pc] != OPCODE_JMPF) {
               // GBL_CTX_WARN("Attempt to change EXT ROM source at invalid PC or without JMPF!");
           // } else {
            if(pSelf_->pExt[pc] == EVMU_OPCODE_JMPF) {


                EvmuCpu_setPc(pDevice->pCpu, (pSelf_->pExt[pc+1]<<8) | pSelf_->pExt[pc+2]);
            }

            if(!mode) pSelf_->pExt = pSelf_->pRom->pStorage->pData;
            else pSelf_->pExt = pSelf_->pFlash->pStorage->pData;
        }
        break;
#endif
    }
    case EVMU_ADDRESS_SFR_XBNK:{ //changing XRAM bank
        if( pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_XBNK)] != val /*&&
                !(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] &
                  0x40)*/) {
            GBL_CTX_VERIFY(val <= 2,
                           GBL_RESULT_ERROR_OUT_OF_RANGE,
                           "[XRAM]: Attempted to set invalid bank. [%u]", val);
            pSelf_->pIntMap[EVMU_RAM__INT_SEGMENT_XRAM_] = pSelf_->xram[val];
        }
        break;
    }
    case EVMU_ADDRESS_SFR_PSW: {
        unsigned char psw = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)];
        //Check if changing RAM bank
        if((psw&EVMU_SFR_PSW_RAMBK0_MASK) != (val&EVMU_SFR_PSW_RAMBK0_MASK)) {
            unsigned newIndex = (val&EVMU_SFR_PSW_RAMBK0_MASK)>>EVMU_SFR_PSW_RAMBK0_POS;
            GBL_ASSERT(newIndex == 0 || newIndex == 1);
            pSelf_->pIntMap[EVMU_RAM__INT_SEGMENT_GP1_] = pSelf_->ram[newIndex];
            pSelf_->pIntMap[EVMU_RAM__INT_SEGMENT_GP2_] = &pSelf_->ram[newIndex][EVMU_RAM__INT_SEGMENT_SIZE_];
        }
        break;
    }
    case EVMU_ADDRESS_SFR_T0PRR:
        pDev_->pTimers->timer0.tscale = 256 - val;
        pDev_->pTimers->timer0.tbase  = 0;
        break;
    case EVMU_ADDRESS_SFR_T0CNT:
        if(!(val&EVMU_SFR_T0CNT_P0LRUN_MASK))
            pDev_->pTimers->timer0.base.tl = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)];
        if(!(val&EVMU_SFR_T0CNT_P0HRUN_MASK))
            pDev_->pTimers->timer0.base.th = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)];
        break;
    case EVMU_ADDRESS_SFR_T0LR:
        if(!(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_P0LRUN_MASK))
            pDev_->pTimers->timer0.base.tl = val;
        break;
    case EVMU_ADDRESS_SFR_T0HR:
        if(!(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&EVMU_SFR_T0CNT_P0HRUN_MASK))
            pDev_->pTimers->timer0.base.th = val;
        break;
    case EVMU_ADDRESS_SFR_T1CNT:
        if(!(val&EVMU_SFR_T1CNT_T1LRUN_MASK))
            pDev_->pTimers->timer1.base.tl = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)];
        if(!(val&EVMU_SFR_T1CNT_T1HRUN_MASK))
            pDev_->pTimers->timer1.base.th = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)];
        break;
    // WHAT ABOUT THE ELCTL OR IMMEDIATE UPDATE FLAG!?!??
    case EVMU_ADDRESS_SFR_T1LR:
        if(!(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)]&EVMU_SFR_T1CNT_T1LRUN_MASK))
            pDev_->pTimers->timer1.base.tl = val;
        break;
    case EVMU_ADDRESS_SFR_T1HR:
        if(!(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)]&EVMU_SFR_T1CNT_T1HRUN_MASK))
            pDev_->pTimers->timer1.base.th = val;
        break;
    case EVMU_ADDRESS_SFR_P3DDR:
        //if((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)]&EVMU_SFR_EXT_MASK) == EVMU_SFR_EXT_USER) {
         //   _gyLog(GY_DEBUG_WARNING, "MEMORY WRITE: Attempted to write to P3DDR register which is not allowed in user mode!");
        //}
        break;
    case EVMU_ADDRESS_SFR_SCON0:
        break;
    case EVMU_ADDRESS_SFR_SCON1:
        break;
    case EVMU_ADDRESS_SFR_VCCR: {
        int prevVal = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)];
        //if true, toggling LCD on, false off
        if((prevVal&EVMU_SFR_VCCR_VCCR7_MASK) ^ (val&EVMU_SFR_VCCR_VCCR7_MASK)) {
            EvmuLcd_setScreenEnabled(pDevice->pLcd, (val&EVMU_SFR_VCCR_VCCR7_MASK));
        }
    }
    case EVMU_ADDRESS_SFR_PCON:
     //   GBL_CTX_VERBOSE("PCON: %x", val);
    default: break;
    }

    GBL_CTX_VERIFY(addr/EVMU_RAM__INT_SEGMENT_SIZE_ < EVMU_RAM__INT_SEGMENT_COUNT_,
                   GBL_RESULT_ERROR_OUT_OF_RANGE,
                   "Out-of-range write attempted: %x to %x",
                   addr, val);


    if((addr >= EVMU_ADDRESS_SEGMENT_XRAM_BASE && addr <= EVMU_ADDRESS_SEGMENT_XRAM_END) &&
            !(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] &
                              0x40)) {
        if(pSelf_->pIntMap[addr/EVMU_RAM__INT_SEGMENT_SIZE_][addr%EVMU_RAM__INT_SEGMENT_SIZE_] != val) {
            pDevice->pLcd->screenChanged = GBL_TRUE;
        }
    }

    //do actual memory write
    pSelf_->pIntMap[addr/EVMU_RAM__INT_SEGMENT_SIZE_][addr%EVMU_RAM__INT_SEGMENT_SIZE_] = val;

    EvmuBuzzer__memorySink_(EVMU_BUZZER_(pDevice->pBuzzer), addr, val);

    GBL_CTX_END();
}

EVMU_EXPORT EvmuWord EvmuRam_readProgram(const EvmuRam* pSelf, EvmuAddress addr) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);

    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);

    if(pSelf_->pExt == pSelf_->pFlash->pStorage->pData) {
        GBL_CTX_VERIFY(addr < EVMU_FLASH_SIZE,
                       GBL_RESULT_ERROR_OUT_OF_RANGE,
                       "[EXT]: Invalid flash read address. [%x]", addr);
    } else {
        GBL_CTX_VERIFY(addr < EVMU_ROM_SIZE,
                       GBL_RESULT_ERROR_OUT_OF_RANGE,
                       "[EXT]: Invalid ROM read address. [%x]", addr);
    }

    value = pSelf_->pExt[addr];

    GBL_CTX_END_BLOCK();
    return value;
}

EVMU_EXPORT EVMU_PROGRAM_SRC EvmuRam_programSrc(const EvmuRam* pSelf) {
    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    return pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)];
}

EVMU_EXPORT EVMU_RESULT EvmuRam_setProgramSrc(EvmuRam* pSelf, EVMU_PROGRAM_SRC src) {
    GBL_CTX_BEGIN(NULL);

    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    EvmuRam_writeData(pSelf, EVMU_ADDRESS_SFR_EXT, src);
/*
    switch(src) {
    case EVMU_PROGRAM_SRC__FLASH_BANK_1:
    case EVMU_PROGRAM_SRC__ROM:
        pSelf_->pExt = pSelf_->rom;
        break;
    case EVMU_PROGRAM_SRC__FLASH_BANK_0:
        pSelf_->pExt = pSelf_->flash;
        break;

        pSelf_->pExt = &pSelf_->flash[EVMU_FLASH_BANK_SIZE];
        break;
    default:
        GBL_CTX_VERIFY(GBL_FALSE,
                       GBL_RESULT_ERROR_INVALID_ARG,
                       "Attempted to change EXT to invalid source: %x",
                        src);
    }

    //pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = src;
*/
    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuRam_writeProgram(EvmuRam* pSelf,
                                            EvmuAddress addr,
                                            EvmuWord    value) {
    GBL_CTX_BEGIN(pSelf);

    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);

    if(pSelf_->pExt == pSelf_->pFlash->pStorage->pData) {
        GBL_CTX_VERIFY(addr < EVMU_FLASH_SIZE,
                       GBL_RESULT_ERROR_OUT_OF_RANGE,
                       "[EXT]: Invalid flash write address. [%x]", addr);
    } else {
        GBL_CTX_VERIFY(addr < EVMU_ROM_SIZE,
                       GBL_RESULT_ERROR_OUT_OF_RANGE,
                       "[EXT]: Invalid ROM write address. [%x]", addr);
    }

    pSelf_->pExt[addr] = value;

    GBL_CTX_END();
}

EVMU_EXPORT int EvmuRam_stackDepth(const EvmuRam* pSelf) {
    int depth = 0;
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);

    depth = EVMU_RAM_(pSelf)->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)] -
            (EVMU_ADDRESS_SEGMENT_STACK_BASE-1);

    GBL_CTX_END_BLOCK();
    return depth;
}

EVMU_EXPORT EvmuWord EvmuRam_viewStack(const EvmuRam* pSelf, size_t depth) {
    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);


    GBL_CTX_VERIFY((int)depth < EvmuRam_stackDepth(pSelf),
                   GBL_RESULT_ERROR_OUT_OF_RANGE);

    value = pSelf_->ram[0][pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)] - depth];

    GBL_CTX_END_BLOCK();
    return value;
}

EVMU_EXPORT EvmuWord EvmuRam_popStack(EvmuRam* pSelf) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);

    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    EvmuWord* pSp    = &pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)];

    value = pSelf_->ram[0][(*pSp)--];

    GBL_CTX_VERIFY(*pSp+1 >= EVMU_ADDRESS_SYSTEM_STACK_BASE,
                   EVMU_RESULT_ERROR_STACK_UNDERFLOW,
                   "POP: Stack underflow detected. [%d]",
                   *pSp - EVMU_ADDRESS_SYSTEM_STACK_BASE);

    GBL_CTX_END_BLOCK();

    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuRam_pushStack(EvmuRam* pSelf, EvmuWord value) {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);

    EvmuRam_* pSelf_ = EVMU_RAM_(pSelf);
    EvmuWord* pSp    = &pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)];

    pSelf_->ram[0][++(*pSp)] = value;

    GBL_CTX_VERIFY(*pSp <= EVMU_ADDRESS_SYSTEM_STACK_END,
                   EVMU_RESULT_ERROR_STACK_OVERFLOW,
                   "PUSH: Stack underflow detected. [%u],",
                   *pSp - EVMU_ADDRESS_SYSTEM_STACK_END);

    GBL_CTX_END();
}

static GBL_RESULT EvmuRam_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructor, pSelf);

    GblObject_setName(pSelf, EVMU_RAM_NAME);
    GBL_CTX_END();
}

static GBL_RESULT EvmuRam_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuRam_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_VERBOSE("Resetting Memory.");

    EvmuRam*  pRam  = EVMU_RAM(pSelf);
    EvmuDevice*  pDevice  = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);

    GBL_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    GblBool skipSetup = pDevice_->pRom->bSetupSkipEnabled;

    unsigned char sfr_bin[] = {
            0x00, 0x00, 0xff, 0x00, 0x66, 0x19, 0x7f, 0x00, 0x83, 0x02, 0x00, 0x00,
            0x00, 0x08, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x05,
            0x20, 0x01, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x80, 0xbf, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x06, 0x00, 0x00, 0x20, 0x00, 0x00,
            0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xc0,
            0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79
    };

    memset(pDevice_->pRam->ram, 0, EVMU_ADDRESS_SEGMENT_RAM_SIZE*EVMU_ADDRESS_SEGMENT_RAM_BANKS);
    memset(pDevice_->pRam->sfr, 0, EVMU_ADDRESS_SEGMENT_SFR_SIZE);
    memset(pDevice_->pRam->xram, 0, EVMU_ADDRESS_SEGMENT_XRAM_SIZE*EVMU_ADDRESS_SEGMENT_XRAM_BANKS);


    pDevice_->pRam->pIntMap[EVMU_RAM__INT_SEGMENT_XRAM_]       = pDevice_->pRam->xram[EVMU_XRAM_BANK_LCD_TOP];
    pDevice_->pRam->pIntMap[EVMU_RAM__INT_SEGMENT_SFR_]        = pDevice_->pRam->sfr;
    pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]   = EVMU_ADDRESS_SEGMENT_STACK_BASE-1;    //Initialize stack pointer
    pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)]   = 0xff;                     //Reset all P3 pins (controller buttons)
    pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]  = EVMU_SFR_PSW_RAMBK0_MASK;

    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P7, EVMU_SFR_P7_P71_MASK);
    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_IE, 0xff);
    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_IP, 0x00);
    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P1FCR,  0xbf);
    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P3INT,  0xfd);
    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_ISL,    0xc0);
    EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_VSEL,   0xfc);
    //EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_BTCR,   0x41);

    pDevice_->pTimers->timer0.tscale = 256;
    GBL_CTX_INFO("Resetting PC.");
    if(skipSetup && EvmuRom_biosType(pDevice->pRom) != EVMU_BIOS_TYPE_EMULATED) {
        EvmuCpu_setPc(pDevice->pCpu, EVMU_BIOS_SKIP_DATE_TIME_PC);
    } else {
        EvmuCpu_setPc(pDevice->pCpu, 0x0);
    }

    if(EvmuRom_biosType(pDevice->pRom) != EVMU_BIOS_TYPE_EMULATED) {
       // pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)]   |= SFR_P7_P71_MASK;
        pDevice_->pRam->pIntMap[EVMU_RAM__INT_SEGMENT_GP1_]        = pDevice_->pRam->ram[0];
        pDevice_->pRam->pIntMap[EVMU_RAM__INT_SEGMENT_GP2_]        = &pDevice_->pRam->ram[0][EVMU_RAM__INT_SEGMENT_SIZE_];
        //EvmuRam_setProgramSrc(pRam, EVMU_PROGRAM_SRC__ROM);
        pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 0;
        pDevice_->pRam->pExt = pDevice_->pRom->pStorage->pData;
    } //else {

        //Initialize System Variables
        GblDateTime dateTime;
        EvmuRom_setDateTime(pDevice->pRom, GblDateTime_nowLocal(&dateTime));

        if(EvmuRom_biosType(pDevice->pRom) == EVMU_BIOS_TYPE_EMULATED) {
            pDevice_->pRam->pIntMap[EVMU_RAM__INT_SEGMENT_GP1_]    = pDevice_->pRam->ram[1];
            pDevice_->pRam->pIntMap[EVMU_RAM__INT_SEGMENT_GP2_]    = &pDevice_->pRam->ram[1][EVMU_RAM__INT_SEGMENT_SIZE_];
            //EvmuRam_setProgramSrc(pRam, EVMU_PROGRAM_SRC__FLASH_BANK_0);
            pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 1;
            pDevice_->pRam->pExt = pDevice_->pFlash->pStorage->pData;
        }
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_ICON);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_XRAM_ICN_GAME, 0x10);           //Enable Game Icon

        //SFR values initialized by BIOS (from Sega Documentation)
        // not according to docs, but testing
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P1DDR,  0xff);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P1FCR,  0xbf);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P3INT,  0xfd);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_P3DDR,  0x00);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_ISL,    0xc0);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_VSEL,   0xfc);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_BTCR,   0x41);

        //pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] = SFR_IE_IE7_MASK;
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_IE, 0xff);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_IP, 0x00);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_OCR, EVMU_SFR_OCR_OCR7_MASK|EVMU_SFR_OCR_OCR0_MASK); //stop main clock, divide active clock by 6
        pDevice_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)] = EVMU_SFR_P7_P71_MASK;

        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_LCD_TOP);
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_VCCR, EVMU_SFR_VCCR_VCCR7_MASK);     //turn on LCD
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_MCR, EVMU_SFR_MCR_MCR3_MASK);        //enable LCD update
        EvmuRam_writeData(pRam, EVMU_ADDRESS_SFR_PCON, 0);                      //Disable HALT/HOLD modes, run CPU normally.

    if (skipSetup && EvmuRom_biosType(pDevice->pRom) != EVMU_BIOS_TYPE_EMULATED) {
        for (int i = 0; i < EVMU_ADDRESS_SEGMENT_SFR_SIZE; i++) {
            EvmuRam_writeData(pRam, 0x100 + i, sfr_bin[i]);
        }
    }

    GBL_CTX_END();
}

static EVMU_RESULT EvmuRam_IMemory_readBytes_(const EvmuIMemory* pSelf,
                                                 EvmuAddress        address,
                                                 void*              pBuffer,
                                                 size_t*            pBytes)
{
    GBL_CTX_BEGIN(NULL);

    for(size_t b = 0; b < *pBytes; ++b)
        ((uint8_t*)pBuffer)[b] =
                EvmuRam_viewData(EVMU_RAM(pSelf),
                                    address + b);

    GBL_CTX_END();
}

static EVMU_RESULT EvmuRam_IMemory_writeBytes_(EvmuIMemory* pSelf,
                                               EvmuAddress  address,
                                               const void*  pBuffer,
                                               size_t*      pBytes)
{
    // Clear our call record
    GBL_CTX_BEGIN(NULL);

    for(size_t b = 0; b < *pBytes; ++b)
        EvmuRam_writeData(EVMU_RAM(pSelf),
                          address + b,
                          ((uint8_t*)pBuffer)[b]);


    GBL_VCALL_DEFAULT(EvmuIMemory, pFnWrite, pSelf, address, pBuffer, pBytes);

    // End call record, return result
    GBL_CTX_END();
}

static GBL_RESULT EvmuRamClass_init_(GblClass* pClass, const void* pData) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(NULL);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuRam_reset_;
    GBL_OBJECT_CLASS(pClass)    ->pFnConstructor = EvmuRam_constructor_;
    GBL_BOX_CLASS(pClass)       ->pFnDestructor  = EvmuRam_destructor_;
    EVMU_IMEMORY_CLASS(pClass)  ->pFnRead        = EvmuRam_IMemory_readBytes_;
    EVMU_IMEMORY_CLASS(pClass)  ->pFnWrite       = EvmuRam_IMemory_writeBytes_;
    EVMU_IMEMORY_CLASS(pClass)  ->capacity       = EVMU_RAM__INT_SEGMENT_COUNT_ *
                                                   EVMU_RAM__INT_SEGMENT_SIZE_;

    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuRam_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    static GblInterfaceImpl ifaces[] = {
        { .classOffset = offsetof(EvmuRamClass, EvmuIMemoryImpl) }
    };

    const static GblTypeInfo info = {
        .pFnClassInit          = EvmuRamClass_init_,
        .classSize             = sizeof(EvmuRamClass),
        .instanceSize          = sizeof(EvmuRam),
        .instancePrivateSize   = sizeof(EvmuRam_),
        .pInterfaceImpls       = ifaces,
        .interfaceCount        = 1
    };

    if(type == GBL_INVALID_TYPE) GBL_UNLIKELY {
        ifaces[0].interfaceType = EVMU_IMEMORY_TYPE;
        type = GblType_register(GblQuark_internStatic("EvmuRam"),
                                EVMU_PERIPHERAL_TYPE,
                                &info,
                                GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}


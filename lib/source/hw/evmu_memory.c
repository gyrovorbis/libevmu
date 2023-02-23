#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_memory_.h"
#include "evmu_device_.h"
#include "evmu_buzzer_.h"
#include "evmu_timers_.h"

EVMU_EXPORT EvmuAddress EvmuMemory_indirectAddress(const EvmuMemory* pSelf, uint8_t mode) {
    EvmuAddress value = 0;
    GBL_CTX_BEGIN(pSelf);

    GBL_CTX_VERIFY(mode <= 3, GBL_RESULT_ERROR_OUT_OF_RANGE, "Invalid indirection mode: [%x]", mode);
    value = (EvmuMemory_readInt(pSelf,
                mode |
                ((EvmuMemory_viewInt(pSelf, EVMU_ADDRESS_SFR_PSW) &
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
EVMU_EXPORT EvmuWord EvmuMemory_readIntLatch(const EvmuMemory* pSelf, EvmuAddress addr) {
    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
    switch(addr) {
    case EVMU_ADDRESS_SFR_T1L:
    case EVMU_ADDRESS_SFR_T1H:
    case EVMU_ADDRESS_SFR_P1:
    case EVMU_ADDRESS_SFR_P3:   //3 and 7 really SHOULDN'T matter, since the port output should equal the latch...
    case EVMU_ADDRESS_SFR_P7:
        return pSelf_->pIntMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE];
    default:
        return EvmuMemory_readInt(pSelf, addr); //fall through to memory for non-latch data
    }
}

EVMU_EXPORT EvmuWord EvmuMemory_readInt(const EvmuMemory* pSelf, EvmuAddress addr) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
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
        value = 0xFF;    //Return (hopefully hardware-accurate) bullshit.
        GBL_CTX_DONE();
    }

    switch(addr) {
    case EVMU_ADDRESS_SFR_VTRBF: { //Reading from separate working memory
        value = pSelf_->wram[0x1ff&((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8)
                                   | pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])];
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
    case EVMU_ADDRESS_SFR_P7:
        value = 0xf0|(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)]);
        GBL_CTX_DONE();
    default: {
        GBL_CTX_VERIFY(addr/EVMU_MEMORY__INT_SEGMENT_SIZE_ < EVMU_MEMORY__INT_SEGMENT_COUNT_,
                       GBL_RESULT_ERROR_OUT_OF_RANGE,
                       "Out-of-range read attempted: [%x]", addr);
        value = pSelf_->pIntMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE];
    }
        GBL_CTX_END_BLOCK();
    }
    return value;
}

EVMU_EXPORT EvmuWord EvmuMemory_viewInt(const EvmuMemory* pSelf, EvmuAddress address) {
    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);

    EvmuWord value = 0;
    if(address == EVMU_ADDRESS_SFR_VTRBF) {
        value = pSelf_->wram[0x1ff&((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8)
                                   | pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])];
    } else {
        value = EvmuMemory_readInt(pSelf, address);
    }

    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeInt(EvmuMemory* pSelf, EvmuAddress addr, EvmuWord val) {
    GBL_CTX_BEGIN(pSelf);

    EvmuMemory_* pSelf_  = EVMU_MEMORY_(pSelf);
    EvmuDevice*  pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDev_   = EVMU_DEVICE_(pDevice);
    VMUDevice*   dev     = EVMU_DEVICE_REEST(pDevice);

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
        pSelf_->wram[0x1ff & ((pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8)
                           | pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])] = val;
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

            if(!mode) pSelf_->pExt = pSelf_->rom;
            else pSelf_->pExt = pSelf_->flash;
        }
        break;
#endif
    }
    case EVMU_ADDRESS_SFR_XBNK:{ //changing XRAM bank
        if(/* pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_XBNK)] != val && */
                !(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] &
                  0x40)) {
            GBL_CTX_VERIFY(val <= 2,
                           GBL_RESULT_ERROR_OUT_OF_RANGE,
                           "[XRAM]: Attempted to set invalid bank. [%u]", val);
            pSelf_->pIntMap[VMU_MEM_SEG_XRAM] = pSelf_->xram[val];
        }
        break;
    }
    case EVMU_ADDRESS_SFR_PSW: {
        unsigned char psw = pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)];
        //Check if changing RAM bank
        if((psw&EVMU_SFR_PSW_RAMBK0_MASK) != (val&EVMU_SFR_PSW_RAMBK0_MASK)) {
            unsigned newIndex = (val&EVMU_SFR_PSW_RAMBK0_MASK)>>EVMU_SFR_PSW_RAMBK0_POS;
            GBL_ASSERT(newIndex == 0 || newIndex == 1);
            pSelf_->pIntMap[VMU_MEM_SEG_GP1] = pSelf_->ram[newIndex];
            pSelf_->pIntMap[VMU_MEM_SEG_GP2] = &pSelf_->ram[newIndex][VMU_MEM_SEG_SIZE];
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
        if((prevVal&EVMU_SFR_VCCR_VCCR7_MASK) != (val&EVMU_SFR_VCCR_VCCR7_MASK)) {
            EvmuLcd_setUpdated(pDevice->pLcd, GBL_TRUE);
        }
    }
    case EVMU_ADDRESS_SFR_PCON:
     //   GBL_CTX_VERBOSE("PCON: %x", val);
    default: break;
    }

    GBL_CTX_VERIFY(addr/EVMU_MEMORY__INT_SEGMENT_SIZE_ < EVMU_MEMORY__INT_SEGMENT_COUNT_,
                   GBL_RESULT_ERROR_OUT_OF_RANGE,
                   "Out-of-range write attempted: %x to %x",
                   addr, val);


    if((addr >= EVMU_ADDRESS_SEGMENT_XRAM_BASE && addr <= EVMU_ADDRESS_SEGMENT_XRAM_END) &&
            !(pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)] &
                              0x40)) {
        if(pSelf_->pIntMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE] != val) {
            EvmuLcd_setUpdated(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pLcd, GBL_TRUE);
        }
    }

    //do actual memory write
    pSelf_->pIntMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE] = val;

    EvmuBuzzer__memorySink(EVMU_DEVICE_PRISTINE(dev)->pBuzzer, addr, val);

    if(dev->pFnMemoryChange)
        dev->pFnMemoryChange(dev, (uint16_t)addr);

    GBL_CTX_END();
}

EVMU_EXPORT EvmuWord EvmuMemory_readExt(const EvmuMemory* pSelf, EvmuAddress addr) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);

    if(pSelf_->pExt == pSelf_->flash) {
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

EVMU_EXPORT EVMU_MEMORY_EXT_SRC EvmuMemory_extSource(const EvmuMemory* pSelf) {
    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
    return pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)];
}

EVMU_EXPORT EVMU_RESULT EvmuMemory_setExtSource(EvmuMemory* pSelf, EVMU_MEMORY_EXT_SRC src) {
    GBL_CTX_BEGIN(NULL);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
    EvmuMemory_writeInt(pSelf, EVMU_ADDRESS_SFR_EXT, src);
/*
    switch(src) {
    case EVMU_MEMORY_EXT_SRC_FLASH_BANK_1:
    case EVMU_MEMORY_EXT_SRC_ROM:
        pSelf_->pExt = pSelf_->rom;
        break;
    case EVMU_MEMORY_EXT_SRC_FLASH_BANK_0:
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

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeExt(EvmuMemory* pSelf,
                                            EvmuAddress addr,
                                            EvmuWord    value) {
    GBL_CTX_BEGIN(pSelf);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);

    if(pSelf_->pExt == pSelf_->flash) {
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

EVMU_EXPORT EvmuWord EvmuMemory_readFlash(const EvmuMemory* pSelf, EvmuAddress addr) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);

    value = EVMU_MEMORY_(pSelf)->flash[addr];

    GBL_CTX_END_BLOCK();
    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeFlash(EvmuMemory* pSelf,
                                              EvmuAddress addr,
                                              EvmuWord    value) {
    GBL_CTX_BEGIN(pSelf);

    EVMU_MEMORY_(pSelf)->flash[addr] = value;

    GBL_CTX_END();
}

EVMU_EXPORT int EvmuMemory_stackDepth(const EvmuMemory* pSelf) {
    int depth = 0;
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);

    depth = EVMU_MEMORY_(pSelf)->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)] -
            (EVMU_ADDRESS_SEGMENT_STACK_BASE-1);

    GBL_CTX_END_BLOCK();
    return depth;
}

EVMU_EXPORT EvmuWord EvmuMemory_viewStack(const EvmuMemory* pSelf, GblSize depth) {
    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);


    GBL_CTX_VERIFY((int)depth < EvmuMemory_stackDepth(pSelf),
                   GBL_RESULT_ERROR_OUT_OF_RANGE);

    value = pSelf_->ram[0][pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)] - depth];

    GBL_CTX_END_BLOCK();
    return value;
}

EVMU_EXPORT EvmuWord EvmuMemory_popStack(EvmuMemory* pSelf) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
    EvmuWord*    pSp    = &pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)];

    value = pSelf_->ram[0][(*pSp)--];

    GBL_CTX_VERIFY(*pSp+1 >= EVMU_ADDRESS_SYSTEM_STACK_BASE,
                   EVMU_RESULT_ERROR_STACK_UNDERFLOW,
                   "POP: Stack underflow detected. [%d]",
                   *pSp - EVMU_ADDRESS_SYSTEM_STACK_BASE);

    GBL_CTX_END_BLOCK();

    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuMemory_pushStack(EvmuMemory* pSelf, EvmuWord value) {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);
    EvmuWord*    pSp    = &pSelf_->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)];

    pSelf_->ram[0][++(*pSp)] = value;

    GBL_CTX_VERIFY(*pSp <= EVMU_ADDRESS_SYSTEM_STACK_END,
                   EVMU_RESULT_ERROR_STACK_OVERFLOW,
                   "PUSH: Stack underflow detected. [%u],",
                   *pSp - EVMU_ADDRESS_SYSTEM_STACK_END);

    GBL_CTX_END();
}

EVMU_EXPORT EvmuWord EvmuMemory_readWram(const EvmuMemory* pSelf, EvmuAddress addr) {
    EvmuWord value = 0;
    GBL_CTX_BEGIN(pSelf);

    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY(addr < EVMU_WRAM_SIZE,
                   GBL_RESULT_ERROR_OUT_OF_RANGE,
                   "WRAM: read out-of-range. [%x]", addr);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);

    value = pSelf_->wram[addr];

    GBL_CTX_END_BLOCK();
    return value;
}

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeWram(EvmuMemory* pSelf,
                                             EvmuAddress addr,
                                             EvmuWord    value) {
    GBL_CTX_BEGIN(pSelf);
    GBL_CTX_VERIFY_POINTER(pSelf);
    GBL_CTX_VERIFY(addr < EVMU_WRAM_SIZE,
                   GBL_RESULT_ERROR_OUT_OF_RANGE,
                   "WRAM: write out-of-range. [%x]", addr);

    EvmuMemory_* pSelf_ = EVMU_MEMORY_(pSelf);

    pSelf_->wram[addr] = value;

    GBL_CTX_END();
}

static GBL_RESULT EvmuMemory_constructor_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructor, pSelf);

    GblObject_setName(pSelf, EVMU_MEMORY_NAME);
    GBL_CTX_END();
}

static GBL_RESULT EvmuMemory_destructor_(GblBox* pSelf) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.base.pFnDestructor, pSelf);
    GBL_CTX_END();
}

static GBL_RESULT EvmuMemory_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERBOSE("Resetting Memory.");

    EvmuMemory*  pMemory  = EVMU_MEMORY(pSelf);
    EvmuDevice*  pDevice  = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    memset(pDevice_->pMemory->ram, 0, EVMU_ADDRESS_SEGMENT_RAM_SIZE*EVMU_ADDRESS_SEGMENT_RAM_BANKS);
    memset(pDevice_->pMemory->sfr, 0, EVMU_ADDRESS_SEGMENT_SFR_SIZE);
    memset(pDevice_->pMemory->wram, 0, EVMU_WRAM_SIZE);
    memset(pDevice_->pMemory->xram, 0, EVMU_ADDRESS_SEGMENT_XRAM_SIZE*EVMU_ADDRESS_SEGMENT_XRAM_BANKS);


    pDevice_->pMemory->pIntMap[VMU_MEM_SEG_XRAM]       = pDevice_->pMemory->xram[EVMU_XRAM_BANK_LCD_TOP];
    pDevice_->pMemory->pIntMap[VMU_MEM_SEG_SFR]        = pDevice_->pMemory->sfr;
    pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SP)]   = EVMU_ADDRESS_SEGMENT_STACK_BASE-1;    //Initialize stack pointer
    pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P3)]   = 0xff;                     //Reset all P3 pins (controller buttons)
    pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)]  = EVMU_SFR_PSW_RAMBK0_MASK;

    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P7, EVMU_SFR_P7_P71_MASK);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IE, 0xff);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IP, 0x00);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P1FCR,  0xbf);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P3INT,  0xfd);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ISL,    0xc0);
    EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_VSEL,   0xfc);
    //EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_BTCR,   0x41);

    pDevice_->pTimers->timer0.tscale = 256;
    EvmuCpu_setPc(pDevice->pCpu, 0x0);

    if(EvmuRom_biosLoaded(pDevice->pRom)) {
       // pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)]   |= SFR_P7_P71_MASK;
        pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP1]        = pDevice_->pMemory->ram[VMU_RAM_BANK0];
        pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP2]        = &pDevice_->pMemory->ram[VMU_RAM_BANK0][VMU_MEM_SEG_SIZE];
        //EvmuMemory_setExtSource(pMemory, EVMU_MEMORY_EXT_SRC_ROM);
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 0;
        pDevice_->pMemory->pExt = pDevice_->pMemory->rom;
    } //else {

        //Initialize System Variables
#if 1
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB_BCD]  = GBL_BCD_BYTE_PACK(tm->tm_year/100+19);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB_BCD]  = GBL_BCD_BYTE_PACK(tm->tm_year%100);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH_BCD]     = GBL_BCD_BYTE_PACK(tm->tm_mon+1);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY_BCD]       = GBL_BCD_BYTE_PACK(tm->tm_mday);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR_BCD]      = GBL_BCD_BYTE_PACK(tm->tm_hour);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE_BCD]    = GBL_BCD_BYTE_PACK(tm->tm_min);
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB]      = (tm->tm_year+1900)>>8;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB]      = (tm->tm_year+1900)&0xff;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH]         = tm->tm_mon+1;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY]           = tm->tm_mday;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR]          = tm->tm_hour;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE]        = tm->tm_min;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_SEC]           = tm->tm_sec;
        pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DATE_SET]      = 0xff;
#endif
        if(!EvmuRom_biosLoaded(pDevice->pRom)) {
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP1]    = pDevice_->pMemory->ram[VMU_RAM_BANK1];
            pDevice_->pMemory->pIntMap[VMU_MEM_SEG_GP2]    = &pDevice_->pMemory->ram[VMU_RAM_BANK1][VMU_MEM_SEG_SIZE];
            //EvmuMemory_setExtSource(pMemory, EVMU_MEMORY_EXT_SRC_FLASH_BANK_0);
            pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)] = 1;
            pDevice_->pMemory->pExt = pDevice_->pMemory->flash;
        }
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_ICON);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_XRAM_ICN_GAME, 0x10);           //Enable Game Icon

        //SFR values initialized by BIOS (from Sega Documentation)
        // not according to docs, but testing
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P1DDR,  0xff);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P1FCR,  0xbf);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_P3INT,  0xfd);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_ISL,    0xc0);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_VSEL,   0xfc);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_BTCR,   0x41);

        //pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_IE)] = SFR_IE_IE7_MASK;
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IE, 0xff);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_IP, 0x00);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_OCR, EVMU_SFR_OCR_OCR7_MASK|EVMU_SFR_OCR_OCR0_MASK); //stop main clock, divide active clock by 6
        pDevice_->pMemory->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)] = EVMU_SFR_P7_P71_MASK;

        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_XBNK, EVMU_XRAM_BANK_LCD_TOP);
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_VCCR, EVMU_SFR_VCCR_VCCR7_MASK);     //turn on LCD
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_MCR, EVMU_SFR_MCR_MCR3_MASK);        //enable LCD update
        EvmuMemory_writeInt(pMemory, EVMU_ADDRESS_SFR_PCON, 0);                      //Disable HALT/HOLD modes, run CPU normally.

    GBL_CTX_END();
}

static GBL_RESULT EvmuMemoryClass_init_(GblClass* pClass, const void* pData, GblContext* pCtx) {
    GBL_UNUSED(pData);
    GBL_CTX_BEGIN(pCtx);

    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset   = EvmuMemory_reset_;
    GBL_OBJECT_CLASS(pClass)->pFnConstructor = EvmuMemory_constructor_;
    GBL_BOX_CLASS(pClass)->pFnDestructor     = EvmuMemory_destructor_;

    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuMemory_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .pFnClassInit          = EvmuMemoryClass_init_,
        .classSize             = sizeof(EvmuMemoryClass),
        .instanceSize          = sizeof(EvmuMemory),
        .instancePrivateSize   = sizeof(EvmuMemory_)
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuMemory"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}


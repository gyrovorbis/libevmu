#ifndef EVMU_MEMORY__H
#define EVMU_MEMORY__H

//#include <evmu/evmu_api.h>
#include <gimbal/algorithms/gimbal_numeric.h>
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_address_space.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_flash.h>
#include <evmu/hw/evmu_wram.h>
#include <evmu/hw/evmu_rom.h>
#include "evmu_cpu_.h"

#define SELF    EvmuMemory_* pSelf
#define CSELF   const SELF

#define EVMU_MEMORY__INT_SEGMENT_SIZE_           128

GBL_DECLS_BEGIN

typedef enum EVMU_MEMORY__INT_SEGMENT_ {
    EVMU_MEMORY__INT_SEGMENT_GP1_,
    EVMU_MEMORY__INT_SEGMENT_GP2_,
    EVMU_MEMORY__INT_SEGMENT_SFR_,
    EVMU_MEMORY__INT_SEGMENT_XRAM_,
    EVMU_MEMORY__INT_SEGMENT_COUNT_
} EVMU_MEMORY__INT_SEGMENT_;

typedef struct EvmuMemory_ {
    EvmuMemory*     pPublic;
    EvmuCpu_*       pCpu;
    EvmuWord        flash   [EVMU_FLASH_SIZE];
    EvmuWord        rom     [EVMU_ROM_SIZE];
    EvmuWord        xram    [EVMU_ADDRESS_SEGMENT_XRAM_BANKS][EVMU_ADDRESS_SEGMENT_XRAM_SIZE];
    EvmuWord        wram    [EVMU_WRAM_SIZE];
    EvmuWord        sfr     [EVMU_ADDRESS_SEGMENT_SFR_SIZE];                     //not including XRAM
    EvmuWord        ram     [EVMU_ADDRESS_SEGMENT_RAM_BANKS][EVMU_ADDRESS_SEGMENT_RAM_SIZE];    //general-purpose RAM
    EvmuWord*       pIntMap [EVMU_MEMORY__INT_SEGMENT_COUNT_];                //contiguous RAM address space
    EvmuWord*       pExt;
} EvmuMemory_;


GBL_INLINE EvmuWord EvmuMemory__sfrRead_        (CSELF, EvmuAddress address)                GBL_NOEXCEPT;
GBL_INLINE void     EvmuMemory__sfrWrite_       (SELF, EvmuAddress address, EvmuWord value) GBL_NOEXCEPT;

GBL_INLINE EvmuWord EvmuMemory__intRead_        (SELF, EvmuAddress addr)                    GBL_NOEXCEPT;
GBL_INLINE EvmuWord EvmuMemory__intReadLatch_   (SELF, EvmuAddress addr)                    GBL_NOEXCEPT;
GBL_INLINE void     EvmuMemory__intWrite_       (SELF, EvmuAddress addr, EvmuWord val)      GBL_NOEXCEPT;
GBL_INLINE EvmuWord EvmuMemory__extRead_        (CSELF, EvmuAddress address)                GBL_NOEXCEPT;
GBL_INLINE void     EvmuMemory__extWrite_       (SELF, EvmuAddress address, EvmuWord value) GBL_NOEXCEPT;
GBL_INLINE void     EvmuMemory__stackPush_      (SELF, EvmuWord val)                        GBL_NOEXCEPT;
GBL_INLINE EvmuWord EvmuMemory__stackPop_       (SELF)                                      GBL_NOEXCEPT;







// ===== INLINE IMPLEMENTATION =====

GBL_INLINE EvmuWord EvmuMemory__sfrRead_(CSELF, EvmuAddress address) GBL_NOEXCEPT {
    return pSelf->sfr[EVMU_SFR_OFFSET(address)];
}

GBL_INLINE EvmuWord EvmuMemory__sfrMask_(CSELF, EvmuAddress address, EvmuWord mask) GBL_NOEXCEPT {
    return EvmuMemory__sfrRead_(pSelf, address) & mask;
}
GBL_INLINE GblBool EvmuMemory__sfrMaskTest_(CSELF, EvmuAddress address, EvmuWord mask) GBL_NOEXCEPT {
    return EvmuMemory__sfrMask_(pSelf, address, mask) != 0;
}
GBL_INLINE void EvmuMemory__sfrMaskClear_(SELF, EvmuAddress address, EvmuWord mask) GBL_NOEXCEPT {
    pSelf->sfr[EVMU_SFR_OFFSET(address)] &= ~mask;
}

GBL_INLINE void EvmuMemory__sfrMaskSet_(SELF, EvmuAddress address, EvmuWord mask) GBL_NOEXCEPT {
    pSelf->sfr[EVMU_SFR_OFFSET(address)] |= mask;
}

GBL_INLINE void EvmuMemory__sfrWrite_(SELF, EvmuAddress address, EvmuWord value) GBL_NOEXCEPT {
    pSelf->sfr[EVMU_SFR_OFFSET(address)] = value;
}

GBL_INLINE EvmuWord EvmuMemory__intRead_(SELF, EvmuAddress addr) GBL_NOEXCEPT {
    //Write out other DNE SFR register bits to return 1s for H regions.

    //CHECK IF WRITE-ONLY REGISTER:
    switch(addr) {
    case EVMU_ADDRESS_SFR_P1DDR:
    case EVMU_ADDRESS_SFR_P1FCR:
    case EVMU_ADDRESS_SFR_P3DDR:
    case EVMU_ADDRESS_SFR_MCR:
    case EVMU_ADDRESS_SFR_VCCR:
        //_gyLog(GY_DEBUG_WARNING, "MEMORY[%x]: READ from WRITE-ONLY register!", addr);
        return 0xFF;    //Return (hopefully hardware-accurate) bullshit.
    }

    switch(addr) {
    case EVMU_ADDRESS_SFR_VTRBF: { //Reading from separate working memory
        int r = pSelf->wram[0x1ff&((pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8)|pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])];
        //must auto-increment pointer if VSEL_INCE is set
        if(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VSEL)]&EVMU_SFR_VSEL_INCE_MASK)
            //check for 8-bit overflow after incrementing VRMAD1
            if(!++pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])
                //carry 9th bit to VRMAD2
                pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]^=1;
        return r;
    }
    case EVMU_ADDRESS_SFR_T0L:
      //return pSelf->timer0._base.tl;
    case EVMU_ADDRESS_SFR_T0H:
      //return pSelf->timer0._base.th;
    case EVMU_ADDRESS_SFR_T1L:
      //return pSelf->timer1._base.tl;
    case EVMU_ADDRESS_SFR_T1H:
      //return pSelf->timer1._base.th;
    case EVMU_ADDRESS_SFR_VRMAD2:
      return 0xfe|(EvmuMemory__sfrRead_(pSelf, EVMU_ADDRESS_SFR_VRMAD2)&0x1);
    case EVMU_ADDRESS_SFR_P1:
        //return pSelf->port1.pins;
    case EVMU_ADDRESS_SFR_P7:
        return 0xf0|(EvmuMemory__sfrRead_(pSelf, EVMU_ADDRESS_SFR_P7));
    default: {
        EvmuWord val = pSelf->pIntMap[addr/EVMU_MEMORY__INT_SEGMENT_SIZE_][addr%EVMU_MEMORY__INT_SEGMENT_SIZE_];
        return val;
    }
    }
}


GBL_INLINE EvmuWord EvmuMemory__intReadLatch_(SELF, EvmuAddress addr) GBL_NOEXCEPT {
    switch(addr) {
    case EVMU_ADDRESS_SFR_T1L:
    case EVMU_ADDRESS_SFR_T1H:
    case EVMU_ADDRESS_SFR_P1:
    case EVMU_ADDRESS_SFR_P3:   //3 and 7 really SHOULDN'T matter, since the port output should equal the latch...
    case EVMU_ADDRESS_SFR_P7:
        return pSelf->pIntMap[addr/EVMU_MEMORY__INT_SEGMENT_SIZE_][addr%EVMU_MEMORY__INT_SEGMENT_SIZE_];
    default:
        return EvmuMemory__intRead_(pSelf, addr); //fall through to memory for non-latch data
    }
}

GBL_INLINE void EvmuMemory__intWrite_(SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT {

    //Check for SFRs with side-effects
    switch(addr) {
    case EVMU_ADDRESS_SFR_ACC: {
        if(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)] != val) {
            //pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_ACC)] &= ~SFR_PSW_P_MASK|getParity(val);
            pSelf->sfr[0x01] = (pSelf->sfr[0x01]&0xfe)|gblParity(val);
        }
        break;
    }
    case EVMU_ADDRESS_SFR_VTRBF: //Writing to separate working memory
        pSelf->wram[0x1ff&((pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)]<<8)|pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])] = val;
        //must auto-increment pointer if VSEL_INCE is set
        if(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VSEL)]&EVMU_SFR_VSEL_INCE_MASK)
            //check for 8-bit overflow after incrementing VRMAD1
            if(!++pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD1)])
                //carry 9th bit to VRMAD2
                pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VRMAD2)] ^= 1;
        return;
    case EVMU_ADDRESS_SFR_EXT: {
        //changing CPU mode (change imem between BIOS in rom and APP in flash)
        unsigned mode = val&EVMU_SFR_EXT_MASK;
        if((pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)]&EVMU_SFR_EXT_MASK) != mode) {
            const EvmuAddress pc = EvmuCpu__pc_(pSelf->pCpu);
            //next instr must be JMPF, do it now, since imem is changing
            if(pc > 0xfffd || pSelf->pExt[pc] != EVMU_OPCODE_JMPF) {
                //assert(0);
                //EXT 0 changed without a proceeding JMPF!
            } else {
                EvmuCpu__pcSet_(pSelf->pCpu,
                                (pSelf->pExt[pc+1]<<8) | pSelf->pExt[pc+2]);
            }

            if(!mode) {
                pSelf->pExt = pSelf->rom;
            } else {
                pSelf->pExt = pSelf->flash;
            }

        }
        break;
    }
    case EVMU_ADDRESS_SFR_XBNK:{ //changing XRAM bank
        if(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_XBNK)] != val) {
            assert(val <= 2);
            pSelf->pIntMap[EVMU_MEMORY__INT_SEGMENT_XRAM_] = pSelf->xram[val];
            //lcdrefresh();
        }
        break;
    }
    case EVMU_ADDRESS_SFR_PSW: {
        unsigned char psw = pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_PSW)];
        //Check if changing RAM bank
        if((psw&EVMU_SFR_PSW_RAMBK0_MASK) != (val&EVMU_SFR_PSW_RAMBK0_MASK)) {
            unsigned newIndex = (val&EVMU_SFR_PSW_RAMBK0_MASK)>>EVMU_SFR_PSW_RAMBK0_POS;
            assert(newIndex == 0 || newIndex == 1);
            pSelf->pIntMap[EVMU_MEMORY__INT_SEGMENT_GP1_] = pSelf->ram[newIndex];
            pSelf->pIntMap[EVMU_MEMORY__INT_SEGMENT_GP2_] = &pSelf->ram[newIndex][EVMU_MEMORY__INT_SEGMENT_SIZE_];
        }
        break;
    }
#if 0
    case EVMU_ADDRESS_SFR_T0PRR:
        //pSelf->timer0.tscale = 256-val;
        //pSelf->timer0.tbase = 0;
        break;
    case EVMU_ADDRESS_SFR_T0CNT:
        if(!(val&SFR_T0CNT_P0LRUN_MASK))
            pSelf->timer0._base.tl = pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0LR)];
        if(!(val&SFR_T0CNT_P0HRUN_MASK))
            pSelf->timer0._base.th = pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0HR)];
        break;
    case EVMU_ADDRESS_SFR_T0LR:
        if(!(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&SFR_T0CNT_P0LRUN_MASK))
            pSelf->timer0._base.tl = val;
        break;
    case EVMU_ADDRESS_SFR_T0HR:
        if(!(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T0CNT)]&SFR_T0CNT_P0HRUN_MASK))
            pSelf->timer0._base.th = val;
        break;
    case EVMU_ADDRESS_SFR_T1CNT:
        if(!(val&SFR_T1CNT_T1LRUN_MASK))
            pSelf->timer1._base.tl = pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1LR)];
        if(!(val&SFR_T1CNT_T1HRUN_MASK))
            pSelf->timer1._base.th = pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1HR)];
        break;
    case EVMU_ADDRESS_SFR_T1LR:
        if(!(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)]&SFR_T1CNT_T1LRUN_MASK))
            pSelf->timer1._base.tl = val;
        break;
    case EVMU_ADDRESS_SFR_T1HR:
        if(!(pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_T1CNT)]&SFR_T1CNT_T1HRUN_MASK))
            pSelf->timer1._base.th = val;
        break;
#endif
    case EVMU_ADDRESS_SFR_P3DDR:
        if((pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_EXT)]&EVMU_SFR_EXT_MASK) == EVMU_SFR_EXT_USER) {
         //   _gyLog(GY_DEBUG_WARNING, "MEMORY WRITE: Attempted to write to P3DDR register which is not allowed in user mode!");
        }
        break;
#if 0
    case EVMU_ADDRESS_SFR_SCON0:
      //  _sconPrintUpdate(0, pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SCON0)], val);
#if 1
        gyVmuSerialTcpSconUpdate(dev, EVMU_ADDRESS_SFR_SCON0, val);
#endif
        //pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SCON0)] = val;
        //return;
        //return; Let SBUF0 be written with the byte?
    case EVMU_ADDRESS_SFR_SCON1:
#if 1
        gyVmuSerialTcpSconUpdate(dev, EVMU_ADDRESS_SFR_SCON1, val);
#endif
      //  _sconPrintUpdate(1, pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_SCON0)], val);

        break;
#endif
    case EVMU_ADDRESS_SFR_VCCR: {
        int prevVal = pSelf->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)];
        //if true, toggling LCD on, false off
        if((prevVal&EVMU_SFR_VCCR_VCCR7_MASK)  != (val&EVMU_SFR_VCCR_VCCR7_MASK)) {
            //pSelf->display.screenChanged = 1;
        }
    }
    default: break;
    }

    //gyVmuSerialMemorySink(dev, addr, val);

    //do actual memory write
    pSelf->pIntMap[addr/EVMU_MEMORY__INT_SEGMENT_SIZE_][addr%EVMU_MEMORY__INT_SEGMENT_SIZE_] = val;
#if 0
    if((addr >= EVMU_ADDRESS_SFR_XRAM_BASE && addr <= EVMU_ADDRESS_SFR_XRAM_END)/* || addr == EVMU_ADDRESS_SFR_STAD || addr == EVMU_ADDRESS_SFR_XBNK*/) {
        if(pSelf->memMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE] != val) {
            pSelf->display.screenChanged = 1;
        }
    }

    gyVmuPort1MemorySink(dev, addr, val);
    gyVmuBuzzerMemorySink(dev, addr, val);
#endif
}




GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_MEMORY__H

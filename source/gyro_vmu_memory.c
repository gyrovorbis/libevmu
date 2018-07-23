#include <assert.h>
#include <gyro_system_api.h>
#include "gyro_vmu_memory.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_buzzer.h"
#include "gyro_vmu_display.h"
#include "gyro_vmu_port1.h"
#include "gyro_vmu_tcp.h"

//#define VMU_DEBUG

static inline dbgEnabled(VMUDevice* dev) {
    return gyVmuSerialConnectionType(dev) == VMU_SERIAL_CONNECTION_DC;
}

extern void lcdrefresh(void);

inline static int getParity(unsigned char n) {
    int p = 0;
    while(n) {
        p = !p;
        n &= (n-1);
    }
    return p;
}

/* On the VMU, all read-modify-write instructions read from
 * latches, rather than the actual pin/port output to keep
 * data coherent. Otherwise instructions would have unintended
 * side-effects since latch and port values can be diffferent,
 * meaning a SET1 could be setting more than just 1 bit.
 */
int gyVmuMemReadLatch(VMUDevice* dev, int addr) {
    switch(addr) {
    case SFR_ADDR_T1L:
    case SFR_ADDR_T1H:
    case SFR_ADDR_P1:
    case SFR_ADDR_P3:   //3 and 7 really SHOULDN'T matter, since the port output should equal the latch...
    case SFR_ADDR_P7:
        return dev->memMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE];
    default:
        return gyVmuMemRead(dev, addr); //fall through to memory for non-latch data
    }
}

int gyVmuMemRead(VMUDevice* dev, int addr) {
    //Write out other DNE SFR register bits to return 1s for H regions.

    //CHECK IF WRITE-ONLY REGISTER:
    switch(addr) {
    case SFR_ADDR_P1DDR:
    case SFR_ADDR_P1FCR:
    case SFR_ADDR_P3DDR:
    case SFR_ADDR_MCR:
    case SFR_ADDR_VCCR:
        _gyLog(GY_DEBUG_WARNING, "MEMORY[%x]: READ from WRITE-ONLY register!", addr);
        return 0xFF;    //Return (hopefully hardware-accurate) bullshit.
    }

    switch(addr) {
    case SFR_ADDR_VTRBF: { //Reading from separate working memory
        int r = dev->wram[0x1ff&((dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD2)]<<8)|dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD1)])];
        //must auto-increment pointer if VSEL_INCE is set
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_VSEL)]&SFR_VSEL_INCE_MASK)
            //check for 8-bit overflow after incrementing VRMAD1
            if(!++dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD1)])
                //carry 9th bit to VRMAD2
                dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD2)]^=1;
        return r;
    }
    case SFR_ADDR_T0L:
      return dev->timer0._base.tl;
    case SFR_ADDR_T0H:
      return dev->timer0._base.th;
    case SFR_ADDR_T1L:
      return dev->timer1._base.tl;
    case SFR_ADDR_T1H:
      return dev->timer1._base.th;
    case SFR_ADDR_VRMAD2:
      return 0xfe|(dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD2)]&0x1);
    case SFR_ADDR_P1:
        return dev->port1.pins;
    case SFR_ADDR_P7:
        return 0xf0|(dev->sfr[SFR_OFFSET(SFR_ADDR_P7)]);
    default: {
        int val = dev->memMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE];
#ifdef VMU_DEBUG
        if(dbgEnabled(dev))
        _gyLog(GY_DEBUG_VERBOSE, "READMEM[%x] = %d", addr, val);
#endif
        return val;
    }
    }
}

void _sconBitUpdate(uint8_t oldVal, uint8_t newVal, uint8_t bit, const char* name) {
    if((oldVal&(0x1<<bit)) != (newVal&(0x1<<bit))) {
        int val = (newVal&(0x1<<bit));
        val = val? 1 : 0;
        _gyLog(GY_DEBUG_VERBOSE, "%s - %d", name, val);
    }
}

void _sconPrintUpdate(int port, uint8_t oldVal, uint8_t newVal) {

    _gyLog(GY_DEBUG_VERBOSE, "SCON%d UPDATE", port);
    _gyPush();

    _sconBitUpdate(oldVal, newVal, 0, "IE");
    _sconBitUpdate(oldVal, newVal, 1, "XFER END");
    _sconBitUpdate(oldVal, newVal, 2, "LSB/MSB");
    _sconBitUpdate(oldVal, newVal, 3, "XFER CONTROL");
    _sconBitUpdate(oldVal, newVal, 4, "8-bit/Continuous");
    _sconBitUpdate(oldVal, newVal, 5, "UNUSED");
    _sconBitUpdate(oldVal, newVal, 6, "Overrun Flag");
    _sconBitUpdate(oldVal, newVal, 7, "Polarity Control");

    _gyPop(1);

}

void gyVmuMemWrite(VMUDevice* dev, int addr, int val) {
    val &= 0xff; //clamp to single byte
#ifdef VMU_DEBUG
            if(dbgEnabled(dev))
      _gyLog(GY_DEBUG_VERBOSE, "WRITEMEM[%x] = %d", addr, val);
#endif

    //Check for SFRs with side-effects
    switch(addr) {
    case SFR_ADDR_ACC: {
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)] != val) {
            //dev->sfr[SFR_OFFSET(SFR_ADDR_ACC)] &= ~SFR_PSW_P_MASK|getParity(val);
            dev->sfr[0x01] = (dev->sfr[0x01]&0xfe)|getParity(val);
        }
        break;
    }
    case SFR_ADDR_VTRBF: //Writing to separate working memory
        dev->wram[0x1ff&((dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD2)]<<8)|dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD1)])] = val;
        //must auto-increment pointer if VSEL_INCE is set
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_VSEL)]&SFR_VSEL_INCE_MASK)
            //check for 8-bit overflow after incrementing VRMAD1
            if(!++dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD1)])
                //carry 9th bit to VRMAD2
                dev->sfr[SFR_OFFSET(SFR_ADDR_VRMAD2)] ^= 1;
        return;
    case SFR_ADDR_EXT: {
        //changing CPU mode (change imem between BIOS in rom and APP in flash)
        unsigned mode = val&SFR_EXT_MASK;
        if((dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)]&SFR_EXT_MASK) != mode) {

            //next instr must be JMPF, do it now, since imem is changing
            if(dev->pc > 0xfffd || dev->imem[dev->pc] != OPCODE_JMPF) {
                //assert(0);
                //EXT 0 changed without a proceeding JMPF!
            } else {
                dev->pc = (dev->imem[dev->pc+1]<<8) | dev->imem[dev->pc+2];
            }

            if(!mode) {
                dev->imem = dev->rom;
            } else {
                dev->imem = dev->flash;
            }

        }
        break;
    }
    case SFR_ADDR_XBNK:{ //changing XRAM bank
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_XBNK)] != val) {
            assert(val <= 2);
            dev->memMap[VMU_MEM_SEG_XRAM] = dev->xram[val];
            //lcdrefresh();
        }
        break;
    }
    case SFR_ADDR_PSW: {
        unsigned char psw = dev->sfr[SFR_OFFSET(SFR_ADDR_PSW)];
        //Check if changing RAM bank
        if((psw&SFR_PSW_RAMBK0_MASK) != (val&SFR_PSW_RAMBK0_MASK)) {
            unsigned newIndex = (val&SFR_PSW_RAMBK0_MASK)>>SFR_PSW_RAMBKO_POS;
            assert(newIndex == 0 || newIndex == 1);
            dev->memMap[VMU_MEM_SEG_GP1] = dev->ram[newIndex];
            dev->memMap[VMU_MEM_SEG_GP2] = &dev->ram[newIndex][VMU_MEM_SEG_SIZE];
        }
        break;
    }
    case SFR_ADDR_T0PRR:
        dev->timer0.tscale = 256-val;
        dev->timer0.tbase = 0;
        break;
    case SFR_ADDR_T0CNT:
        if(!(val&SFR_T0CNT_P0LRUN_MASK))
            dev->timer0._base.tl = dev->sfr[SFR_OFFSET(SFR_ADDR_T0LR)];
        if(!(val&SFR_T0CNT_P0HRUN_MASK))
            dev->timer0._base.th = dev->sfr[SFR_OFFSET(SFR_ADDR_T0HR)];
        break;
    case SFR_ADDR_T0LR:
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&SFR_T0CNT_P0LRUN_MASK))
            dev->timer0._base.tl = val;
        break;
    case SFR_ADDR_T0HR:
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&SFR_T0CNT_P0HRUN_MASK))
            dev->timer0._base.th = val;
        break;
    case SFR_ADDR_T1CNT:
        if(!(val&SFR_T1CNT_T1LRUN_MASK))
            dev->timer1._base.tl = dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)];
        if(!(val&SFR_T1CNT_T1HRUN_MASK))
            dev->timer1._base.th = dev->sfr[SFR_OFFSET(SFR_ADDR_T1HR)];
        break;
    case SFR_ADDR_T1LR:
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)]&SFR_T1CNT_T1LRUN_MASK))
            dev->timer1._base.tl = val;
        break;
    case SFR_ADDR_T1HR:
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)]&SFR_T1CNT_T1HRUN_MASK))
            dev->timer1._base.th = val;
        break;
    case SFR_ADDR_P3DDR:
        if((dev->sfr[SFR_OFFSET(SFR_ADDR_EXT)]&SFR_EXT_MASK) == SFR_EXT_USER) {
            _gyLog(GY_DEBUG_WARNING, "MEMORY WRITE: Attempted to write to P3DDR register which is not allowed in user mode!");
        }
        break;
    case SFR_ADDR_SCON0:
        _sconPrintUpdate(0, dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)], val);
#if 1
        gyVmuSerialTcpSconUpdate(dev, SFR_ADDR_SCON0, val);
#endif
        //dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] = val;
        //return;
        //return; Let SBUF0 be written with the byte?
    case SFR_ADDR_SCON1:
#if 1
        gyVmuSerialTcpSconUpdate(dev, SFR_ADDR_SCON1, val);
#endif
        _sconPrintUpdate(1, dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)], val);

        break;

    case SFR_ADDR_VCCR:
        if(val&SFR_VCCR_VCCR7_MASK) {
#ifdef VMU_DEBUG
                    if(dbgEnabled(dev))
            _gyLog(GY_DEBUG_VERBOSE, "LCD TURNED ON!!!");
#endif
        } else {
 #ifdef VMU_DEBUG
                    if(dbgEnabled(dev))
            _gyLog(GY_DEBUG_VERBOSE, "LCD TURNED OFF!!!!");
#endif
        }
       // lcdrefresh();
    default: break;
    }

    gyVmuSerialMemorySink(dev, addr, val);

    //do actual memory write
    dev->memMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE] = val;

    if((addr >= SFR_ADDR_XRAM_BASE && addr <= SFR_ADDR_XRAM_END)/* || addr == SFR_ADDR_STAD || addr == SFR_ADDR_XBNK*/) {
        if(dev->memMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE] != val) {
            dev->display.screenChanged = 1;
        }
    }

    gyVmuPort1MemorySink(dev, addr, val);
    gyVmuBuzzerMemorySink(dev, addr, val);

}


#include "gyro_vmu_device.h"
#include "gyro_vmu_instr.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_isr.h"
#include "gyro_vmu_osc.h"
#include "gyro_vmu_cpu.h"
#include <gyro_system_api.h>

VMU_TIMER1_MODE gyVmuTimer1ModeGet(const struct VMUDevice* dev) {
    return (dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)]&(SFR_T1CNT_T1LONG_MASK|SFR_T1CNT_ELDT1C_MASK))>>SFR_T1CNT_ELDT1C_POS;
}

int gyVmuTimersUpdate(VMUDevice *dev) {
    gyVmuTimerBaseUpdate(dev);
    gyVmuTimer0Update(dev);
    gyVmuTimer1Update(dev);
    return 1;
}

#if 0
//BTCR - Base Timer Control Register (0x17f)
#define SFR_BTCR_INT0_CYCLE_CTRL_POS    7
#define SFR_BTCR_INT0_CYCLE_CTRL_MASK   0x80
#define SFR_BTCR_OP_CTRL_POS            6
#define SFR_BTCR_OP_CTRL_MASK           0x40
#define SFR_BTCR_INT1_CYCLE_CTRL_POS    4
#define SFR_BTCR_INT1_CYCLE_CTRL_MASK   0x30
#define SFR_BTCR_INT1_SRC_POS           3
#define SFR_BTCR_INT1_SRC_MASK          0x8
#define SFR_BTCR_INT1_REQ_EN_POS        2
#define SFR_BTCR_INT1_REQ_EN_MASK       0x4
#define SFR_BTCR_INT0_SRC_POS           1
#define SFR_BTCR_INT0_SRC_MASK          0x2
#define SFR_BTCR_INT0_REQ_EN_POS        0
#define SFR_BTCR_INT0_REQ_EN_MASK       0x1
#endif


int gyVmuTimerBaseUpdate(struct VMUDevice* dev) {

    if(dev->sfr[SFR_OFFSET(SFR_ADDR_BTCR)] & SFR_BTCR_OP_CTRL_MASK) {
    //hard-coded to generate interrupt every 0.5s by VMU

        dev->tBaseDeltaTime += gyVmuCpuTCyc(dev);
        dev->tBase1DeltaTime += gyVmuCpuTCyc(dev);
        if(dev->tBase1DeltaTime >= 0.1f) { //call this many cycles 0.1s...
            dev->tBase1DeltaTime -= 0.1f;
            dev->sfr[SFR_OFFSET(SFR_ADDR_BTCR)] |= SFR_BTCR_INT1_SRC_MASK;
            if(dev->sfr[SFR_OFFSET(SFR_ADDR_BTCR)] & SFR_BTCR_INT1_REQ_EN_MASK)
                gyVmuInterruptSignal(dev, VMU_INT_EXT_INT3_TBASE);
        }

        if(dev->tBaseDeltaTime >= 0.5f) { //call this many cycles 0.5s...
            dev->tBaseDeltaTime -= 0.5f;
            dev->sfr[SFR_OFFSET(SFR_ADDR_BTCR)] |= SFR_BTCR_INT0_SRC_MASK;
            if(dev->sfr[SFR_OFFSET(SFR_ADDR_BTCR)] & SFR_BTCR_INT0_REQ_EN_MASK)
                gyVmuInterruptSignal(dev, VMU_INT_EXT_INT3_TBASE);
        }

    }

    return 1;
}

int gyVmuTimer0Update(struct VMUDevice* dev) {
    int cy = _instrMap[dev->curInstr.instrBytes[INSTR_BYTE_OPCODE]].cc;

    /* Timer 0 */
    //T0H overflow or interrupts enabled
       // if(sfr[0x10] & 0xc0) {
    if(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&(SFR_T0CNT_P0HRUN_MASK|SFR_T0CNT_P0LRUN_MASK)) {
        int c0=0;

        //find out how many times greater t0base is than t0scale
        if((dev->timer0.tbase += cy) >= dev->timer0.tscale)
            do c0++;
            while((dev->timer0.tbase -= dev->timer0.tscale) >= dev->timer0.tscale);

    //c0*= 4;
            //only update if t0base > t0scale
        if(c0)  {
            //16-bit counter and both T0L and T0H are in run state
            if((dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&(SFR_T0CNT_P0LONG_MASK|SFR_T0CNT_P0LRUN_MASK|SFR_T0CNT_P0HRUN_MASK))
                    == (SFR_T0CNT_P0LONG_MASK|SFR_T0CNT_P0LRUN_MASK|SFR_T0CNT_P0HRUN_MASK))
            {
                dev->timer0._base.tl += c0;
                if(dev->timer0._base.tl >= 256) {
                    dev->timer0._base.tl -= 256;
                    if(++dev->timer0._base.th >= 256) {
                        dev->timer0._base.th -= 256;
                        if((dev->timer0._base.tl += dev->sfr[SFR_OFFSET(SFR_ADDR_T0LR)] )>= 256) {
                            dev->timer0._base.tl -= 256;
                            if((dev->timer0._base.th += dev->sfr[SFR_OFFSET(SFR_ADDR_T0HR)]) >= 256) {
                                dev->timer0._base.tl = dev->sfr[SFR_OFFSET(SFR_ADDR_T0LR)];
                                dev->timer0._base.th = dev->sfr[SFR_OFFSET(SFR_ADDR_T0HR)];
                            }
                        }
                        //set overflow flags for both T0L and T0H
                        dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)] |= SFR_T0CNT_P0HOVF_MASK|SFR_T0CNT_T0LOVF_MASK;
                        //if T0H interrupts are enabled
                        if(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&SFR_T0CNT_T0HIE_MASK)
                            gyVmuInterruptSignal(dev, VMU_INT_T0H);
                    }
                }

            } else {
                //Update T0L as 8-bit
                if(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)] & SFR_T0CNT_P0LRUN_MASK) {
                    dev->timer0._base.tl += c0;
                    if(dev->timer0._base.tl >= 256) {
                        dev->timer0._base.tl -= 256;
                        if((dev->timer0._base.tl += dev->sfr[SFR_OFFSET(SFR_ADDR_T0LR)]) >= 256)
                            dev->timer0._base.tl = dev->sfr[SFR_OFFSET(SFR_ADDR_T0LR)];
                        dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)] |= SFR_T0CNT_T0LOVF_MASK;
                        if(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&SFR_T0CNT_T0LIE_MASK)
                            gyVmuInterruptSignal(dev, VMU_INT_EXT_INT2_T0L);
                    }
                }

                //Update T0H as 8-bit
                if(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)] & SFR_T0CNT_P0HRUN_MASK) {
                    dev->timer0._base.th += c0;
                    if(dev->timer0._base.th >= 256) {
                        dev->timer0._base.th -= 256;
                        if((dev->timer0._base.th += dev->sfr[SFR_OFFSET(SFR_ADDR_T0HR)]) >= 256)
                            dev->timer0._base.th = dev->sfr[SFR_OFFSET(SFR_ADDR_T0HR)];
                        dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)] |= SFR_T0CNT_P0HOVF_MASK;
                        if(dev->sfr[SFR_OFFSET(SFR_ADDR_T0CNT)]&SFR_T0CNT_T0HIE_MASK)
                            gyVmuInterruptSignal(dev, VMU_INT_T0H);
                    }
                }
            }
        }
    }
    return 1;
}

int gyVmuTimer1Update(struct VMUDevice* dev) {
    int cy = _instrMap[dev->curInstr.instrBytes[INSTR_BYTE_OPCODE]].cc;

    //Interrupts enabled for T1H or overflow on T1H
    if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & (SFR_T1CNT_T1HRUN_MASK|SFR_T1CNT_T1LRUN_MASK)) {

        //Both T1H and T1L running, T1 set to 16-bit mode
        if((dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & (SFR_T1CNT_T1LONG_MASK|SFR_T1CNT_T1HRUN_MASK|SFR_T1CNT_T1LRUN_MASK)) ==
                (SFR_T1CNT_T1LONG_MASK|SFR_T1CNT_T1HRUN_MASK|SFR_T1CNT_T1LRUN_MASK))
        {
            dev->timer1._base.tl += cy;
            if(dev->timer1._base.tl >= 256) {
                dev->timer1._base.tl -= 256;
                if(++dev->timer1._base.th >= 256) {
                    dev->timer1._base.th -= 256;
                    if((dev->timer1._base.tl += dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)]) >= 256) {
                        dev->timer1._base.tl -= 256;
                        if((dev->timer1._base.th += dev->sfr[SFR_OFFSET(SFR_ADDR_T1HR)]) >= 256) {
                            dev->timer1._base.tl = dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)];
                            dev->timer1._base.th = dev->sfr[SFR_OFFSET(SFR_ADDR_T1HR)];
                        }
                    }
                    dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] |= (SFR_T1CNT_T1HOVF_MASK|SFR_T1CNT_T1LONG_MASK);
                    if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & SFR_T1CNT_T1HIE_MASK)
                        gyVmuInterruptSignal(dev, VMU_INT_T1);
                }
            }
        } else {
            //If T1L is running as 8-bit timer
            if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & SFR_T1CNT_T1LRUN_MASK) {
                dev->timer1._base.tl += cy;
                if(dev->timer1._base.tl >= 256) {
                    dev->timer1._base.tl -= 256;
                    if((dev->timer1._base.tl += dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)]) >= 256)
                        dev->timer1._base.tl = dev->sfr[SFR_OFFSET(SFR_ADDR_T1LR)];
                    dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] |= SFR_T1CNT_T1LOVF_MASK;
                    if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & SFR_T1CNT_T1LIE_MASK)
                        gyVmuInterruptSignal(dev, VMU_INT_T1);
                }
            }
            //If T1H is running as 8-bit timer
            if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & SFR_T1CNT_T1HRUN_MASK) {
                dev->timer1._base.th += cy;
                if(dev->timer1._base.th >= 256) {
                    dev->timer1._base.th -= 256;
                    if((dev->timer1._base.th += dev->sfr[SFR_OFFSET(SFR_ADDR_T1HR)]) >= 256)
                        dev->timer1._base.th = dev->sfr[SFR_OFFSET(SFR_ADDR_T1HR)];
                    dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] |= SFR_T1CNT_T1HOVF_MASK;
                    if(dev->sfr[SFR_OFFSET(SFR_ADDR_T1CNT)] & SFR_T1CNT_T1HIE_MASK)
                        gyVmuInterruptSignal(dev, VMU_INT_T1);
                }
            }
        }

    }

    return 1;
}


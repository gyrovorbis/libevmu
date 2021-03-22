#include <evmu/hw/evmu_pic.h>
#include "evmu_device_.h"
#include <evmu/hw/evmu_memory.h>
#include <evmu/hw/evmu_sfr.h>
#if 0
extern void _gyVmuPush(VMUDevice* dev, unsigned val);
extern int  _gyVmuPop(VMUDevice* dev);

#define INT_MASK_FROM_IP_BIT(bit, priority, interrupt) \
        ((((dev->sfr[SFR_OFFSET(SFR_ADDR_IP)] & bit##_MASK) >> bit##_POS) ^ !(priority))) << interrupt;

void gyVmuInterruptControllerInit(struct VMUDevice* dev) {
    memset(&dev->intCont, 0, sizeof(VMUInterruptController));
    dev->intCont.processThisInstr = 1;
}

void gyVmuInterruptSignal(struct VMUDevice* dev, VMU_INT interrupt) {
    VMU_INT_PRIORITY intPriority = gyVmuInterruptPriority(dev, interrupt);
    VMU_INT_PRIORITY curPriority = gyVmuInterruptPriority(dev, gyVmuInterruptCurrent(dev));

    //if(intPriority != curPriority)
        dev->intCont.intReq |= 1 << interrupt;

}

int gyVmuInterruptDepth(const struct VMUDevice* dev) {
    int depth = 0;
    for(int p = VMU_INT_PRIORITY_HIGHEST; p >= VMU_INT_PRIORITY_LOW; --p) {
        if(dev->intCont.intStack[p]) {
            ++depth;
        }
    }
    return depth;
}

VMU_INT_PRIORITY gyVmuInterruptPriority(const struct VMUDevice* dev, VMU_INT interrupt) {
    for(int p = VMU_INT_PRIORITY_HIGHEST; p >= VMU_INT_PRIORITY_LOW; --p) {
        uint16_t priorityMask = gyVmuInterruptPriorityMask(dev, (VMU_INT_PRIORITY)p);
        if(priorityMask & interrupt) {
            return (VMU_INT_PRIORITY)p;
        }
    }
    return VMU_INT_PRIORITY_NONE;
}

int gyVmuInterruptsActive(const struct VMUDevice* dev) {
    uint16_t activeMask = 0;
    for(int p = VMU_INT_PRIORITY_HIGHEST; p >= VMU_INT_PRIORITY_LOW; --p) {
        activeMask |= dev->intCont.intStack[p];
    }
    return activeMask;
}

int gyVmuInterruptCurrent(const struct VMUDevice* dev) {
    for(int p = VMU_INT_PRIORITY_HIGHEST; p >= VMU_INT_PRIORITY_LOW; --p) {
        if(dev->intCont.intStack[p]) return dev->intCont.intStack[p];
    }
    return 0;
}

uint16_t gyVmuInterruptPriorityMask(const struct VMUDevice* dev, VMU_INT_PRIORITY priority) {
    uint16_t mask = 0;

    if(priority == VMU_INT_PRIORITY_HIGHEST) {
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] & SFR_IE_IE0_MASK)) {
            mask |= 1 << VMU_INT_EXT_INT0;
        }
        if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] & SFR_IE_IE1_MASK) && !(dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] & SFR_IE_IE0_MASK)) {
            mask |= 1 << VMU_INT_EXT_INT1;
        }

    } else {

#if 0
        //IP - Interrupt Priority Control (0x109)
        #define SFR_IP_P3_POS           7
        #define SFR_IP_P3_MASK          0x80
        #define SFR_IP_RBF_POS          6       //I'm assuming, undocumented
        #define SFR_IP_RBF_MASK         0x40    //Undocumented, assuming!
        #define SFR_IP_SIO1_POS         5
        #define SFR_IP_SIO1_MASK        0x20
        #define SFR_IP_SIO0_POS         4
        #define SFR_IP_SIO0_MASK        0x10
        #define SFR_IP_T1_POS           3
        #define SFR_IP_T1_MASK          0x8
        #define SFR_IP_T0H_POS          2
        #define SFR_IP_T0H_MASK         0x4
        #define SFR_IP_INT3_POS         1
        #define SFR_IP_INT3_MASK        0x2
        #define SFR_IP_INT2_POS         0
        #define SFR_IP_INT2_MASK        0x1

#define INT_MASK_FROM_IP_BIT(bit, priority, interrupt) \
        ((((dev->sfr[SFR_OFFSET(SFR_ADDR_IP)] & bit##_MASK) >> bit##_POS) ^ ~(priority))) << interrupt;
#endif

        //Only check for high and low priority interrupts if bit IE7 of the Interrupt Enable SFR is set (unmasking them)
        if(priority == VMU_INT_PRIORITY_HIGH || (dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] & SFR_IE_IE7_MASK)) {
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_P3,     priority, VMU_INT_P3);
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_SIO1,   priority, VMU_INT_SIO1);
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_SIO0,   priority, VMU_INT_SIO0);
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_T1,     priority, VMU_INT_T1);
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_T0H,    priority, VMU_INT_T0H);
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_INT3,   priority, VMU_INT_EXT_INT3_TBASE);
            mask |= INT_MASK_FROM_IP_BIT(SFR_IP_INT2,   priority, VMU_INT_EXT_INT2_T0L);

            if(priority == VMU_INT_PRIORITY_LOW) {
                //Both interrupts(IE0+IE1) are set to low if IE0 is set to low.
                if(dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] & SFR_IE_IE0_MASK) {
                    mask |= 1 << VMU_INT_EXT_INT0;
                    mask |= 1 << VMU_INT_EXT_INT1;
                }

                if(dev->sfr[SFR_OFFSET(SFR_ADDR_IE)] & SFR_IE_IE1_MASK) {
                    mask |= 1 << VMU_INT_EXT_INT1;
                }
            }
        }
    }
    return mask;
}

int _gyVmuInterruptRetiInstr(struct VMUDevice* dev) {
    int r   =   _gyVmuPop(dev) << 8u;
    r       |=  _gyVmuPop(dev);
    dev->pc = r;
    dev->intCont.processThisInstr = 0;
    for(int p = VMU_INT_PRIORITY_HIGHEST; p >= VMU_INT_PRIORITY_LOW; --p) {
        if(dev->intCont.intStack[p]) {
            dev->intCont.prevIntPriority = p;
            dev->intCont.intStack[p] = 0;
            return 1;
        }
    }
    return 0;
}

static int _gyVmuInterruptCheck(struct VMUDevice* dev, VMU_INT_PRIORITY p) {

    uint16_t priorityMask = gyVmuInterruptPriorityMask(dev, (VMU_INT_PRIORITY)p);

    for(uint16_t i = 0; i < VMU_INT_COUNT; ++i) {
        uint16_t interrupt = (1 << i);
        if(priorityMask & interrupt & dev->intCont.intReq) {
            dev->intCont.intReq &= ~interrupt;          //clear request
            //++dev->intMask;                           //increment mask/depth
            dev->intCont.intStack[p] = interrupt;
            _gyVmuPush(dev, dev->pc & 0xff);
            _gyVmuPush(dev, (dev->pc & 0xff00) >> 8);   //push return address
#ifdef VMU_DEBUG
            if(dbgEnabled(dev))
                _gyLog(GY_DEBUG_VERBOSE, "INTERRUPT - %d, depth - %d", r, dev->intMask);
#endif
            dev->sfr[SFR_OFFSET(SFR_ADDR_PCON)] &= ~SFR_PCON_HALT_MASK;
            dev->pc = gyVmuInterruptAddr((VMU_INT)i);   //jump to ISR address
            return 1;
        }

    }
    return 0;
}


int gyVmuInterruptControllerUpdate(struct VMUDevice* dev) {
    if((dev->sfr[SFR_OFFSET(SFR_ADDR_P3INT)]&(SFR_P3INT_P31INT_MASK|SFR_P3INT_P30INT_MASK)) ==
            (SFR_P3INT_P31INT_MASK|SFR_P3INT_P30INT_MASK))
        gyVmuInterruptSignal(dev, VMU_INT_P3);
#if 1
    if(!dev->intCont.processThisInstr) {
        dev->intCont.processThisInstr = 1;
        return 0;
    }

   // if(!gyVmuInterruptsActive(dev)) {


    //}

    if(!gyVmuInterruptsActive(dev)) {
// wrong! yOU NEED TO CHECK FOR PREVINT PRIORITY EVEN IF THEY'RE ACTIVE
        // You're gonna starve the bitch!
        for(int p = dev->intCont.prevIntPriority - 1; p >= VMU_INT_PRIORITY_LOW; --p) {
            if(_gyVmuInterruptCheck(dev, (VMU_INT_PRIORITY)p)) return 1;
        }

        for(int p = VMU_INT_PRIORITY_HIGH; p >= dev->intCont.prevIntPriority; --p) {
            if(_gyVmuInterruptCheck(dev, (VMU_INT_PRIORITY)p)) return 1;
        }
    } else {


        for(int p = VMU_INT_PRIORITY_HIGHEST; p >= VMU_INT_PRIORITY_LOW; --p) {
            if(dev->intCont.intStack[p]) {
                break;
            }
            if(_gyVmuInterruptCheck(dev, (VMU_INT_PRIORITY)p)) return 1;

        }
    }
    return 0;

#else
    if(!dev->intMask) {

        if((dev->sfr[SFR_OFFSET(SFR_ADDR_P3INT)]&(SFR_P3INT_P31INT_MASK|SFR_P3INT_P30INT_MASK)) ==
                (SFR_P3INT_P31INT_MASK|SFR_P3INT_P30INT_MASK))
            dev->intReq |= 1<<VMU_INT_P3;

    }

    if((dev->sfr[SFR_OFFSET(SFR_ADDR_P7)]&SFR_P7_P70_MASK) && (dev->sfr[SFR_OFFSET(SFR_ADDR_I01CR)]&0x3)) {
        dev->intReq |= 1<<VMU_INT_EXT_INT0;
        _gyLog(GY_DEBUG_VERBOSE, "RAISING EXTERNAL P7 INTERRUPT!!!!");
    }

    uint8_t ie = dev->sfr[SFR_OFFSET(SFR_ADDR_IE)];

    //Check if there are no interrupts, we're already in an interrupt, or we've disabled them.
    if(!dev->intReq || dev->intMask || !(ie&SFR_IE_IE7_MASK)) return 0;

    for(uint16_t r = 0; r < VMU_INT_COUNT; ++r) {
        if(dev->intReq&(1<<r)) {                //pending interrupt request
            dev->intReq &= ~(1<<r);             //clear request
            ++dev->intMask;                     //increment mask/depth
            _push(dev, dev->pc&0xff);
            _push(dev, (dev->pc&0xff00)>>8);    //push return address
#ifdef VMU_DEBUG
                                                    if(dbgEnabled(dev))
            _gyLog(GY_DEBUG_VERBOSE, "INTERRUPT - %d, depth - %d", r, dev->intMask);
#endif
            dev->sfr[SFR_OFFSET(SFR_ADDR_PCON)] &= ~SFR_PCON_HALT_MASK;
            dev->pc = gyVmuIsrAddr(r);          //jump to ISR address
            return 1;
        }
    }

    return 0; //ain't no pending interrupts...
#endif
}



#if 0
//-------------------------------------------------
//  check_irqs - check for interrupts request
//-------------------------------------------------

static void check_irqs()
{
    // update P3 interrupt
    check_p3int();

    if (m_irq_flag && !m_after_reti)
    {
        int irq = 0;
        uint8_t priority = 0;

        // highest priority IRQ
        if (!(REG_IE & 0x01) && (m_irq_flag & 0x02))
        {
            irq = 0x01;
            priority = 2;
        }
        else if (!(REG_IE & 0x02) && (m_irq_flag & 0x04))
        {
            irq = 0x02;
            priority = 2;
        }

        // high priority IRQ
        else if ((REG_IE & 0x80) && ((REG_IP<<3) & m_irq_flag))
        {
            for(int i=3; i<=10; i++)
                if ((m_irq_flag & (REG_IP<<3)) & (1<<i))
                {
                    irq = i;
                    priority = 1;
                    break;
                }
        }

        // low priority IRQ
        else if ((REG_IE & 0x80) && (m_irq_flag & 0x02))
        {
            irq = 0x01;
            priority = 0;
        }
        else if ((REG_IE & 0x80) && (m_irq_flag & 0x04))
        {
            irq = 0x02;
            priority = 0;
        }
        else if (REG_IE & 0x80)
        {
            for(int i=3; i<=10; i++)
                if (m_irq_flag & (1<<i))
                {
                    irq = i;
                    priority = 0;
                    break;
                }
        }

        // IRQ with less priority of current interrupt are not executed until the end of the current interrupt routine
        if (irq != 0 && ((m_irq_lev & (1<<priority)) || (priority == 0 && (m_irq_lev & 0x06)) || (priority == 1 && (m_irq_lev & 0x04))))
        {
            if (LOG_IRQ)    logerror("%s: interrupt %d (Priority=%d, Level=%d) delayed\n", tag(), irq, priority, m_irq_lev);
            irq = 0;
        }

        if (irq != 0)
        {
            if (LOG_IRQ)    logerror("%s: interrupt %d (Priority=%d, Level=%d) executed\n", tag(), irq, priority, m_irq_lev);

            m_irq_lev |= (1<<priority);

            push((m_pc>>0) & 0xff);
            push((m_pc>>8) & 0xff);

            set_pc(s_irq_vectors[irq]);

            REG_PCON &= ~HALT_MODE;     // interrupts resume from HALT state

            // clear the IRQ flag
            m_irq_flag &= ~(1<<irq);

            standard_irq_callback(irq);
        }
    }

    // at least one opcode need to be executed after a RETI before another IRQ can be accepted
    m_after_reti = false;
}

#endif

#endif

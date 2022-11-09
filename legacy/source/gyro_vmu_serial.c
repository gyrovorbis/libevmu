#include "gyro_vmu_serial.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_memory.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_port1.h"
#include "gyro_vmu_isr.h"
#include "gyro_vmu_osc.h"
#include "gyro_vmu_cpu.h"
#include <gyro_system_api.h>
#include <assert.h>

static inline int _ifaceSerialActive(const VMUDevice* dev, int ifaceIndex) {
    const VMUSerialIface* iface = &dev->serial.ifaces[ifaceIndex];

    //(P1DDR.RECV) && (SCON.CTRL || SCON.LEN)
    return ((dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_CTRL_MASK)
                || (dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_LEN_MASK));
}

static int _advanceSerialBuffTransferPos(struct VMUDevice* dev, int ifaceIndex) {
    VMUSerialIface* iface = &dev->serial.ifaces[ifaceIndex];

    //MSB-first transfers
    if(dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_MSB_MASK) {
        //End of byte
        if(iface->sbufCurPos == 0) {
            //Continuous mode
            if(dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_LEN_MASK) {
                iface->sbufCurPos = 7; //Reset position to beginning of byte
            } else { //8-bit mode
                //Iface 0 with reverse polarity
                if(!ifaceIndex && dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)]&SFR_SCON0_POL_MASK) {
                    iface->sbufCurPos = 0;
                }
            }
            return 0;
        } else { //Mid-byte
            --iface->sbufCurPos;
        }
    } else { //LSB-first transfers
        //End of byte
        if(iface->sbufCurPos == 7) {
            //Continuous mode
            if(dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_LEN_MASK) {
                iface->sbufCurPos = 0; //Reset position to beginning of byte
            } else { //8-bit mode
                //Iface 0 with reverse polarity
                if(!ifaceIndex && dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)]&SFR_SCON0_POL_MASK) {
                    iface->sbufCurPos = 0;
                }
            }
            return 0;
        } else { //Mid-byte
            ++iface->sbufCurPos;
        }
    }
    return 1;
}

static void _vmuSerialTransferStart(VMUDevice* dev, int ifaceIndex) {
    VMUSerialIface* iface = &dev->serial.ifaces[ifaceIndex];
    const unsigned cfgMask = gyVmuSerialIfaceConfigMask(dev, ifaceIndex);

    //Check if iface is configured as Tx
    if(cfgMask&VMU_SERIAL_IFACE_SEND_MASK) {
        _gyLog(GY_DEBUG_VERBOSE, "SERIAL: Starting TX on iface[%d].", ifaceIndex);

        dev->serial.timeElapsed = 0;
        dev->serial.tsbr = (256.0f - dev->sfr[SFR_OFFSET(SFR_ADDR_SBR)]) * 2.0f * gyVmuOscSecPerCycle(dev);
        iface->sbufCurPos =
                (dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_MSB_MASK)? 7 : 0;
        iface->sbufVal = dev->sfr[SFR_OFFSET(iface->sbufReg)];

        //Set clock starting position
        gyVmuPort1PinsWriteBegin(dev);
        //Only SIO0 supports configurable polarity.
        if(!ifaceIndex &&
                dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_POL_MASK) {
            gyVmuPort1PinSerialWrite(dev, iface->sckPin, 1);
        } else {
            gyVmuPort1PinSerialWrite(dev, iface->sckPin, 0);
            gyVmuPort1PinSerialWrite(dev, iface->soPin,
                                     (iface->sbufVal>>iface->sbufCurPos)&0x1);
            _advanceSerialBuffTransferPos(dev, ifaceIndex);
        }
        gyVmuPort1PinsWriteEnd(dev);

    } else if(cfgMask&VMU_SERIAL_IFACE_RECV_MASK) {
        iface->sbufCurPos =
                (dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_MSB_MASK)? 7 : 0;
        iface->ovWaiting = 0;
    }

}

static void _serialTransferComplete(VMUDevice* dev, int ifaceIndex) {
    const VMUSerialIface* iface = &dev->serial.ifaces[ifaceIndex];

    _gyLog(GY_DEBUG_VERBOSE, "SERIAL: full byte transmitted on iface[%d]!", ifaceIndex);
    _gyPush();

    //signal transfer complete
    dev->sfr[SFR_OFFSET(iface->sconReg)] |= SFR_SCON0_END_MASK;

    //Raise interrupt after byte has been received, if IE bit is set
    if(dev->sfr[SFR_OFFSET(iface->sconReg)] & SFR_SCON0_IE_MASK) {
        _gyLog(GY_DEBUG_VERBOSE, "Raising interrupt - %d", iface->intReq);
        gyVmuInterruptSignal(dev, iface->intReq);
    }

    //Stop transfer if it's only 8-bit.
    if(!(dev->sfr[SFR_OFFSET(iface->sconReg)] & SFR_SCON0_LEN_MASK)) {
        _gyLog(GY_DEBUG_VERBOSE, "Transfer complete");
        dev->sfr[SFR_OFFSET(iface->sconReg)] &= ~SFR_SCON0_CTRL_MASK;
    } else {
        _gyLog(GY_DEBUG_VERBOSE, "Continuing next 8 bits.");
       // dev->sfr[SFR_OFFSET(iface->sconReg)] |= SFR_SCON0_OV_MASK; //overflow flag is set every 8 bits in continuous mode? RX only?!
    }

    _gyPop(1);
}

void gyVmuSerialUpdate(struct VMUDevice* dev, float deltaTime) {
#if 1

#else
    const float halfPeriod = dev->serial.tsbr/2.0f;
    if(halfPeriod == 0.0f) return;

    do {
        int stateChange = 0;
        if(deltaTime > halfPeriod) {
            dev->serial.timeElapsed += halfPeriod;
            deltaTime -= halfPeriod;
        } else {
            dev->serial.timeElapsed += deltaTime;
            deltaTime = 0;
        }

        if(dev->serial.sbrState == VMU_SERIAL_CLK_STATE1) {
            if(dev->serial.timeElapsed >= halfPeriod) {
                dev->serial.sbrState = VMU_SERIAL_CLK_STATE2;
                stateChange = 1;
            }
        } else {    //VMU_SERIAL_CLK_STATE2
            if(dev->serial.timeElapsed >= dev->serial.tsbr) {
                dev->serial.sbrState = VMU_SERIAL_CLK_STATE1;
                stateChange = 1;
                //Carry over remainder time!
                dev->serial.timeElapsed -= dev->serial.tsbr;
            }
        }

        //Update serial output if clock changed states
        if(stateChange) {
            gyVmuPort1PinsWriteBegin(dev);
            for(unsigned i = 0; i < VMU_SERIAL_IFACE_COUNT; ++i) {
                 const VMUSerialIface* iface = &dev->serial.ifaces[i];

                 //Check if interface is currently transmitting
                if(gyVmuSerialIfaceConfigMask(dev, i)&VMU_SERIAL_IFACE_SEND_MASK &&
                        dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_CTRL_MASK) {

                    //Iface 0 with inverted polarity
                    if(!i && dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_POL_MASK) {
                        gyVmuPort1PinSerialWrite(dev, iface->sckPin,
                                                 dev->serial.sbrState == VMU_SERIAL_CLK_STATE1? 1 : 0);

                        if(dev->serial.sbrState == VMU_SERIAL_CLK_STATE2) {
                            gyVmuPort1PinSerialWrite(dev, iface->soPin, (iface->sbufVal>>iface->sbufCurPos)&0x1);

                            if(!_advanceSerialBuffTransferPos(dev, 0)) {
                                _serialTransferComplete(dev, 0);
                            }

                        }

                    } else { //Iface 0 or 1 with default polarity
                        gyVmuPort1PinSerialWrite(dev, iface->sckPin,
                                                 dev->serial.sbrState == VMU_SERIAL_CLK_STATE1? 0 : 1);

                        if(dev->serial.sbrState == VMU_SERIAL_CLK_STATE1) {
                            gyVmuPort1PinSerialWrite(dev, iface->soPin, (iface->sbufVal>>iface->sbufCurPos)&0x1);

                            if(!_advanceSerialBuffTransferPos(dev, i)) {
                                _serialTransferComplete(dev, i);
                            }
                        }
                    }
                }
            }
            gyVmuPort1PinsWriteEnd(dev);
        }

    } while(deltaTime >= 0.000001f); //account for floating-point inaccuracy
#endif
}

static void _vmuSerialTransferStop(VMUDevice* dev, int ifaceIndex) {
    const unsigned cfgMask = gyVmuSerialIfaceConfigMask(dev, ifaceIndex);
    _gyLog(GY_DEBUG_VERBOSE, "SERIAL: STOP on interface: %d", ifaceIndex);

    const VMUSerialIface* iface = &dev->serial.ifaces[ifaceIndex];

    if(cfgMask) {

        //Clear start bit
        if(!(dev->sfr[SFR_OFFSET(iface->sconReg)] & SFR_SCON0_LEN_MASK)) { //8-bit mode
            dev->sfr[SFR_OFFSET(iface->sconReg)] &= ~SFR_SCON0_CTRL_MASK;
        } else { //continuous mode
            dev->sfr[SFR_OFFSET(iface->sconReg)] &= ~SFR_SCON0_LEN_MASK;
        }

        if(cfgMask&VMU_SERIAL_IFACE_SEND_MASK) {

            //Set pins
            int sckVal;
            gyVmuPort1PinsWriteBegin(dev);

            //Iface 0 with reverse polarity
            if(!ifaceIndex && dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)]&SFR_SCON0_POL_MASK) {
                sckVal  = 0;
                gyVmuPort1PinSerialWrite(dev, iface->soPin, iface->sbufVal&0x1);
            } else { //normal polarity
                sckVal = 1;
            }

            gyVmuPort1PinSerialWrite(dev, iface->sckPin, sckVal);
            gyVmuPort1PinsWriteEnd(dev);
        }
    }
}

void gyVmuSerialMemorySink(struct VMUDevice* dev, int addr, uint8_t value) {
    uint8_t oldVal = dev->memMap[addr/VMU_MEM_SEG_SIZE][addr%VMU_MEM_SEG_SIZE];
    for(unsigned i = 0; i < VMU_SERIAL_IFACE_COUNT; ++i) {
        const VMUSerialIface* iface = &dev->serial.ifaces[i];
        if(addr == iface->sconReg) {
            if(oldVal != value) _gyLog(GY_DEBUG_VERBOSE, "SCON%d Register Set", i);
            _gyPush();
            for(uint8_t j = 0; j < GYRO_VMU_CPU_WORD_SIZE; ++j) {
                const uint8_t active = (value>>j)&0x1;
                const uint8_t changed = (((oldVal>>j)&0x1) != active);
                switch(j) {
                    case SFR_SCON0_IE_POS:
                    if(changed) _gyLog(GY_DEBUG_VERBOSE, "Interrupts: %s", active? "ENABLED" : "DISABLED");
                    break;
                case SFR_SCON0_END_POS:
                    if(changed) _gyLog(GY_DEBUG_VERBOSE, "Status: %s", active? "END" : "IN PROGRESS");
                    break;
                case SFR_SCON0_MSB_POS:
                    if(changed) _gyLog(GY_DEBUG_VERBOSE, "Endianness: %s", active? "MSB" : "LSB");
                    break;
                case SFR_SCON0_CTRL_POS:
                    if(changed) {
                            _gyLog(GY_DEBUG_VERBOSE, "Control: %s", active? "START" : "STOP");
                        if(active) {
                            if(!(value&SFR_SCON0_LEN_MASK))
                                _vmuSerialTransferStart(dev, i); //start transfer only if configured for 8-bit mode
                            else
                                _gyLog(GY_DEBUG_WARNING, "SCON%d.CTRL set to START when in continuous mode! The fuck do we do!?", i);
                        } else if(!active && changed) {
                            if(!(value&SFR_SCON0_LEN_MASK)) _vmuSerialTransferStop(dev, i); //stop transfer only if there was an active 8-bit transfer
                            else _gyLog(GY_DEBUG_WARNING, "SCON%d.CTRL set to STOP when in continuous mode! The fuck do we do!?", i);
                        }
                    }
                    break;
                case SFR_SCON0_LEN_POS:
                    if(changed) _gyLog(GY_DEBUG_VERBOSE, "Length: %s", active? "8-BIT" : "CONTINUOUS");
                    if(active && changed) {
                        if(dev->sfr[SFR_OFFSET(iface->sconReg)]&SFR_SCON0_CTRL_MASK)
                            _gyLog(GY_DEBUG_WARNING, "SERIAL: Starting a continuous xfer on iface[%d] while an 8-bit transfer was still going on! WTF!", i);
                        _vmuSerialTransferStart(dev, i); //start continuous transfer
                    } else if(!active && changed) {
                        _vmuSerialTransferStop(dev, i); //only "ends" transfer if a continuous transfer was active?
                    }
                    break;
                case SFR_SCON0_OV_POS:
                    if(changed) _gyLog(GY_DEBUG_VERBOSE, "Overrun: %d", active);
                    break;
                case SFR_SCON0_POL_POS:
                    if(i == 0)
                        if(changed)_gyLog(GY_DEBUG_VERBOSE, "Polarity: %d", active);
                    break;
                }
            }
            _gyPop(1);
            break;
        } else if(addr == dev->serial.ifaces[i].sbufReg) {

            break;
        }
        //CHECK FOR CLOCK SPEED CHANGE TO ADJUST SBR!!1111 ? Not really... that would be mid Xfer...
    }

}

static int _validateSerialRecv(const VMUDevice* dev, uint8_t pin, uint8_t bit) {
    assert(pin < VMU_SERIAL_SIGNAL_COUNT);
    //if(gyVmuPort1PinSrc(dev, pin) != PORT1_PIN_SERIAL) return 0; //this pin is none of our fucking business...
    //or is it? P1FCR comes before P1DDR, so this is going to the serial module either way.

    const int ifaceIndex = (pin < VMU_SERIAL_SO1);
    static const char* dirStr[] = {
        "IN",
        "OUT"
    };

    switch(pin) {
    case VMU_SERIAL_SO0:
    case VMU_SERIAL_SO1:
        _gyLog(GY_DEBUG_WARNING, "SERIAL Rx: Receiving bit %d on SO%d with DIR: %s... WTF do we do!?",
               bit, ifaceIndex, dirStr[gyVmuPort1PinDir(dev, pin)]);
        return 0;
    case VMU_SERIAL_SCK0:
    case VMU_SERIAL_SCK1:
    case VMU_SERIAL_SI1:
    case VMU_SERIAL_SI0:
        if(gyVmuPort1PinDir(dev, pin) == PORT1_PIN_IN) {
            if(!_ifaceSerialActive(dev, ifaceIndex)) {
                _gyLog(GY_DEBUG_WARNING, "SERIAL Rx: Receiving bit %d on SI%d while not ready to receive!",
                       bit, ifaceIndex);
                return (pin == VMU_SERIAL_SCK0 || pin == VMU_SERIAL_SCK1)? 1 : 0; //invalid clock needs to be examined for OV
            }
        } else {
            _gyLog(GY_DEBUG_WARNING, "SERIAL Rx: Receiving bit %d on SI%d with DIR: OUT",
                   bit, ifaceIndex);
            return 0;
        }
    default:
        break;
    }

    return 1; //You made it this far, you better be fucking valid...

}


void gyVmuSerialInit(struct VMUDevice* dev) {
    dev->serial.sbrState = VMU_SERIAL_CLK_STATE1;

    for(unsigned i = 0; i < VMU_SERIAL_IFACE_COUNT; ++i) {
        VMUSerialIface* iface
                            = &dev->serial.ifaces[i];
        const int offset    = i? 3 : 0;
        iface->sbufCurPos  = -1;
        iface->sbufReg      = i? SFR_ADDR_SBUF1 : SFR_ADDR_SBUF0;
        iface->sconReg      = i? SFR_ADDR_SCON1 : SFR_ADDR_SCON0;
        iface->intReq       = i? VMU_INT_SIO1 : VMU_INT_SIO0;
        iface->soPin        = 0 + offset;
        iface->siPin        = 1 + offset;
        iface->sckPin       = 2 + offset;
    }
}

void gyVmuSerialP1Recv(struct VMUDevice* dev, uint8_t pins, uint8_t changedMask) {
    /*don't forget to check for trailing clock or whatever-the-fuck for
    overflow on the receive side!!!!*/

    for(uint8_t i = 0; i < VMU_SERIAL_SIGNAL_COUNT; ++i) {
        //Check if the pin changed
        if(changedMask&i) {
            const uint8_t pinVal    = (pins>>i)&0x1;
            const int ifaceIndex    = (i < 3);
            VMUSerialIface* iface   = &dev->serial.ifaces[ifaceIndex];

            if(_validateSerialRecv(dev, i, pinVal)) {
                switch(i) {
                case VMU_SERIAL_SO1:    //No idea how the fuck to handle this... shouldn't be possible.
                case VMU_SERIAL_SO0:
                    break;
                case VMU_SERIAL_SI0:    //wait until clock edge to actually receive Rx signals
                case VMU_SERIAL_SI1:
                    if(!(changedMask&iface->sckPin)) {
                        _gyLog(GY_DEBUG_WARNING, "SERIAL Rx: SI%d changed without a corresponding change in clock! Shouldn't be possible!",
                               ifaceIndex);
                    }
                    break;
                case VMU_SERIAL_SCK0:
                case VMU_SERIAL_SCK1: {
                    int clkChgEdge =
                            (!ifaceIndex && (dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)]&SFR_SCON0_POL_MASK))?
                                0 : 1; //0 for iface 0 with reverse polarity, 1 for normal polarity

                    if(pinVal == clkChgEdge) { //Check if clock is at the trigger edge
                        int siVal = (pins>>iface->siPin)&0x1;

                        if(siVal) iface->sbufVal |= (0x1<<iface->siPin);
                        else iface->sbufVal &= (0x1<<iface->siPin);

                        if(!_advanceSerialBuffTransferPos(dev, 0)) {
                            _serialTransferComplete(dev, ifaceIndex);
                            iface->ovWaiting = 1;
                        }
                    }
                    if(!pinVal && iface->ovWaiting) {
                        dev->sfr[SFR_OFFSET(iface->sconReg)] |= SFR_SCON0_OV_MASK;
                        iface->ovWaiting = 0;
                    }
                }
                default:
                    break;
                }
            }
        }
    }




}

static inline int _pinConfiguredAsSerialOut(const VMUDevice* dev, uint8_t pin) {
    return gyVmuPort1PinDir(dev, pin) == PORT1_PIN_OUT &&
            gyVmuPort1PinSrc(dev, pin) == PORT1_PIN_SERIAL;
}

int gyVmuSerialIfaceConfigMask(const VMUDevice* dev, int ifaceIndex) {
    assert(ifaceIndex >= 0 && ifaceIndex < VMU_SERIAL_IFACE_COUNT); //There are only two serial ifaces, dipshit...
    const VMUSerialIface* iface = &dev->serial.ifaces[ifaceIndex];
    int mask = 0;

    if(_pinConfiguredAsSerialOut(dev, iface->sckPin) ||
            _pinConfiguredAsSerialOut(dev, iface->soPin)
           // ||_pinConfiguredAsSerialOut(dev, iface->siPin)
            )
        mask |= VMU_SERIAL_IFACE_SEND_MASK;

    if(gyVmuPort1PinDir(dev, iface->sckPin) == PORT1_PIN_IN &&
            gyVmuPort1PinDir(dev, iface->siPin == PORT1_PIN_IN))
        mask |= VMU_SERIAL_IFACE_RECV_MASK;

    return mask;
}



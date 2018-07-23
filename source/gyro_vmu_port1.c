#include "gyro_vmu_port1.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_device.h"
#include <gyro_system_api.h>
#include <assert.h>

/* Have to make the whole "PENDING PINS" mechanism look at ALL changes to the pins
 * (P1, P1DDR, P1FCR, P1 Serial Data) and modify the resultant "pins" based on ALL
 * values then only broadcast pins that are configured for Serial, configured as
 * output, and changed!
 *
 */

/*
 *
 * INPUT SIGNALS:
 * port1.serialInSignals - serial signals as input to port1
 * port1.extSignals - external signals applied to port1
 * SFR_ADDR_P1 - latch value of port1
 *
 * also taking into account
 * P1DDR
 * P1FCR
 *
 */

inline static void _updatePin(VMUDevice* dev, uint8_t pin) {
    assert(dev->port1.writing);
    //add pin to pendingPins
    dev->port1.pendingMask |= (0x1<<pin);
}

inline static void _updatePins(VMUDevice* dev, uint8_t mask) {
    gyVmuPort1PinsWriteBegin(dev);
    for(uint8_t i = 0; i < PORT1_SIZE; ++i)
        if(mask&(0x1<<i))
            _updatePin(dev, i);
    gyVmuPort1PinsWriteEnd(dev);
}

void gyVmuPort1PinsWriteBegin(struct VMUDevice* dev) {
    if(dev->port1.writing) {
        _gyLog(GY_DEBUG_WARNING, "Attempting write to P1 before last write ended.");
    }
    dev->port1.writing = true;
    dev->port1.pendingMask = 0;
}

void gyVmuPort1PinsWriteEnd(struct VMUDevice* dev) {
    if(!dev->port1.writing) {
        _gyLog(GY_DEBUG_WARNING, "Attempting to end writing to P1 before a write even began.");
    }

    int writeSerialOut  = 0;
    int newPinsVal      = 0;

    //Flush the pins if they have been modified
    if(dev->port1.pendingMask) {
        for(uint8_t i = 0; i < PORT1_SIZE; ++i) {
            const uint8_t curMask = (0x1<<i);
            //current pin has pending change
            if(dev->port1.pendingMask&curMask) {
                if(gyVmuPort1PinDir(dev, i) == PORT1_PIN_IN) {
                    //Input comes from external signals
                    newPinsVal |= (dev->port1.extSignals&curMask);
                } else {
                    if(gyVmuPort1PinSrc(dev, i) == PORT1_PIN_DATA) {
                        //data output comes from latch
                        newPinsVal |= (dev->sfr[SFR_OFFSET(SFR_ADDR_P1)]&curMask);
                    } else { //serial output
                        //SI0 and SI1 are tied to SO0 and SO1 when used as output
                        int inputSignalMask = (i == VMU_SERIAL_SI0 || i == VMU_SERIAL_SI1)?
                                    (curMask>>0x1) : curMask;

                        //Serial output is SIO pin ORed with P1 latch (so P1 latch better be reset to 0!)
                        newPinsVal |= ((dev->port1.serialInSignals&inputSignalMask)
                                      || (dev->sfr[SFR_OFFSET(SFR_ADDR_P1)]&curMask));

                        //Serial output pin has changed!
                        if((newPinsVal&curMask) != (dev->port1.pins&curMask)) writeSerialOut = 1;
                    }
                }
            }
        }

        //Update pins
        dev->port1.pins &= ~dev->port1.pendingMask;
        dev->port1.pins |= newPinsVal;
        //If any serial output pins changed, write to serial
        if(writeSerialOut && dev->port1.ext) dev->port1.ext->write(dev->port1.ext->context, &dev->port1.pins, sizeof(dev->port1.pins));
    }
    dev->port1.writing = 0;
}

void gyVmuPort1MemorySink(VMUDevice *dev, int addr, uint8_t val) {
    switch(addr) {
    case SFR_ADDR_P1DDR:
        //_gyLog(GY_DEBUG_VERBOSE, "P1DDR Register Set");
        //_gyPush();
        for(unsigned i = 0; i < PORT1_SIZE; ++i) {
            int active = val&(0x1<<i);
            if(i == SFR_P1DDR_P10DDR_POS) {
          //      _gyLog(GY_DEBUG_VERBOSE, "P1.0 (SO0): %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P11DDR_POS) {
            //    _gyLog(GY_DEBUG_VERBOSE, "P1.1 (S10): %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P12DDR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "P1.2 (SCK0): %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P13DDR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "P1.3 (SO1): %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P14DDR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "P1.4 (S11): %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P15DDR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "P1.5 (SCK1): %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P16DDR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "P1.6: %s", active? "out" : "in");
            } else if(i == SFR_P1DDR_P17DDR_POS) {
                //_gyLog(GY_DEBUG_VERBOSE, "P1.7: %s", active? "out" : "in");
            }
        }
        _updatePins(dev, 0xff);
       // _gyPop(1);
        break;
    case SFR_ADDR_P1FCR:
        //_gyLog(GY_DEBUG_VERBOSE, "P1FCR Register Set");
      //  _gyPush();
        for(unsigned i = 0; i < PORT1_SIZE; ++i) {
            int active = val&(0x1<<i);
            if(i == SFR_P1FCR_P10FCR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "Port1.0 Output: %s", active? "SO1" : "P1.0 Data");
            } else if(i == SFR_P1FCR_P11FCR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "Port1.1 Output: %s", active? "SI1" : "P1.1 Data");
            } else if(i == SFR_P1FCR_P12FCR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "Port1.2 Output: %s", active? "SCK0" : "P1.2 Data");
            } else if(i == SFR_P1FCR_P13FCR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "Port1.3 Output: %s", active? "SO1" : "P1.3 Data");
            } else if(i == SFR_P1FCR_P14FCR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "Port1.4 Output: %s", active? "SI1" : "P1.4 Data");
            } else if(i == SFR_P1FCR_P15FCR_POS) {
              //  _gyLog(GY_DEBUG_VERBOSE, "Port1.5 Output: %s", active? "SCK1" : "P1.5 Data");
            } else if(i == SFR_P1FCR_P16FCR_POS) {
               // if(active) _gyLog(GY_DEBUG_WARNING, "Port1.6 is fixed to P1.6 Data! Cannot be reassigned!");
            } else if(i == SFR_P1FCR_P17FCR_POS) {
               // _gyLog(GY_DEBUG_VERBOSE, "Port1.7 Output: %s", active? "PWM" : "P1.7 Data");
            }
        }
        _updatePins(dev, 0xff);
        //_gyPop(1);
        break;
    case SFR_ADDR_P1: {
        int tx = 0;
        for(uint8_t i = 0; i < PORT1_SIZE; ++i) {
            if(gyVmuPort1PinSrc(dev, i) == PORT1_PIN_DATA &&
                    gyVmuPort1PinDir(dev, i) == PORT1_PIN_OUT) {
                tx = 1;
                break;
            }
        }
        _updatePins(dev, 0xff);
        if(!tx) {
         //   _gyLog(GY_DEBUG_WARNING, "P1 data write [%d] with no output pins sourced from data!", val);
        }
    }
        break;
    }
}

PORT1_PIN_SRC gyVmuPort1PinSrc(const struct VMUDevice* dev, uint8_t pin) {
    return (uint8_t)dev->sfr[SFR_OFFSET(SFR_ADDR_P1FCR)]&(0x1<<pin);
}

PORT1_PIN_DIR gyVmuPort1PinDir(const struct VMUDevice* dev, uint8_t pin) {
    return (uint8_t)dev->sfr[SFR_OFFSET(SFR_ADDR_P1DDR)]&(0x1<<pin);
}

void gyVmuPort1PinSerialWrite(struct VMUDevice* dev, uint8_t pin, uint8_t value) {
    if(value) dev->port1.serialInSignals |= (0x1<<pin);
    else dev->port1.serialInSignals &= ~(0x1<<pin);
    _updatePin(dev, pin);
}

void gyVmuPort1PinsSerialWrite(VMUDevice *dev, uint8_t value) {
    gyVmuPort1PinsWriteBegin(dev);
    for(uint8_t i = 0; i < VMU_SERIAL_SIGNAL_COUNT; ++i)
        gyVmuPort1PinSerialWrite(dev, i, value&(0x1<<i));
    gyVmuPort1PinsWriteEnd(dev);
}

void gyVmuPort1PollRecv(struct VMUDevice* dev) {
    uint8_t recvPinMask = 0;
    for(uint8_t i = 0; i < PORT1_SIZE; ++i) {
        if(gyVmuPort1PinDir(dev, i) == PORT1_PIN_IN) {
            recvPinMask |= (0x1<<i);
        }
    }

    //Don't bother polling anything if we don't have any input pins
    if(recvPinMask) {
        if(dev->port1.ext && dev->port1.ext->readyRead(dev->port1.ext->context)) {
            uint8_t newPinsState;
            if(dev->port1.ext->read(dev->port1.ext->context, &newPinsState, sizeof(newPinsState))
                    == sizeof(newPinsState))
            {
                uint8_t changedMask     = 0;

                for(uint8_t i = 0; i < PORT1_SIZE; ++i) {
                    const uint8_t curMask = 0x1<<i;
                    //Check if the pin is set to receive data
                    if(recvPinMask&curMask) {
                        const uint8_t newHigh = newPinsState&curMask;
                        //Check if the state of the pin changed
                        if((dev->port1.extSignals&curMask) != newHigh) {
                            changedMask |= curMask;
                            if(newHigh) dev->port1.extSignals |= newHigh;
                            else dev->port1.extSignals &= ~newHigh;
                        }
                    }
                }

                _updatePins(dev, changedMask);

                //Notify the upper layer of any state change on relevant pins
                if(changedMask) {
                    gyVmuSerialP1Recv(dev, newPinsState, changedMask);
                }
            }
        }
    }
}

void gyVmuPort1ExtConnect(struct VMUDevice* dev, VMUPort1Ext* ext) {
    dev->port1.ext = ext;
}

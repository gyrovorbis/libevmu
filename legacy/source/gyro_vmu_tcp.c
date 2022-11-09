#if 1

#include "gyro_vmu_tcp.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_isr.h"
#include "gyro_vmu_maple.h"
#include "gyro_vmu_lcd.h"
#include <gyro_net_api.h>
#include <gyro_system_api.h>
#include <assert.h>

//FOR FUCKING HACK, REMOVE ME!!!!
#include "gyro_vmu_cpu.h"
//======


typedef enum TCP_MSG_TYPE {
    TCP_MSG_CONNECTION_TYPE,
    TCP_MSG_SIO0_BUFFER,
    TCP_MSG_SIO0_READY,
    TCP_MSG_SIO0_INACTIVE,
    TCP_MSG_SIO0_ACK,
    TCP_MSG_SIO1_BUFFER,
    TCP_MSG_SIO1_READY,
    TCP_MSG_SIO1_INACTIVE,
    TCP_MSG_SIO1_ACK
} TCP_MSG_TYPE;

static void _setSerialConnectionPins(VMUDevice* dev, VMU_SERIAL_CONNECTION_TYPE type) {
#if 1
    switch(type) {
    case VMU_SERIAL_CONNECTION_NONE:
        _gyLog(GY_DEBUG_VERBOSE, "Disconnecting VMU external serial pins.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~(SFR_P7_P70_MASK|SFR_P7_P72_MASK|SFR_P7_P73_MASK); //Clear all external pins
        break;
    case VMU_SERIAL_CONNECTION_VMU:
        _gyLog(GY_DEBUG_VERBOSE, "Configuring VMU external serial pins for VMU to VMU connection.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~(SFR_P7_P70_MASK|SFR_P7_P72_MASK); //Clear DC/PC pins
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= SFR_P7_P73_MASK;   //Set VMU pin
        dev->serialPort._txState = VMU_SERIAL_PORT_INACTIVE; //reset remote RX port state
        dev->serialPort._rxState = VMU_SERIAL_PORT_INACTIVE;
        break;

    case VMU_SERIAL_CONNECTION_DC:
        _gyLog(GY_DEBUG_VERBOSE, "Configuring VMU external serial pins for DC to VMU connection.");
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] &= ~SFR_P7_P73_MASK; //Clear VMU/VMU connection pin
        dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] |= (SFR_P7_P70_MASK|SFR_P7_P72_MASK|SFR_P7_P73_MASK); //set external voltage and DC controller pins
        gyVmuInterruptSignal(dev, VMU_INT_EXT_INT0);
        break;
    }
#endif
}

static int _handleIncomingTcpConnection(VMUDevice* dev) {
#if 1
    int newConnection = 0;

    _gyLog(GY_DEBUG_VERBOSE, "Receiving incoming TCP connection request.");
    _gyPush();

    if(dev->serialPort._tcpSocket) {
        if(!dev->serialPort._useMostRecentClient) {
            _gyLog(GY_DEBUG_WARNING, "Already connected to a client. Refusing request.");
            _gyPop(1);
            return 0;
        } else {
            _gyLog(GY_DEBUG_VERBOSE, "Disconnecting previous client and destroying socket.");
            gyNetTcpSocketClose(dev->serialPort._tcpSocket);
        }
    }

    dev->serialPort._tcpSocket = gyNetTcpSocketServerCheckIncomingConnections(dev->serialPort._tcpServer);
    if(dev->serialPort._tcpSocket) {
        newConnection = gyNetMonitoredConnectionsAdd(dev->serialPort._tcpSocket);
        if(newConnection) {
            _gyLog(GY_DEBUG_VERBOSE, "Connection established successfully.");
        } else {
            _gyLog(GY_DEBUG_ERROR, "Problem adding new connection to monitored connection list.");
        }
    } else {
        _gyLog(GY_DEBUG_ERROR, "Problem establishing socket.");
        newConnection = 0;
    }

    _gyPop(1);

    return newConnection;
#endif
}

static int _updateTcpServer(VMUDevice* dev) {
#if 1
    int newConnection = 0;

    //Incoming TCP client connection request
    if(gyNetTcpSocketReady(dev->serialPort._tcpServer)) {
         newConnection = _handleIncomingTcpConnection(dev);
    }

    return newConnection;
#endif
}

static int _mapleExtractBlockWriteDataSize(const GYMapleFrame* frame) {
    switch(frame->header.cmd) {
    case MAPLE_CMD_BLOCK_WRITE:
    case MAPLE_CMD_DATA_XFER:
        return (frame->header.dataWords * MAPLE_WORD_SIZE) - 4; //subtract parameter size
    default:
        return -1;

    }

}

static int _mapleExtractVMUFunctionCode(const GYMapleFrame* frame) {
    switch(frame->header.cmd) {
    case MAPLE_CMD_GET_COND:
    case MAPLE_CMD_GET_MEM_INFO:
    case MAPLE_CMD_BLOCK_READ:
    case MAPLE_CMD_BLOCK_WRITE:
    case MAPLE_CMD_SET_COND:
        switch(frame->payload.bytes[0]) {
        case MAPLE_FUNC_MEMCARD:
        case MAPLE_FUNC_LCD:
        case MAPLE_FUNC_CLOCK:
                return frame->payload.bytes[0];     //Valid and supported function code
        case MAPLE_FUNC_CONTROLLER:
        case MAPLE_FUNC_MIC:
        case MAPLE_FUNC_AR_GUN:
        case MAPLE_FUNC_KEYBOARD:
        case MAPLE_FUNC_LIGHT_GUN:
        case MAPLE_FUNC_PURUPURU:   // ==== !!!! OVERFLOW!?!? WTF, HOW CAN THIS FIELD BE LARGER THAN A BYTE!?!? ===
        case MAPLE_FUNC_MOUSE:
                return -1;                          //Unsupported function code
        default: return -2;                         //Invalid/unknown function code
        }
    default:                                        //CMD type has no function code
        return 0;
    }

}

static int _mapleRecvProcessBuffer(VMUDevice* dev, const unsigned char* packetBuff, int* bytesRemaining) {

    if(*bytesRemaining < MAPLE_HEADER_SIZE) {
        _gyLog(GY_DEBUG_ERROR, "[EVMU_MAPLE_RECV]: Recv buffer size [%d bytes] is too small to be a valid Maple header [%d bytes].", *bytesRemaining, MAPLE_HEADER_SIZE);
        return 0;
    }

    GYMapleFrame    txFrame;
    uint32_t        txBytes;
    GYMapleFrame*   rxFrame     = (GYMapleFrame*)packetBuff;
    unsigned        rxFrameSize = MAPLE_HEADER_SIZE + rxFrame->header.dataWords*MAPLE_WORD_SIZE;

    if(*bytesRemaining < rxFrameSize) {
        _gyLog(GY_DEBUG_ERROR, "[EVMU_MAPLE_RECV]: Recv buffer size [%d bytes] is smaller than Maple frame size: [%d bytes]", *bytesRemaining, rxFrameSize);
        return 0;
    }

    const int funcCode = _mapleExtractVMUFunctionCode(rxFrame);

    if(funcCode == -1) {
        _gyLog(GY_DEBUG_WARNING, "[EVMU_MAPLE_RECV]: Command [%d] sent for unsupported Function Code [%d]", rxFrame->header.cmd, funcCode);
        return 0;
    } else if(funcCode == -2) {
        _gyLog(GY_DEBUG_ERROR, "[EVMU_MAPLE_RECV]: Command [%d] sent for invalid/unknown Function Code [%d]", rxFrame->header.cmd, funcCode);
        return 0;
    }

    memset(&txFrame, 0, sizeof(GYMapleFrame));

    switch(rxFrame->header.cmd) {
        case MAPLE_CMD_EXT_DEV_INFO_REQ:
        case MAPLE_CMD_DEV_INFO_REQ:
            memcpy(&txFrame.payload.devInfoResp, VMUMapleDeviceInfoGet(dev), sizeof(VMUMapleDeviceInfo));
            if(gyVisualMemoryMapleFrame(MAPLE_CMD_DEV_INFO_RESP, rxFrame->header.recvAddr, rxFrame->header.sendAddr, &txFrame, sizeof(VMUMapleDeviceInfo), &txBytes))
                gyVmuSerialTcpSend(dev, &txFrame, txBytes);
            break;


        case MAPLE_CMD_BLOCK_READ: {
            unsigned char* flashBlock = gyVmuFlashBlock(dev, rxFrame->payload.blockRead.block);
            if(flashBlock) {
                _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: Reading block %d from flash.", rxFrame->payload.blockRead.block);
                memcpy(txFrame.payload.blockReadDataXferResp.data, flashBlock, VMU_FLASH_BLOCK_SIZE);
                if(gyVisualMemoryMapleFrame(MAPLE_CMD_DATA_XFER, rxFrame->header.recvAddr, rxFrame->header.sendAddr, &txFrame, VMU_FLASH_BLOCK_SIZE, &txBytes))
                    gyVmuSerialTcpSend(dev, &txFrame, txBytes);
                break;
            } else {
                _gyLog(GY_DEBUG_ERROR, "[EVMU_MAPLE_RECV]: Read from Flash block %d returned NULL!", rxFrame->payload.blockRead.block);
                return 0;
            }
        } break;

        case MAPLE_CMD_BLOCK_WRITE: {
            switch(funcCode) {
                case MAPLE_FUNC_MEMCARD: {
                    unsigned char* flashBlock = gyVmuFlashBlock(dev, rxFrame->payload.blockWrite.block);
                    int dataSize = _mapleExtractBlockWriteDataSize(rxFrame);
                    if(dataSize != VMU_FLASH_BLOCK_SIZE) {
                        _gyLog(GY_DEBUG_WARNING, "[EVMU_MAPLE_RECV]: Write to flash requested, but data size does not match block size: %d", dataSize);
                        return 0;
                    }
                    if(flashBlock)
                        memcpy(flashBlock, &rxFrame->payload.blockWrite.data, dataSize);
                } break;

            case MAPLE_FUNC_LCD:
                _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: Writing to VMU LCD Screen!");
                int dataSize = _mapleExtractBlockWriteDataSize(rxFrame);
                if(dataSize != 192) {
                    _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: Write issued to VMU LCD with incorrect data size: %d", dataSize);
                    return 0;
                }

                for(unsigned byte = 0; byte < 192; ++byte) {
                    for(unsigned bit = 0; bit < 8; ++bit) {
                        uint8_t val = (rxFrame->payload.blockWrite.data[byte]>>bit)&0x1;
                        unsigned y = byte / 6;
                        unsigned x = (byte % 6)*8 + bit;
                        gyVmuDisplayPixelSet(dev, x, y, val);
                    }
                }
                break;

            case MAPLE_FUNC_CLOCK:
                _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: Writing to VMU Clock!");
                //0-1: year (lower order byte in 0, upper order byte in 1
                //2 : month
                //3 : day
                //4 : hour
                //5 : minute
                //6 : second
                //7 : fixed value (0x00)


                break;
            default: break;
            }

            if(gyVisualMemoryMapleFrame(MAPLE_CMD_ACK, rxFrame->header.recvAddr, rxFrame->header.sendAddr, &txFrame, 0, &txBytes))
                gyVmuSerialTcpSend(dev, &txFrame, txBytes);

            break;
        case MAPLE_CMD_SET_COND:

                if(funcCode == MAPLE_FUNC_CLOCK) {
                    _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: Playing Sound [%x]!", *(uint32_t*)rxFrame->payload.setCond.cond);
                    uint8_t pulse = rxFrame->payload.setCond.cond[1];
                    uint8_t width = rxFrame->payload.setCond.cond[3];

                    gyVmuBuzzerSoundPlay(dev, pulse, width, 0);

                } else {
                    _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: (Virtual) Dreamcast device sent SETCOND command on unsupported function: %d (ignoring)", funcCode);
                }

                if(gyVisualMemoryMapleFrame(MAPLE_CMD_ACK, rxFrame->header.recvAddr, rxFrame->header.sendAddr, &txFrame, 0, &txBytes))
                    gyVmuSerialTcpSend(dev, &txFrame, txBytes);
                break;

        case MAPLE_CMD_NO_RESP:
            _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: (Virtual) Dreamcast device sent NO RESPONSE command! (ignoring)");
            break;

        case MAPLE_CMD_SHUTDOWN_DEV:
            _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: (Virtual) Dreamcast device sent SHUTDOWN command! (ignoring)");
            break;
        case MAPLE_CMD_RESET_DEVICE:
            _gyLog(GY_DEBUG_VERBOSE, "[EVMU_MAPLE_RECV]: (Virtual) Dreamcast device sent RESET command! (ignoring)");
            break;


        default:
            _gyLog(GY_DEBUG_WARNING, "[EVMU_MAPLE_RECV]: Unknown Maple command received: %d (Ignoring)", rxFrame->header.cmd);
        }

    }

    *bytesRemaining -= rxFrameSize;

    return 1;
}

static int _vmuRecvProcessBuffer(VMUDevice* dev, const unsigned char* packetBuff, int* bytesRemaining) {
    if(*bytesRemaining < 2) {
        _gyLog(GY_DEBUG_ERROR, "[EVMU_VMU_RECV]: Packet size of %d bytes is too small for a valid VMU transfer!", *bytesRemaining);
        return 0;
    }

    switch(packetBuff[0]) {
    case TCP_MSG_SIO0_BUFFER: //receiving from SBUF0 on other device, goes to SBUF1 on this device
       // if(!(dev->sfr[SFR_OFFSET(SFR_ADDR_P1DDR)]&(SFR_P1DDR_P14DDR_MASK)) &&   //P14/SI11/SB1 as receive
       //    !(dev->sfr[SFR_OFFSET(SFR_ADDR_P1FCR)]&(SFR_P1FCR_P13FCR_MASK)))     //P13/SO1 as general input
       // {
            //Check if receiving on SBUF1
            if(dev->serialPort._rxState == VMU_SERIAL_PORT_READY &&
                    (dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] & SFR_SCON1_CTRL_MASK))
            {
                _gyLog(GY_DEBUG_VERBOSE, "RECEIVED BYTE - %x", packetBuff[1]);
                _gyPush();
                //receive data
                dev->sfr[SFR_OFFSET(SFR_ADDR_SBUF1)] = packetBuff[1];

                char ack;

                if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] & SFR_SCON1_LEN_MASK) {
#if 1
                    ack = TCP_MSG_SIO1_READY;
#else
                    ack = TCP_MSG_SIO1_ACK;
#endif

                    _gyLog(GY_DEBUG_VERBOSE, "CONTINUOUS RECEIVE SET - setting SCON1_OV_MASK overflow mask");
                } else {
                ack = TCP_MSG_SIO1_ACK;
                    _gyLog(GY_DEBUG_VERBOSE, "CONTINUOUS RECEIVE NOT SET - not setting SCON1_OV_MASK");
                                             dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] &= ~SFR_SCON1_CTRL_MASK;

                }

                                            dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] |= SFR_SCON1_OV_MASK;

                //signal transfer complete
                dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] |= SFR_SCON1_END_MASK;

                //Raise interrupt after byte has been received, if IE bit is set
                if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] & SFR_SCON1_IE_MASK) {
                    _gyLog(GY_DEBUG_VERBOSE, "RAISING SCON1 INTERRUPT!");
                    gyVmuInterruptSignal(dev, VMU_INT_SIO1);
                }

                _gyPop(1);

                //Send ACK back to TX
                if(gyVmuSerialTcpSend(dev, &ack, 1)) {
                   // if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] & SFR_SCON1_LEN_MASK) {
#if 1
                    if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] & SFR_SCON1_LEN_MASK) {
                        dev->serialPort._rxState = VMU_SERIAL_PORT_READY;
                    } else {
                        dev->serialPort._rxState = VMU_SERIAL_PORT_INACTIVE;
                    }
#else
                    dev->serialPort._rxState = VMU_SERIAL_PORT_INACTIVE;
#endif
                   // }
                }
            } else {
                _gyLog(GY_DEBUG_ERROR, "Remote device sent a byte [%x] while local VMU's was not ready to receive!", packetBuff[1]);
            }
        break;
    case TCP_MSG_SIO1_BUFFER: //Normally SBUF1 should not be a sending buffer, but check config to support it
        _gyLog(GY_DEBUG_ERROR, "JUST RECIEVED SOME SHIT ON SI01 MSG!!! - %x", packetBuff[1]);
        break;
    case TCP_MSG_SIO1_ACK:

        if(dev->serialPort._txState == VMU_SERIAL_PORT_WAITING) {
            _gyLog(GY_DEBUG_VERBOSE, "ACK RECEIVED!");

            if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] & SFR_SCON0_LEN_MASK) {

                _gyLog(GY_DEBUG_VERBOSE, "CONTINUOUS SEND SET");
#if 0
                dev->serialPort._txState = VMU_SERIAL_PORT_READY;
#else
                dev->serialPort._txState = VMU_SERIAL_PORT_INACTIVE;
#endif
            } else {
                _gyLog(GY_DEBUG_VERBOSE, "CONTINUOUS SEND NOT SET");
                //signal transfer done
                dev->serialPort._txState = VMU_SERIAL_PORT_INACTIVE;
                dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] &= ~SFR_SCON0_CTRL_MASK;
            }
                dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] |= SFR_SCON0_OV_MASK;
            dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] |= SFR_SCON0_END_MASK;
            //Raise interrupt if interrupt bit is set
            if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] & SFR_SCON0_IE_MASK) {
                gyVmuInterruptSignal(dev, VMU_INT_SIO0);
                _gyLog(GY_DEBUG_VERBOSE, "RAISING SCON0 INTERRUPT!");
            }


        } else {
            _gyLog(GY_DEBUG_ERROR, "RECEIVED AN UNEXPECTED ACK!");
        }

        break;

    case TCP_MSG_SIO1_READY:
        _gyLog(GY_DEBUG_ERROR, "READY RECV!!!");
        dev->serialPort._txState = VMU_SERIAL_PORT_READY;
        break;
    default:
        _gyLog(GY_DEBUG_ERROR, "Received unknown VMU TCP message type [%d].", packetBuff[0]);
    }

    return 1;
}


static int _tcpRecvProcessBuffer(VMUDevice* dev, const unsigned char* packetBuff, int* bytesRemaining) {
    int success = 1;

    switch(gyVmuSerialConnectionType(dev)) {
    case VMU_SERIAL_CONNECTION_NONE:
        if(*bytesRemaining >= 2) {
            if(packetBuff[0] == TCP_MSG_CONNECTION_TYPE) {
                switch(packetBuff[1]) {
                case VMU_SERIAL_CONNECTION_DC:
                case VMU_SERIAL_CONNECTION_VMU:
                    _setSerialConnectionPins(dev, packetBuff[1]);
                    //FUCKING SHITTY-ASS HACK, FIXME!!!
                    for(unsigned i = 0; i < 10000; ++i) gyVmuCpuInstrExecuteNext(dev);
                    *bytesRemaining -= 2;
                    break;
                default:
                    _gyLog(GY_DEBUG_ERROR, "[EVMU_TCP_RECV]: Received pin connection message with invalid type: %d", packetBuff[1]);
                    success = 0;
                }
            } else {
                _gyLog(GY_DEBUG_ERROR, "[EVMU_TCP_RECV]: Received invalid TCP message of %d bytes before connection type was established!", *bytesRemaining);
                success = 0;
            }
        } else {
            _gyLog(GY_DEBUG_ERROR, "[EVMU_TCP_RECV]: Expecting 2 byte pin connection message, but only received %d byte(s)!", *bytesRemaining);
            success = 0;
        }
        break;
    case VMU_SERIAL_CONNECTION_DC:
        success = _mapleRecvProcessBuffer(dev, packetBuff, bytesRemaining);
        break;
    case VMU_SERIAL_CONNECTION_VMU:
        success = _vmuRecvProcessBuffer(dev, packetBuff, bytesRemaining);
        break;
    }

    return success;
}


static void _updateTcpSocketRecv(VMUDevice* dev) {
    unsigned char packetBuff[VMU_SERIAL_TCP_PACKET_SIZE_MAX];
    int recvBytes;

    //Check for activity on socket
    if(gyNetTcpSocketReady(dev->serialPort._tcpSocket)) {
        size_t reqBytes;
        switch(gyVmuSerialConnectionType(dev)) {
        case GY_VMU_TCP_CONNECTION_NONE:
            reqBytes = 2;
            break;
        default: reqBytes = VMU_SERIAL_TCP_PACKET_SIZE_MAX;
        }
        recvBytes = gyNetTcpSocketRecv(dev->serialPort._tcpSocket, packetBuff, reqBytes);

        if(recvBytes <= 0) {
            gyVmuSerialTcpClose(dev);

        } else {
            int bytesRemaining = recvBytes;

            //Keep parsing messages out of TCP packet, because they're gonna potentially be grouped together, thanks to Nagle's bitch ass!
            while(bytesRemaining > 0) {
                if(!_tcpRecvProcessBuffer(dev, &packetBuff[recvBytes-bytesRemaining], &bytesRemaining)) {
                    _gyLog(GY_DEBUG_WARNING, "[EVMU_TCP_RECV] Leaving %d bytes unprocessed from packet!", bytesRemaining);
                    break;
                }
            }
        }
    }

}


static void _updateTcpSocketSend(VMUDevice* dev) {
#if 1 //TEMPORARY BULLSHIT REESTED TO ALL FUCK


#else
            if(gyVmuSerialConnectionType(dev) == VMU_SERIAL_CONNECTION_DC) {
                if(dev->serialPort._txStart) {
                    _gyLog(GY_DEBUG_VERBOSE, "[EVMU] - Sending 256 bytes from WRAM!!!");
                    gyVmuSerialTcpSend(dev, dev->wram, 256);
                    dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] |= SFR_SCON0_OV_MASK;
                dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] |= SFR_SCON0_END_MASK;
                //Raise interrupt if interrupt bit is set
                if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] & SFR_SCON0_IE_MASK) {
                    dev->intReq |= 1 << VMU_INT_SIO0;
                    _gyLog(GY_DEBUG_VERBOSE, "RAISING SCON0 INTERRUPT!");
                }


                }

            } else {
#if 1
    //Send SBUF0 to remote if it's ready and SCON0.CTRL is set.
    if(dev->serialPort._txState == VMU_SERIAL_PORT_READY //remote SCON1 is ready to recieve
    &&
#if 0
            (
                (dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] & SFR_SCON0_CTRL_MASK)
             || (dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] & SFR_SCON0_LEN_MASK)
            )
#else
            dev->serialPort._txStart
#endif
            ) //local SCON0 is ready to send
    {   //check pin directions
        if((dev->sfr[SFR_OFFSET(SFR_ADDR_P1DDR)]&(SFR_P1DDR_P10DDR_MASK|SFR_P1DDR_P13DDR_MASK)) && //P1.SO0 and P1.SCK0 set as OUT pins
           (dev->sfr[SFR_OFFSET(SFR_ADDR_P1FCR)]&(SFR_P1FCR_P10FCR_MASK|SFR_P1FCR_P13FCR_MASK))) //SO0 and SCK0 sourced from serial interface
        {
            //send SBUF0 to remote
            unsigned char sbuf0[2] = {
                TCP_MSG_SIO0_BUFFER,
                dev->sfr[SFR_OFFSET(SFR_ADDR_SBUF0)]
            };
            //update txStat if send is successful
            if(gyVmuSerialTcpSend(dev, sbuf0, 2)) {
                _gyLog(GY_DEBUG_VERBOSE, "SENT BYTE - %x", dev->sfr[SFR_OFFSET(SFR_ADDR_SBUF0)]);
                dev->serialPort._txState = VMU_SERIAL_PORT_WAITING;
            }

            dev->serialPort._txStart = 0;

            if(dev->sfr[SFR_OFFSET(SFR_ADDR_SCON0)] & SFR_SCON0_OV_MASK) {

            }
        }
    }

    //Tell remote that we're ready to receive and change our state to ready.
    if(dev->serialPort._rxState == VMU_SERIAL_PORT_INACTIVE &&
        (dev->sfr[SFR_OFFSET(SFR_ADDR_SCON1)] & SFR_SCON1_CTRL_MASK))
    {
        unsigned char ready = TCP_MSG_SIO1_READY;
        if(gyVmuSerialTcpSend(dev, &ready, 1)) {
            dev->serialPort._rxState = VMU_SERIAL_PORT_READY;
        }
    }
            }
#endif
#endif
}


#if 1
void _gyVmuSerialUpdateTcp(VMUDevice* dev) {

    int newConnection = 0;

    //Check if we're using serial over TCP
    if(dev->serialPort._tcpServer || dev->serialPort._tcpSocket) {

        //Check the status of our active connections
        gyNetMonitoredConnectionsPoll(VMU_SERIAL_TCP_POLL_TIMEOUT);

        //update TCP Server
        if(dev->serialPort._tcpServer) {
            newConnection = _updateTcpServer(dev);
        }

        //Update TCP Socket (only if it existed when we polled the sockets)
        if(!newConnection && dev->serialPort._tcpSocket) {
            _updateTcpSocketSend(dev);
            _updateTcpSocketRecv(dev);
        }

    }

}

void gyVmuSerialPortUpdate(VMUDevice* dev) {
    _gyVmuSerialUpdateTcp(dev);
    //update other shit like GPIO pins or physical connection here
}

VMU_SERIAL_CONNECTION_TYPE gyVmuSerialConnectionType(const struct VMUDevice* dev) {
    if(dev->serialPort._tcpSocket) {
       // if(dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] & SFR_P7_P73_MASK) {
         //   return VMU_SERIAL_CONNECTION_VMU;
        //} else
        if(dev->sfr[SFR_OFFSET(SFR_ADDR_P7)] & SFR_P7_P70_MASK) {
            return VMU_SERIAL_CONNECTION_DC;
        }
    }

    return VMU_SERIAL_CONNECTION_NONE;
}

int gyVmuSerialTcpServerStart(struct VMUDevice* dev, unsigned port) {
    dev->serialPort._tcpServer = gyNetTcpSocketServerCreate(port);
    return dev->serialPort._tcpServer? 1 : 0;
}

int gyVmuSerialTcpServerStop(struct VMUDevice* dev) {
    int success = 1;

    success &= gyVmuSerialTcpClose(dev);

    if(dev->serialPort._tcpServer) {
        success &= gyNetTcpSocketServerDestroy(dev->serialPort._tcpServer);
        dev->serialPort._tcpServer = NULL;
    }

    return success;
}

int gyVmuSerialTcpOpen(VMUDevice* dev, const char* host, unsigned port) {
    int success = 0;
    dev->serialPort._tcpSocket = gyNetTcpSocketOpen(host, port);
    if(dev->serialPort._tcpSocket) {
        if(!gyNetMonitoredConnectionsAdd(dev->serialPort._tcpSocket)) {
            _gyLog(GY_DEBUG_VERBOSE, "Could not add socket to monitored connection list!");

        } else {

            //Send connection type to remote immediately.
            char bytes[2] = {
                TCP_MSG_CONNECTION_TYPE,
                VMU_SERIAL_CONNECTION_VMU
            };
            if(gyNetTcpSocketSend(dev->serialPort._tcpSocket, bytes, 2)) {
                _gyLog(GY_DEBUG_VERBOSE, "Connection type sent to remote.");
                _setSerialConnectionPins(dev, VMU_SERIAL_CONNECTION_VMU);
                success = 1;
            } else {
                _gyLog(GY_DEBUG_ERROR, "Unable to send connection type over TCP!");
            }
        }

    }
    return success;
}

int gyVmuSerialTcpClose(VMUDevice *dev) {
    int success = 1;
    if(dev->serialPort._tcpSocket) {
        _setSerialConnectionPins(dev, VMU_SERIAL_CONNECTION_NONE);
        success &= gyNetMonitoredConnectionsRemove(dev->serialPort._tcpSocket);
        success &= gyNetTcpSocketClose(dev->serialPort._tcpSocket);
        dev->serialPort._tcpSocket = NULL;
    }

    return success;
}

void gyVmuSerialTcpServerUseMostRecentClientSet(struct VMUDevice* dev, int val) {
    dev->serialPort._useMostRecentClient = val;
}

int gyVmuSerialTcpSend(struct VMUDevice* dev, void* bytes, int size) {
    int bytesSent = 0;
    int success = 0;

    if(dev->serialPort._tcpSocket) {
        if((bytesSent = gyNetTcpSocketSend(dev->serialPort._tcpSocket, bytes, size)) != size) {
            _gyLog(GY_DEBUG_ERROR, "Could not send packet over TCP! [Bytes Sent: %d]", bytesSent);
        } else success = 1;
    } else {
        _gyLog(GY_DEBUG_ERROR, "No open TCP socket! Cannot send packet!");
    }

    return success;
}

void gyVmuSerialTcpSconUpdate(struct VMUDevice* dev, int sfr, unsigned char val) {
    if(!dev->serialPort._tcpSocket) return;

    switch(sfr) {
    case SFR_ADDR_SCON0:
        if(val & (SFR_SCON0_LEN_MASK|SFR_SCON0_CTRL_MASK)) {
            dev->serialPort._txStart = 1;
            _gyLog(GY_DEBUG_ERROR, "TXSTART! STATE - %d", dev->serialPort._txState);
        }
        break;
    case SFR_ADDR_SCON1:

        break;
    default: assert(0); //What the fuck, you should not have passed a non-SCON sfr here, fuckface!!!
    }

}
#endif
#endif


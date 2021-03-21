#include <gyro_file_api.h>
#include <gyro_system_api.h>
#include <string.h>
#include <assert.h>
#include "gyro_vmu_memory.h"
#include "gyro_vmu_bios.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_sfr.h"

int gyVmuBiosSystemCodeActive(struct VMUDevice* device) {
    return device->imem == device->rom;
}


static inline int _monthDays(const VMUDevice* dev) {
  int m = dev->ram[0][0x19];
  if(m==2) {
    int y = dev->ram[0][0x18] | (dev->ram[0][0x17] << 8);
    if(y&3)
      return 28;
    if(!(y%4000))
      return 29;
    if(!(y%1000))
      return 28;
    if(!(y%400))
      return 29;
    if(!(y%100))
      return 28;
    return 29;
  } else return (m>7? ((m&1)? 30:31) : ((m&1)? 31:30));
}

int gyVmuBiosLoadImage(VMUDevice *device, const unsigned char* buffer, int size) {
    _gyLog(GY_DEBUG_VERBOSE, "Copying BIOS image from buffer [%d bytes].", size);
    _gyPush();

    //Clear ROM
    memset(device->rom, 0, sizeof(device->rom));

    if(!buffer || size <= 0 || size > ROM_SIZE) {
        _gyLog(GY_DEBUG_ERROR, "Could not copy image buffer!");
    } else {
        memcpy(device->rom, buffer, size);
    }
    _gyPop(1);

    return 1;
}

int gyVmuBiosLoad(VMUDevice* device, const char* path) {
    GYFile* file;

    _gyLog(GY_DEBUG_VERBOSE, "Loading BIOS image from file [%s].", path);
    _gyPush();

    if (!gyFileOpen(path, "rb", &file)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open file!");
        _gyPop(0);
        return 0;
    }

    //Clear ROM
    memset(device->rom, 0, sizeof(device->rom));

    size_t bytesRead   = 0;
    size_t bytesTotal  = 0;

    while(bytesTotal < sizeof(device->rom)) {
        if(gyFileRead(file, device->rom+bytesTotal, sizeof(device->rom)-bytesTotal, &bytesRead)) {
            bytesTotal += bytesRead;
        } else break;
    }

    gyFileClose(&file);

    _gyLog(GY_DEBUG_VERBOSE, "Read %d bytes.", bytesTotal);

    //assert(bytesRead <= 0);       //Didn't read shit
    assert(bytesTotal >= 0);

    _gyPop(0);
    return 1;

}


void _biosWriteFlashRom(VMUDevice* dev) {
    int i, a = ((dev->ram[1][0x7d]<<16)|(dev->ram[1][0x7e]<<8)|dev->ram[1][0x7f])&0x1ffff;
    if(a>=dev->gameSize)
        gyVmuMemWrite(dev, 0x100, 0xff);
    else {
        gyVmuMemWrite(dev, 0x100, 0x00);
        for(i=0; i<0x80; i++)
            dev->flash[(a&~0xff)|((a+i)&0xff)] = dev->ram[1][i+0x80];
#if 0
#ifdef __DC__
        if(!flash_written(a))
        writemem(0x100, 0xff);
#endif
#endif
    }

}

/*
 * TimeShooter.VMS is entering FM at unknown address!!!!! Marcus's emulator can't handle it either.
 *
 *
 *
 */
int gyVmuBiosHandleCall(VMUDevice* dev) {
    switch(dev->pc) {
    case BIOS_ADDR_FM_WRT_EX: //fm_wrt_ex(ORG 0100H)
        _biosWriteFlashRom(dev);
        return 0x105;
    case BIOS_ADDR_FM_WRTA_EX: //fm_vrf_ex(ORG 0108H)
        _biosWriteFlashRom(dev);
        return 0x10b;
    case BIOS_ADDR_FM_VRF_EX: { //fm_vrf_ex(ORG 0110H)
        int i, a = ((dev->ram[1][0x7d]<<16)|(dev->ram[1][0x7e]<<8)|dev->ram[1][0x7f])&0x1ffff;
        int r = 0;
        for(i=0; i<0x80; i++)
        if((r = (dev->flash[(a&~0xff)|((a+i)&0xff)] ^ dev->ram[1][i+0x80])) != 0)
        break;
        //writemem(0x100, r);
        //printf("READ FLASH[%x] = %d\n", 0, r);
        gyVmuMemWrite(dev, 0x100, r);
        return 0x115;
    }
    case BIOS_ADDR_FM_PRD_EX: { //fm_prd_ex(ORG 0120H)
        int i, a = ((dev->ram[1][0x7d]<<16)|(dev->ram[1][0x7e]<<8)|dev->ram[1][0x7f])&0x1ffff;
        for(i=0; i<0x80; i++) {
        dev->ram[1][i+0x80] = dev->flash[(a&~0xff)|((a+i)&0xff)];
                //printf("READ FLASH[%x] = %d\n", (a&~0xff)|((a+i)&0xff), dev->ram[1][i+0x80]);
        }


        /*
        fprintf(stderr, "ROM read @ %05x\n", a);
        */
        return 0x125;
    }
    case BIOS_ADDR_TIMER_EX: //timer_ex fm_prd_ex(ORG 0130H)
        if(!((dev->ram[0][0x1e]^=1)&1))
            if(++dev->ram[0][0x1d]>=60) {
                dev->ram[0][0x1d] = 0;
                if(++dev->ram[0][0x1c]>=60) {
                    dev->ram[0][0x1c] = 0;
                    if(++dev->ram[0][0x1b]>=24) {
                        dev->ram[0][0x1b] = 0;
                        if(++dev->ram[0][0x1a] > _monthDays(dev)) {
                            dev->ram[0][0x1a] = 1;
                            if(++dev->ram[0][0x19]>=13) {
                                dev->ram[0][0x19] = 1;
                                if(dev->ram[0][0x18]==0xff) {
                                    dev->ram[0][0x18]=0;
                                    dev->ram[0][0x17]++;
                                } else
                                    dev->ram[0][0x18]++;
                            }
                        }
                    }
                }
            }
        return 0x139;
    case BIOS_ADDR_SLEEP_EX: //fm_prd_ex(ORG 0140H)
    // AT LEAST ENTER HALT MODE!!!!
        _gyLog(GY_DEBUG_WARNING, "Entered firmare at SLEEP mode address! Unimplemented!");
        return 0;
    default:
        //assert(0);
        _gyLog(GY_DEBUG_ERROR, "Entering firmware at unknown address! [%x]", dev->pc);
        return 0;
    }

}






#ifndef GYRO_VMU_DEVICE_H
#define GYRO_VMU_DEVICE_H

#include <stdint.h>
#include "gyro_vmu_instr.h"
#include "gyro_vmu_flash.h"
#include <gimbal/core/gimbal_decls.h>

#define EVMU_DEVICE_PRISTINE(gyDev)         ((EvmuDevice_*)(gyDev->pPristineDevice))
#define EVMU_DEVICE_PRISTINE_PUBLIC(gyDev)  ((EvmuDevice*)GBL_INSTANCE_PUBLIC(gyDev->pPristineDevice, EVMU_DEVICE_TYPE))

#ifdef __cplusplus
extern "C" {
#endif

GBL_FORWARD_DECLARE_STRUCT(EvmuDevice);

#define VMU_TRIGGER_SPEED_FACTOR    10.0f

#define VMU_MEM_SEG_MASK            0x180
#define VMU_MEM_SEG_POS             7
#define VMU_MEM_OFFSET_MASK         0x7f
#define VMU_MEM_SEG_SIZE            128
#define VMU_MEM_ADDR_SPACE_RANGE    (VMU_MEM_SEG_COUNT * VMU_MEM_SEG_SIZE)

struct LCDFile;

typedef enum VMU_RAM_BANK {
    VMU_RAM_BANK0,
    VMU_RAM_BANK1
} VMU_RAM_BANK;

typedef enum VMU_MEM_SEGMENT {
    VMU_MEM_SEG_GP1,
    VMU_MEM_SEG_GP2,
    VMU_MEM_SEG_SFR,
    VMU_MEM_SEG_XRAM,
    VMU_MEM_SEG_COUNT
} VMU_MEM_SEGMENT;

struct VMUDevice;

typedef void (*VMUMemoryChangeFn)(struct VMUDevice* pDevice, uint16_t address);
typedef void (*VMUFlashChangeFn)(struct VMUDevice* pDevice, uint16_t address);

typedef struct VMUDevice {
//    unsigned char   flash   [FLASH_SIZE];
//    unsigned char   rom     [ROM_SIZE];
//    unsigned char   xram    [XRAM_BANK_COUNT][XRAM_BANK_SIZE];
//    unsigned char   wram    [WRAM_SIZE];
//    unsigned char   sfr     [RAM_SFR_SIZE];                     //not including XRAM
//    unsigned char   ram     [RAM_BANK_COUNT][RAM_BANK_SIZE];    //general-purpose RAM
//    unsigned char*  memMap  [VMU_MEM_SEG_COUNT];                //contiguous RAM address space
//    unsigned char*  imem;

    VMUInstr            curInstr;               // last

//    VMUInterruptController
//                        intCont;                // PIC - soon
//    VMUTimer0           timer0;                 // last
//    VMUTimer1           timer1;                 // last
//    VMUPort1            port1;                  // n/a
//    VMUSerial           serial;                 // n/a
//    VMUSerialPort       serialPort;             // n/a
//    VMUBuzzer           buzzer;                 // soon
//    VMUGamepad          gamepad;                // soon
//    VMUDisplay          display;

    int                 speed;
//    float               tBaseDeltaTime;
//    float               tBase1DeltaTime;
    struct LCDFile*     lcdFile;

//    uint16_t            pc;
    VMUFlashPrg         flashPrg;

//    unsigned char       biosLoaded;

    void*               pMemoryUserData;
    VMUMemoryChangeFn   pFnMemoryChange;
    void*               pFlashUserData;
    VMUFlashChangeFn    pFnFlashChange;

    void*         pPristineDevice;
} VMUDevice;

VMUDevice*          gyVmuDeviceCreate(void);
int                 gyVmuDeviceInit(VMUDevice* device);
void                gyVmuDeviceDestroy(VMUDevice* device);
//int                 gyVmuDeviceUpdate(VMUDevice* device, double deltaTime);
//void                gyVmuDeviceReset(VMUDevice* device);

int                 gyVmuDeviceSaveState(VMUDevice* device, const char* path);
int                 gyVmuDeviceLoadState(VMUDevice* device, const char* path);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_DEVICE_H


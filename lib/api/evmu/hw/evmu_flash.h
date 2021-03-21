#ifndef EVMU_FLASH_H
#define EVMU_FLASH_H

#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

//2 - Flash Memory
#define FLASH_BANK_SIZE         65536
#define FLASH_BANKS             2
#define FLASH_SIZE              (FLASH_BANK_SIZE*FLASH_BANKS)

#define EVMU_FLASH_PROGRAM_BYTE_COUNT                    128     //number of bytes software can write to flash once unlocked
#define EVMU_FLASH_PROGRAM_STATE0_ADDRESS                0x5555  //Key-value pairs for flash unlock sequence used by STF instruction
#define EVMU_FLASH_PROGRAM_STATE0_VALUE                  0xaa
#define EVMU_FLASH_PROGRAM_STATE1_ADDRESS                0x2aaa
#define EVMU_FLASH_PROGRAM_STATE1_VALUE                  0x55
#define EVMU_FLASH_PROGRAM_STATE2_ADDRESS                0x5555
#define EVMU_FLASH_PROGRAM_STATE2_VALUE                  0xa0

typedef enum EVMU_FLASH_PROGRAM_STATE {
    EVMU_FLASH_PROGRAM_STATE0,
    EVMU_FLASH_PROGRAM_STATE1,
    EVMU_FLASH_PROGRAM_STATE2
} EVMU_FLASH_PROGRAM_STATE;


GBL_DEFINE_HANDLE(EvmuFlash) // Programmable Interrupt Controller

GBL_DECLARE_ENUM(EVMU_FLASH_PROPERTY) {
    EVMU_FLASH_PROPERTY_ADDRESS_BIT_9 = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_FLASH_PROPERTY_TARGET_ADDRESS, //current address that would be used for accessing Flash
    EVMU_FLASH_PROPERTY_UNLOCKED,
    EVMU_FLASH_PROPERTY_PROGRAM_STATE,
            // Previously written program addresses and values?
    EVMU_FLASH_PROPERTY_PROGRAM_BYTES,
    EVMU_FLASH_PROPERTY_PROGRAM_CYCLES, // # of cycles to to wait/elapsed before safe to write?
    EVMU_FLASH_PROPERTY_PROGRAM_STATUS, //Status/error SFR?
    EVMU_FLASH_PROPERTY_COUNT
};

// FLASH ADDRESSES ARE 17 bits large!! (2 byte address + bank bit)
EVMU_API    evmuFlashWriteBytes(EvmuFlash*      pFlash, EvmuAddress address, EvmuSize* pSize, const void*   pData);
EVMU_API    evmuFlashReadBytes(const EvmuFlash  pFlash, EvmuAddress address, EvmuSize* pSize, void*         pData);






#ifdef __cplusplus
}
#endif

#endif // EVMU_FLASH_H


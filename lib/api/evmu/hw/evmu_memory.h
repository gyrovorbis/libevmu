#ifndef EVMU_MEMORY_H
#define EVMU_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "evmu_address_space.h"
#include "evmu_peripheral.h"

//General-Purpose Registers
#define REG_R0_OFFSET           0x0
#define REG_R1_OFFSET           0x1
#define REG_R2_OFFSET           0x2

struct  VMUDevice;
GBL_DECLARE_HANDLE(EvmuMemory);

GBL_DECLARE_ENUM(EVMU_MEMORY_ACCESS_TYPE) {
    EVMU_MEMORY_ACCESS_PORT,
    EVMU_MEMORY_ACCESS_LATCH,
    EVMU_MEMORY_ACCESS_OTHER //hide me from API, so internal shit only can use it
};

typedef enum EVMU_MEMORY_BANK {
    EVMU_MEMORY_BANK_INVALID,
    EVMU_MEMORY_BANK_CURRENT,
    EVMU_MEMORY_BANK_0,
    EVMU_MEMORY_BANK_1,
    EVMU_MEMORY_BANK_3,
    EVMU_MEMORY_BANK_COUNT
} EVMU_MEMORY_BANK;

typedef enum EVMU_RAM_BANK {
    EVMU_RAM_BANK_SYSTEM,
    EVMU_RAM_BANK_APP,
    EVMU_RAM_BANK_COUNT
} EVMU_RAM_BANK;

typedef enum EVMU_MEMORY_SEGMENT_TYPE {
    EVMU_MEMORY_SEGMENT_TYPE_RAM,
    EVMU_MEMORY_SEGMENT_TYPE_SFR,
    EVMU_MEMORY_SEGMENT_TYPE_XRAM,
    EVMU_MEMORY_SEGMENT_TYPE_WRAM,
    EVMU_MEMORY_SEGMENT_TYPE_ROM,
    EVMU_MEMORY_SEGMENT_TYPE_FLASH,
    EVMU_MEMORY_SEGMENT_TYPE_COUNT
} EVMU_MEMORY_SEGMENT_TYPE;

typedef enum EVMU_MEM_SEGMENT {
    EVMU_MEM_SEG_GP1,
    EVMU_MEM_SEG_GP2,
    EVMU_MEM_SEG_SFR,
    EVMU_MEM_SEG_XRAM,
    EVMU_MEM_SEG_COUNT
} EVMU_MEM_SEGMENT;


typedef enum EVMU_MEMORY_EXT_SOURCE_TYPE {
    EVMU_MEMORY_EXT_ROM,
    EVMU_MEMORY_EXT_FLASH
} EVMU_MEMORY_EXT_SOURCE_TYPE;

typedef struct EvmuAddressRange {
    EvmuAddress baseAddress;
    uint16_t    count;
} EvmuAddressRange;


GBL_DECLARE_ENUM(EVMU_MEMORY_PROPERTY) {
    EVMU_MEMORY_PROPERTY_ACTIVE_RAM_BANK = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_MEMORY_EXT_SOURCE,
    EVMU_MEMORY_PROPERTY_COUNT
};

// Generic read/write over all BUSes
EVMU_API evmuMemorySegmentCount(const EvmuMemory* pMem, uint32_t* pCount);
EVMU_API evmuMemorySegmentInfo(const EvmuMemory* pMem, EVMU_MEMORY_SEGMENT_TYPE segmentType, EvmuSize* pPageSize, uint8_t* pBankCount);

EVMU_API evmuMemorySegmentRead(const EvmuMemory* pMem, EVMU_MEMORY_SEGMENT_TYPE segment, EVMU_MEMORY_BANK bank, EvmuAddress baseAddress, EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);
EVMU_API evmuMemorySegmentWrite(const EvmuMemory* pMem, EVMU_MEMORY_SEGMENT_TYPE segment, EVMU_MEMORY_BANK bank, EvmuAddress baseAddress, const EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);

// Generic read/write over only ROM/Flash EXT/Imem sources
EVMU_API evmuMemoryExtRead(const EvmuMemory* pMem, EvmuAddress baseAddress, EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);
EVMU_API evmuMemoryExtWrite(EvmuMemory* pMem, EvmuAddress baseAddress, const EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);

//Generic read/write over memory addres space
EVMU_API evmuMemoryDataRead(const EvmuMemory* pMem, EvmuAddress baseAddres, EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);
EVMU_API evmuMemoryDataWrite(EvmuMemory* pMem, EvmuAddress baseAddress, const EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);

// Read/write into only RAM (current bank, SYSTEM, or APPLICATION RAM bank
EVMU_API evmuMemoryRamRead(const EvmuMemory* pMem, EVMU_MEMORY_BANK bank, EvmuAddress baseAddress, EvmuWord* pData, EvmuSize* pBytes);
EVMU_API evmuMemoryRamWrite(const EvmuMemory* pMem, EVMU_MEMORY_BANK bank, EvmuAddress baseAddress, const EvmuWord* pData, EvmuSize* pBytes);

// Read/write into only SFRs
EVMU_API evmuMemorySfrRead(const EvmuMemory* pMem, uint8_t baseOffset, EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);
EVMU_API evmuMemorySfrWrite(const EvmuMemory* pMem, uint8_t baseOffset, const EvmuWord* pData, EvmuSize* pBytes, EVMU_MEMORY_ACCESS_TYPE accessType);

// truncate and return appropriate error on overflow/underflow and shit!
EVMU_API evmuMemoryStackPush(EvmuMemory hMem, const EvmuWord* pData, EvmuSize* pBytes);
EVMU_API evmuMemoryStackPop(EvmuMemory hMem, EvmuWord* pData, EvmuSize* pBytes);
// stack depth?


//====== private memory controller header ========

#if 0
typedef struct MemoryController_ {
    EvmuPeripheral  peripheral;
    EvmuWord        ram     [EVMU_RAM_BANK_COUNT][RAM_BANK_SIZE];    //general-purpose RAM
    uint8_t         sfrPeripheralMap[EVMU_ADDRESS_SEGMENT_SFR_SIZE];
} MemoryController_;

static EVMU_RESULT init(EvmuPeripheral* pPeripheral) {
    MemoryController_* pCont = (MemoryController_*)pPeripheral;
    memset(pCont)
}


typedef EVMU_RESULT (*EvmuPeripheralInitFn)             (EvmuPeripheral* pPeripheral);
typedef EVMU_RESULT (*EvmuPeripheralDeinitFn)           (EvmuPeripheral* pPeripheral);
typedef EVMU_RESULT (*EvmuPeripheralResetFn)            (EvmuPeripheral* pPeripheral, EVMU_PERIPHERAL_MODE);
typedef EVMU_RESULT (*EvmuPeripheralUpdateFn)           (EvmuPeripheral* pPeripheral, EvmuTicks deltaTicks);
typedef EVMU_RESULT (*EvmuPeripheralEventFn)            (EvmuPeripheral* pPeripheral, EVMU_EVENT_TYPE eventType, void* pData, EvmuSize size);

typedef EVMU_RESULT (*EvmuPeripheralMemoryReadFn)       (EvmuPeripheral* pPeripheral, EvmuAddress address, EVMU_MEMORY_ACCESS_TYPE acessType, EvmuWord* pValue);
typedef EVMU_RESULT (*EvmuPeripheralMemoryWriteFn)      (EvmuPeripheral* pPeripheral, EvmuAddress address, EVMU_MEMORY_ACCESS_TYPE acessType, EvmuWord value);

typedef EVMU_RESULT (*EvmuPeripheralPropertyFn)         (EvmuPeripheral* pPeripheral, GblEnum property, void* pData, EvmuSize* pSize);
typedef EVMU_RESULT (*EvmuPeripheralPropertySetFn)      (EvmuPeripheral* pPeripheral, GblEnum property, void* pData, EvmuSize* pSize);

typedef EVMU_RESULT (*EvmuPeripheralStateSaveFn)        (EvmuPeripheral* pPeripheral, const void* pData, EvmuSize* pSize);
typedef EVMU_RESULT (*EvmuPeripheralStateLoadFn)        (EvmuPeripheral* pPeripheral, const void* pData, EvmuSize pSize);

typedef EVMU_RESULT (*EvmuPeripheralParseCmdLineArgs)   (EvmuPeripheral* pPeripheral, int, const char*[]);
typedef EVMU_RESULT (*EvmuPeripheralDebugDumpStateFn)   (EvmuPeripheral* pPeripheral, const char*, EvmuSize* pSize);
typedef EVMU_RESULT (*EvmuPeripheralDebugCommandFn)     (EvmuPeripheral* pPeripheral, const char* pCmd);

#endif


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_H


#ifndef EVMU_PERIPHERAL_H
#define EVMU_PERIPHERAL_H

#include <gimbal/types/gimbal_variant.h>
#include "../util/evmu_context.h"


#ifdef __cplusplus
extern "C" {
#endif

// VmuContext is analogous to ES Area
// VmuDevice is analogous to ES Entities
// Peripherals are 100% analogous to ES Behaviors

GBL_FORWARD_DECLARE_ENUM(EVMU_EVENT_TYPE);
GBL_FORWARD_DECLARE_ENUM(EVMU_MEMORY_ACCESS_TYPE);

#define EVMU_PERIPHERAL_INDEX_INVALID 0xff


typedef EVMU_RESULT (*EvmuPeripheralInitFn)             (EvmuPeripheral);
typedef EVMU_RESULT (*EvmuPeripheralDeinitFn)           (EvmuPeripheral);
typedef EVMU_RESULT (*EvmuPeripheralResetFn)            (EvmuPeripheral);//, EVMU_PERIPHERAL_MODE);
typedef EVMU_RESULT (*EvmuPeripheralUpdateFn)           (EvmuPeripheral, EvmuTicks);
//typedef EVMU_RESULT (*EvmuPeripheralClockEvent)         (EvmuPeripheral hPeripheral, EvmuClockEvent event);
typedef EVMU_RESULT (*EvmuPeripheralEventFn)            (EvmuPeripheral, EVMU_EVENT_TYPE, void*, EvmuSize size);

typedef EVMU_RESULT (*EvmuPeripheralMemoryReadFn)       (EvmuPeripheral, EvmuAddress, EVMU_MEMORY_ACCESS_TYPE, EvmuWord);
typedef EVMU_RESULT (*EvmuPeripheralMemoryWriteFn)      (EvmuPeripheral, EvmuAddress, EVMU_MEMORY_ACCESS_TYPE, EvmuWord);

typedef EVMU_RESULT (*EvmuPeripheralStateSaveFn)        (EvmuPeripheral, char*, EvmuSize*);
typedef EVMU_RESULT (*EvmuPeripheralStateLoadFn)        (EvmuPeripheral, const char*, EvmuSize*);

typedef EVMU_RESULT (*EvmuPeripheralParseCmdLineArgs)   (EvmuPeripheral, int, const char**);
typedef EVMU_RESULT (*EvmuPeripheralDebugCommandFn)     (EvmuPeripheral, const char*);

GBL_DECLARE_ENUM(EVMU_PERIPHERAL_INDEX) {
    EVMU_PERIPHERAL_MEMORY,
    EVMU_PERIPHERAL_CLOCK,
    EVMU_PERIPHERAL_CPU,
    EVMU_PERIPHERAL_PIC,
    EVMU_PERIPHERAL_FLASH,
    EVMU_PERIPHERAL_LCD,

    EVMU_PERIPHERAL_TIMER0,
    EVMU_PERIPHERAL_TIMER1,
    EVMU_PERIPHERAL_DISPLAY,
    EVMU_PERIPHERAL_SERIAL_PORT,
    EVMU_PERIPHERAL_HW_COUNT = EVMU_PERIPHERAL_DISPLAY + 1,
    EVMU_PERIPHERAL_SW_LCD_PLAYER,
    EVMU_PERIPHERAL_CUSTOM
};

GBL_DECLARE_ENUM(EVMU_PERIPHERAL_TYPE) {
    EVMU_PERIPHERAL_HW_BUILTIN,
    EVMU_PERIPHERAL_SW_BUILTIN,
    EVMU_PERIPHERAL_HW_DYNAMIC,
    EVMU_PERIPHERAL_SW_DYNAMIC
};

GBL_DECLARE_ENUM(EVMU_PERIPHERAL_MODE) {
    EVMU_PERIPHERAL_MODE_DISABLED,      // Do not process peripheral logic
    EVMU_PERIPHERAL_MODE_EMULATION,     // Driven by normal CPU emulator execution
    EVMU_PERIPHERAL_MODE_DIRECT         // Driven manually via direct API (ignores emulator)
};

typedef struct EvmuPeripheralProperty {
    EvmuEnum                    propertyId;
    const char*                 pName;
    const char*                 pDescription; //ifdef me out!
    GBL_VARIANT_TYPE            valueType;
    //valid values!           //ifdef me out!
    EvmuSize                     size;
} EvmuPeripheralProperty;

typedef struct EvmuPeripheralDriver {
    EVMU_PERIPHERAL_TYPE                type;
    const char*                         pName;
    const char*                         pDescription;
    EvmuSize                            instanceSize;

    struct {
        EvmuPeripheralInitFn            pFnInit;
        EvmuPeripheralDeinitFn          pFnDeinit;
        EvmuPeripheralResetFn           pFnReset;
        EvmuPeripheralUpdateFn          pFnUpdate;
        EvmuPeripheralEventFn           pFnEvent;
        EvmuPeripheralMemoryReadFn      pFnMemoryRead;
        EvmuPeripheralMemoryWriteFn     pFnMemoryWrite;
//        EvmuPeripheralPropertyFn        pFnProperty;
 //       EvmuPeripheralPropertySetFn     pFnPropertySet;
        EvmuPeripheralStateSaveFn       pFnStateSave;
        EvmuPeripheralStateLoadFn       pFnStateLoad;
        EvmuPeripheralParseCmdLineArgs  pFnParseCmdLineArgs;
        EvmuPeripheralDebugCommandFn    pFnDebugCmd;
    }                                   dispatchTable;

    const EvmuPeripheralProperty**            ppProperties;
} EvmuPeripheralDriver;

GBL_DECLARE_ENUM(EVMU_PERIPHERAL_PROPERTY) {
    EVMU_PERIPHERAL_PROPERTY_ID,
    EVMU_PERIPHERAL_PROPERTY_NAME,
    EVMU_PERIPHERAL_PROPERTY_DESCRIPTION,
    EVMU_PERIPHERAL_PROPERTY_MODE,
    EVMU_PERIPHERAL_PROPERTY_LOG_LEVEL,
    EVMU_PERIPHERAL_PROPERTY_BASE_COUNT
};
#if 0
EVMU_API evmuPeripheralReset(EvmuPeripheral hPeripheral);
EVMU_API evmuPeripheralUpdate(EvmuPeripheral hPeripheral, EvmuTicks ticks);

//EVMU_API evmuPeripheralProperty(EvmuPeripheral hPeripheral, EvmuEnum propertyId, EvmuPeripheralProperty* pProperty, EvmuSize* pSize);
EVMU_API evmuPeripheralPropertyValue(EvmuPeripheral hPeripheral, EvmuEnum propertyId, void* pData, EvmuSize* pSize);
EVMU_API evmuPeripheralPropertyValueSet(EvmuPeripheral hPeripheral, EvmuEnum propertyId, const void* pData, EvmuSize* pSize);
#endif


EVMU_API evmuPeripheralDevice(EvmuPeripheral hPeripheral, EvmuDevice* ppDevice);
EVMU_API evmuPeripheralDriver(EvmuPeripheral hPeripheral, const EvmuPeripheralDriver** ppDriver);
EVMU_API evmuPeripheralUserdata(EvmuPeripheral hPeripheral, void** ppUserdata);



//PRIVATE API
// init, deinit



#ifdef __cplusplus
}
#endif

#endif // EVMU_PERIPHERAL_H

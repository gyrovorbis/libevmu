#ifndef EVMU_TYPES_H
#define EVMU_TYPES_H

#include <gimbal/types/gimbal_typedefs.h>
#include "../evmu_api.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t                    EvmuTicks;
typedef uint64_t                    EvmuCycles;
typedef uint32_t                    EvmuAddress;
typedef uint8_t                     EvmuWord;

GBL_FORWARD_DECLARE_ENUM(EVMU_LOGIC);
GBL_FORWARD_DECLARE_ENUM(EvmuWave);
GBL_FORWARD_DECLARE_STRUCT(EvmuMemoryEvent);
GBL_FORWARD_DECLARE_STRUCT(EvmuEntity);
GBL_FORWARD_DECLARE_STRUCT(EvmuSimulation);
GBL_FORWARD_DECLARE_STRUCT(EvmuDevice);
GBL_FORWARD_DECLARE_STRUCT(EvmuPeripheral);
GBL_FORWARD_DECLARE_STRUCT(EvmuMemory);
GBL_FORWARD_DECLARE_STRUCT(EvmuCpu);
GBL_FORWARD_DECLARE_STRUCT(EvmuClock);
GBL_FORWARD_DECLARE_STRUCT(EvmuPic);
GBL_FORWARD_DECLARE_STRUCT(EvmuFlash);
GBL_FORWARD_DECLARE_STRUCT(EvmuLcd);


#define EVMU_META_RESULT_TABLE (                                                                                                                            \
        ( EVMU_RESULT, Result, "C API Return Status Code", evmuResultString),                                                                               \
        (                                                                                                                                                   \
            (EVMU_RESULT_ERROR_BEGIN,                    GBL_RESULT_COUNT           + 1,        ErrorBegin,                     "First EVMU Error"),        \
            (EVMU_RESULT_ERROR_INVALID_ADDRESS,          EVMU_RESULT_ERROR_BEGIN    + 1,        ErrorInvalidAddress,            "Invalid Address"),         \
            (EVMU_RESULT_ERROR_INVALID_CONTEXT,          EVMU_RESULT_ERROR_BEGIN    + 2,        ErrorInvalidContext,            "Invalid Context"),         \
            (EVMU_RESULT_ERROR_INVALID_DEVICE,           EVMU_RESULT_ERROR_BEGIN    + 3,        ErrorInvalidDevice,             "Invalid Device"),          \
            (EVMU_RESULT_ERROR_INVALID_PERIPHERAL,       EVMU_RESULT_ERROR_BEGIN    + 4,        ErrorInvalidPeripheral,         "Invalid Peripheral"),      \
            (EVMU_RESULT_ERROR_INVALID_PROPERTY,         EVMU_RESULT_ERROR_BEGIN    + 5,        ErrorInvalidProperty,           "Invalid Property")         \
        )                                                                                                                                                   \
    )

GBL_ENUM_TABLE_DECLARE(EVMU_META_RESULT_TABLE)

#if 0
/*Events are used for ONE-WAY communication from
  libGyro/EVMU API to client. If the client needs to RETURN something back to us
  (like malloc/realloc), use callback functions. These are one-way *notifiers*,
    Although this means they will work for telling the client to do something without
    needing confirmation (writing to a log).
*/
GBL_DECLARE_ENUM(EVMU_EVENT_TYPE) {
    EVMU_EVENT_DEVICE_CREATE,
    EVMU_EVENT_DEVICE_DESTROY,

    EVMU_EVENT_PERIPHERAL_ADDED,
    EVMU_EVENT_PERIPHERAL_REMOVED,

    EVMU_EVENT_DEVICE_RESET,
    EVMU_EVENT_DEVICE_UPDATE,
    
            
    EVMU_EVENT_CONTEXT_PRE_UPDATE,
    EVMU_EVENT_CONTEXT_POST_UPDATE,

    EVMU_EVENT_DEVICE_CONNECT,

    EVMU_DEVICE_EVENT_PSW_CHANGED,
    EVMU_DEVICE_EVENT_MEM_READ,
    EVMU_DEVICE_EVENT_MEM_WRITE,
    EVMU_DEVICE_EVENT_FLASH_READ,
    EVMU_DEVICE_EVENT_FLASH_WRITE,
    EVMU_DEVICE_EVENT_PC_CHANGE,
    EVMU_DEVICE_EVENT_RESET,
    EVMU_DEVICE_RAM_BANK_CHANGE,
    EVMU_DEVICE_IMEM_CHANGE,
    EVMU_DEVICE_STACK_PUSH,
    EVMU_DEVICE_STACK_POP,
    EVMU_DEVICE_STACK_OVERFLOW,
    EVMU_DEVICE_EVENT_SERIAL_CONNECTED,
    EVMU_DEVICE_EVENT_XMEM_READ,
    EVMU_DEVICE_EVENT_XMEM_WRITE,
    EVMU_DEVICE_WRAM_READ,
    EVMU_DEVICE_WRAM_WRITE,
    EVMU_DEVICE_FLASH_FORMAT,

    EVMU_EVENT_DEVICE_OSC_CHANGE,

    EVMU_EVENT_DEVICE_RAM_BANK_CHANGE,
    EVMU_EVENT_DEVICE_XRAM_BANK_CHANGE,

    EVMU_EVENT_DEVICE_BATTERY_LOW_SET,
    EVMU_EVENT_DEVICE_BATTERY_LOW_UNSET,
    EVMU_EVENT_DEVICE_BATTERY_MONITOR_SET,
    EVMU_EVENT_DEVICE_BATTERY_MONITOR_UNSET,

    //buttons change
    //0.5s timer clock update
    //display updated
    //audio changed
    //BIOS mode changed
    //low battery detected
    //low battery monitor changed
    //audio on/off?
    //display on/off?
    //display update enabled/disabled
    //oscillator/clock changed?
    //interrupt raised
    //interrupt accepted
    // MAPLE cmd receive
    // MAPLE cmd sends
    //overflow, underflow?

    //ram bank swapped
    //bios vs game enabled
    //breakpoints

};
#endif


#if 0
typedef struct EvmuEventHandler {
    EvmuEventHandlerFn  pFnCallback;
    void*               pUserdata;
    EvmuEnum            monitoredEvents;
} EvmuEventHandler;


typedef struct EvmuEvent {
    EVMU_EVENT_TYPE eventType;
} EvmuEvent;

typedef enum EVMU_DEVICE_EVENT_STATUS {
    EVMU_DEVICE_EVENT_STATUS_PENDING,
    EVMU_DEVICE_EVENT_STATUS_ACCEPTED
} EVMU_DEVICE_EVENT_STATUS;


typedef uint64_t EVMUDeviceEventMask;

#endif


#ifdef __cplusplus
}
#endif

#endif // EVMU_TYPES_H

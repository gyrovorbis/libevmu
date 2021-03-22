#ifndef EVMU_TYPES_H
#define EVMU_TYPES_H

#include <gimbal/gimbal_types.h>

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVMU_FALSE  GBL_FALSE
#define EVMU_TRUE   GBL_TRUE

typedef uint64_t    EvmuTicks;
typedef uint64_t    EvmuCycles;
typedef uint32_t    EvmuAddress;
typedef uint8_t     EvmuWord;
typedef void*       EvmuFile;
typedef GblSize     EvmuSize;
typedef GblBool     EvmuBool;
typedef GblEnum     EvmuEnum;

GBL_DECLARE_HANDLE(EvmuContext);
GBL_DECLARE_HANDLE(EvmuDevice);
GBL_DECLARE_HANDLE(EvmuPeripheral);

GBL_DECLARE_ENUM(EVMU_RESULT) {
    EVMU_RESULT_UNKNOWN                     = 0x0,
    EVMU_RESULT_SUCCESS                     = 0x1,
    EVMU_RESULT_PARTIAL_SUCCESS             = 0x2,          // Some inner stuff failed, but outer is okay (getlasterror)
    EVMU_RESULT_NOT_READY                   = 0x3,
    EVMU_RESULT_NOT_FOUND                   = 0x4,          // Finding an item by name or index with NULL return
    EVMU_RESULT_TIMEOUT                     = 0x5,
    EVMU_RESULT_INCOMPLETE                  = 0x6,
    EVMU_RESULT_TRUNCATED                   = 0x7,          // Buffer not large enough
    EVMU_RESULT_UNIMPLEMENTED               = 0x8,          // Not implemented on this build yet (but planning)
    EVMU_RESULT_UNSUPPORTED                 = 0x9,          // Either cannot or will not support it intentionally
    EVMU_RESULT_SIZE_MISMATCH               = 0xa,
    EVMU_RESULT_UNKNOWN_CMD_LINE_ARG        = 0xb,
    EVMU_RESULT_ERROR                       = 0xbad00000,
    EVMU_RESULT_ERROR_VERSION_INCOMPATIBLE  = EVMU_RESULT_ERROR | 1,
    EVMU_RESULT_ERROR_INVALID_HANDLE        = EVMU_RESULT_ERROR | 2,
    EVMU_RESULT_ERROR_INVALID_ARG           = EVMU_RESULT_ERROR | 3,
    EVMU_RESULT_ERROR_INVALID_SIZE          = EVMU_RESULT_ERROR | 4,
    EVMU_RESULT_ERROR_MEM_MALLOC            = EVMU_RESULT_ERROR | 5,
    EVMU_RESULT_ERROR_MEM_REALLOC           = EVMU_RESULT_ERROR | 6,
    EVMU_RESULT_ERROR_MEM_FREE              = EVMU_RESULT_ERROR | 7,
    EVMU_RESULT_ERROR_FILE_OPEN             = EVMU_RESULT_ERROR | 8,
    EVMU_RESULT_ERROR_FILE_READ             = EVMU_RESULT_ERROR | 9,
    EVMU_RESULT_ERROR_FILE_WRITE            = EVMU_RESULT_ERROR | 10,
    EVMU_RESULT_ERROR_FILE_CLOSE            = EVMU_RESULT_ERROR | 11,
    EVMU_RESULT_ERROR_UNDERFLOW             = EVMU_RESULT_ERROR | 12, //integers, log depth, api cookie/context
    EVMU_RESULT_ERROR_OVERFLOW              = EVMU_RESULT_ERROR | 13,
    EVMU_RESULT_ERROR_VSNPRINTF             = EVMU_RESULT_ERROR | 14,

    EVMU_RESULT_ERROR_PROPERTY_UNKNOWN      = EVMU_RESULT_ERROR | 15,
    EVMU_RESULT_ERROR_INVALID_ADDRESS       = EVMU_RESULT_ERROR | 16,
    EVMU_RESULT_ERROR_UNSUPPORTED_COMMAND   = EVMU_RESULT_ERROR | 17,
    EVMU_RESULT_ERROR_INVALID_PERIPHERAL    = EVMU_RESULT_ERROR | 18,

    EVMU_RESULT_ERROR_INVALID_DEVICE        = EVMU_RESULT_ERROR | 19,



    EVMU_RESULT_ERROR_INTERNAL_ASSERT       = EVMU_RESULT_ERROR | 14
};

#define EVMU_API EVMU_RESULT GBL_EXPORT

/*Events are used for ONE-WAY communication from
  libGyro/EVMU API to client. If the client needs to RETURN something back to us
  (like malloc/realloc), use callback functions. These are one-way *notifiers*,
    Although this means they will work for telling the client to do something without
    needing confirmation (writing to a log).
*/
GBL_DECLARE_ENUM(EVMU_EVENT_TYPE) {
    //libGyro base shit
    EVMU_EVENT_MEM_MALLOC,
    EVMU_EVENT_MEM_REALLOC,
    EVMU_EVENT_MEM_FREE,

    EVMU_EVENT_LOG_DEBUG,
    EVMU_EVENT_LOG_VERBOSE,
    EVMU_EVENT_LOG_INFO,
    EVMU_EVENT_LOG_WARNING,
    EVMU_EVENT_LOG_ERROR,
    EVMU_EVENT_LOG_PUSH,
    EVMU_EVENT_LOG_POP,
    EVMU_EVENT_API_COOKIE_PUSH,
    EVMU_EVENT_API_COOKIE_POP,

    EVMU_EVENT_CONTEXT_CREATE,  //intercept constructor style and add shit?
    EVMU_EVENT_CONTEXT_DESTROY,

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


typedef EVMU_RESULT         (*EvmuEventHandlerFn)(void*, EvmuContext, EvmuDevice, EvmuPeripheral, EvmuEnum, const void*, EvmuSize);

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




#ifdef __cplusplus
}
#endif

#endif // EVMU_DEFINES_H

#ifndef EVMU_PERIPHERAL__H
#define EVMU_PERIPHERAL__H

#include <evmu/hw/evmu_peripheral.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EvmuPeripheral_ {
    GblHandle_                      gblBase;
    EvmuEventMask                   eventMask;
    const EvmuPeripheralDriver*     pDriver;
    EvmuEnum                        id;
    EVMU_PERIPHERAL_MODE            mode;
    GBL_LOG_LEVEL                   logLevel;
} EvmuPeripheral_;


GBL_INLINE const char* evmuPeripheralName_(EvmuPeripheral hPeripheral) {
    GBL_ASSERT(hPeripheral);
    GBL_ASSERT(hPeripheral->pDriver);
    return hPeripheral->pDriver->pName;

}

#define EVMU_PERIPHERAL_DISPATCH_(PERIPHERAL, NAME, ...) \
    return PERIPHERAL->pDriver->dispatchTable.pFn##NAME ?             \
        PERIPHERAL->pDriver->dispatchTable.pFn##NAME(__VA_ARGS__) :   \
        GBL_RESULT_SUCCESS

#define EVMU_PERIPHERAL_SIBLING_(PERIPHERAL, NAME) \
    ((const EvmuPeripheral_*)PERIPHERAL)->pDevice->p##NAME

inline EVMU_RESULT evmuPeripheralInit_(EvmuPeripheral hPeripheral) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, Init, hPeripheral);
}

inline EVMU_RESULT evmuPeripheralDeinit_(EvmuPeripheral hPeripheral) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, Deinit, hPeripheral);
}

inline EVMU_RESULT evmuPeripheralMemoryRead_(EvmuPeripheral hPeripheral, EvmuAddress address, EVMU_MEMORY_ACCESS_TYPE accessType, EvmuWord value) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, MemoryRead, hPeripheral, address, accessType, value);
}

inline EVMU_RESULT evmuPeripheralMemoryWrite_(EvmuPeripheral hPeripheral, EvmuAddress address, EVMU_MEMORY_ACCESS_TYPE accessType, EvmuWord value) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, MemoryWrite, hPeripheral, address, accessType, value);
}
#if 0
inline EVMU_RESULT evmuPeripheralEvent_(EvmuPeripheral hPeripheral, EVMU_EVENT_TYPE eventType, void* pData, EvmuSize size) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, Event, hPeripheral, eventType, pData, size);
}
#endif

inline EVMU_RESULT evmuPeripheralStateSave_(EvmuPeripheral hPeripheral, char* pData, GblSize* pBytes) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, StateSave, hPeripheral, pData, pBytes);
}

inline EVMU_RESULT evmuPeripheralStateLoad_(EvmuPeripheral hPeripheral, const char* pData, GblSize* pBytes) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, StateLoad, hPeripheral, pData, pBytes);
}

// Accept default command-line args to configure log verbosity and event tracking and shit
inline EVMU_RESULT evmuPeripheralParseCmdLineArgs_(EvmuPeripheral hPeripheral, int argc, const char** argv) {
    EVMU_PERIPHERAL_DISPATCH_(hPeripheral, ParseCmdLineArgs, hPeripheral, argc, argv);
}

#ifdef __cplusplus
}
#endif


#endif // EVMU_PERIPHERAL__H

#ifndef EVMU_CONTEXT_H
#define EVMU_CONTEXT_H

#include "../evmu_types.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <gimbal/gimbal_context.h>
#include <evmu/evmu_api.h>


#ifdef __cplusplus
extern "C" {
#endif

GBL_DECLARE_HANDLE(EvmuDevice);
GBL_DECLARE_HANDLE(EvmuPeripheral);
struct EvmuPeripheralDriver;

GBL_DECLARE_HANDLE(EvmuContext);

typedef EVMU_RESULT         (*EvmuFileOpenFn)(void*, const char*, const char*, EvmuFile*);
typedef EVMU_RESULT         (*EvmuFileCloseFn)(EvmuFile, void*);
typedef EVMU_RESULT         (*EvmuFileReadFn)(EvmuFile, void*, void*, EvmuSize*);
typedef EVMU_RESULT         (*EvmuFileWriteFn)(EvmuFile, void*, const void*, EvmuSize*);
typedef EVMU_RESULT         (*EvmuFileLengthFn)(EvmuFile, void*, EvmuSize*);


/* Simplify all of this shit ATM by not linking a context to its
   parent. Simply copy over all of its relevant shit. */
typedef struct EvmuContextCreateInfo {
    GblContextCreateInfo            baseInfo;

    EvmuEventHandler                eventHandler;

    struct {
        EvmuFileOpenFn              pOpenFn;
        EvmuFileCloseFn             pCloseFn;
        EvmuFileReadFn              pReadFn;
        EvmuFileWriteFn             pWriteFn;
        EvmuFileLengthFn            pLengthFn;
    }                               fileCallbacks;

} EvmuContextCreateInfo;

// THIS NEEDS TO BE CREATE/DESTROY SO THAT INTERNALS CAN BE HIDDEN!!!
EVMU_API    evmuContextInit(EvmuContext hCtx,
                            EvmuContextCreateInfo* pInfo);


EVMU_API    evmuContextDeinit(EvmuContext hCtx);

//===== implemented ====

EVMU_API    evmuContextUpdate(EvmuContext hCtx, EvmuTicks ticks);

EVMU_API    evmuContextDeviceCount(EvmuContext hCtx,
                                    uint32_t* pCount);

EVMU_API     evmuContextDevice(EvmuContext hCtx,
                               uint32_t index,
                               EvmuDevice* phDevice);
EVMU_API    evmuContextUserdata(EvmuContext hCtx,
                                void** ppUserdata);
typedef struct EvmuError {
   GblError    gblError;
   EvmuDevice* pDevice;
} EvmuError;
// == HAVENT FUCKED WITH ====

/*
 *

EVMU_API    evmuContextDeviceConnectDevice(EvmuContext hCtx,
                                           EvmuDevice hDev1,
                                           EvmuDevice hDev2);

EVMU_API    evmuContextDeviceDisconnect(EvmuContext* pCtx,
                                           EvmuDevice* pDev);

EVMU_API     evmuContextLastError(const EvmuContext hCtx,
                                  const EvmuError** ppError);

*/




/*
EVMU_API    evmuContextFileFormatRegister(EvmuContext* pCtx,
                                          const EvmuFileFormat* pFormat);
*/

#ifdef __cplusplus
}
#endif



#endif // Evmu_CONTEXT_H

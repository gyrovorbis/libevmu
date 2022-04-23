#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/core/gimbal_api_frame.h>

#ifdef __cplusplus
extern "C" {
#endif


#define EVMU_API GBL_EXPORT EVMU_RESULT

GBL_API evmuVersion         (GblVersion* pVersion, const char** ppString); //hard-compiled

#ifdef __cplusplus
}
#endif

#endif // EVMU_API_H

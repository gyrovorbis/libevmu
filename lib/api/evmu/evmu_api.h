#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/core/gimbal_api_frame.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVMU_EXPORT GBL_EXPORT
#define EVMU_INLINE GBL_INLINE

EVMU_EXPORT GBL_RESULT evmuVersion         (GblVersion* pVersion, const char** ppString); //hard-compiled

#ifdef __cplusplus
}
#endif

#endif // EVMU_API_H

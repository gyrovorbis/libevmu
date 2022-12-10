#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/core/gimbal_ctx.h>

#define EVMU_EXPORT         GBL_EXPORT
#define EVMU_INLINE         GBL_INLINE

#define EVMU_API_VERBOSE    GBL_CTX_VERBOSE
#define EVMU_API_DEBUG      GBL_CTX_DEBUG
#define EVMU_API_INFO       GBL_CTX_INFO
#define EVMU_API_WARN       GBL_CTX_WARN
#define EVMU_API_ERROR      GBL_CTX_ERROR

GBL_DECLS_BEGIN

GBL_DECLS_END

#endif // EVMU_API_H

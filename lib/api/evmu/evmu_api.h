#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/core/gimbal_ctx.h>
#include <gimbal/core/gimbal_logger.h>

#define EVMU_EXPORT         GBL_EXPORT
#define EVMU_INLINE         GBL_INLINE

#define EVMU_LOG_VERBOSE(...)   GBL_LOG_VERBOSE("evmu", __VA_ARGS__)
#define EVMU_LOG_DEBUG(...)     GBL_LOG_DEBUG("evmu", __VA_ARGS__)
#define EVMU_LOG_INFO(...)      GBL_LOG_INFO("evmu", __VA_ARGS__)
#define EVMU_LOG_WARN(...)      GBL_LOG_WARN("evmu", __VA_ARGS__)
#define EVMU_LOG_ERROR(...)     GBL_LOG_ERROR("evmu", __VA_ARGS__)
#define EVMU_LOG_PUSH()         GBL_LOG_PUSH()
#define EVMU_LOG_POP(n)         GBL_LOG_POP(n)

GBL_DECLS_BEGIN

GBL_DECLS_END

#endif // EVMU_API_H

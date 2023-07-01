/*! \file
 *  \brief Top-level defines for EVMU API macros
 *
 *  This file contains common, top-level macro declarations
 *  used throughout the codebase.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/core/gimbal_ctx.h>
#include <gimbal/core/gimbal_logger.h>

//! Define used for adding attributes to export public symbols
#define EVMU_EXPORT             GBL_EXPORT
//! Define used for inlining a funcion within a C header file
#define EVMU_INLINE             GBL_INLINE

/*! \name  Logging
 *  \brief Macros used for logging to the "evmu" domain
 *  @{
 */
//! Writes a message to the log with the \ref GBL_LOG_VERBOSE flag within the "evmu" domain.
#define EVMU_LOG_VERBOSE(...)   GBL_LOG_VERBOSE("evmu", __VA_ARGS__)
//! Writes a message to the log with the \ref GBL_LOG_DEBUG flag within the "evmu" domain.
#define EVMU_LOG_DEBUG(...)     GBL_LOG_DEBUG("evmu", __VA_ARGS__)
//! Writes a message to the log with the \ref GBL_LOG_INFO flag within the "evmu" domain.
#define EVMU_LOG_INFO(...)      GBL_LOG_INFO("evmu", __VA_ARGS__)
//! Writes a message to the log with the \ref GBL_LOG_WARN flag within the "evmu" domain.
#define EVMU_LOG_WARN(...)      GBL_LOG_WARN("evmu", __VA_ARGS__)
//! Writes a message to the log with the \ref GBL_LOG_ERROR flag within the "evmu" domain.
#define EVMU_LOG_ERROR(...)     GBL_LOG_ERROR("evmu", __VA_ARGS__)
//! Pushes a level to the log stack within the "evmu" domain. See \ref GBL_LOG_PUSH().
#define EVMU_LOG_PUSH()         GBL_LOG_PUSH()
//! Pops n levels from the log stack within the "evmu" domain. See \ref GBL_LOG_POP().
#define EVMU_LOG_POP(n)         GBL_LOG_POP(n)
//! @}

GBL_DECLS_BEGIN

GBL_DECLS_END

#endif // EVMU_API_H

#ifndef EVMU_TYPES_H
#define EVMU_TYPES_H

#include <gimbal/core/gimbal_typedefs.h>
#include "../evmu_api.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

GBL_DECLS_BEGIN

typedef uint64_t    EvmuTicks;
typedef uint64_t    EvmuCycles;
typedef uint32_t    EvmuAddress;
typedef uint8_t     EvmuWord;

#define EVMU_META_RESULT_TABLE (                                                                                                                            \
    ( EVMU_RESULT, Result, "C API Return Status Code", evmuResultString),                                                                               \
    (                                                                                                                                                   \
        (EVMU_RESULT_ERROR_BEGIN,                    GBL_RESULT_COUNT           + 1,        ErrorBegin,                     "First EVMU Error"),        \
        (EVMU_RESULT_ERROR_INVALID_ADDRESS,          EVMU_RESULT_ERROR_BEGIN    + 1,        ErrorInvalidAddress,            "Invalid Address"),         \
        (EVMU_RESULT_ERROR_INVALID_CONTEXT,          EVMU_RESULT_ERROR_BEGIN    + 2,        ErrorInvalidContext,            "Invalid Context"),         \
        (EVMU_RESULT_ERROR_INVALID_DEVICE,           EVMU_RESULT_ERROR_BEGIN    + 3,        ErrorInvalidDevice,             "Invalid Device"),          \
        (EVMU_RESULT_ERROR_INVALID_PERIPHERAL,       EVMU_RESULT_ERROR_BEGIN    + 4,        ErrorInvalidPeripheral,         "Invalid Peripheral"),      \
        (EVMU_RESULT_ERROR_INVALID_PROPERTY,         EVMU_RESULT_ERROR_BEGIN    + 5,        ErrorInvalidProperty,           "Invalid Property"),        \
        (EVMU_RESULT_ERROR_STACK_UNDERFLOW,          EVMU_RESULT_ERROR_BEGIN    + 6,        ErrorStackUnderflow,            "Stack Underflow"),         \
        (EVMU_RESULT_ERROR_STACK_OVERFLOW,           EVMU_RESULT_ERROR_BEGIN    + 7,        ErrorStackOverflow,             "Stack Overflow")           \
    )                                                                                                                                                           \
)

GBL_ENUM_TABLE_DECLARE(EVMU_META_RESULT_TABLE)

GBL_DECLS_END

#endif // EVMU_TYPES_H

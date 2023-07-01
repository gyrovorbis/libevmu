/*! \file
 *  \brief Declares common typedefs used throughout the codebase
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_TYPEDEFS_H
#define EVMU_TYPEDEFS_H

#include <gimbal/core/gimbal_typedefs.h>
#include "../evmu_api.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

GBL_DECLS_BEGIN

typedef uint64_t EvmuTicks;     //!< Represents a delta time in milliseconds
typedef uint64_t EvmuCycles;    //!< Represent a delta time in cycles
typedef uint32_t EvmuAddress;   //!< Represents a generic absolute address
typedef uint8_t  EvmuWord;      //!< Represents a single 8-bit CPU word

#define EVMU_META_RESULT_TABLE (                                                                                                                        \
    ( EVMU_RESULT, Result, "C API Return Status Code", evmuResultString),                                                                               \
    (                                                                                                                                                   \
        (EVMU_RESULT_ERROR_BEGIN,                    GBL_RESULT_COUNT           + 1,        ErrorBegin,                     "First EVMU Error"),        \
        (EVMU_RESULT_ERROR_INVALID_ADDRESS,          EVMU_RESULT_ERROR_BEGIN    + 1,        ErrorInvalidAddress,            "Invalid Address"),         \
        (EVMU_RESULT_ERROR_INVALID_BLOCK,            EVMU_RESULT_ERROR_BEGIN    + 2,        ErrorInvalidBlock,              "Invalid Block"),           \
        (EVMU_RESULT_ERROR_INVALID_DIR_ENTRY,        EVMU_RESULT_ERROR_BEGIN    + 3,        ErrorInvalidDirEntry,           "Invalid Directory Entry"), \
        (EVMU_RESULT_ERROR_INVALID_FILE,             EVMU_RESULT_ERROR_BEGIN    + 4,        ErrorInvalidFile,               "Invalid File"),            \
        (EVMU_RESULT_ERROR_INVALID_FILE_SYSTEM,      EVMU_RESULT_ERROR_BEGIN    + 5,        ErrorinvalidFileSystem,         "Invalid File System"),     \
        (EVMU_RESULT_ERROR_INVALID_CONTEXT,          EVMU_RESULT_ERROR_BEGIN    + 6,        ErrorInvalidContext,            "Invalid Context"),         \
        (EVMU_RESULT_ERROR_INVALID_DEVICE,           EVMU_RESULT_ERROR_BEGIN    + 7,        ErrorInvalidDevice,             "Invalid Device"),          \
        (EVMU_RESULT_ERROR_INVALID_PERIPHERAL,       EVMU_RESULT_ERROR_BEGIN    + 8,        ErrorInvalidPeripheral,         "Invalid Peripheral"),      \
        (EVMU_RESULT_ERROR_STACK_UNDERFLOW,          EVMU_RESULT_ERROR_BEGIN    + 9,        ErrorStackUnderflow,            "Stack Underflow"),         \
        (EVMU_RESULT_ERROR_STACK_OVERFLOW,           EVMU_RESULT_ERROR_BEGIN    + 10,       ErrorStackOverflow,             "Stack Overflow"),          \
        (EVMU_RESULT_ERROR_FLASH_LOCKED,             EVMU_RESULT_ERROR_BEGIN    + 11,       ErrorFlashLocked,               "Flash Locked"),            \
        (EVMU_RESULT_ERROR_UNFORMATTED,              EVMU_RESULT_ERROR_BEGIN    + 12,       ErrorUnformatted,               "Unformatted")              \
    )                                                                                                                                                   \
)

/*! \enum EVMU_RESULT
 *  \brief extended GBL_RESULT type representing any status for the codebase
 */
GBL_ENUM_TABLE_DECLARE(EVMU_META_RESULT_TABLE)

GBL_DECLS_END

#endif // EVMU_TYPEDEFS_H

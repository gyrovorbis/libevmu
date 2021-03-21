#ifndef EVMU_UTILS_H
#define EVMU_UTILS_H

#include <stdlib.h>
#include "evmu_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

inline static int evmuToBCD(int n) {
    return ((n/10)<<4)|(n%10);
}

inline static int evmuFromBCD(int n) {
    return (unsigned)(((n>>4)*10)+(n&0xf));

}

EVMU_API int evmuWeekDay(void);

EVMU_API const char* evmuWeekDayStr(int day);


inline static void evmuResultString(EVMU_RESULT result, char* pBuff, size_t size) {
#define CASE_STRINGIFY(c)   \
    pStringName = #c;       \
    break

    EVMU_ASSERT(pBuff && size, "Invalid output buffer!");
    const char* pStringName = NULL;

    switch(result) {
        CASE_STRINGIFY(EVMU_RESULT_SUCCESS);
        CASE_STRINGIFY(EVMU_RESULT_ERROR);
        CASE_STRINGIFY(EVMU_RESULT_UNKNOWN);
        default: "Invalid";
    }

    snprintf(pBuff, size, "%s [Code: %x]", pStringName, result);

#undef CASE_STRINGIFY
}

#ifdef __cplusplus
}
#endif

#endif // EVMU_UTILS_H


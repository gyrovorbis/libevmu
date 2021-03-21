#ifndef EVMU_VERSION_H
#define EVMU_VERSION_H

#include "evmu_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// Add build + general compiler information to shared C shit
// Add headers/copyrights to all files

#ifndef EVMU_VERSION_MAJOR
#   define EVMU_VERSION_MAJOR  0
#endif

#ifndef EVMU_VERSION_MINOR
#   define EVMU_VERSION_MINOR  0
#endif

#ifndef EVMU_VERSION_PATCH
#   define EVMU_VERSION_PATCH  0
#endif

#ifndef EVMU_COPYRIGHT_YEAR
#   define EVMU_COPYRIGHT_YEAR "2021"
#endif

#define EVMU_VERSION_STRING EVMU_STRINGIFY_VERSION(EVMU_VERSION_MAJOR, EVMU_VERSION_MINOR, EVMU_VERSION_PATCH)

/* Use EVMU_VERSION_STRING for compile-time version.
 * Use evmuVersionString() for run-time version.
 */
EVMU_API const char* evmuVersionString(void);

#ifdef __cplusplus
}
#endif

#endif // EVMU_VERSION_H


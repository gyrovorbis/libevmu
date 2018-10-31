#ifndef GYRO_VMU_VERSION_H
#define GYRO_VMU_VERSION_H

#include <libGyro/gyro_system_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GYRO_VMU_VERSION_MAJOR  0
#define GYRO_VMU_VERSION_MINOR  2
#define GYRO_VMU_VERSION_PATCH  0

#define GYRO_VMU_VERSION_STRING GYRO_STRINGIFY_VERSION(GYRO_VMU_VERSION_MAJOR, GYRO_VMU_VERSION_MINOR, GYRO_VMU_VERSION_PATCH)

/* Use GYRO_VMU_VERSION_STRING for compile-time version.
 * Use gyVmuVersion() for run-time version.
 */
const char* gyVmuVersion(void);


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_VERSION_H


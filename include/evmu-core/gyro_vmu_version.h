#ifndef GYRO_VMU_VERSION_H
#define GYRO_VMU_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define GYRO_VMU_VERSION_MAJOR  0
#define GYRO_VMU_VERSION_MINOR  3
#define GYRO_VMU_VERSION_PATCH  1

#define GYRO_VMU_VERSION_STRING "0.3.1"

/* Use GYRO_VMU_VERSION_STRING for compile-time version.
 * Use gyVmuVersion() for run-time version.
 */
const char* gyVmuVersion(void);


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_VERSION_H


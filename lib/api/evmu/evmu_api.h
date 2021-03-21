#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/gimbal_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVMU_API_BEGIN()

// can compile-time override to not be dynamic!
#define EVMU_API_FILE_OPEN()
#define EVMU_API_FILE_CLOSE()
#define EVMU_API_FILE_READ()
#define EVMU_API_FILE_WRITE()
#define EVMU_API_FILE_LENGTH()



#ifdef __cplusplus
}
#endif

#endif // EVMU_API_H

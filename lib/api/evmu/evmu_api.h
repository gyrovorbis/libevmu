#ifndef EVMU_API_H
#define EVMU_API_H

#include <gimbal/gimbal_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVMU_API_BEGIN(S)
#define EVMU_API_END(S) return 0
#define EVMU_API_DONE() //goto end

#define EVMU_API_RETURN_CND(C) if(!C) return 0

#define EVMU_API_RESULT_ACCUM(R)
#define EVMU_API_RESULT_SET(...)


#define EVMU_API_VERIFY(S)
#define EVMU_API_VERIFY_POINTER(S)
#define EVMU_API_VERIFY_HANDLE(H)

// can compile-time override to not be dynamic!
#define EVMU_API_FILE_OPEN(A)
#define EVMU_API_FILE_CLOSE(A)
#define EVMU_API_FILE_READ(A, B, c)
#define EVMU_API_FILE_WRITE(A, B, C)
#define EVMU_API_FILE_LENGTH(A)

#define EVMU_API_PUSH(...)
#define EVMU_API_POP(...)
#define EVMU_API_VERBOSE(...)
#define EVMU_API_ERROR(...)
#define EVMU_API_WARN(...)

#define EVMU_API_MALLOC(...) NULL
#define EVMU_API_FREE(...)




#ifdef __cplusplus
}
#endif

#endif // EVMU_API_H

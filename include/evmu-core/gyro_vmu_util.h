#ifndef GYRO_VMU_UTIL_H
#define GYRO_VMU_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

inline static int gyVmuToBCD(int n) {
    return ((n/10)<<4)|(n%10);
}

inline static int gyVmuFromBCD(int n) {
    return (unsigned)(((n>>4)*10)+(n&0xf));

}

int gyVmuWeekDay(void);

const char* gyVmuWeekDayStr(int day);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_UTIL_H


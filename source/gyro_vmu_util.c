#include "gyro_vmu_util.h"
#include <gyro_timer_api.h>
#include <time.h>

int gyVmuWeekDay(void) {
    char str[20];
    struct tm tm;
    if (gyTimerStrpTime(str, "%d-%m-%Y", &tm) != NULL) {
        time_t t = mktime(&tm);
        return localtime(&t)->tm_wday; // Sunday=0, Monday=1, etc.
    }
    return -1;
}

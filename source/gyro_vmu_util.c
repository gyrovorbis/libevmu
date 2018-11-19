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


const char* gyVmuWeekDayStr(int day) {
    switch(day) {
    case 0:     return "Sunday";
    case 1:     return "Monday";
    case 2:     return "Tuesday";
    case 3:     return "Wednesday";
    case 4:     return "Thursday";
    case 5:     return "Friday";
    case 6:     return "Saturday";
    default:    return "Invalid Weekday";
    }

}

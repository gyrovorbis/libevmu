#ifndef GYRO_VMU_TIMERS_H
#define GYRO_VMU_TIMERS_H

#ifdef __cplusplus
extern "C" {
#endif

struct VMUDevice;

typedef enum VMU_TIMER0_MODE {
    VMU_TIMER0_MODE_TIMER8_TIMER8,
    VMU_TIMER0_MODE_TIMER8_COUNTER8,
    VMU_TIMER0_MODE_TIMER16,
    VMU_TIMER0_MODE_COUNTER16
} VMU_TIMER0_MODE;

typedef enum VMU_TIMER1_MODE {
    VMU_TIMER1_MODE_TIMER8_TIMER8,
    VMU_TIMER1_MODE_TIMER8_PULSE8,
    VMU_TIMER1_MODE_TIMER16,
    VMU_TIMER1_MODE_PULSEVAR
} VMU_TIMER1_MODE;

typedef struct VMUTimer {
    int         tl;
    int         th;
} VMUTimer;

typedef struct VMUTimer0 {
    VMUTimer    _base;
    int         tbase;
    int         tscale;
} VMUTimer0;

typedef struct VMUTimer1 {
    VMUTimer    _base;
} VMUTimer1;


VMU_TIMER1_MODE gyVmuTimer1ModeGet(const struct VMUDevice* dev);
int gyVmuTimersUpdate(struct VMUDevice* dev);
int gyVmuTimerBaseUpdate(struct VMUDevice* dev);
int gyVmuTimer0Update(struct VMUDevice* dev);
int gyVmuTimer1Update(struct VMUDevice* dev);

#ifdef __cplusplus
}
#endif


#endif // GYRO_VMU_TIMERS_H


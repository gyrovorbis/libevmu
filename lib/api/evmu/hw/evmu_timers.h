#ifndef GYRO_VMU_TIMERS_H
#define GYRO_VMU_TIMERS_H

#include "../hw/evmu_peripheral.h"

#ifdef __cplusplus
extern "C" {
#endif

//BASE TIMER DOES A LOT MORE THAN THE SHIT THE BIOS HARDCODES IT TO DO!!111

GBL_DEFINE_HANDLE(EvmuTimerBase)
GBL_DEFINE_HANDLE(EvmuTimer)
GBL_DEFINE_HANDLE(EvmuTimer0)
GBL_DEFINE_HANDLE(EvmuTimer1)

GBL_DECLARE_ENUM(EVMU_TIMER_PROPERTY) {
    EVMU_TIMER_PROPERTY_MODE = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_TIMER_PROPERTY_TL,
    EVMU_TIMER_PROPERTY_TH,
    EVMU_TIMER1_PROPERTY_COUNT
    EVMU_TIMER0_PROPERTY_TBASE = EVMU_TIMER1_PROPERTY_COUNT,
    EVMU_TIMER0_PROPERTY_TSCALE,
    //Rising edge, falling edge, interrupt enabled, interrupt source
    //Input, output ports, signal generation, clock dividers, clock sources
    EVMU_TIMER0_PROPERTY_COUNT
};

typedef enum EVMU_TIMER0_MODE {
    EVMU_TIMER0_MODE_TIMER8_TIMER8,
    EVMU_TIMER0_MODE_TIMER8_COUNTER8,
    EVMU_TIMER0_MODE_TIMER16,
    EVMU_TIMER0_MODE_COUNTER16
} EVMU_TIMER0_MODE;

typedef enum EVMU_TIMER1_MODE {
    EVMU_TIMER1_MODE_TIMER8_TIMER8,
    EVMU_TIMER1_MODE_TIMER8_PULSE8,
    EVMU_TIMER1_MODE_TIMER16,
    EVMU_TIMER1_MODE_PULSEVAR
} EVMU_TIMER1_MODE;


// High-level API to make configuring easier
// configure upcounter/downcounter for seconds/whatever
// Configure driving signal output, falling/rising edge


EVMU_API evmuTimerStart(EvmuTimer hTimer, GblEnum mode, EvmuTicks ticks);
EVMU_API evmuTimerStop(EvmuTimer hTimer);


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


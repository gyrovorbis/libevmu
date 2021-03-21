#ifndef GYRO_SIGNAL_H
#define GYRO_SIGNAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


GBL_DECLARE_ENUM(GY_LOGIC_STRUCT_TYPE) {
    GY_LOGIC_STRUCT_INVALID,
    GY_LOGIC_STRUCT_SIGNAL
    GY_LOGIC_STRUCT_STATELESS_SIGNAL,
    GY_LOGIC_STRUCT_REGISTER,
    GY_LOGIC_STRUCT_SYNCHRONOUS_ELEMENT,
    GY_LOGIC_STRUCT_LATCH,
    GY_LOGIC_STRUCT_WIRE,
    GY_LOGIC_STRUCT_INPUT_PIN,
    GY_LOGIC_STRUCT_OUTPUT_PIN,
    GY_LOGIC_STRUCT_INPUT_OUTPUT_PIN,
    GY_LOGIC_STRUCT_TYPE_UNKNOWN
} GY_LOGIC_STRUCT_TYPE;


GBL_DECLARE_STRUCT(GYLogicVTableBase) {
    GY_LOGIC_STRUCT_TYPE type;
} GYLogicVtable;

typedef union GYLogicVptr {
    GYLogicVTableBase*  pBase;
    void*               pVoid;
} GYLogicVptr;


#define GY_LOGIC_STRUCT_VTABLE(Vtable) \
    union {

    } vptr;
}

#define GY_STRUCT_INHERIT(S) \
    S base##S_

#define GY_STRUCT_BASE(s) \
    (s).base##S_


#ifndef GY_LOGIC_HIGH
#   define GY_LOGIC_HIGH          1
#endif

#ifndef GY_LOGIC_LOW
#   define GY_LOGIC_LOW           0
#endif



// Lowest-level boolean logic values
GBL_DECLARE_ENUM(GY_LOGIC_VALUE) {
    GY_LOGIC_0      = GY_LOGIC_LOW,
    GY_LOGIC_1      = GY_LOGIC_HIGH,
    GY_LOGIC_Z      = 0x2,
    GY_LOGIC_X      = 0x3,
    GY_LOGIC_COUNT  = GY_LOGIC_Z + 1
};




typedef GY_LOGIC_VALUE GYLogicValue;

GY_LOGIC_API gyLogicValueValid(const GYLogicValue* pValue, GblBool* pValid) {
    *pValid = (*pValue == GY_LOGIC_0 || *pValue == GY_LOGIC_1);
}

GY_LOGIC_API gyLogicValuesCombine(GYLogicValue* pOutput, const GYLogicValue* pInput1, const GYLogicValue* pInput2) {
    const static GY_LOGIC_VALUE combin
}


#define DECLARE_SUBWAVE_ENUMS(L)    \
    GY_WAVE_##L##_0,                \
    GY_WAVE_##L##_1,                \
    GY_WAVE_##L##_Z,                \
    GY_WAVE_##L##_X

GBL_DECLARE_ENUM(GY_WAVE) {
    DECLARE_SUBWAVE_ENUMS(0),
    DECLARE_SUBWAVE_ENUMS(1),
    DECLARE_SUBWAVE_ENUMS(Z),
    DECLARE_WUBWAVE_ENUMS(X),
    GY_WAVE_COUNT
};

typedef GY_WAVE GYWave;

GY_LOGIC_API gyWaveLogicSet(GYWave* pWave, GY_LOGIC_VALUE curLogicValue, GY_LOGIC_VALUE prevLogicValue);
GY_LOGIC_API gyWaveLogicGet(const GYWave* pWave, GY_LOGIC_VALUE* pCurLogicValue, GY_LOGIC_VALUE* pPrevLogicValue);
GY_LOGIC_API gyWaveLogicUpdate(GYWave* pWave, GY_LOGIC_VALUE newValue);



#undef DECLARE_SUBWAVE_ENUMS

typedef GY_WAVE (GYSignalWaveGetter*)(const GYSignal*);

GBL_DECLARE_STRUCT(GYSignal) {
    struct {
        WaveGetterFn waveGetterFn;
    } vtable;
};


GY_LOGIC_API gySignalInit(GYSignal* pSignal, const GYSignalWaveGetter* pGetterFn);
GY_LOGIC_API gySignalWave(const GYSignal* pSignal, GYWave* pWave);
GY_LOGIC_API gySignalDeinit(GYSignal* pSignal);


GBL_DECLARE_STRUCT(GYStatelessSignal) {
    GY_LOGIC_STRUCT_INHERIT(GYSignal);

    GY_LOGIC_VALUE cachedLogic;
    GY_LOGIC_VALUE (GYSignalLogicGetter*)(const GYSignal*) signalGetterFn;
}

GY_LOGIC_API gyStatelessSignalInit(GYStatelessSignal* pSignal, const GYSignalLogicGetter* pGetterFn);



GBL_DECLARE_STRUCT(GYRegister) {
    GY_LOGIC_STRUCT_INHERIT(GYStatelessSignal);
    GY_LOGIC_VALUE currentLogic; //actual storage for actual current logic levels
};


GY_LOGIC_API gyRegisterInit(GYRegister* pRegister, GYLogicValue value);
GY_LOGIC_API gyRegisterLogicValueSet(GYRegister* pRegister, GYLogicValue value);


GBL_DECLARE_STRUCT(GYSynchronousElement) {
    struct {
        typedef void (ClkUpdateFn*)(GYSynchronousBlock*) onClkUpdate;
    } vtable;
};

GY_LOGIC_API gySynchronousElementInit(GYSynchronousElement* pElement, const ClkUpdateFn* pClkUpdateFn);
GY_LOGIC_API gySynchronousElementDeinit(GYSynchronousElement* pElement);


GBL_DECLARE_STRUCT(GYLatch) {
    GY_LOGIC_INHERIT(GYSignal);
    GY_LOGIC_INHERIT(GYSynchronousElement);
    SIGNAL_STATE latched;
};





// Waveform state, contains current and previous logic values
typedef struct GYLogicWave {
    struct {
        uint8_t previous    : 2;
        uint8_t current     : 2;
    } values;
} GYLogicWave;

typedef struct GYLogicSignal {


} GYLogicSignal;






typedef enum EVMU_PIN_SIGNAL_STATE {
    EVMU_PIN_SIGNAL_STATE_LEVEL_LOW     = 0x0
    GY_LOGIC cachedLogic;
    EVMU_PIN_SIGNAL_STATE_RISING_EDGE   = 0x01,
    EVMU_PIN_SIGNAL_STATE_LEVEL_HIGH    = 0x11,
    EVMU_PIN_SIGNAL_STATE_FALLING_EDGE  = 0x10
} EVMU_PIN_SIGNAL_STATE;


//Level-based interrupt detection generates an interrupt continuously
typedef enum EVMU_SIGNAL_DETECT_CONDITION {
    EVMU_SIGNAL_DETECT_FALLING_EDGE     = 0x00,
    EVMU_SIGNAL_DETECT_LOW_LEVEL        = 0x01,
    EVMU_SIGNAL_DETECT_RISING_EDGE      = 0x10,
    EVMU_SIGNAL_DETECT_HIGH_LEVEL       = 0x11,
    EVMU_SIGNAL_DETECT_CONDITION_COUNT
} EVMU_SIGNAL_DETECT_CONDITION;


#ifdef __cplusplus
}
#endif


#endif // GYRO_SIGNAL_H

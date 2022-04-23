#ifndef EVMU_WAVE_H
#define EVMU_WAVE_H

#include "../types/evmu_typedefs.h"

#define EVMU_LOGIC_HIGH                 1
#define EVMU_LOGIC_LOW                  0

#define EVMU_WAVE_LOGIC_BITS            2
#define EVMU_WAVE_LOGIC_CURRENT_MASK    (0x3)
#define EVMU_WAVE_LOGIC_PREVIOUS_MASK   (0x70)


#define SELF    EvmuWave* pSelf
#define CSELF   const SELF


GBL_DECLS_BEGIN

// Lowest-level boolean logic values
GBL_DECLARE_ENUM(EVMU_LOGIC) {
    EVMU_LOGIC_0      = EVMU_LOGIC_LOW,
    EVMU_LOGIC_1      = EVMU_LOGIC_HIGH,
    EVMU_LOGIC_Z      = 0x2,
    EVMU_LOGIC_X      = 0x3,
    GY_LOGIC_COUNT  = EVMU_LOGIC_Z + 1
};

#define EVMU_DECLARE_SUBWAVE_ENUMS_(L) \
    EVMU_WAVE_##L##_0,                 \
    EVMU_WAVE_##L##_1,                 \
    EVMU_WAVE_##L##_Z,                 \
    EVMU_WAVE_##L##_X

GBL_DECLARE_ENUM(EvmuWave) {
    EVMU_DECLARE_SUBWAVE_ENUMS_(0),
    EVMU_DECLARE_SUBWAVE_ENUMS_(1),
    EVMU_DECLARE_SUBWAVE_ENUMS_(Z),
    EVMU_DECLARE_SUBWAVE_ENUMS_(X),
    EVMU_WAVE_COUNT
};

GBL_INLINE void        EvmuWave_reset                   (SELF)                                              GBL_NOEXCEPT;
GBL_INLINE void        EvmuWave_fill                    (SELF, EVMU_LOGIC values)                           GBL_NOEXCEPT;
GBL_INLINE void        EvmuWave_set                     (SELF, EVMU_LOGIC prevValue, EVMU_LOGIC curValue)   GBL_NOEXCEPT;
GBL_INLINE void        EvmuWave_update                  (SELF, EVMU_LOGIC value)                            GBL_NOEXCEPT;

GBL_INLINE EVMU_LOGIC  EvmuWave_logicCurrent            (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogic                 (CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicHigh             (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicLow              (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicInactive         (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicUnknown          (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicActive           (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicKnown            (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicValid            (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_isLogicInvalid          (CSELF)                                             GBL_NOEXCEPT;

GBL_INLINE EVMU_LOGIC  EvmuWave_logicPrevious           (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogic                (CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicHigh            (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicLow             (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicInactive        (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicUnknown         (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicActive          (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicKnown           (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicValid           (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_wasLogicInvalid         (CSELF)                                             GBL_NOEXCEPT;

GBL_INLINE GblBool     EvmuWave_hasStayed               (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedLogic          (CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedLow            (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedHigh           (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedInactive       (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedUnknown        (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedActive         (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedKnown          (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedValid          (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasStayedInvalid        (CSELF)                                             GBL_NOEXCEPT;

GBL_INLINE GblBool     EvmuWave_hasChanged              (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedLogic         (CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedEdge          (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedEdgeRising    (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedEdgeFalling   (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedActive        (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedInactive      (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedKnown         (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedUnknown       (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedValid         (CSELF)                                             GBL_NOEXCEPT;
GBL_INLINE GblBool     EvmuWave_hasChangedInvalid       (CSELF)                                             GBL_NOEXCEPT;





// ========== INLINE IMPLEMENTATION =======

#define EVMU_WAVE_LOGIC_MASK_            (EVMU_WAVE_LOGIC_CURRENT_MASK | EVMU_WAVE_LOGIC_PREVIOUS_MASK)


GBL_INLINE void EvmuWave_logicPreviousSet_(SELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    *pSelf &= ~EVMU_WAVE_LOGIC_PREVIOUS_MASK;
    *pSelf |= (value << EVMU_WAVE_LOGIC_BITS);
}
GBL_INLINE void EvmuWave_logicCurrentSet_(SELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    *pSelf &= ~EVMU_WAVE_LOGIC_CURRENT_MASK;
    *pSelf |= (value);
}
GBL_INLINE void EvmuWave_reset(SELF) GBL_NOEXCEPT {
    *pSelf = EVMU_WAVE_X_X;
}
GBL_INLINE void EvmuWave_fill(SELF, EVMU_LOGIC values) GBL_NOEXCEPT {
    EvmuWave_set(pSelf, values, values);
}
GBL_INLINE void EvmuWave_set(SELF, EVMU_LOGIC prevValue, EVMU_LOGIC curValue) GBL_NOEXCEPT {
    EvmuWave_logicPreviousSet_(pSelf, prevValue);
    EvmuWave_logicCurrentSet_(pSelf, curValue);
}
GBL_INLINE EVMU_LOGIC EvmuWave_logicCurrent(CSELF) GBL_NOEXCEPT {
    return *pSelf & EVMU_WAVE_LOGIC_CURRENT_MASK;
}
GBL_INLINE EVMU_LOGIC EvmuWave_logicPrevious(CSELF) GBL_NOEXCEPT {
    return *pSelf & EVMU_WAVE_LOGIC_PREVIOUS_MASK;
}
GBL_INLINE void EvmuWave_update(SELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    *pSelf <<= EVMU_WAVE_LOGIC_BITS;
    *pSelf &= (value & EVMU_WAVE_LOGIC_MASK_);
}
GBL_INLINE GblBool EvmuWave_hasStayed(CSELF) GBL_NOEXCEPT {
    return EvmuWave_logicCurrent(pSelf) == EvmuWave_logicPrevious(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasStayedLogic(CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, value) && EvmuWave_wasLogic(pSelf, value);
}
GBL_INLINE GblBool EvmuWave_hasStayedLow(CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_0);
}
GBL_INLINE GblBool EvmuWave_hasStayedHigh(CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_1);
}
GBL_INLINE GblBool EvmuWave_hasStayedInactive(CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_Z);
}
GBL_INLINE GblBool EvmuWave_hasStayedUnknown(CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_X);
}
GBL_INLINE GblBool EvmuWave_hasStayedActive(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicActive(pSelf) && EvmuWave_isLogicActive(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasStayedKnown(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicKnown(pSelf) && EvmuWave_isLogicKnown(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasStayedValid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicValid(pSelf) && EvmuWave_isLogicValid(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasStayedInvalid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInvalid(pSelf) && EvmuWave_isLogicInvalid(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChanged(CSELF) GBL_NOEXCEPT {
    return !EvmuWave_hasStayed(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedLogic(CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return !EvmuWave_wasLogic(pSelf, value) && EvmuWave_isLogic(pSelf, value);
}
GBL_INLINE GblBool EvmuWave_hasChangedEdge(CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasChangedEdgeRising(pSelf) || EvmuWave_hasChangedEdgeFalling(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedEdgeRising(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicLow(pSelf) && EvmuWave_isLogicHigh(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedEdgeFalling(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicHigh(pSelf) && EvmuWave_isLogicLow(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedActive(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInactive(pSelf) && EvmuWave_isLogicActive(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedInactive(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicActive(pSelf) && EvmuWave_isLogicInactive(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedKnown(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicUnknown(pSelf) && EvmuWave_isLogicKnown(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedUnknown(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicKnown(pSelf) && EvmuWave_isLogicUnknown(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedValid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInvalid(pSelf) && EvmuWave_isLogicValid(pSelf);
}
GBL_INLINE GblBool EvmuWave_hasChangedInvalid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicValid(pSelf) && EvmuWave_isLogicInvalid(pSelf);
}
GBL_INLINE GblBool EvmuWave_isLogic(CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return EvmuWave_logicCurrent(pSelf) == value;
}
GBL_INLINE GblBool EvmuWave_isLogicHigh(CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_1);
}
GBL_INLINE GblBool EvmuWave_isLogicLow(CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_0);
}
GBL_INLINE GblBool EvmuWave_isLogicInactive(CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_Z);
}
GBL_INLINE GblBool EvmuWave_isLogicUnknown(CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_X);
}
GBL_INLINE GblBool EvmuWave_isLogicActive(CSELF) GBL_NOEXCEPT {
    return !EvmuWave_isLogicInactive(pSelf);
}
GBL_INLINE GblBool EvmuWave_isLogicKnown(CSELF) GBL_NOEXCEPT {
    return !EvmuWave_isLogicUnknown(pSelf);
}
GBL_INLINE GblBool EvmuWave_isLogicValid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogicActive(pSelf) && EvmuWave_isLogicKnown(pSelf);
}
GBL_INLINE GblBool EvmuWave_isLogicInvalid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogicInactive(pSelf) || EvmuWave_isLogicUnknown(pSelf);
}
GBL_INLINE GblBool EvmuWave_wasLogic(CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return EvmuWave_logicPrevious(pSelf) == value;
}
GBL_INLINE GblBool EvmuWave_wasLogicHigh(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_1);
}
GBL_INLINE GblBool EvmuWave_wasLogicLow(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_0);
}
GBL_INLINE GblBool EvmuWave_wasLogicInactive(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_Z);
}
GBL_INLINE GblBool EvmuWave_wasLogicUnknown(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_X);
}
GBL_INLINE GblBool EvmuWave_wasLogicActive(CSELF) GBL_NOEXCEPT {
    return !EvmuWave_wasLogicInactive(pSelf);
}
GBL_INLINE GblBool EvmuWave_wasLogicKnown(CSELF) GBL_NOEXCEPT {
    return !EvmuWave_wasLogicUnknown(pSelf);
}
GBL_INLINE GblBool EvmuWave_wasLogicValid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicActive(pSelf) && EvmuWave_wasLogicKnown(pSelf);
}
GBL_INLINE GblBool EvmuWave_wasLogicInvalid(CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInactive(pSelf) || EvmuWave_wasLogicUnknown(pSelf);
}

#undef EVMU_WAVE_LOGIC_MASK_

GBL_DECLS_END


#undef CSELF
#undef SELF


#endif // EVMU_WAVE_H

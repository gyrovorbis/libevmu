#ifndef EVMU_WAVE_H
#define EVMU_WAVE_H

#include "../types/evmu_typedefs.h"

#define EVMU_LOGIC_HIGH                 1
#define EVMU_LOGIC_LOW                  0

#define EVMU_WAVE_LOGIC_BITS            2
#define EVMU_WAVE_LOGIC_CURRENT_MASK    (0x3)
#define EVMU_WAVE_LOGIC_PREVIOUS_MASK   (0x70)

#define GBL_SELF_TYPE GblEnum

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

EVMU_INLINE void        EvmuWave_reset                   (GBL_SELF)                                              GBL_NOEXCEPT;
EVMU_INLINE void        EvmuWave_fill                    (GBL_SELF, EVMU_LOGIC values)                           GBL_NOEXCEPT;
EVMU_INLINE void        EvmuWave_set                     (GBL_SELF, EVMU_LOGIC prevValue, EVMU_LOGIC curValue)   GBL_NOEXCEPT;
EVMU_INLINE void        EvmuWave_update                  (GBL_SELF, EVMU_LOGIC value)                            GBL_NOEXCEPT;

EVMU_INLINE EVMU_LOGIC  EvmuWave_logicCurrent            (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogic                 (GBL_CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicHigh             (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicLow              (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicInactive         (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicUnknown          (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicActive           (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicKnown            (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicValid            (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_isLogicInvalid          (GBL_CSELF)                                             GBL_NOEXCEPT;

EVMU_INLINE EVMU_LOGIC  EvmuWave_logicPrevious           (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogic                (GBL_CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicHigh            (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicLow             (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicInactive        (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicUnknown         (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicActive          (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicKnown           (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicValid           (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_wasLogicInvalid         (GBL_CSELF)                                             GBL_NOEXCEPT;

EVMU_INLINE GblBool     EvmuWave_hasStayed               (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedLogic          (GBL_CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedLow            (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedHigh           (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedInactive       (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedUnknown        (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedActive         (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedKnown          (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedValid          (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasStayedInvalid        (GBL_CSELF)                                             GBL_NOEXCEPT;

EVMU_INLINE GblBool     EvmuWave_hasChanged              (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedLogic         (GBL_CSELF, EVMU_LOGIC value)                           GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedEdge          (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedEdgeRising    (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedEdgeFalling   (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedActive        (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedInactive      (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedKnown         (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedUnknown       (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedValid         (GBL_CSELF)                                             GBL_NOEXCEPT;
EVMU_INLINE GblBool     EvmuWave_hasChangedInvalid       (GBL_CSELF)                                             GBL_NOEXCEPT;

// ========== INLINE IMPLEMENTATION =======

#define EVMU_WAVE_LOGIC_MASK_            (EVMU_WAVE_LOGIC_CURRENT_MASK | EVMU_WAVE_LOGIC_PREVIOUS_MASK)


EVMU_INLINE void EvmuWave_logicPreviousSet_(GBL_SELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    *pSelf &= ~EVMU_WAVE_LOGIC_PREVIOUS_MASK;
    *pSelf |= (value << EVMU_WAVE_LOGIC_BITS);
}
EVMU_INLINE void EvmuWave_logicCurrentSet_(GBL_SELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    *pSelf &= ~EVMU_WAVE_LOGIC_CURRENT_MASK;
    *pSelf |= (value);
}
EVMU_INLINE void EvmuWave_reset(GBL_SELF) GBL_NOEXCEPT {
    *pSelf = EVMU_WAVE_X_X;
}
EVMU_INLINE void EvmuWave_fill(GBL_SELF, EVMU_LOGIC values) GBL_NOEXCEPT {
    EvmuWave_set(pSelf, values, values);
}
EVMU_INLINE void EvmuWave_set(GBL_SELF, EVMU_LOGIC prevValue, EVMU_LOGIC curValue) GBL_NOEXCEPT {
    EvmuWave_logicPreviousSet_(pSelf, prevValue);
    EvmuWave_logicCurrentSet_(pSelf, curValue);
}
EVMU_INLINE EVMU_LOGIC EvmuWave_logicCurrent(GBL_CSELF) GBL_NOEXCEPT {
    return (EVMU_LOGIC)(*pSelf & EVMU_WAVE_LOGIC_CURRENT_MASK);
}
EVMU_INLINE EVMU_LOGIC EvmuWave_logicPrevious(GBL_CSELF) GBL_NOEXCEPT {
    return (EVMU_LOGIC)(*pSelf & EVMU_WAVE_LOGIC_PREVIOUS_MASK);
}
EVMU_INLINE void EvmuWave_update(GBL_SELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    *pSelf <<= EVMU_WAVE_LOGIC_BITS;
    *pSelf &= (value & EVMU_WAVE_LOGIC_MASK_);
}
EVMU_INLINE GblBool EvmuWave_hasStayed(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_logicCurrent(pSelf) == EvmuWave_logicPrevious(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasStayedLogic(GBL_CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, value) && EvmuWave_wasLogic(pSelf, value);
}
EVMU_INLINE GblBool EvmuWave_hasStayedLow(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_0);
}
EVMU_INLINE GblBool EvmuWave_hasStayedHigh(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_1);
}
EVMU_INLINE GblBool EvmuWave_hasStayedInactive(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_Z);
}
EVMU_INLINE GblBool EvmuWave_hasStayedUnknown(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasStayedLogic(pSelf, EVMU_LOGIC_X);
}
EVMU_INLINE GblBool EvmuWave_hasStayedActive(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicActive(pSelf) && EvmuWave_isLogicActive(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasStayedKnown(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicKnown(pSelf) && EvmuWave_isLogicKnown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasStayedValid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicValid(pSelf) && EvmuWave_isLogicValid(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasStayedInvalid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInvalid(pSelf) && EvmuWave_isLogicInvalid(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChanged(GBL_CSELF) GBL_NOEXCEPT {
    return !EvmuWave_hasStayed(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedLogic(GBL_CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return !EvmuWave_wasLogic(pSelf, value) && EvmuWave_isLogic(pSelf, value);
}
EVMU_INLINE GblBool EvmuWave_hasChangedEdge(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_hasChangedEdgeRising(pSelf) || EvmuWave_hasChangedEdgeFalling(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedEdgeRising(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicLow(pSelf) && EvmuWave_isLogicHigh(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedEdgeFalling(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicHigh(pSelf) && EvmuWave_isLogicLow(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedActive(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInactive(pSelf) && EvmuWave_isLogicActive(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedInactive(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicActive(pSelf) && EvmuWave_isLogicInactive(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedKnown(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicUnknown(pSelf) && EvmuWave_isLogicKnown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedUnknown(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicKnown(pSelf) && EvmuWave_isLogicUnknown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedValid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInvalid(pSelf) && EvmuWave_isLogicValid(pSelf);
}
EVMU_INLINE GblBool EvmuWave_hasChangedInvalid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicValid(pSelf) && EvmuWave_isLogicInvalid(pSelf);
}
EVMU_INLINE GblBool EvmuWave_isLogic(GBL_CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return EvmuWave_logicCurrent(pSelf) == value;
}
EVMU_INLINE GblBool EvmuWave_isLogicHigh(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_1);
}
EVMU_INLINE GblBool EvmuWave_isLogicLow(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_0);
}
EVMU_INLINE GblBool EvmuWave_isLogicInactive(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_Z);
}
EVMU_INLINE GblBool EvmuWave_isLogicUnknown(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogic(pSelf, EVMU_LOGIC_X);
}
EVMU_INLINE GblBool EvmuWave_isLogicActive(GBL_CSELF) GBL_NOEXCEPT {
    return !EvmuWave_isLogicInactive(pSelf);
}
EVMU_INLINE GblBool EvmuWave_isLogicKnown(GBL_CSELF) GBL_NOEXCEPT {
    return !EvmuWave_isLogicUnknown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_isLogicValid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogicActive(pSelf) && EvmuWave_isLogicKnown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_isLogicInvalid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_isLogicInactive(pSelf) || EvmuWave_isLogicUnknown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_wasLogic(GBL_CSELF, EVMU_LOGIC value) GBL_NOEXCEPT {
    return EvmuWave_logicPrevious(pSelf) == value;
}
EVMU_INLINE GblBool EvmuWave_wasLogicHigh(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_1);
}
EVMU_INLINE GblBool EvmuWave_wasLogicLow(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_0);
}
EVMU_INLINE GblBool EvmuWave_wasLogicInactive(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_Z);
}
EVMU_INLINE GblBool EvmuWave_wasLogicUnknown(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogic(pSelf, EVMU_LOGIC_X);
}
EVMU_INLINE GblBool EvmuWave_wasLogicActive(GBL_CSELF) GBL_NOEXCEPT {
    return !EvmuWave_wasLogicInactive(pSelf);
}
EVMU_INLINE GblBool EvmuWave_wasLogicKnown(GBL_CSELF) GBL_NOEXCEPT {
    return !EvmuWave_wasLogicUnknown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_wasLogicValid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicActive(pSelf) && EvmuWave_wasLogicKnown(pSelf);
}
EVMU_INLINE GblBool EvmuWave_wasLogicInvalid(GBL_CSELF) GBL_NOEXCEPT {
    return EvmuWave_wasLogicInactive(pSelf) || EvmuWave_wasLogicUnknown(pSelf);
}

#undef EVMU_WAVE_LOGIC_MASK_

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_WAVE_H

#ifndef EVMU_GAMEPAD__H
#define EVMU_GAMEPAD__H

#include <evmu/hw/evmu_gamepad.h>

#define EVMU_GAMEPAD_(instance)     (GBL_PRIVATE(EvmuGamepad, instance))
#define EVMU_GAMEPAD_PUBLIC_(priv)  (GBL_PUBLIC(EvmuGamepad, priv))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuGamepad_) {
    EvmuRam_* pRam;
};

EvmuWord EvmuGamepad__port3Value_(const EvmuGamepad_* pSelf_);

GBL_DECLS_END

#endif // EVMU_GAMEPAD__H

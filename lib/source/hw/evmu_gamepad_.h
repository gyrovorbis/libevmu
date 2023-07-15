#ifndef EVMU_GAMEPAD__H
#define EVMU_GAMEPAD__H

#include <evmu/hw/evmu_gamepad.h>

#define EVMU_GAMEPAD_(instance)     ((EvmuGamepad_*)GBL_INSTANCE_PRIVATE(instance, EVMU_GAMEPAD_TYPE))
#define EVMU_GAMEPAD_PUBLIC_(priv)  ((EvmuGamepad*)GBL_INSTANCE_PUBLIC(priv, EVMU_GAMEPAD_TYPE))

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam_);

GBL_DECLARE_STRUCT(EvmuGamepad_) {
    EvmuRam_* pRam;
};

EvmuWord EvmuGamepad__port3Value_(const EvmuGamepad_* pSelf_);

GBL_DECLS_END

#endif // EVMU_GAMEPAD__H

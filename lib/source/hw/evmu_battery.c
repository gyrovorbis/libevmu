#include "evmu_device_.h"
#include "evmu_battery_.h"
#include "evmu_ram_.h"
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_address_space.h>

EVMU_EXPORT GblBool EvmuBattery_lowAlarm(const EvmuBattery* pSelf) {
    EvmuBattery_* pSelf_ = EVMU_BATTERY_(pSelf);
    return !((pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)] & EVMU_SFR_P7_P71_MASK) >> EVMU_SFR_P7_P71_POS);
}

EVMU_EXPORT void EvmuBattery_setLowAlarm(EvmuBattery* pSelf, GblBool enabled) {
    EvmuBattery_* pSelf_ = EVMU_BATTERY_(pSelf);
    if(enabled) pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)] &= ~EVMU_SFR_P7_P71_MASK;
    else        pSelf_->pRam->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_P7)] |= EVMU_SFR_P7_P71_MASK;
}

EVMU_EXPORT GblBool EvmuBattery_monitorEnabled(const EvmuBattery* pSelf) {
    EvmuBattery_* pSelf_ = EVMU_BATTERY_(pSelf);
    return pSelf_->pRam->ram[0][EVMU_ADDRESS_SYSTEM_BATTERY_CHECK]? 0 : 1;
}

EVMU_EXPORT void EvmuBattery_setMonitorEnabled(EvmuBattery* pSelf, GblBool enabled) {
    EvmuBattery_* pSelf_ = EVMU_BATTERY_(pSelf);
    pSelf_->pRam->ram[0][EVMU_ADDRESS_SYSTEM_BATTERY_CHECK] = enabled? 0 : 0xff;
}

static GBL_RESULT EvmuBattery_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_BATTERY_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuBatteryClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_OBJECT_CLASS(pClass)->pFnConstructed = EvmuBattery_GblObject_constructed_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuBattery_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuBatteryClass),
        .pFnClassInit           = EvmuBatteryClass_init_,
        .instanceSize           = sizeof(EvmuBattery),
        .instancePrivateSize    = sizeof(EvmuBattery_)
    };

    if(!GblType_verify(type)) {
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuBattery"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
    }

    return type;
}


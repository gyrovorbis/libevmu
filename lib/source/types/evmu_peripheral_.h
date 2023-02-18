#ifndef EVMU_PERIPHERAL__H
#define EVMU_PERIPHERAL__H

#include <evmu/types/evmu_peripheral.h>

#define EVMU_PERIPHERAL_(instance)  ((EvmuPeripheral_*)GBL_INSTANCE_PRIVATE(instance, EVMU_PERIPHERAL_TYPE))

#define EVMU_PERIPHERAL_LOG_(src, peripheral, level, ...) \
    GBL_STMT_START { \
        if(((EvmuPeripheral*)peripheral)->logLevel & level) \
            GBL_CTX_LOG_(src, level, __VA_ARGS__); \
    } GBL_STMT_END

#define EVMU_PERIPHERAL_VERBOSE(peripheral, ...) \
    EVMU_PERIPHERAL_LOG_(GBL_SRC_LOC(GBL_SRC_FILE, GBL_SRC_FN, GBL_SRC_LN), peripheral, GBL_LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define EVMU_PERIPHERAL_DEBUG(peripheral, ...) \
    EVMU_PERIPHERAL_LOG_(GBL_SRC_LOC(GBL_SRC_FILE, GBL_SRC_FN, GBL_SRC_LN), peripheral, GBL_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define EVMU_PERIPHERAL_INFO(peripheral, ...) \
    EVMU_PERIPHERAL_LOG_(GBL_SRC_LOC(GBL_SRC_FILE, GBL_SRC_FN, GBL_SRC_LN), peripheral, GBL_LOG_LEVEL_INFO, __VA_ARGS__)
#define EVMU_PERIPHERAL_WARN(peripheral, ...) \
    EVMU_PERIPHERAL_LOG_(GBL_SRC_LOC(GBL_SRC_FILE, GBL_SRC_FN, GBL_SRC_LN), peripheral, GBL_LOG_LEVEL_WARN, __VA_ARGS__)
#define EVMU_PERIPHERAL_ERROR(peripheral, ...) \
    EVMU_PERIPHERAL_LOG_(GBL_SRC_LOC(GBL_SRC_FILE, GBL_SRC_FN, GBL_SRC_LN), peripheral, GBL_LOG_LEVEL_ERROR, __VA_ARGS__)

GBL_FORWARD_DECLARE_STRUCT(EvmuDevice_);

GBL_DECLARE_STRUCT(EvmuPeripheral_) {
    EvmuDevice_* pDevice_;
};



#endif // EVMU_PERIPHERAL__H

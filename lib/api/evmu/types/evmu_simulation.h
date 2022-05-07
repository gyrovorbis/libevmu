#ifndef EVMU_SIMULATION_H
#define EVMU_SIMULATION_H

#include "evmu_entity.h"

#define EVMU_SIMULATION_TYPE                        (EvmuSimulation_type())
#define EVMU_SIMULATION_STRUCT                      EvmuSimulation
#define EVMU_SIMULATION_CLASS_STRUCT                EvmuSimulationClass
#define EVMU_SIMULATION(instance)                   (GBL_INSTANCE_CAST_PREFIX  (instance,  EVMU_SIMULATION))
#define EVMU_SIMULATION_CHECK(instance)             (GBL_INSTANCE_CHECK_PREFIX (instance,  EVMU_SIMULATION))
#define EVMU_SIMULATION_CLASS(klass)                (GBL_CLASS_CAST_PREFIX     (klass,     EVMU_SIMULATION))
#define EVMU_SIMULATION_CLASS_CHECK(klass)          (GBL_CLASS_CHECK_PREFIX    (klass,     EVMU_SIMULATION))
#define EVMU_SIMULATION_GET_CLASS(instance)         (GBL_INSTANCE_CAST_CLASS_PREFIX (instance,  EVMU_SIMULATION))

#define SELF    EvmuSimulation* pSelf
#define CSELF   const SELF

GBL_DECLS_BEGIN

typedef struct EvmuSimulationClass {
    EvmuEntityClass base;
} EvmuSimulationClass;

typedef struct EvmuSimulation {
    EvmuEntity              base;
    //virtual gettimeofday
    //virtual fileOpen
    //virtual fileRead
    //virtual fileWrite
    //virtual fileSeek
} EvmuSimulation;

GBL_EXPORT GblType             EvmuSimulation_type             (void)                          GBL_NOEXCEPT;

GBL_EXPORT EvmuSimulation*     EvmuSimulation_create           (GblContext* pContext)          GBL_NOEXCEPT;
GBL_EXPORT void                EvmuSimulation_destroy          (SELF)                          GBL_NOEXCEPT;
GBL_EXPORT EvmuDevice*         EvmuSimulation_deviceCreate     (SELF)                          GBL_NOEXCEPT;
GBL_EXPORT void                EvmuSimulation_deviceDestroy    (SELF, EvmuDevice* pDevice)     GBL_NOEXCEPT;
GBL_EXPORT GblSize             EvmuSimulation_deviceCount      (CSELF)                         GBL_NOEXCEPT;
GBL_EXPORT EvmuDevice*         EvmuSimulation_deviceFindByIndex(CSELF, GblSize index)          GBL_NOEXCEPT;

GBL_DECLS_END

#undef CSELF
#undef SELF

#endif // EVMU_CONTEXT_H

#ifndef EVMU_CPU_TEST_SUITE_H
#define EVMU_CPU_TEST_SUITE_H

#include <gimbal/test/gimbal_test_suite.h>

#define EVMU_CPU_TEST_SUITE_TYPE                (GBL_TYPEOF(EvmuCpuTestSuite))
#define EVMU_CPU_TEST_SUITE(instance)           (GBL_INSTANCE_CAST(instannce, EvmuCpuTestSuite))
#define EVMU_CPU_TEST_SUITE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuCpuTestSuite))
#define EVMU_CPU_TEST_SUITE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuCpuTestSuite))

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuCpuTestSuite, GblTestSuite)
GBL_INSTANCE_DERIVE_EMPTY(EvmuCpuTestSuite, GblTestSuite)

GBL_EXPORT GblType EvmuCpuTestSuite_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#endif

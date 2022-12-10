#ifndef EVMU_MEMORY_TEST_SUITE_H
#define EVMU_MEMORY_TEST_SUITE_H

#include <gimbal/test/gimbal_test_suite.h>

#define EVMU_MEMORY_TEST_SUITE_TYPE                (GBL_TYPEOF(EvmuMemoryTestSuite))
#define EVMU_MEMORY_TEST_SUITE(instance)           (GBL_INSTANCE_CAST(instannce, EvmuMemoryTestSuite))
#define EVMU_MEMORY_TEST_SUITE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuMemoryTestSuite))
#define EVMU_MEMORY_TEST_SUITE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuMemoryTestSuite))

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuMemoryTestSuite, GblTestSuite)
GBL_INSTANCE_DERIVE_EMPTY(EvmuMemoryTestSuite, GblTestSuite)

GBL_EXPORT GblType EvmuMemoryTestSuite_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#endif

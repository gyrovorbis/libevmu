#ifndef EVMU_ISA_TEST_SUITE_H
#define EVMU_ISA_TEST_SUITE_H

#include <gimbal/test/gimbal_test_suite.h>

#define EVMU_ISA_TEST_SUITE_TYPE                (GBL_TYPEOF(EvmuIsaTestSuite))
#define EVMU_ISA_TEST_SUITE(instance)           (GBL_INSTANCE_CAST(instannce, EvmuIsaTestSuite))
#define EVMU_ISA_TEST_SUITE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuIsaTestSuite))
#define EVMU_ISA_TEST_SUITE_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuIsaTestSuite))

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuIsaTestSuite, GblTestSuite)
GBL_INSTANCE_DERIVE_EMPTY(EvmuIsaTestSuite, GblTestSuite)

GBL_EXPORT GblType EvmuIsaTestSuite_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#endif

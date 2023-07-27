#ifndef EVMU_RAM_TEST_SUITE_H
#define EVMU_RAM_TEST_SUITE_H

#include <gimbal/test/gimbal_test_suite.h>

#define EVMU_RAM_TEST_SUITE_TYPE                (GBL_TYPEID(EvmuRamTestSuite))
#define EVMU_RAM_TEST_SUITE(instance)           (GBL_CAST(instannce, EvmuRamTestSuite))
#define EVMU_RAM_TEST_SUITE_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuRamTestSuite))
#define EVMU_RAM_TEST_SUITE_GET_CLASS(instance) (GBL_CLASSOF(instance, EvmuRamTestSuite))

GBL_DECLS_BEGIN

GBL_CLASS_DERIVE_EMPTY   (EvmuRamTestSuite, GblTestSuite)
GBL_INSTANCE_DERIVE_EMPTY(EvmuRamTestSuite, GblTestSuite)

GBL_EXPORT GblType EvmuRamTestSuite_type(void) GBL_NOEXCEPT;

GBL_DECLS_END

#endif

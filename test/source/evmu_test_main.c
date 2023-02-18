#include <gimbal/test/gimbal_test_scenario.h>
#include "evmu_memory_test_suite.h"
#include "evmu_cpu_test_suite.h"
#include "evmu_isa_test_suite.h"

#if defined(__DREAMCAST__) && !defined(NDEBUG)
#   include <arch/gdb.h>
#endif

int main(int argc, char* pArgv[]) {
#if defined(__DREAMCAST__) && !defined(NDEBUG)
    gdb_init();
#endif
    GblTestScenario* pScenario = GblTestScenario_create("ElysianVmuTests");
    GblContext_setLogFilter(GBL_CONTEXT(pScenario), GBL_LOG_LEVEL_INFO | GBL_LOG_LEVEL_WARNING | GBL_LOG_LEVEL_ERROR );

    GblTestScenario_enqueueSuite(pScenario,
                                 GBL_TEST_SUITE(GBL_OBJECT_NEW(EvmuMemoryTestSuite)));
    GblTestScenario_enqueueSuite(pScenario,
                                 GBL_TEST_SUITE(GBL_OBJECT_NEW(EvmuCpuTestSuite)));
    GblTestScenario_enqueueSuite(pScenario,
                                 GBL_TEST_SUITE(GBL_OBJECT_NEW(EvmuIsaTestSuite)));

    const GBL_RESULT result = GblTestScenario_run(pScenario, argc, pArgv);

    GblTestScenario_unref(pScenario);

    return GBL_RESULT_SUCCESS(result)? 0 : -1;
}

#include <gimbal/test/gimbal_test_scenario.h>
#include <gimbal/meta/instances/gimbal_context.h>
#include <evmu/types/evmu_emulator.h>
#include <evmu/hw/evmu_device.h>

#if defined(__DREAMCAST__) && !defined(NDEBUG)
#   include <arch/gdb.h>
#endif

int main(int argc, char* pArgv[]) {
#if defined(__DREAMCAST__) && !defined(NDEBUG)
    gdb_init();
#endif
    GblTestScenario* pScenario = GblTestScenario_create("ElysianVmuTests");

    EvmuEmulator* pEvmuEmu = EvmuEmulator_create(GBL_CONTEXT(pScenario));
    EvmuDevice* pDevice = EVMU_DEVICE(GblObject_create(EVMU_DEVICE_TYPE, NULL));
    EvmuEmulator_addDevice(pEvmuEmu, pDevice);


    GblContext_setLogFilter(GBL_CONTEXT(pScenario), GBL_LOG_LEVEL_INFO | GBL_LOG_LEVEL_WARNING | GBL_LOG_LEVEL_ERROR );

    const GBL_RESULT result = GblTestScenario_run(pScenario, argc, pArgv);

    GblTestScenario_destroy(pScenario);

    return GBL_RESULT_SUCCESS(result)? 0 : -1;
}

#include <evmu/events/evmu_memory_event.h>

EVMU_EXPORT GblType EvmuMemoryEvent_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) {
        GBL_API_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuMemoryEvent"),
                                      GBL_EVENT_TYPE,
                                      &(GblTypeInfo){
                                          .classSize    = sizeof(EvmuMemoryEventClass),
                                          .instanceSize = sizeof(EvmuMemoryEvent)
                                      }, GBL_TYPE_FLAGS_NONE);
        GBL_API_VERIFY_LAST_RECORD();
        GBL_API_END_BLOCK();
    }

    return type;
}

#include <evmu/events/evmu_memory_event.h>

EVMU_EXPORT GblType EvmuMemoryEvent_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuMemoryEvent"),
                                      GBL_EVENT_TYPE,
                                      &(GblTypeInfo){
                                          .classSize    = sizeof(EvmuMemoryEventClass),
                                          .instanceSize = sizeof(EvmuMemoryEvent)
                                      }, GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}

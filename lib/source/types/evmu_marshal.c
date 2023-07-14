#include "evmu_marshal_.h"
#include <gimbal/meta/types/gimbal_variant.h>
#include <gimbal/meta/signals/gimbal_c_closure.h>

GBL_DEFINE_CCLOSURE_MARSHAL_VOID__(INSTANCE_UINT16_UINT8,
                                   3,
                                   (GblInstance*, uint16_t, uint8_t),
                                   (GblVariant_toPointer(&pArgs[0]),
                                    GblVariant_toUint16(&pArgs[1]),
                                    GblVariant_toUint8(&pArgs[2])))

GBL_DEFINE_CCLOSURE_MARSHAL_VOID__(INSTANCE_UINT32_SIZE_POINTER,
                                   4,
                                   (GblInstance*, uint32_t, size_t, const void*),
                                   (GblVariant_toPointer(&pArgs[0]),
                                    GblVariant_toUint32(&pArgs[1]),
                                    GblVariant_toSize(&pArgs[2]),
                                    GblVariant_toPointer(&pArgs[3])));

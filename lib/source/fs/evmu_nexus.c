#include <evmu/fs/evmu_nexus.h>

EVMU_EXPORT void* EvmuNexus_applyByteOrdering(void* pData, size_t bytes) {
    GBL_ASSERT(!(bytes % 4), "This function can only apply to word-multiples!");

    uint8_t* pBytes    = pData;
    size_t   wordCount = bytes / 4;
    if(bytes % 4) ++wordCount;

    for(size_t w = 0; w < wordCount; ++w) {
        for(int b = 0; b < 2; ++b) {
            GBL_SWAP(pBytes[w * 4 + b],
                     pBytes[w * 4 + 4 - b - 1]);
        }
    }

    return pData;
}

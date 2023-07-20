#include <evmu/fs/evmu_vms.h>
#include <evmu/fs/evmu_fat.h>

#define EVMU_VMS_STRING_GET_(method, field, size) \
    EVMU_EXPORT const char* EvmuVms_##method(const EvmuVms*   pSelf, \
                                            GblStringBuffer* pBuff) \
    { \
        GblStringBuffer_set(pBuff, GBL_STRV(pSelf->field, size)); \
        return GblStringBuffer_cString(pBuff); \
    }

EVMU_VMS_STRING_GET_(vmuDescription, vmuDesc,    EVMU_VMS_VMU_DESCRIPTION_SIZE)
EVMU_VMS_STRING_GET_(dcDescription,  dcDesc,     EVMU_VMS_DC_DESCRIPTION_SIZE)
EVMU_VMS_STRING_GET_(creatorApp,     creatorApp, EVMU_VMS_CREATOR_APP_SIZE)

#define EVMU_VMS_STRING_SET_(method, field, size) \
    EVMU_EXPORT size_t EvmuVms_set##method(EvmuVms* pSelf, const char* pStr) { \
        const size_t copySize = strnlen(pStr, size); \
        memset(pSelf->field, 0, size); \
        memcpy(pSelf->field, pStr, copySize); \
        return copySize; \
    }

EVMU_VMS_STRING_SET_(VmuDescription, vmuDesc,    EVMU_VMS_VMU_DESCRIPTION_SIZE)
EVMU_VMS_STRING_SET_(DcDescription,  dcDesc,     EVMU_VMS_DC_DESCRIPTION_SIZE)
EVMU_VMS_STRING_SET_(CreatorApp,     creatorApp, EVMU_VMS_CREATOR_APP_SIZE)

EVMU_EXPORT GblBool EvmuVms_isValid(const EvmuVms* pSelf) {
    if(pSelf->iconCount    <= EVMU_VMS_ICON_COUNT_MAX &&
       pSelf->eyecatchType <  EVMU_VMS_EYECATCH_COUNT)
    {
        for(unsigned b = 0; b < EVMU_VMS_RESERVED_SIZE; ++b) {
            if(pSelf->reserved[b])
                return GBL_FALSE;
        }
        return GBL_TRUE;
    }
    return GBL_FALSE;
}

EVMU_EXPORT size_t EvmuVms_headerBytes(const EvmuVms* pSelf) {
    //VMS header + icon palette
    size_t size = sizeof(EvmuVms);

    //Icons (Each frame of the animation icon is 512 bytes)
    size += pSelf->iconCount * EVMU_VMS_ICON_BITMAP_SIZE;

    //Eyecatch
    switch(pSelf->eyecatchType) {
    case EVMU_VMS_EYECATCH_16BIT:
        //No palette, all image
        size += EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_16BIT;
        break;
    case EVMU_VMS_EYECATCH_PALETTE_256:
        //512 bytes palette, 4032 bytes bitmap
        size += EVMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_256;
        size += EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_256;
        break;
    case EVMU_VMS_EYECATCH_PALETTE_16:
        //32 bytes palette, 2016 bytes bitmap
        size += EVMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_16;
        size += EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_16;
        break;
    default: //No extra shit if no eyecatch is used
        break;
    }

    return size;
}

EVMU_EXPORT size_t EvmuVms_totalBytes(const EvmuVms* pSelf) {
    return EvmuVms_headerBytes(pSelf) + pSelf->dataBytes;
}

EVMU_EXPORT const char* EvmuVms_eyecatchTypeStr(const EvmuVms* pSelf) {
    switch(pSelf->eyecatchType) {
    case EVMU_VMS_EYECATCH_NONE:        return "None";
    case EVMU_VMS_EYECATCH_16BIT:       return "16-Bit Color";
    case EVMU_VMS_EYECATCH_PALETTE_256: return "256 Color Paletted";
    case EVMU_VMS_EYECATCH_PALETTE_16:  return "16 Color Paletted";
    default:                            return "Invalid";
    }
}

EVMU_EXPORT void EvmuVms_log(const EvmuVms* pSelf) {
    struct {
        GblStringBuffer buff;
        char            stackBytes[128];
    } str;

    GblStringBuffer_construct(&str.buff, GBL_STRV(""), sizeof(str));

    EVMU_LOG_INFO("VMS File Attributes");
    EVMU_LOG_PUSH();

    const EVMU_FILE_TYPE fileType = EvmuVms_guessFileType(pSelf);

    EVMU_LOG_VERBOSE("%-20s: %40s",  "Valid",           EvmuVms_isValid(pSelf)? "YES" : "NO");
    EVMU_LOG_VERBOSE("%-20s: %40s",  "File Type Guess", fileType == EVMU_FILE_TYPE_GAME? "GAME" :
                                                        fileType == EVMU_FILE_TYPE_DATA? "DATA" : "UNKNOWN");
    EVMU_LOG_VERBOSE("%-20s: %40s",  "VMU Description", EvmuVms_vmuDescription(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40s",  "DC Description",  EvmuVms_dcDescription(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40s",  "Creator App",     EvmuVms_creatorApp(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40u",  "Icon Count",      pSelf->iconCount);
    EVMU_LOG_VERBOSE("%-20s: %40u",  "Animation Speed", pSelf->animSpeed);
    EVMU_LOG_VERBOSE("%-20s: %40s",  "Eyecatch Type",   EvmuVms_eyecatchTypeStr(pSelf));
    EVMU_LOG_VERBOSE("%-20s: %40u",  "CRC",             pSelf->crc);
    EVMU_LOG_VERBOSE("%-20s: %40zu", "Header Bytes",    EvmuVms_headerBytes(pSelf));
    EVMU_LOG_VERBOSE("%-20s: %40u",  "Data Bytes",      pSelf->dataBytes);
    EVMU_LOG_VERBOSE("%-20s: %40s",  "Reserved",        GblStringBuffer_set(&str.buff,
                                                            GBL_STRV(pSelf->reserved,
                                                                     EVMU_VMS_RESERVED_SIZE)));
    EVMU_LOG_POP(1);

    GblStringBuffer_destruct(&str.buff);
}

EVMU_EXPORT EVMU_FILE_TYPE EvmuVms_guessFileType(const EvmuVms* pSelf) {
    if(EvmuVms_isValid(pSelf))
        return EVMU_FILE_TYPE_DATA;
    else if(EvmuVms_isValid((const EvmuVms*)((uintptr_t)pSelf + EVMU_FAT_BLOCK_SIZE)))
        return EVMU_FILE_TYPE_GAME;
    else
        return EVMU_FILE_TYPE_NONE;
}

static const void* EvmuVms_eyecatch_(const EvmuVms* pSelf) {
    return (void*)(uintptr_t)(pSelf + 1) + pSelf->iconCount * EVMU_VMS_ICON_BITMAP_SIZE;
}

static const void* EvmuVms_icon_(const EvmuVms* pSelf, size_t index) {
    if(index < pSelf->iconCount) {
        return ((uint8_t*)(pSelf + 1)) + index * EVMU_VMS_ICON_BITMAP_SIZE;
    }
    return NULL;
}

EVMU_EXPORT GblByteArray* EvmuVms_createEyecatchArgb4444(const EvmuVms* pSelf) {
    GblByteArray* pByteArray = NULL;
    struct {
        GblStringBuffer buff;
        char            stackBytes[128];
    } str;

    GBL_CTX_BEGIN(NULL);
    GblStringBuffer_construct(&str.buff, GBL_STRV(""), sizeof(str));

    EVMU_LOG_VERBOSE("Creating eyecatch for VMS file: [%s]",
                     EvmuVms_vmuDescription(pSelf, &str.buff));
    EVMU_LOG_PUSH();

    GBL_CTX_VERIFY(EvmuVms_isValid(pSelf),
                   EVMU_RESULT_ERROR_INVALID_FILE);

    pByteArray = GblByteArray_create(sizeof(uint16_t) * EVMU_VMS_EYECATCH_BITMAP_WIDTH * EVMU_VMS_EYECATCH_BITMAP_HEIGHT);

    if(pSelf->eyecatchType == EVMU_VMS_EYECATCH_16BIT) {
        const void* pEyecatch = EvmuVms_eyecatch_(pSelf);

        GBL_CTX_VERIFY_CALL(
            GblByteArray_write(pByteArray, 0, pByteArray->size, pEyecatch)
        );
    } else {
        const uint16_t* pPalette = EvmuVms_eyecatch_(pSelf);

        if(pSelf->eyecatchType == EVMU_VMS_EYECATCH_PALETTE_256) {
            const uint8_t* pImage = ((uint8_t*)pPalette) + EVMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_256;

            for(size_t b = 0; b < EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_256; ++b) {
                GBL_CTX_VERIFY_CALL(
                    GblByteArray_write(pByteArray, 0, sizeof(uint16_t), &pPalette[pImage[b]])
                );
            }
        } else if(pSelf->eyecatchType == EVMU_VMS_EYECATCH_PALETTE_16) {
            const uint8_t* pImage = ((uint8_t*)pPalette) + EVMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_16;

            for(size_t b = 0; b < EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_16 * 2; ++b) {
                const uint8_t palIndex = b % 2? pImage[b / 2] & 0xf : (pImage[b / 2] >> 4) & 0xf;

                GBL_CTX_VERIFY_CALL(
                    GblByteArray_write(pByteArray, 0, sizeof(uint16_t), &pPalette[palIndex])
                );
            }
        } else GBL_ASSERT(GBL_FALSE, "Unknown VMS eyecatch type!");
    }

    EVMU_LOG_POP(1);

    GBL_CTX_END_BLOCK();
    GblStringBuffer_destruct(&str.buff);
    return pByteArray;
}

EVMU_EXPORT GblRingList* EvmuVms_createIconsArgb4444(const EvmuVms* pSelf) {
    GblRingList* pList = NULL;
    struct {
        GblStringBuffer buff;
        char            stackBytes[128];
    } str;

    GBL_CTX_BEGIN(NULL);
    GblStringBuffer_construct(&str.buff, GBL_STRV(""), sizeof(str));

    EVMU_LOG_VERBOSE("Creating icons for VMS file: [%s]",
                     EvmuVms_vmuDescription(pSelf, &str.buff));
    EVMU_LOG_PUSH();

    for(size_t i = 0 ; i < pSelf->iconCount; ++i) {
        GblByteArray*  pByteArray = GblByteArray_create(EVMU_VMS_ICON_BITMAP_SIZE);
        const uint8_t* pImage     = EvmuVms_icon_(pSelf, i);

        for(size_t b = 0; b < EVMU_VMS_ICON_BITMAP_SIZE * 2; ++b) {
            const uint8_t palIndex = b % 2? pImage[b / 2] & 0xf : (pImage[b / 2] >> 4) & 0xf;

            GBL_CTX_CALL(
                GblByteArray_write(pByteArray, 0, sizeof(uint16_t), &pSelf->palette[palIndex])
            );
        }

        if(!i) pList = GblRingList_create(pByteArray);
        else GblRingList_pushBack(pList, pByteArray);
    }

    EVMU_LOG_POP(1);

    GBL_CTX_END_BLOCK();
    GblStringBuffer_destruct(&str.buff);
    return pList;
}

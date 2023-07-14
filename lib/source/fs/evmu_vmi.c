#include <evmu/fs/evmu_vmi.h>
#include <evmu/fs/evmu_vms.h>
#include <evmu/fs/evmu_fat.h>
#include <evmu/fs/evmu_file_manager.h>

#define EVMU_VMI_COPYRIGHT_STRING_ "Powered by ElysianVMU"

#define EVMU_VMI_STRING_GET_(method, field, size) \
    EVMU_EXPORT const char* EvmuVmi_##method(const EvmuVmi*   pSelf, \
                                            GblStringBuffer* pBuff) \
    { \
        GblStringBuffer_set(pBuff, GBL_STRV(pSelf->field, size)); \
        return GblStringBuffer_cString(pBuff); \
    }

EVMU_VMI_STRING_GET_(description, description,     EVMU_VMI_DESCRIPTION_SIZE)
EVMU_VMI_STRING_GET_(copyright,   copyright,       EVMU_VMI_COPYRIGHT_SIZE);
EVMU_VMI_STRING_GET_(vmsResource, vmsResourceName, EVMU_VMI_VMS_RESOURCE_SIZE)
EVMU_VMI_STRING_GET_(fileName,    fileNameOnVms,   EVMU_VMI_VMS_NAME_SIZE)

EVMU_EXPORT GblBool EvmuVmi_isValid(const EvmuVmi* pSelf) {
    return (pSelf->checksum   == EvmuVmi_computeChecksum(pSelf) &&
            pSelf->vmiVersion == EVMU_VMI_VERSION &&
            pSelf->fileSize   >= EVMU_VMS_SIZE &&
            pSelf->unknown    == 0 &&
            !(pSelf->fileMode & ~(EVMU_VMI_GAME_MASK |
                                  EVMU_VMI_PROTECTED_MASK)));
}

EVMU_EXPORT GblDateTime* EvmuVmi_creation(const EvmuVmi* pSelf, GblDateTime* pDateTime) {
    return EvmuTimestamp_dateTime(&pSelf->creationTimestamp, pDateTime);
}

EVMU_EXPORT GblBool EvmuVmi_isGame(const EvmuVmi* pSelf) {
    return ((pSelf->fileMode & EVMU_VMI_GAME_MASK) >> EVMU_VMI_GAME_POS);
}

EVMU_EXPORT GblBool EvmuVmi_isProtected(const EvmuVmi* pSelf) {
    return ((pSelf->fileMode & EVMU_VMI_PROTECTED_MASK) >> EVMU_VMI_PROTECTED_POS);
}

EVMU_EXPORT uint32_t EvmuVmi_computeChecksum(const EvmuVmi* pSelf) {
    uint32_t checksum;
    unsigned char* byte = (unsigned char*)&checksum;

    byte[0] = pSelf->vmsResourceName[0] & 'S';
    byte[1] = pSelf->vmsResourceName[1] & 'E';
    byte[2] = pSelf->vmsResourceName[2] & 'G';
    byte[3] = pSelf->vmsResourceName[3] & 'A';

    return checksum;
}

#define EVMU_VMI_STRING_SET_(method, field, size) \
    EVMU_EXPORT size_t EvmuVmi_set##method(EvmuVmi* pSelf, const char* pStr) { \
        const size_t copySize = strnlen(pStr, size); \
        memset(pSelf->field, 0, size); \
        memcpy(pSelf->field, pStr, copySize); \
        return copySize; \
    }

EVMU_VMI_STRING_SET_(Description, description,     EVMU_VMI_DESCRIPTION_SIZE)
EVMU_VMI_STRING_SET_(Copyright,   copyright,       EVMU_VMI_COPYRIGHT_SIZE)
EVMU_VMI_STRING_SET_(VmsResource, vmsResourceName, EVMU_VMI_VMS_RESOURCE_SIZE)
EVMU_VMI_STRING_SET_(FileName,    fileNameOnVms,   EVMU_VMI_VMS_NAME_SIZE)

EVMU_EXPORT void EvmuVmi_setCreation(EvmuVmi* pSelf, const GblDateTime* pDateTime) {
    EvmuTimestamp_setDateTime(&pSelf->creationTimestamp, pDateTime);
}

EVMU_EXPORT void EvmuVmi_setGame(EvmuVmi* pSelf, GblBool value) {
    pSelf->fileMode &= ~EVMU_VMI_GAME_MASK;

    if(value)
        pSelf->fileMode |= EVMU_VMI_GAME_MASK;
}

EVMU_EXPORT void EvmuVmi_setProtected(EvmuVmi* pSelf, GblBool value) {
    pSelf->fileMode &= ~EVMU_VMI_PROTECTED_MASK;

    if(value)
        pSelf->fileMode |= EVMU_VMI_PROTECTED_MASK;
}

void EvmuVmi_log(const EvmuVmi* pSelf) {
    struct {
        GblStringBuffer buff;
        char            stackStorage[256];
    } str;
    GblDateTime dt;

    GblStringBuffer_construct(&str.buff, GBL_STRV(""), sizeof(str));

    const char* pCheckSumOk = pSelf->checksum == EvmuVmi_computeChecksum(pSelf)?
                                  "VALID" : "INVALID";

    EVMU_LOG_VERBOSE("VMI File Attributes");
    EVMU_LOG_PUSH();
    EVMU_LOG_VERBOSE("%-20s: %40s", "Valid",         EvmuVmi_isValid(pSelf)? "YES" : "NO");
    EVMU_LOG_VERBOSE("%-20s: %40u [%s]", "Checksum", pSelf->checksum, pCheckSumOk);
    EVMU_LOG_VERBOSE("%-20s: %40s", "Description",   EvmuVmi_description(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40s", "Copyright",     EvmuVmi_copyright(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40s", "Creation Date", GblDateTime_toIso8601(EvmuVmi_creation(pSelf, &dt), &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40u", "VMI Version",   pSelf->vmiVersion);
    EVMU_LOG_VERBOSE("%-20s: %40u", "File Number",   pSelf->fileNumber);
    EVMU_LOG_VERBOSE("%-20s: %40s", "VMS Resource",  EvmuVmi_vmsResource(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40s", "VMS Filename",  EvmuVmi_fileName(pSelf, &str.buff));
    EVMU_LOG_VERBOSE("%-20s: %40s", "File Type",     EvmuVmi_isGame(pSelf)? "GAME" : "DATA");
    EVMU_LOG_VERBOSE("%-20s: %40s", "Copyable",      EvmuVmi_isProtected(pSelf)? "PROTECTED" : "OK");
    EVMU_LOG_VERBOSE("%-20s: %40u", "File Size",     pSelf->fileSize);
    EVMU_LOG_VERBOSE("%-20s: %40u", "Unknown Field", pSelf->unknown);
    EVMU_LOG_POP(1);

    GblStringBuffer_destruct(&str.buff);
}

EVMU_EXPORT EVMU_RESULT EvmuVmi_load(EvmuVmi* pSelf, const char* pPath) {
    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Loading VMI File [%s].", pPath);
    EVMU_LOG_PUSH();

    memset(pSelf, 0, sizeof(EvmuVmi));

    FILE* pFile = fopen(pPath, "rb");
    GBL_CTX_VERIFY(pFile, GBL_RESULT_ERROR_FILE_OPEN);

    const size_t bytesRead = fread(pSelf, 1, sizeof(EvmuVmi), pFile);
    fclose(pFile);

    GBL_CTX_VERIFY(bytesRead >= EVMU_VMI_FILE_SIZE,
                   GBL_RESULT_ERROR_FILE_READ,
                   "File was insufficiently small: [%zu actual vs %zu expected bytes]",
                   bytesRead,
                   EVMU_VMI_FILE_SIZE);

    if(bytesRead > EVMU_VMI_FILE_SIZE)
        EVMU_LOG_WARN("File was larger than expected: [%zu actual vs %zu expected bytes]",
                      bytesRead,
                      EVMU_VMI_FILE_SIZE);

    EVMU_LOG_POP(1);

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuVmi_save(const EvmuVmi* pSelf, const char* pPath) {
    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Saving VMI File [%s].", pPath);
    EVMU_LOG_PUSH();

    FILE* pFile = fopen(pPath, "w");
    GBL_CTX_VERIFY(pFile, GBL_RESULT_ERROR_FILE_OPEN);

    const size_t bytesWritten = fwrite(pSelf, EVMU_VMI_FILE_SIZE, 1, pFile);
    fclose(pFile);
    GBL_CTX_VERIFY(bytesWritten == EVMU_VMI_FILE_SIZE,
                   GBL_RESULT_ERROR_FILE_WRITE,
                   "Only wrote %zu of expected %zu bytes!",
                   bytesWritten,
                   EVMU_VMI_FILE_SIZE);

    EVMU_LOG_POP(1);

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuVmi_fromVms(EvmuVmi*       pSelf,
                                       const EvmuVms* pVms,
                                       size_t         vmsFileSize,
                                       GblBool        gameType)
{
    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Generating VMI from VMS header.");
    EVMU_LOG_PUSH();

    memset(pSelf, 0, sizeof(EvmuVmi));

    strcpy(pSelf->copyright,       EVMU_VMI_COPYRIGHT_STRING_);
    memcpy(pSelf->description,     pVms->dcDesc,  EVMU_VMS_DC_DESCRIPTION_SIZE);
    memcpy(pSelf->vmsResourceName, pVms->vmuDesc, EVMU_VMI_VMS_RESOURCE_SIZE);
    memcpy(pSelf->fileNameOnVms,   pVms->dcDesc,  EVMU_VMI_VMS_NAME_SIZE);

    GblDateTime dt;
    EvmuTimestamp_setDateTime(&pSelf->creationTimestamp,
                              GblDateTime_nowLocal(&dt));

    pSelf->fileNumber = 1;
    pSelf->vmiVersion = EVMU_VMI_VERSION;
    pSelf->fileSize   = vmsFileSize;
    pSelf->fileMode   = gameType? EVMU_VMI_GAME_MASK : 0;
    pSelf->checksum   = gblHashCrc16BitPartial(pVms, pSelf->fileSize, NULL);

    EvmuVmi_log(pSelf);

    EVMU_LOG_POP(1);

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuVmi_fromDirEntry(EvmuVmi*           pSelf,
                                            const EvmuFat*      pFat,
                                            const EvmuDirEntry* pDirEntry,
                                            const char*         pVmsName)
{
    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Generatinga a VMI from a Directory Entry.");
    EVMU_LOG_PUSH();

    // Grab the VMS header from the directory entry
    const EvmuVms* pVms = EvmuFileManager_vms(EVMU_FILE_MANAGER(pFat), pDirEntry);
    GblDateTime dt;

    // Zero initialize VMI structure
    memset(pSelf, 0, EVMU_VMI_FILE_SIZE);

    // Set description

    memcpy(pSelf->description, pVms->dcDesc, EVMU_VMS_DC_DESCRIPTION_SIZE);
    // Set copyright

    memcpy(pSelf->copyright,
           EVMU_VMI_COPYRIGHT_STRING_,
           strnlen(EVMU_VMI_COPYRIGHT_STRING_,
                   EVMU_VMI_COPYRIGHT_SIZE));

    // Set creation timestamp
    EvmuTimestamp_setDateTime(&pSelf->creationTimestamp,
                              GblDateTime_nowLocal(&dt));
    // Set VMI version
    pSelf->vmiVersion = EVMU_VMI_VERSION;

    // Set file number
    pSelf->fileNumber = 1;

    // SetVMS resource name
    size_t vmsNameLen = strlen(pVmsName);
    if(vmsNameLen > EVMU_VMI_VMS_RESOURCE_SIZE) {
        GBL_CTX_RECORD_SET(GBL_RESULT_TRUNCATED,
                           "Truncating VMS resource name [from %zu to %zu bytes]",
                           vmsNameLen,
                           EVMU_VMI_VMS_RESOURCE_SIZE);
        vmsNameLen = EVMU_VMI_VMS_RESOURCE_SIZE;
    }
    memcpy(pSelf->vmsResourceName, pVmsName, vmsNameLen);

    // Set VMS filename
    memcpy(pSelf->fileNameOnVms, pDirEntry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    // Set file mode
    if(pDirEntry->fileType == EVMU_FILE_TYPE_GAME)
        pSelf->fileMode |= EVMU_VMI_GAME_MASK;
    if(pDirEntry->copyProtection == EVMU_COPY_TYPE_PROTECTED)
        pSelf->fileMode |= EVMU_VMI_PROTECTED_MASK;

    // Set file size
    pSelf->fileSize = (pDirEntry->fileType == EVMU_FILE_TYPE_DATA)?
                          EvmuVms_headerBytes(pVms) + pVms->dataBytes :
                          pDirEntry->fileSize * EvmuFat_blockSize(pFat);

    // Set checkshum
    pSelf->checksum = EvmuVmi_computeChecksum(pSelf);

    EvmuVmi_log(pSelf);

    EVMU_LOG_POP(1);

    GBL_CTX_END();
}

EVMU_EXPORT const char* EvmuVmi_findVmsPath(const EvmuVmi*   pSelf,
                                           const char*      pVmiPath,
                                           GblStringBuffer* pVmsPath)
{
    const char*    pPath    = NULL;
    GblStringList* pStrList = NULL;
    GblStringRef*  pStrRef  = NULL;
    FILE*          pFile    = NULL;

    struct {
        GblStringBuffer buff;
        char            stackStorage[256];
    } str;

    GBL_CTX_BEGIN(NULL);

    GblStringBuffer_construct(&str.buff, GBL_STRV(""), sizeof(str));

    EVMU_LOG_INFO("Attemping to locate VMS file for VMI [%s]",
                  EvmuVmi_fileName(pSelf, &str.buff));
    EVMU_LOG_PUSH();

    if(pVmiPath) {
        EVMU_LOG_VERBOSE("Starting with VMI path [%s]", pVmiPath);
        pStrList = GblStringList_createSplit(pVmiPath, ".");

        if(GblStringList_size(pStrList) < 2) {
            EVMU_LOG_WARN("Didn't even detect an extension?");
        } else {
            GblStringList_erase(pStrList, -1);
        }

        GblStringList_pushBack(pStrList, "vms");
        pStrRef = GblStringList_join(pStrList, ".");

        pFile = fopen(pStrRef, "r");
        if(pFile) {
            GblStringBuffer_set(pVmsPath, GBL_STRV(pStrRef));
            GBL_CTX_DONE();
        }
        fclose(pFile);
        pFile = NULL;

        GblStringRef_unref(pStrRef);
        GblStringList_popBack(pStrList);

        GblStringList_pushBack(pStrList, "VMS");
        pStrRef = GblStringList_join(pStrList, ".");

        pFile = fopen(pStrRef, "r");
        if(pFile) {
            GblStringBuffer_set(pVmsPath, GBL_STRV(pStrRef));
            GBL_CTX_DONE();
        }
        fclose(pFile);
        pFile = NULL;

        GblStringRef_unref(pStrRef);

    }

tryResource:



    EVMU_LOG_POP(1);

    GBL_CTX_END_BLOCK();
    if(pFile) fclose(pFile);
    GblStringBuffer_destruct(&str.buff);
    GblStringList_destroy(pStrList);
    GblStringRef_unref(pStrRef);
    return pPath;
}

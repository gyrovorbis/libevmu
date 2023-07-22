#include "gyro_vmu_flash.h"
#include "gyro_vmu_vmi.h"
#include "gyro_vmu_vms.h"
#include "gyro_vmu_lcd.h"
#include "gyro_vmu_extra_bg_pvr.h"
#include "gyro_vmu_icondata.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#include <gimbal/preprocessor/gimbal_macro_utils.h>

#include "hw/evmu_device_.h"
#include "hw/evmu_ram_.h"
#include "fs/evmu_fat_.h"
#include <evmu/fs/evmu_vmi.h>
#include <evmu/fs/evmu_nexus.h>

#define GYRO_PATH_MAX_SIZE  1024

static char _lastErrorMsg[VMU_FLASH_LOAD_IMAGE_ERROR_MESSAGE_SIZE] = { '\0' };


static EvmuDirEntry* EvmuFileManager_alloc_(EvmuFileManager* pSelf, const EvmuNewFileInfo* pInfo, const void* pData, VMU_LOAD_IMAGE_STATUS* pStatus) {
    EvmuDirEntry* pEntry = NULL;
    GBL_CTX_BEGIN(NULL);
    pEntry = EvmuFileManager_alloc(pSelf, pInfo, pData);
    GBL_CTX_VERIFY_LAST_RECORD();
    GBL_CTX_END_BLOCK();
    if(GBL_RESULT_SUCCESS(GBL_CTX_RESULT()))
        *pStatus = VMU_LOAD_IMAGE_SUCCESS;
    else *pStatus = VMU_LOAD_IMAGE_OPEN_FAILED;
    return pEntry;
}

const char* gyVmuFlashLastErrorMessage(void) {
    return _lastErrorMsg;
}

int gyVmuFlashIsIconDataVms(const struct EvmuDirEntry* entry) {
    return (memcmp(entry->fileName, VMU_ICONDATA_VMS_FILE_NAME, EVMU_DIRECTORY_FILE_NAME_SIZE) == 0);
}

int gyVmuFlashIsExtraBgPvr(const struct EvmuDirEntry* entry) {
    return (memcmp(entry->fileName, GYRO_VMU_EXTRA_BG_PVR_FILE_NAME, EVMU_DIRECTORY_FILE_NAME_SIZE) == 0);
}

uint8_t* gyVmuFlashLoadVMS(const char *vmspath, size_t* fileSize) {
    assert(vmspath);
    uint8_t* vmsData = NULL;
    if(fileSize) *fileSize = 0;

    EVMU_LOG_VERBOSE("Loading VMS image into buffer.", vmspath);
    EVMU_LOG_PUSH();

    FILE* vmsFp = fopen(vmspath, "rb");
    if(/*!retVal ||*/ !vmsFp) {
        EVMU_LOG_ERROR("Could not open VMS file: [%s]", vmspath);
        goto end;
    }

    size_t fileBytes = 0;
    fseek(vmsFp, 0, SEEK_END); // seek to end of file
    fileBytes = ftell(vmsFp); // get current file pointer
    fseek(vmsFp, 0, SEEK_SET); // seek back to beginning of file
    if(/*!retVal ||*/ !fileBytes) {
        EVMU_LOG_ERROR("Could not retrieve VMS file length");
        goto cleanup_file;
    }

    vmsData = malloc(fileBytes);
    size_t bytesRead = 0;
    bytesRead = fread(vmsData, 1, fileBytes, vmsFp);
    if(fileSize) *fileSize = bytesRead;
    if(/*!retVal ||*/ bytesRead != fileBytes) {
        EVMU_LOG_ERROR("Could not read entire file contents! [bytes read %d/%d]", bytesRead, fileBytes);
        goto cleanup_data;
    }

    goto cleanup_file;

cleanup_data:
    free(vmsData);
    vmsData = NULL;
cleanup_file:
    fclose(vmsFp);
end:

    EVMU_LOG_POP(1);
    return vmsData;
}


int gyVmuFlashFileRead(EvmuDevice* dev, const EvmuDirEntry* entry, unsigned char* buffer, int includeHeader) {
    size_t bytesRead = 0;
    const size_t byteSize = entry->fileSize * EvmuFat_blockSize(dev->pFat);
    bytesRead = EvmuFileManager_read(dev->pFat,
                                        entry,
                                        buffer,
                                        byteSize,
                                        includeHeader?  0 : entry->headerOffset * EvmuFat_blockSize(dev->pFat), includeHeader);
    return (bytesRead == byteSize)? 1 : 0;
}

void gyVmuFlashNewFilePropertiesFromVmi(VMUFlashNewFileProperties* fileProperties, const VMIFileInfo* vmi) {
    assert(fileProperties && vmi);

    memcpy(fileProperties->fileName, vmi->fileNameOnVms, EVMU_DIRECTORY_FILE_NAME_SIZE);
    fileProperties->fileSizeBytes = vmi->fileSize;

    switch((vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK) >> VMU_VMI_FILE_INFO_FILE_MODE_GAME_POS) {
    case VMI_FILE_MODE_GAME_DATA:
        fileProperties->fileType = EVMU_FILE_TYPE_DATA;
        break;
    case VMI_FILE_MODE_GAME_GAME:
        fileProperties->fileType = EVMU_FILE_TYPE_GAME;
        break;
    default:
        fileProperties->fileType = EVMU_FILE_TYPE_NONE;
    }

    switch((vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_MASK) >> VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_POS) {
    case VMI_FILE_MODE_PROTECTED_COPY_OK:
        fileProperties->copyProtection = EVMU_COPY_ALLOWED;
        break;
    case VMI_FILE_MODE_PROTECTED_COPY_PROTECTED:
        fileProperties->copyProtection = EVMU_COPY_PROTECTED;
        break;
    default:
        fileProperties->copyProtection = EVMU_COPY_UNKNOWN;
    }
}

void gyVmuFlashNewFilePropertiesFromDirEntry(VMUFlashNewFileProperties* fileProperties, const EvmuDirEntry* entry) {
    assert(fileProperties && entry);

    memcpy(fileProperties->fileName, entry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);
    fileProperties->fileSizeBytes = entry->fileSize * EVMU_FAT_BLOCK_SIZE;
    fileProperties->fileType = entry->fileType;
    fileProperties->copyProtection = entry->copyProtection;
}

int gyVmuVmiFindVmsPath(const char* vmiPath, char* vmsPath) {
    int success = 0;
    char basePath[GYRO_PATH_MAX_SIZE] = { '\0' };
    char tmpVmiPath[GYRO_PATH_MAX_SIZE] = { '\0' };

    EVMU_LOG_VERBOSE("Trying to find VMS file corresponding VMI: [%s]", vmiPath);
    EVMU_LOG_PUSH();

    strcpy(tmpVmiPath, vmiPath);
    const char*curTok  = strtok(tmpVmiPath, ".");
    strcpy(basePath, curTok);

    strcpy(vmsPath, basePath);
    strcat(vmsPath, ".vms");
    EVMU_LOG_VERBOSE("Trying the same filename with .VMS extension: [%s]", vmsPath);
    EVMU_LOG_PUSH();
    FILE* fp = fopen(vmsPath, "rb");
    uintptr_t wasOpen = (uintptr_t)fp;
    if(fp) fclose(fp);

    if(/*retVal &&*/ wasOpen) {
        EVMU_LOG_VERBOSE("Success! File exists.");
        EVMU_LOG_POP(1);
        success = 1;
        goto end;
    }
    EVMU_LOG_VERBOSE("Nope.");
    EVMU_LOG_POP(1);
    VMIFileInfo vmiHeader;
    char vmsFileName[VMU_VMI_FILE_INFO_VMS_RESOURCE_NAME_SIZE] = { '\0' };

    if(EvmuVmi_load(&vmiHeader, vmiPath)) {
        gyVmuVmiFileInfoResourceNameGet(&vmiHeader, vmsFileName);

        for(int i = strlen(basePath)-1; i >= 0; --i) {
            if(basePath[i] == '\\' || basePath[i] == '/') {
                basePath[i+1] = '\0';
                break;
            }
        }

        strcat(basePath, vmsFileName);
        strcat(basePath, ".vms");

        EVMU_LOG_VERBOSE("Trying filename referenced by VM file: [%s]", basePath);
        EVMU_LOG_PUSH();

        fp = fopen(basePath, "rb");
        wasOpen = (uintptr_t)fp;
        fclose(fp);

        if(/*retVal &&*/ wasOpen) {
            EVMU_LOG_VERBOSE("Success. File exists.");
            strcpy(vmsPath, basePath);
            success = 1;

        } else {
            success = 0;
        }
        EVMU_LOG_POP(1);

    } else {
        success = 0;
    }

end:
    if(!success) EVMU_LOG_WARN("None found!");
    else EVMU_LOG_VERBOSE("Found VMS: [%s]", vmsPath);
    EVMU_LOG_POP(1);
    return success;
}

int gyVmuVmsFindVmiPath(const char* vmsPath, char* vmiPath) {
    assert(vmsPath);
    int success = 0;
    char basePath[GYRO_PATH_MAX_SIZE] = { '\0' };
    char tmpVmsPath[GYRO_PATH_MAX_SIZE] = { '\0' };
    EVMU_LOG_VERBOSE("Trying to find VMI file corresponding VMS: [%s]", vmsPath);
    EVMU_LOG_PUSH();

    strcpy(tmpVmsPath, vmsPath);
    const char*curTok  = strtok(tmpVmsPath, ".");
    strcpy(basePath, curTok);

    strcat(basePath, ".vmi");
    FILE* fp = fopen(basePath, "rb");
    int wasOpen = (uintptr_t)fp;
    if(fp) fclose(fp);

    if(/*!retVal ||*/ !wasOpen) {
        EVMU_LOG_WARN("None found!");
        success = 0;
    } else {
        strcpy(vmiPath, basePath);
        EVMU_LOG_VERBOSE("Found VMI: [%s]", vmiPath);
        success = 1;
    }
    EVMU_LOG_POP(1);
    return success;
}

#if 0
EvmuDirEntry* gyVmuFlashLoadImage(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    char tempPath[GYRO_PATH_MAX_SIZE];
    char basePath[GYRO_PATH_MAX_SIZE];
    char fileName[GYRO_PATH_MAX_SIZE];
    EvmuDirEntry* entry = NULL;

    //Extract file name
    int nameStartPos = strlen(path)-1;
    for(; nameStartPos >= 0; --nameStartPos) {
        if(path[nameStartPos] == '/' || path[nameStartPos] == '\\') {
           ++nameStartPos;
            break;
        }
    }

    strcpy(fileName, &path[nameStartPos]);

    strcpy(tempPath, path);
    const char*curTok  = strtok(tempPath, ".");
    strcpy(basePath, curTok);
    const char*prevTok = NULL;
    char ext[100] = { 0 };

    EVMU_LOG_VERBOSE("Loading Generic Flash Image [%s]", path);
    EVMU_LOG_PUSH();
#if 0
    if(dev->lcdFile) {
        gyVmuLcdFileUnload(dev->lcdFile);
        dev->lcdFile = NULL;
    }
#endif
    //Load files with special file names
    if(strcmp(fileName, VMU_ICONDATA_VMS_FILE_NAME) == 0 ||
            strcmp(fileName, "ICONDATA.VMS") == 0)
    {
        entry = gyVmuFlashLoadIconDataVms(dev, path, status);

    } else {    //Load based on file extension

        while(curTok) {
            prevTok = curTok;
            curTok = strtok(NULL, ".");
        }

        if(prevTok) {
            for(unsigned i = 0; i < strlen(prevTok); ++i)
                ext[i] = tolower(prevTok[i]);

            if(!strcmp(ext, "bin") || !strcmp(ext, "vmu")) {
                entry = gyVmuFlashLoadImageBin(dev, path, status);
            } else if(!strcmp(ext, "dcm")) {
                entry = gyVmuFlashLoadImageDcm(dev, path, status);
            } else if(!strcmp(ext, "dci")) {
                entry = gyVmuFlashLoadImageDci(dev, path, status);

            } else if(!strcmp(ext, "vms")) {
                //We have to find the corresponding VMI file
                char vmiPath[GYRO_PATH_MAX_SIZE];
                if(gyVmuVmsFindVmiPath(path, vmiPath)) {
                    entry = gyVmuFlashLoadImageVmiVms(dev, vmiPath, path, status);
                } else {
                    *status = VMU_LOAD_IMAGE_VMS_NO_VMI;
                }
            } else if(!strcmp(ext, "vmi")) {
                //We have to find the corresponding VMS file
                char vmsPath[GYRO_PATH_MAX_SIZE];
                if(gyVmuVmiFindVmsPath(path, vmsPath)) {
                    entry = gyVmuFlashLoadImageVmiVms(dev, path, vmsPath, status);
                } else {
                    *status = VMU_LOAD_IMAGE_VMI_NO_VMS;
                }

            } else {
                *status = VMU_LOAD_IMAGE_UNKNOWN_FORMAT;
            }

        } else {
            EVMU_LOG_ERROR("No extension found on file!");

        }
    }

    EVMU_LOG_POP(1);
    return entry;

}
#endif

EvmuDirEntry* gyVmuFlashCreateFileVmiVms(EvmuDevice* dev, const struct VMIFileInfo* vmi, const uint8_t* vms, VMU_LOAD_IMAGE_STATUS* status) {
    assert(dev && vmi && vms);

    EVMU_LOG_VERBOSE("Creating new flash file from raw VMI and VMS data.");
    EVMU_LOG_PUSH();
    gyVmuFlashPrintVMIFileInfo(vmi);

    const VMSFileInfo* vmsHeader = (VMSFileInfo*)(((vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK)>>VMU_VMI_FILE_INFO_FILE_MODE_GAME_POS == VMI_FILE_MODE_GAME_GAME)?
                &vms[EVMU_FAT_GAME_VMS_HEADER_OFFSET] :
                vms);
    gyVmuPrintVMSFileInfo(vmsHeader);

    VMUFlashNewFileProperties fileProperties;
    gyVmuFlashNewFilePropertiesFromVmi(&fileProperties, vmi);

    //If this is a special kind of file, lets print its extra data and shit
    if(memcmp(fileProperties.fileName, GYRO_VMU_EXTRA_BG_PVR_FILE_NAME, sizeof(fileProperties.fileName)) == 0) {
        VmuExtraBgPvrFileInfo payload;
        gyVmuExtraBgPvrFileInfo(vmsHeader, &payload);
        gyVmuExtraBgPvrFileInfoPrint(&payload);
    }

    EvmuDirEntry* dirEntry = EvmuFileManager_alloc_(dev->pFileMgr, &fileProperties, vms, status);

    EVMU_LOG_POP(1);

    return dirEntry;
}

EvmuDirEntry* gyVmuFlashLoadImageVmiVms(EvmuDevice* dev, const char* vmipath, const char* vmspath, VMU_LOAD_IMAGE_STATUS* status) {
    EvmuDirEntry* dirEntry = NULL;
    EVMU_LOG_VERBOSE("Load VMI+VMS file pair: [vmi: %s, vms: %s]", vmipath, vmspath);
    EVMU_LOG_PUSH();

    size_t vmsSize = 0;
    uint8_t* vms = gyVmuFlashLoadVMS(vmspath, &vmsSize);
    if(!vms) {
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        goto end;
    }

    EvmuVmi vmi;
    if(!EvmuVmi_load(&vmi, vmipath)) {
        if(!EvmuVmi_fromVmsFile(&vmi, vms, vmsSize)) {
            *status = VMU_LOAD_IMAGE_OPEN_FAILED;
            goto end;
        }
    }

    dirEntry = gyVmuFlashCreateFileVmiVms(dev, &vmi, vms, status);

    free(vms);

end:
    EVMU_LOG_POP(1);
    return dirEntry;
}


EvmuDirEntry* gyVmuFlashLoadImageDcm(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(dev);
    EvmuFlash_*  pFlash_  = pDevice_->pFlash;
    FILE* file = NULL;

    EVMU_LOG_VERBOSE("Loading DCM Flash Image from file: [%s]", path);
    EVMU_LOG_PUSH();

    if (!(file = fopen(path, "rb"))) {
        EVMU_LOG_ERROR("Could not open binary file for reading!");
        EVMU_LOG_POP(1);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        return NULL;
    }

    //Clear ROM
    memset(pFlash_->pStorage->pData, 0, pFlash_->pStorage->size);

    size_t bytesRead   = 0;
    size_t bytesTotal  = 0;

    size_t fileLen = 0;
    fseek(file, 0, SEEK_END); // seek to end of file
    fileLen = ftell(file); // get current file pointer
    fseek(file, 0, SEEK_SET); // seek back to beginning of file

    size_t toRead = fileLen < pFlash_->pStorage->size? fileLen : pFlash_->pStorage->size;

    if(fileLen != pFlash_->pStorage->size) {
        EVMU_LOG_WARN("File size does not match flash size. Probaly not a legitimate image. [File Size: %u, Flash Size: %u]",
                      fileLen,
                      pFlash_->pStorage->size);
    }

    int retVal = fread(pFlash_->pStorage->pData, 1, toRead, file);

    if(!retVal || toRead != bytesRead) {
        EVMU_LOG_ERROR("All bytes were not read properly! [Bytes Read: %u/%u]", bytesRead, toRead);
    }

    fclose(file);

    EvmuNexus_applyByteOrdering(pFlash_->pStorage->pData, EVMU_FLASH_SIZE);

    EVMU_LOG_VERBOSE("Read %d bytes.", bytesTotal);
    //assert(bytesTotal >= 0);
    //assert(bytesTotal == sizeof(pDevice_->pMemory->flash));

    *status = VMU_LOAD_IMAGE_SUCCESS;
    EvmuFat_log(dev->pFat);

    if(!EvmuFat_isFormatted(dev->pFat)) {
        strncpy(_lastErrorMsg, "Root Block does not contain the proper format sequence!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_FLASH_UNFORMATTED;
    }

    EVMU_LOG_POP(1);

    return NULL;
}

EvmuDirEntry* gyVmuFlashLoadImageDci(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    EvmuDirEntry tempEntry;
    EvmuDirEntry* entry = NULL;
    uint8_t dataBuffer[EVMU_FLASH_SIZE] = { 0 };

    EVMU_LOG_VERBOSE("Loading DCI image from file: [%s]", path);
    EVMU_LOG_PUSH();

    FILE* fp = NULL;
    if(!(fp = fopen(path, "rb"))) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Failed to open file!");
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        goto end;
    }

    size_t bytesRead = 0;
    if((bytesRead = fread(&tempEntry, 1, sizeof(EvmuDirEntry), fp) )
            != sizeof(EvmuDirEntry))
    {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Failed to read directory entry! [Bytes read: %zu/%zu]",
                 bytesRead,
                 sizeof(EvmuDirEntry));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_READ_FAILED;
        goto cleanup_file;
    }

    if(tempEntry.fileType == EVMU_FILE_TYPE_GAME && EvmuFileManager_game(dev->pFat)) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Only one GAME file may be present at a time, and the current image already has one!");
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_GAME_DUPLICATE;
        goto cleanup_file;
    }

    EvmuFatUsage memUsage;
    EvmuFat_usage(dev->pFat, &memUsage);
    if(memUsage.blocksFree < tempEntry.fileSize) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Not enough free blocks available on device. [Available: %d, Required: %d]",
                 memUsage.blocksFree,
                 tempEntry.fileSize);
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS;
        goto cleanup_file;
    }

#if 0
    if(gyEvmuDirEntryFind(dev, tempEntry.fileName)) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "A file already exists with the same name!");
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_NAME_DUPLICATE;
        goto cleanup_file;
    }
#endif

    if(!(entry = EvmuFat_dirEntryAlloc(dev->pFat, tempEntry.fileType))) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Failed to allocate new Flash Directory Entry!");
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_FILES_MAXED;
        goto cleanup_file;
    }

    const size_t fileSize = tempEntry.fileSize * EvmuFat_blockSize(dev->pFat);
    if((bytesRead = fread(dataBuffer, 1, fileSize, fp)) != fileSize)
    {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Failed to read entire contents of file. [Bytes read: %zu/%zu]",
                 bytesRead,
                 fileSize);
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_READ_FAILED;
        goto cleanup_file;
    }

    EvmuNexus_applyByteOrdering(dataBuffer, tempEntry.fileSize * EvmuFat_blockSize(dev->pFat));

    VMUFlashNewFileProperties properties;
    gyVmuFlashNewFilePropertiesFromDirEntry(&properties, &tempEntry);

    entry = EvmuFileManager_alloc_(dev->pFileMgr, &properties, dataBuffer, status);

cleanup_file:
    if(fclose(fp)) {
        EVMU_LOG_WARN("File was not closed gracefully for some reason...");
    }
end:
    EVMU_LOG_POP(1);
    return entry;
}

int gyVmuFlashExportImage(const EvmuDevice* dev, const char* path) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(dev);
    FILE* fp = NULL;
    int success = 1;

    EVMU_LOG_VERBOSE("Exporting VMU Flash image: [%s]", path);
    EVMU_LOG_PUSH();

    fp = fopen(path, "wb");

    if(/*!retVal ||*/ !fp) {
        EVMU_LOG_ERROR("Failed to create the file!");
        success = 0;
        goto end;
    }

    size_t bytesWritten = fwrite(pDevice_->pFlash->pStorage->pData, sizeof(char), EVMU_FLASH_SIZE, fp);

    if(bytesWritten < EVMU_FLASH_SIZE) {
        EVMU_LOG_ERROR("Couldn't write entire file! [bytes written %d/%d]", bytesWritten, EVMU_FLASH_SIZE);
        success = 0;
    }

    if(fclose(fp) || !success) {
        EVMU_LOG_ERROR("Could not gracefully close file.");
    }

end:
    EVMU_LOG_POP(1);
    return success;
}



int gyVmuFlashExportDcm(const EvmuDevice* dev, const char* path) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(dev);
    uint8_t data[EVMU_FLASH_SIZE];
    FILE* fp = NULL;
    int success = 1;

    EVMU_LOG_VERBOSE("Exporting DCM (Nexus) Flash Image: [%s]", path);
    EVMU_LOG_PUSH();

    fp = fopen(path, "wb");;

    if(/*!retVal ||*/ !fp) {
        EVMU_LOG_ERROR("Failed to create the file!");
        success = 0;
        goto end;
    }

    memcpy(data, pDevice_->pFlash->pStorage->pData, EVMU_FLASH_SIZE);
    EvmuNexus_applyByteOrdering(data, EVMU_FLASH_SIZE);

    size_t bytesWritten = fwrite(data, sizeof(char), EVMU_FLASH_SIZE, fp);

    if(bytesWritten < EVMU_FLASH_SIZE) {
        EVMU_LOG_ERROR("Couldn't write entire file! [bytes written %d/%d]", bytesWritten, EVMU_FLASH_SIZE);
        success = 0;
    }

    if(fclose(fp) || !success) {
        EVMU_LOG_ERROR("Could not gracefully close file.");
    }

end:
    EVMU_LOG_POP(1);
    return success;
}




EvmuDirEntry* gyEvmuDirEntryIconData(EvmuDevice* dev) {
    return EvmuFileManager_find(dev->pFileMgr, VMU_ICONDATA_VMS_FILE_NAME);
}

EvmuDirEntry* gyEvmuDirEntryExtraBgPvr(EvmuDevice* dev) {
    return EvmuFileManager_find(dev->pFileMgr, GYRO_VMU_EXTRA_BG_PVR_FILE_NAME);
}


EvmuDirEntry* gyVmuFlashLoadIconDataVms(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    EvmuDirEntry*   entry       = NULL;
    uint8_t*            dataBuffer  = NULL;
    size_t              bytesRead;

    EVMU_LOG_VERBOSE("Loading ICONDATA_VMS File [%s].", path);
    EVMU_LOG_PUSH();

   const EvmuDirEntry* dirEntry = gyEvmuDirEntryIconData(dev);

    if(dirEntry) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "An ICONDATA.VMS file is already present in flash. [File Index: %zu, First Block: %u, Size: %d]",
                 EvmuFat_dirEntryIndex(dev->pFat, dirEntry),
                 dirEntry->firstBlock,
                 dirEntry->fileSize);
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        *status = VMU_LOAD_IMAGE_NAME_DUPLICATE;
        goto done;
    }

    FILE* file = fopen(path, "rb");

    if(!file) {
        snprintf(_lastErrorMsg,
                 sizeof(_lastErrorMsg),
                 "Could not open binary file for reading!");
        EVMU_LOG_ERROR("%s", _lastErrorMsg);

    } else {
        size_t fileSize = 0;
        fseek(file, 0, SEEK_END); // seek to end of file
        fileSize = ftell(file); // get current file pointer
        fseek(file, 0, SEEK_SET); // seek back to beginning of file
        fclose(file);
        if(/*!retVal ||*/ !fileSize) {
            snprintf(_lastErrorMsg,
                     sizeof(_lastErrorMsg),
                     "Could not determine file size! [0 bytes]");
            EVMU_LOG_ERROR("%s", _lastErrorMsg);
            *status = VMU_LOAD_IMAGE_READ_FAILED;
            goto done;
        }

        dataBuffer = malloc(sizeof(uint8_t)*fileSize);

        bytesRead = fread(dataBuffer, 1, fileSize, file);
        fclose(file);

       if(!bytesRead) {
           snprintf(_lastErrorMsg,
                    sizeof(_lastErrorMsg),
                    "Could not read from file!");
            EVMU_LOG_ERROR("%s", _lastErrorMsg);
            *status = VMU_LOAD_IMAGE_READ_FAILED;
        } else {
            if(bytesRead != fileSize) {
                EVMU_LOG_WARN("Could not actually read entirety of file, but continuing anyway [%d/%d].", bytesRead, fileSize);
                *status = VMU_LOAD_IMAGE_READ_FAILED;
            }

            gyVmuIconDataPrint((const IconDataFileInfo*)dataBuffer);

            VMUFlashNewFileProperties properties;
            gyVmuFlashNewFilePropertiesFromIconDataVms(&properties, bytesRead);
            entry = EvmuFileManager_alloc_(dev->pFileMgr, &properties, dataBuffer, status);
        }
    }

    free(dataBuffer);
done:
    EVMU_LOG_POP(1);
    return entry;

}


void gyVmuFlashNewFilePropertiesFromIconDataVms(VMUFlashNewFileProperties* fileProperties, size_t byteSize) {
    assert(fileProperties && byteSize);

    memcpy(fileProperties->fileName, VMU_ICONDATA_VMS_FILE_NAME, sizeof(VMU_ICONDATA_VMS_FILE_NAME));
    fileProperties->fileSizeBytes   = byteSize;
    fileProperties->fileType        = EVMU_FILE_TYPE_DATA;
    fileProperties->copyProtection  = 0;
}


int gyVmuFlashExportVms(const EvmuDevice* dev, const struct EvmuDirEntry* entry, const char* path) {
    int success = 1;
    assert(dev && entry && path);

    char entryName[EVMU_DIRECTORY_FILE_NAME_SIZE+1] = { '\0' };
    size_t fileSize = entry->fileSize * EVMU_FAT_BLOCK_SIZE;

    memcpy(entryName, entry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    EVMU_LOG_VERBOSE("Exporting file [%s] to VMS file: [%s]", entryName, path);
    EVMU_LOG_PUSH();


    VMSFileInfo* vmsImg = malloc(sizeof(uint8_t) * fileSize);

    if(!gyVmuFlashFileRead((EvmuDevice*)dev, entry, (unsigned char*)vmsImg, 1)) {
        strncpy(_lastErrorMsg, "Failed to retrieve the file from flash!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto free_img;
    }

    FILE* fp = fopen(path, "wb");
    if(/*!retVal ||*/ !fp) {
        strncpy(_lastErrorMsg, "Failed to create the file!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto free_img;
    }

    if(!fwrite(vmsImg, fileSize, 1, fp)) {
        strncpy(_lastErrorMsg, "Failed to write all bytes to the file.", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
    }

    fclose(fp);

free_img:
    free(vmsImg);
    EVMU_LOG_POP(1);
    return success;

}


int gyVmuFlashExportRaw(const EvmuDevice* dev, const struct EvmuDirEntry* entry, const char* path) {
    int success = 1;
    assert(dev && entry && path);

    char entryName[EVMU_DIRECTORY_FILE_NAME_SIZE+1] = { '\0' };
    size_t fileSize = entry->fileSize * EVMU_FAT_BLOCK_SIZE;

    memcpy(entryName, entry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    EVMU_LOG_VERBOSE("Exporting file [%s] to Raw Binary file: [%s]", entryName, path);
    EVMU_LOG_PUSH();

    assert(entry->fileType == EVMU_FILE_TYPE_DATA);

    VMSFileInfo* vmsImg = malloc(sizeof(uint8_t) * fileSize);

    if(!gyVmuFlashFileRead((EvmuDevice*)dev, entry, (unsigned char*)vmsImg, 1)) {
        strncpy(_lastErrorMsg, "Failed to retrieve the file from flash!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto free_img;
    }

    FILE* fp = fopen(path, "wb");
    if(/*!retVal ||*/ !fp) {
        strncpy(_lastErrorMsg, "Failed to create the file!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto free_img;
    }

    size_t headerSize   = gyVmuVmsFileInfoHeaderSize(vmsImg);
    size_t bytesToWrite = fileSize - headerSize;


    if(!fwrite(((uint8_t*)vmsImg + headerSize), bytesToWrite, 1, fp)) {
        strncpy(_lastErrorMsg, "Failed to write all bytes to the file.", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
    }

    fclose(fp);

free_img:
    free(vmsImg);

    EVMU_LOG_POP(1);
    return success;

}


int gyVmuFlashExportVmi(const EvmuDevice* dev, const EvmuDirEntry* entry, const char* path) {
    int success = 1;
    assert(dev && entry && path);

    char tempFilePath[GYRO_PATH_MAX_SIZE] = { '\0' };
    char entryName[EVMU_DIRECTORY_FILE_NAME_SIZE+1] = { '\0' };
    size_t fileSize = sizeof(VMIFileInfo);
    memcpy(entryName, entry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    EVMU_LOG_VERBOSE("Exporting file [%s] to VMI file: [%s]", entryName, path);
    EVMU_LOG_PUSH();

    strncpy(tempFilePath, path, GYRO_PATH_MAX_SIZE);
    char* curTok  = strtok(tempFilePath, "/\\");
    char* prevTok  = NULL;

    while(curTok) {
        prevTok = curTok;
        curTok = strtok(NULL, "/\\");
    }
    assert(prevTok);
    prevTok = strtok(prevTok, ".");
    assert(prevTok);

    EVMU_LOG_VERBOSE("Extracted resource name: %s", prevTok);

    VMIFileInfo vmi;
    EvmuVmi_fromDirEntry(&vmi, dev->pFat, entry, prevTok);

    FILE* fp = fopen(path, "wb");
    if(/*!retVal || */!fp) {
        strncpy(_lastErrorMsg, "Failed to create the file!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto done;
    }

    if(!fwrite(&vmi, fileSize, 1, fp)) {
        strncpy(_lastErrorMsg, "Failed to write all bytes to the file.", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
    }

    fclose(fp);
done:
    EVMU_LOG_POP(1);
    return success;

}

int gyVmuFlashExportDci(const EvmuDevice* dev, const EvmuDirEntry* entry, const char* path) {
    int success = 1;
    assert(dev && entry && path);

    char entryName[EVMU_DIRECTORY_FILE_NAME_SIZE+1] = { '\0' };
    size_t paddedFileSize = entry->fileSize * EVMU_FAT_BLOCK_SIZE;

    memcpy(entryName, entry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE);

    EVMU_LOG_VERBOSE("Exporting file [%s] to DCI file: [%s]", entryName, path);
    EVMU_LOG_PUSH();

    size_t          dataSize   = sizeof(uint8_t) * paddedFileSize + sizeof(EvmuDirEntry);
    uint8_t*        data       = malloc(dataSize + dataSize%4? 4-dataSize%4 : 0);
    uint8_t*        vms        = data + sizeof(EvmuDirEntry);

    memset(data, 0, dataSize);
    memcpy(data, entry, sizeof(EvmuDirEntry));

    if(!gyVmuFlashFileRead((EvmuDevice*)dev, entry, vms, 1)) {
        strncpy(_lastErrorMsg, "Failed to retrieve the file from flash!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto free_img;
    }

    size_t bytesToWrite   = dataSize;// + sizeof(EvmuDirEntry);

    FILE* fp = fopen(path, "wb");
    if(/*!retVal ||*/ !fp) {
        strncpy(_lastErrorMsg, "Failed to create the file!", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
        goto free_img;
    }

    EvmuNexus_applyByteOrdering(vms, dataSize);

    if(!fwrite(data, 1, bytesToWrite, fp)) {
        strncpy(_lastErrorMsg, "Failed to write all bytes to the file.", sizeof(_lastErrorMsg));
        EVMU_LOG_ERROR("%s", _lastErrorMsg);
        success = 0;
    }


    fclose(fp);

free_img:
    free(data);

    EVMU_LOG_POP(1);
    return success;

}



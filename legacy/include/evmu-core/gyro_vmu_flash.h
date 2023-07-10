#ifndef GYRO_VMU_FLASH_H
#define GYRO_VMU_FLASH_H

#include <stdint.h>
#include <stddef.h>

#include <evmu/hw/evmu_device.h>
#include <evmu/fs/evmu_fat.h>
#include "gyro_vmu_vms.h"

GBL_DECLS_BEGIN

#define VMU_FLASH_LOAD_IMAGE_ERROR_MESSAGE_SIZE     256
#define VMU_FLASH_VMI_EXPORT_COPYRIGHT_STRING       "Created with ElysianVMU"

struct VMIFileInfo;

typedef struct VMUFlashNewFileProperties {
    char            fileName[EVMU_DIRECTORY_FILE_NAME_SIZE];
    size_t          fileSizeBytes;
    uint8_t         fileType;
    uint8_t         copyProtection;
} VMUFlashNewFileProperties;

typedef enum VMU_LOAD_IMAGE_STATUS {
    VMU_LOAD_IMAGE_SUCCESS,
    VMU_LOAD_IMAGE_OPEN_FAILED,
    VMU_LOAD_IMAGE_READ_FAILED,
    VMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS,
    VMU_LOAD_IMAGE_FILES_MAXED,
    VMU_LOAD_IMAGE_DEVICE_READ_ERROR,
    VMU_LOAD_IMAGE_DEVICE_WRITE_ERROR,
    VMU_LOAD_IMAGE_DEFRAG_FAILED,
    VMU_LOAD_IMAGE_VMS_NO_VMI,
    VMU_LOAD_IMAGE_VMI_NO_VMS,
    VMU_LOAD_IMAGE_GAME_DUPLICATE,
    VMU_LOAD_IMAGE_NAME_DUPLICATE,
    VMU_LOAD_IMAGE_UNKNOWN_FORMAT,
    VMU_LOAD_IMAGE_FLASH_UNFORMATTED
} VMU_LOAD_IMAGE_STATUS;

//Mid-level Directory API

EvmuDirEntry* gyEvmuDirEntryIconData(EvmuDevice* dev);
EvmuDirEntry* gyEvmuDirEntryExtraBgPvr(EvmuDevice* dev);

//High-level File API
EvmuDirEntry* gyVmuFlashFileCreate(EvmuDevice* dev, const VMUFlashNewFileProperties* properties, const unsigned char* data, VMU_LOAD_IMAGE_STATUS* status);

void gyVmuFlashNexusByteOrder(uint8_t* data, size_t bytes);

int gyVmuFlashFileRead(EvmuDevice* dev, const EvmuDirEntry* entry, unsigned char* buffer, int includeHeader);

void gyVmuFlashNewFilePropertiesFromVmi(VMUFlashNewFileProperties* fileProperties, const struct VMIFileInfo* vmi);
void gyVmuFlashNewFilePropertiesFromDirEntry(VMUFlashNewFileProperties* fileProperties, const struct EvmuDirEntry* entry);
void gyVmuFlashNewFilePropertiesFromIconDataVms(VMUFlashNewFileProperties* fileProperties, size_t byteSize);

void gyVmuFlashVmiFromDirEntry(struct VMIFileInfo* vmi, const EvmuDevice* dev, const EvmuDirEntry* entry, const char* vmsResourceName);

uint16_t gyVmuFlashFileCalculateCRC(const EvmuDevice* pDev, const EvmuDirEntry* dirEntry);

int gyVmuFlashIsIconDataVms(const struct EvmuDirEntry* entry);
int gyVmuFlashIsExtraBgPvr(const struct EvmuDirEntry* entry);

uint8_t* gyVmuFlashLoadVMS(const char* path, size_t* fileSize);
int gyVmuFlashLoadVMI(struct VMIFileInfo* info, const char* path);

int gyVmuVmiFindVmsPath(const char* vmiPath, char* vmsPath);
int gyVmuVmsFindVmiPath(const char* vmsPath, char* vmiPath);

int gyVmuFlashExportImage(const EvmuDevice* dev, const char* path);
int gyVmuFlashExportDcm(const EvmuDevice* dev, const char* path);
int gyVmuFlashExportVms(const EvmuDevice* dev, const EvmuDirEntry* entry, const char* path);
int gyVmuFlashExportVmi(const EvmuDevice* dev, const EvmuDirEntry* entry, const char* path);
int gyVmuFlashExportDci(const EvmuDevice* dev, const EvmuDirEntry* entry, const char* path);
int gyVmuFlashExportRaw(const EvmuDevice* dev, const EvmuDirEntry* entry, const char* path);

const char* gyVmuFlashLastErrorMessage(void);
EvmuDirEntry* gyVmuFlashLoadImage(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
EvmuDirEntry* gyVmuFlashLoadImageDcm(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
EvmuDirEntry* gyVmuFlashLoadImageDci(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
EvmuDirEntry* gyVmuFlashLoadImageBin(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
EvmuDirEntry* gyVmuFlashLoadImageVmiVms(EvmuDevice* dev, const char* vmipath, const char* vmspath, VMU_LOAD_IMAGE_STATUS* status);
EvmuDirEntry* gyVmuFlashCreateFileVmiVms(EvmuDevice* dev, const struct VMIFileInfo* vmi, const uint8_t* vms, VMU_LOAD_IMAGE_STATUS* status);
EvmuDirEntry* gyVmuFlashLoadIconDataVms(EvmuDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);

GBL_DECLS_END

#endif // GYRO_VMU_FLASH_H


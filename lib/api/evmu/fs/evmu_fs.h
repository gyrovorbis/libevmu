#ifndef EVMU_FS_H
#define EVMU_FS_H

#include "evmu_fat.h"

#define EVMU_FS_TYPE                            (GBL_TYPEOF(EvmuFs))

#define EVMU_FS(instance)                       (GBL_INSTANCE_CAST(instance, EvmuFs))
#define EVMU_FS_CLASS(klass)                    (GBL_CLASS_CAST(klass, EvmuFs))
#define EVMU_FS_GET_CLASS(instance)             (GBL_INSTANCE_GET_CLASS(instance, EvmuFs))

#define EVMU_FS_GAME_VMS_HEADER_OFFSET          0x200
#define EVMU_FS_LOAD_IMAGE_ERROR_MESSAGE_SIZE   256
#define EVMU_FS_VMI_EXPORT_COPYRIGHT_STRING     "Created with ElysianVMU"

#define GBL_SELF_TYPE EvmuFs

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuFile);

typedef struct EvmuNewFileInfo {
    char    fileName[EVMU_FLASH_DIRECTORY_FILE_NAME_SIZE];
    size_t  fileSizeBytes;
    uint8_t fileType;
    uint8_t copyProtection;
} EvmuNewFileInfo;

GBL_CLASS_DERIVE_EMPTY    (EvmuFs, EvmuPeripheral)
GBL_INSTANCE_DERIVE_EMPTY (EvmuFs, EvmuPeripheral)

GBL_PROPERTIES(EvmuFs,
    (fileCount,  GBL_GENERIC, (READ), GBL_UINT32_TYPE),
    (game,       GBL_GENERIC, (READ), GBL_BOOL_TYPE),
    (iconData,   GBL_GENERIC, (READ), GBL_BOOL_TYPE),
    (extraBgPvr, GBL_GENERIC, (READ), GBL_BOOL_TYPE)
)

// ===== File I/O API =====
EVMU_EXPORT EvmuFile*   EvmuFs_fileOpen  (GBL_CSELF, EvmuDirEntry* pDirEntry, const char* pMode) GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuFs_fileRead  (GBL_CSELF, EvmuFile* pFile, void* pBuffer, GblSize bytes) GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuFs_fileWrite (GBL_CSELF, EvmuFile* pFile, const void* pBuffer, GblSize bytes) GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuFs_fileClose (GBL_CSELF, EvmuFile* pFile) GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuFs_fileEof   (GBL_CSELF, EvmuFile* pFile) GBL_NOEXCEPT;
EVMU_EXPORT GblSize     EvmuFs_fileTell  (GBL_CSELF, EvmuFile* pFile) GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuFs_fileSeek  (GBL_CSELF, EvmuFile* pFile, GblSize offset, GblSize whence) GBL_NOEXCEPT;


GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FS_H

#if 0

typedef enum EVMU_LOAD_IMAGE_STATUS {
    EVMU_LOAD_IMAGE_SUCCESS,
    EVMU_LOAD_IMAGE_OPEN_FAILED,
    EVMU_LOAD_IMAGE_READ_FAILED,
    EVMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS,
    EVMU_LOAD_IMAGE_FILES_MAXED,
    EVMU_LOAD_IMAGE_DEVICE_READ_ERROR,
    EVMU_LOAD_IMAGE_DEVICE_WRITE_ERROR,
    EVMU_LOAD_IMAGE_DEFRAG_FAILED,
    EVMU_LOAD_IMAGE_VMS_NO_VMI,
    EVMU_LOAD_IMAGE_VMI_NO_VMS,
    EVMU_LOAD_IMAGE_GAME_DUPLICATE,
    EVMU_LOAD_IMAGE_NAME_DUPLICATE,
    EVMU_LOAD_IMAGE_UNKNOWN_FORMAT,
    EVMU_LOAD_IMAGE_FLASH_UNFORMATTED
} EVMU_LOAD_IMAGE_STATUS;


typedef struct EvmuFileIdentifier {
    FILE_TYPE game/data/whatever;


} EvmuFileIdentifier;

// creating, copying, deleting handled by DirEntry?
//reading, writing, etc handled by File?
//file is for MANIPULATING one


// File open has to come from a FlashDirEntry
// no flash dir entry => create one



EVMU_API evmuFileDirEntry(EvmuFileSystem* pFs, EvmuFile* pFile, EvmuDirEntry** pDirEntry);



// need raw buffer-based API then file-based API above it when file callbacks are provided!
// have to distinguish between whole-flash images, individual files, and maybe shit like BIOS/LCD?
EVMU_API evmuFileFormatRegister(EvmuFileSystem* pFs, const EvmuFileFormat* pFormat);
EVMU_API evmuFileImport(EvmuFileSystem* pFs, const char* pFilePath, EvmuFatDirEntry** pEntry);
EVMU_API evmuFileExport(EvmuFileSystem* pFs, const char* pFilePath, EvmuFatDirEntry* pEntry);






//High-level File API
int                     gyVmuFlashFileCount(const struct VMUDevice* dev);
VMUFlashDirEntry* gyVmuFlashFileAtIndex(const struct VMUDevice* dev, int fileIdx);
VMUFlashDirEntry* gyVmuFlashFileCreate(struct VMUDevice* dev, const VMUFlashNewFileProperties* properties, const unsigned char* data, EVMU_LOAD_IMAGE_STATUS* status);

void gyVmuFlashNexusByteOrder(uint8_t* data, size_t bytes);

int gyVmuFlashFileDelete(struct VMUDevice* dev, VMUFlashDirEntry* entry);
int gyVmuFlashFileCopy(const struct VMUDevice* devSrc, const VMUFlashDirEntry* entrySrc,
                       struct VMUDevice* devDst, const VMUFlashDirEntry* entryDst, int force);
size_t gyVmuFlashFileReadBytes(struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, unsigned char* buffer, size_t bytes, size_t offset, int includeHeader);
int gyVmuFlashFileRead(struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, unsigned char* buffer, int includeHeader);

//Uses existing VMS header information, can realloc data
int gyVmuFlashFileWrite(struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, const char* data, size_t bytes);

void gyVmuFlashNewFilePropertiesFromVmi(VMUFlashNewFileProperties* fileProperties, const struct VMIFileInfo* vmi);
void gyVmuFlashNewFilePropertiesFromDirEntry(VMUFlashNewFileProperties* fileProperties, const struct VMUFlashDirEntry* entry);
void gyVmuFlashNewFilePropertiesFromIconDataVms(VMUFlashNewFileProperties* fileProperties, size_t byteSize);

void gyVmuFlashVmiFromDirEntry(struct VMIFileInfo* vmi, const struct VMUDevice* dev, const VMUFlashDirEntry* entry, const char* vmsResourceName);

//Flash Utilities
int gyVmuFlashPrintFilesystem(const struct VMUDevice* dev);


int gyVmuFlashIsIconDataVms(const struct VMUFlashDirEntry* entry);
int gyVmuFlashIsExtraBgPvr(const struct VMUFlashDirEntry* entry);

int gyVmuFlashLoadVMUImage(struct VMUDevice* dev, const char* path);
uint8_t* gyVmuFlashLoadVMS(const char* path, size_t* fileSize);


int gyVmuFlashLoadVMI(struct VMIFileInfo* info, const char* path);
int gyVmuFlashLoadDCI(struct VMUDevice* dev, const char* path);


int gyVmuVmiFindVmsPath(const char* vmiPath, char* vmsPath);
int gyVmuVmsFindVmiPath(const char* vmsPath, char* vmiPath);

int gyVmuFlashExportImage(struct VMUDevice* dev, const char* path);
int gyVmuFlashExportDcm(struct VMUDevice* dev, const char* path);
int gyVmuFlashExportVms(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, const char* path);
int gyVmuFlashExportVmi(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, const char* path);
int gyVmuFlashExportDci(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, const char* path);
int gyVmuFlashExportRaw(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, const char* path);

const char* gyVmuFlashLastErrorMessage(void);
VMUFlashDirEntry* gyVmuFlashLoadImage(struct VMUDevice* dev, const char* path, EVMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageDcm(struct VMUDevice* dev, const char* path, EVMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageDci(struct VMUDevice* dev, const char* path, EVMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageBin(struct VMUDevice* dev, const char* path, EVMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageVmiVms(struct VMUDevice* dev, const char* vmipath, const char* vmspath, EVMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashCreateFileVmiVms(struct VMUDevice* dev, const struct VMIFileInfo* vmi, const uint8_t* vms, EVMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadIconDataVms(struct VMUDevice* dev, const char* path, EVMU_LOAD_IMAGE_STATUS* status);

//Save API (VMI, VMS, DCM, emulator formats, etc)

\\
#endif



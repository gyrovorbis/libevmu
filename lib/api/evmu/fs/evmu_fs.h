#ifndef EVMU_FS_H
#define EVMU_FS_H

#define EVMU_FLASH_GAME_VMS_HEADER_OFFSET            0x200

#define EVMU_FLASH_LOAD_IMAGE_ERROR_MESSAGE_SIZE     256

#define EVMU_FLASH_VMI_EXPORT_COPYRIGHT_STRING       "Created with ElysianVMU"

typedef struct VMUFlashNewFileProperties {
    char            fileName[EVMU_FLASH_DIRECTORY_FILE_NAME_SIZE];
    size_t          fileSizeBytes;
    uint8_t         fileType;
    uint8_t         copyProtection;
} VMUFlashNewFileProperties;


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

EVMU_API evmuFileOpen(EvmuFilesystem* pFs, EvmuDirEntry* pDirEntry, const char* pMode, EvmuFile* pFile);
EVMU_API evmuFileRead(EvmuFileSystem* pFs, EvmuFile* pFile, void* pBuffer, EvmuSize* pBytes);
EVMU_API evmuFileWrite(EvmuFilesystem* pFs, EvmuFile* pFile, const void* pBuffer, EvmuSize* pBytes);
EVMU_API evmuFileClose(EvmuFileSystem* pFs, EvmuFile* pFile);
EVMU_API evmuFileEof(EvmuFileSystem* pFs, EvmuFile* pFile, EvmuBool* pFeof);
EVMU_API evmuFileTell(EvmuFileSystem* pFs, EvmuFile* pFile, EvmuSize* pBytePos);
EVMU_API evmuFileSeek(EVmuFileSystem* pFs, EvmuFile* pFile, EvmuSize offset, EvmuSize whence);

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


#endif // EVMU_FS_H

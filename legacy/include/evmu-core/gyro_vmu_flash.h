#ifndef GYRO_VMU_FLASH_H
#define GYRO_VMU_FLASH_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "gyro_vmu_vms.h"

#define VMU_FLASH_GAME_VMS_HEADER_OFFSET            0x200
#define VMU_FLASH_BLOCK_SIZE                        512
#define VMU_FLASH_BLOCK_ROOT                        255
#define VMU_FLASH_BLOCK_ROOT_SIZE                   1

#define VMU_FLASH_BLOCK_COUNT_DEFAULT               256
#define VMU_FLASH_BLOCK_USERDATA_DEFAULT            0
#define VMU_FLASH_BLOCK_USERDATA_SIZE_DEFAULT       200
#define VMU_FLASH_BLOCK_UNUSED_DEFAULT              200
#define VMU_FLASH_BLOCK_UNUSED_SIZE_DEFAULT         41
#define VMU_FLASH_BLOCK_DIRECTORY_DEFAULT           253
#define VMU_FLASH_BLOCK_DIRECTORY_SIZE_DEFAULT      13
#define VMU_FLASH_BLOCK_DIRECTORY_ENTRIES_DEFAULT   200
#define VMU_FLASH_BLOCK_FAT_DEFAULT                 254
#define VMU_FLASH_BLOCK_FAT_SIZE_DEFAULT            1
#define VMU_FLASH_BLOCK_FAT_COUNT_DEFAULT           256

#define VMU_FLASH_BLOCK_ROOT_FORMATTED_BYTE         0x55
#define VMU_FLASH_BLOCK_FAT_UNALLOCATED             0xfffc
#define VMU_FLASH_BLOCK_FAT_LAST_IN_FILE            0xfffa
#define VMU_FLASH_BLOCK_FAT_DAMAGED                 0xffff

#define VMU_FLASH_DIRECTORY_ENTRY_SIZE              32
#define VMU_FLASH_DIRECTORY_FILE_NAME_SIZE          12
#define VMU_FLASH_DIRECTORY_DATE_CENTURY            0
#define VMU_FLASH_DIRECTORY_DATE_YEAR               1
#define VMU_FLASH_DIRECTORY_DATE_MONTH              2
#define VMU_FLASH_DIRECTORY_DATE_DAY                3
#define VMU_FLASH_DIRECTORY_DATE_HOUR               4
#define VMU_FLASH_DIRECTORY_DATE_MINUTE             5
#define VMU_FLASH_DIRECTORY_DATE_SECOND             6
#define VMU_FLASH_DIRECTORY_DATE_WEEKDAY            7
#define VMU_FLASH_DIRECTORY_DATE_SIZE               8

#define VMU_FLASH_PRG_BYTE_COUNT                    128     //number of bytes software can write to flash once unlocked
#define VMU_FLASH_PRG_STATE0_ADDR                   0x5555  //Key-value pairs for flash unlock sequence used by STF instruction
#define VMU_FLASH_PRG_STATE0_VALUE                  0xaa
#define VMU_FLASH_PRG_STATE1_ADDR                   0x2aaa
#define VMU_FLASH_PRG_STATE1_VALUE                  0x55
#define VMU_FLASH_PRG_STATE2_ADDR                   0x5555
#define VMU_FLASH_PRG_STATE2_VALUE                  0xa0

#define VMU_FLASH_ROOT_BLOCK_FORMATTED_SIZE         16
#define VMU_FLASH_ROOT_BLOCK_FORMATTED_BYTE         0x55
#define VMU_FLASH_ROOT_BLOCK_UNUSED1_SIZE           26
#define VMU_FLASH_ROOT_BLOCK_TIMESTAMP_SIZE         8
#define VMU_FLASH_ROOT_BLOCK_UNUSED2_SIZE           15
#define VMU_FLASH_ROOT_BLOCK_ICON_SHAPE_MAX         123

#define VMU_FLASH_LOAD_IMAGE_ERROR_MESSAGE_SIZE     256

#define VMU_FLASH_VMI_EXPORT_COPYRIGHT_STRING       "Created with ElysianVMU"

struct VMUDevice;
struct VMIFileInfo;
struct COL_IMAGEFileInfo;

typedef enum VMU_FLASH_PRG_STATE {
    VMU_FLASH_PRG_STATE0,
    VMU_FLASH_PRG_STATE1,
    VMU_FLASH_PRG_STATE2
} VMU_FLASH_PRG_STATE;


//Flash controller for VMU (note actual flash blocks are stored within device)
typedef struct VMUFlashPrg {
    uint8_t prgState;
} VMUFlashPrg;

/*
 * To extract a file:
 * 1) find start block in directory
 * 2) find subsequent blocks as linked list from FAT
 *
 * Note that mini-game files are allocated starting at block 0 and upwards,
 * while a data file is allocated starting at block 199 selecting the highest available free block.
 * This is probably because a mini-game should be able to run directly from the flash,
 * and thus needs to be placed in a linear memory space starting at a known address (i.e. 0).
 *
 * FAT table allows each entry to be two bytes, and is only really using a single byte.
 * Sega seems to allow for a "double VMU" that should be fully compatible with everything,
 * provided they're all using firmware calls to write/read from Flash (should be).
 */
#if 0
typedef struct VMUFlashRootBlock {
    char        formatted[VMU_FLASH_ROOT_BLOCK_FORMATTED_SIZE];  //set to 0x55 for formatting
    char        customColor;//17
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
    uint8_t     a; //21
    uint8_t     unused1[VMU_FLASH_ROOT_BLOCK_UNUSED1_SIZE];    //all zeroes
    uint8_t     timeStamp[VMU_FLASH_ROOT_BLOCK_TIMESTAMP_SIZE];   //BCD 47
    char        unused2[VMU_FLASH_ROOT_BLOCK_UNUSED2_SIZE];     //all zeroes //56
    uint16_t    fatBlock;       //254 //63
    uint16_t    fatSize;        //1
    uint16_t    dirBlock;       //253
    uint16_t    dirSize;        //13
    uint16_t    iconShape;      //(0-123)
    uint16_t    userBlocks;     //(200 (or 240))
} VMUFlashRootBlock;            //81 bytes
#else
#define VMU_FLASH_ROOT_BLOCK_RESERVED_SIZE 	8
#define VMU_FLASH_ROOT_BLOCK_RESERVED3_SIZE	8
#define VMU_FLASH_ROOT_BLOCK_TIMESTAMP_SIZE	8
typedef struct VMUFlashRootBlock {
    char        formatted[VMU_FLASH_ROOT_BLOCK_FORMATTED_SIZE];  //set to 0x55 for formatting
    union {
        struct {
            char        customColor;//17
            uint8_t     b;
            uint8_t     g;
            uint8_t     r;
            uint8_t     a; //21
            uint8_t     unused1[VMU_FLASH_ROOT_BLOCK_UNUSED1_SIZE];    //all zeroes
        } vmu;
        uint8_t 		bytes[0x20];
    } volumeLabel;
    uint8_t     timeStamp[VMU_FLASH_ROOT_BLOCK_TIMESTAMP_SIZE];   //BCD 47
    char        reserved[VMU_FLASH_ROOT_BLOCK_RESERVED_SIZE];     //all zeroes //56
    uint16_t	totalSize;			//total BLOCK size of partition
    uint16_t 	partition; 			//First partition (0)
    uint16_t	rootBlock;			//Location of Root Block (255)
    uint16_t    fatBlock;       	//254
    uint16_t    fatSize;        	//1
    uint16_t    dirBlock;       	//253
    uint16_t    dirSize;        	//13
    uint16_t    iconShape;      	//(0-123)
    uint16_t	userSize;			//?
    uint16_t    saveAreaBlock;     	//200, 240 (MAYBE NOT USERDATA, MAYBE TALKING ABOUT RESERVED BLOCK!)//(200 (or 240))
    uint16_t 	saveAreaSize;		//(200 (or 240)) (MAYBE SIZE OF RESERVED SHIT!?!!?)
    uint32_t	executionFile;		//00 for no file can execute on partition, otherwise refer to docs?
    uint8_t		reserved3[VMU_FLASH_ROOT_BLOCK_RESERVED3_SIZE];
    //Rest of Root is supposedly reserved
} VMUFlashRootBlock;            	//81 bytes

#endif

typedef enum VMU_FLASH_FILE_TYPE {
    VMU_FLASH_FILE_TYPE_NONE    = 0x00,
    VMU_FLASH_FILE_TYPE_DATA    = 0x33,
    VMU_FLASH_FILE_TYPE_GAME    = 0xcc
} VMU_FLASH_FILE_TYPE;

typedef enum VMU_FLASH_COPY_PROTECTION {
    VMU_FLASH_COPY_PROTECTION_COPY_OK           = 0x00,
    VMU_FLASH_COPY_PROTECTION_COPY_PROTECTED    = 0xff,
    VMU_FLASH_COPY_PROTECTION_COPY_UNKNOWN      = 0x01
} VMU_FLASH_COPY_PROTECTION;

typedef struct VMUFlashNewFileProperties {
    char            fileName[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE];
    size_t          fileSizeBytes;
    uint8_t         fileType;
    uint8_t         copyProtection;
} VMUFlashNewFileProperties;

typedef struct VMUFlashDirEntry {
    uint8_t         fileType;
    uint8_t         copyProtection;
    uint16_t        firstBlock;
    char            fileName[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE]; //Shift-JIS
    unsigned char   timeStamp[VMU_FLASH_DIRECTORY_DATE_SIZE]; //BCD
    uint16_t        fileSize;     //Blocks
    uint16_t        headerOffset; //Blocks
    char            unused[4];    //all 0s
} VMUFlashDirEntry; //sizeof(VMUFlashDirEntry) BETTER FUCKING == 32

typedef struct VMUFlashMemUsage {
    uint16_t    blocksUsed;
    uint16_t    blocksFree;
    uint16_t    blocksHidden;
    uint16_t    blocksDamaged;
} VMUFlashMemUsage;

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

//System API
VMUFlashRootBlock*  gyVmuFlashBlockRoot(struct VMUDevice* dev);
void                gyVmuFlashRootBlockPrint(const struct VMUDevice* dev);

uint16_t            gyVmuFlashBlockCount(const struct VMUDevice* dev);
uint16_t            gyVmuFlashBlockFat(const struct VMUDevice* dev);
uint16_t            gyVmuFlashBlockDirectory(const struct VMUDevice* dev);

size_t              gyVmuFlashBytes(const struct VMUDevice* dev);
uint16_t            gyVmuFlashDirEntryCount(const struct VMUDevice* dev);

//Low-level FAT Block API
unsigned char*      gyVmuFlashBlock(struct VMUDevice* dev, uint16_t block);
uint16_t*           gyVmuFlashBlockFATEntry(struct VMUDevice* dev, uint16_t block);
uint16_t            gyVmuFlashBlockAlloc(struct VMUDevice* dev, uint16_t previous, int fileType); //Returns VMU_FLASH_BLOCK_FAT_UNALLOCATED if full
void                gyVmuFlashBlockFree(struct VMUDevice* dev, uint16_t block);
uint16_t            gyVmuFlashBlockNext(const struct VMUDevice* dev, uint16_t block);

//Mid-level Directory API
VMUFlashDirEntry*       gyVmuFlashDirEntryGame(struct VMUDevice* dev);
VMUFlashDirEntry*       gyVmuFlashDirEntryIconData(struct VMUDevice* dev);
VMUFlashDirEntry*       gyVmuFlashDirEntryExtraBgPvr(struct VMUDevice* dev);
VMUFlashDirEntry*       gyVmuFlashDirEntryDataNext(struct VMUDevice* dev, const VMUFlashDirEntry* prev);
VMUFlashDirEntry*       gyVmuFlashDirEntryByIndex(struct VMUDevice* dev, uint16_t index);
VMUFlashDirEntry*       gyVmuFlashDirEntryFind(struct VMUDevice* dev, const char* name);
uint8_t                 gyVmuFlashDirEntryIndex(const struct VMUDevice* dev, const VMUFlashDirEntry* entry);
VMUFlashDirEntry*       gyVmuFlashDirEntryAlloc(struct VMUDevice* dev);
void                    gyVmuFlashDirEntryFree(struct VMUFlashDirEntry* entry);
void                    gyVmuFlashDirEntryPrint(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry);
VMSFileInfo*            gyVmuFlashDirEntryVmsHeader(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry);
void                    gyVmuFlashDirEntryName(const VMUFlashDirEntry* entry, char* buffer);

//High-level File API
int                     gyVmuFlashFileCount(const struct VMUDevice* dev);
VMUFlashDirEntry* gyVmuFlashFileAtIndex(const struct VMUDevice* dev, int fileIdx);
VMUFlashDirEntry* gyVmuFlashFileCreate(struct VMUDevice* dev, const VMUFlashNewFileProperties* properties, const unsigned char* data, VMU_LOAD_IMAGE_STATUS* status);

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

int gyVmuFlashBytesToBlocks(int bytes);
int gyVmuFlashContiguousFreeBlocks(const struct VMUDevice* dev);
VMUFlashMemUsage gyVmuFlashMemUsage(const struct VMUDevice* dev);
uint16_t gyVmuFlashUserDataBlocks(const struct VMUDevice* dev);

int gyVmuFlashFormatDefault(struct VMUDevice* dev);
int gyVmuFlashFormat(struct VMUDevice* dev, const VMUFlashRootBlock* rootBlock);
int gyVmuFlashCheckFormatted(const struct VMUDevice* dev);
int gyVmuFlashDefragment(struct VMUDevice* dev, int newUserSize);
uint16_t gyVmuFlashFileCalculateCRC(struct VMUDevice* dev, const VMUFlashDirEntry* dirEntry);

//Detect corruption? Wrong block sizes, nonzero unused bocks, etc?
//Fix corruption?

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
VMUFlashDirEntry* gyVmuFlashLoadImage(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageDcm(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageDci(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageBin(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadImageVmiVms(struct VMUDevice* dev, const char* vmipath, const char* vmspath, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashCreateFileVmiVms(struct VMUDevice* dev, const struct VMIFileInfo* vmi, const uint8_t* vms, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadIconDataVms(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);
VMUFlashDirEntry* gyVmuFlashLoadArmBinary(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status);


//Save API (VMI, VMS, DCM, emulator formats, etc)

int gyloadbios(struct VMUDevice* dev, const char *filename);






#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_FLASH_H


#ifndef EVMU_FAT_H
#define EVMU_FAT_H

#include "../evmu_types.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "evmu_vms.h"

#define EVMU_FAT_BLOCK_SIZE                        512
#define EVMU_FAT_BLOCK_COUNT_DEFAULT               256

#define EVMU_FAT_BLOCK_ROOT                        255
#define EVMU_FAT_BLOCK_ROOT_SIZE                   1
#define EVMU_FAT_BLOCK_USERDATA_DEFAULT            0
#define EVMU_FAT_BLOCK_USERDATA_SIZE_DEFAULT       200
#define EVMU_FAT_BLOCK_UNUSED_DEFAULT              200
#define EVMU_FAT_BLOCK_UNUSED_SIZE_DEFAULT         41
#define EVMU_FAT_BLOCK_DIRECTORY_DEFAULT           253
#define EVMU_FAT_BLOCK_DIRECTORY_SIZE_DEFAULT      13
#define EVMU_FAT_BLOCK_DIRECTORY_ENTRIES_DEFAULT   200
#define EVMU_FAT_BLOCK_FAT_DEFAULT                 254
#define EVMU_FAT_BLOCK_FAT_SIZE_DEFAULT            1
#define EVMU_FAT_BLOCK_FAT_COUNT_DEFAULT           256

#define EVMU_FAT_BLOCK_ROOT_FORMATTED_BYTE         0x55
#define EVMU_FAT_BLOCK_FAT_UNALLOCATED             0xfffc
#define EVMU_FAT_BLOCK_FAT_LAST_IN_FILE            0xfffa
#define EVMU_FAT_BLOCK_FAT_DAMAGED                 0xffff

#define EVMU_FAT_DIRECTORY_ENTRY_SIZE              32
#define EVMU_FAT_DIRECTORY_FILE_NAME_SIZE          12
#define EVMU_FAT_DIRECTORY_DATE_CENTURY            0
#define EVMU_FAT_DIRECTORY_DATE_YEAR               1
#define EVMU_FAT_DIRECTORY_DATE_MONTH              2
#define EVMU_FAT_DIRECTORY_DATE_DAY                3
#define EVMU_FAT_DIRECTORY_DATE_HOUR               4
#define EVMU_FAT_DIRECTORY_DATE_MINUTE             5
#define EVMU_FAT_DIRECTORY_DATE_SECOND             6
#define EVMU_FAT_DIRECTORY_DATE_WEEKDAY            7
#define EVMU_FAT_DIRECTORY_DATE_SIZE               8
#define EVMU_FAT_DIRECTORY_UNUSED_SIZE             4

#define EVMU_FAT_ROOT_BLOCK_FORMATTED_SIZE         16
#define EVMU_FAT_ROOT_BLOCK_FORMATTED_BYTE         0x55
#define EVMU_FAT_ROOT_BLOCK_UNUSED1_SIZE           26
#define EVMU_FAT_ROOT_BLOCK_TIMESTAMP_SIZE         8
#define EVMU_FAT_ROOT_BLOCK_UNUSED2_SIZE           15
#define EVMU_FAT_ROOT_BLOCK_ICON_SHAPE_MAX         123
#define EVMU_FAT_ROOT_BLOCK_RESERVED_SIZE          8
#define EVMU_FAT_ROOT_BLOCK_RESERVED3_SIZE         8

typedef uint16_t EvmuFatBlock;

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
 * /******** MEDIAINFO FROM LIB SHINOBI ******************************
#define OFS_FORMAT         0x000   /* tH[}bgîñ                     */
#define OFS_VLABEL         0x010   /* {[x                     */
#define OFS_DATETIME       0x030   /* ì¬ú                             */
#define OFS_NUMBLOCKS      0x040   /* SeÊ(ubN)                   */
#define OFS_PARTITION      0x042   /* p[eBVîñ                   */
#define OFS_SYSTEMAREA     0x044   /* VXeÌæ(???)                    */
#define OFS_FATBLOCK       0x046   /* FATÌæÌæªubNÔ            */
#define OFS_NUMFATBLOCKS   0x048   /* FATÌæÌubN                  */
#define OFS_FILEBLOCK      0x04a   /* t@CîñÌæªubNÔ       */
#define OFS_NUMFILEBLOCKS  0x04c   /* t@CîñÌubN             */
#define OFS_ICONNO         0x04e   /* {[ACRÔ               */
#define OFS_SORTFLAG       0x04f   /* \[gtO                         */
#define OFS_EXTRABLOCK     0x050   /* ÒðÌæÌæªubNÔ           */
#define OFS_NUMEXTRABLOCKS 0x052   /* ÒðÌæÌubN                 */
#define OFS_EXEBLOCK       0x054   /* Àst@CÌæªubNÔ       */
#define OFS_NUMEXEBLOCKS   0x056   /* Àst@CÅåubN           */

#define BLOCKSIZ(_info_)        ((_info_)->block_size + 1)
#define NUMBLOCKS(_info_)       ((_info_)->block + 1)
#define NUMFATBLOCKS(_info_)    ((_info_)->fat_block)
#define NUMFATS(_info_)         (NUMFATBLOCKS(_info_) * BLOCKSIZ(_info_) / 2)
#define NUMFILEBLOCKS(_info_)   ((_info_)->file_block)
#define PARTITION(_info_)       ((_info_)->partition)
#define SYSTEMBLOCK(_info_)     ((_info_)->system_loc)
#define FATBLOCK(_info_)        ((_info_)->fat_loc)
#define FILEBLOCK(_info_)       ((_info_)->file_loc)
#define EXTRABLOCK(_info_)      ((_info_)->extra_loc)
#define NUMEXTRABLOCKS(_info_)  ((_info_)->extra_block)
#define NUMUSERBLOCKS(_info_)   ((_info_)->user_block)
#define USERBLOCKEND(_info_)    (EXTRABLOCK(_info_) - 1)
#define EXEBLOCK(_info_)        ((_info_)->exe_loc)
#define NUMEXEBLOCKS(_info_)    ((_info_)->exe_block)
 */
#endif
typedef struct EvmuFatRootBlock {
    char        formatted[EVMU_FAT_ROOT_BLOCK_FORMATTED_SIZE];  //set to 0x55 for formatting
    union {
        struct {
            char        customColor;//17
            uint8_t     b;
            uint8_t     g;
            uint8_t     r;
            uint8_t     a; //21
            uint8_t     unused1[EVMU_FAT_ROOT_BLOCK_UNUSED1_SIZE];    //all zeroes
        } vmu;
        uint8_t 		bytes[0x20];
    } volumeLabel;
    uint8_t     timeStamp[EVMU_FAT_ROOT_BLOCK_TIMESTAMP_SIZE];   //BCD 47
    char        reserved[EVMU_FAT_ROOT_BLOCK_RESERVED_SIZE];     //all zeroes //56
    uint16_t	totalSize;			//total BLOCK size of partition
    uint16_t 	partition; 			//First partition (0)
    uint16_t	rootBlock;			//Location of Root Block (255)
    uint16_t    fatBlock;       	//254
    uint16_t    fatSize;        	//1
    uint16_t    dirBlock;       	//253
    uint16_t    dirSize;        	//13
    uint8_t     iconShape;      	//(0-123)
    uint8_t     sortFlag;           // no fucking idea
    uint16_t	extraBlock;			//block starting point for extra data
    uint16_t    extraSize;          //number of extra blocks
    uint16_t    gameBlock;          //starting block for game (default 0)
    uint16_t    gameSize;           //number of game blocks
    uint8_t		reserved3[EVMU_FAT_ROOT_BLOCK_RESERVED3_SIZE];
    //Rest of Root is supposedly reserved
} EvmuFatRootBlock;            	//81 bytes

typedef enum EVMU_FAT_FILE_TYPE {
    EVMU_FAT_FILE_TYPE_NONE    = 0x00,
    EVMU_FAT_FILE_TYPE_DATA    = 0x33,
    EVMU_FAT_FILE_TYPE_GAME    = 0xcc
} EVMU_FAT_FILE_TYPE;

typedef enum EVMU_FAT_COPY_PROTECTION {
    EVMU_FAT_COPY_PROTECTION_COPY_OK           = 0x00,
    EVMU_FAT_COPY_PROTECTION_COPY_PROTECTED    = 0xff,
    EVMU_FAT_COPY_PROTECTION_COPY_UNKNOWN      = 0x01
} EVMU_FAT_COPY_PROTECTION;

typedef struct EvmuFatDirEntry {
    uint8_t         fileType;
    uint8_t         copyProtection;
    uint16_t        firstBlock;
    char            fileName[EVMU_FAT_DIRECTORY_FILE_NAME_SIZE]; //Shift-JIS
    unsigned char   timeStamp[EVMU_FAT_DIRECTORY_DATE_SIZE]; //BCD
    uint16_t        fileSize;     //Blocks
    uint16_t        headerOffset; //Blocks
    char            unused[EVMU_FAT_DIRECTORY_UNUSED_SIZE];    //all 0s
} EvmuFatDirEntry; //sizeof(VMUFlashDirEntry) BETTER FUCKING == 32

typedef struct EvmuFatMemUsage {
    uint16_t    blocksUsed;
    uint16_t    blocksFree;
    uint16_t    blocksHidden;
    uint16_t    blocksDamaged;
    //maybe add more shit here and reduce API size
} EvmuFatMemUsage;

GBL_DECLARE_HANDLE(EvmuFlash);
GBL_DECLARE_HANDLE(EvmuFat);

// ===== Top-level utilities =======
// ==== Major Formatting and Defragmenting Routines ====
EVMU_API evmuFlashFormatDefault(const EvmuFlash* pFlash);
EVMU_API evmuFlashFormat(const EvmuFlash* pFlash, const EvmuFatRootBlock* pDefaultRoot);
EVMU_API evmuFlashFormattedCheck(const EvmuFlash* pFlash, GblBool* pBool);
EVMU_API evmuFlashDefragment(const EvmuFlash* pFlash, int newUserSize);

// ===== Statistics, Metrics, Utilities, and Tools ====
EVMU_API evmuFlashBytes(const EvmuFlash* pFlash, GblSize* pSize);
EVMU_API evmuFlashBytesToBlocks(const EvmuFlash* pFlash, uint32_t bytes, uint32_t* pBlocks);
EVMU_API evmuFlashContiguousFreeBlocks(const EvmuFlash* pFlash, uint32_t* pBlocks);
EVMU_API evmuFlashMemoryUsage(const EvmuFlash* pFlash, EvmuFatMemUsage* pUsage);
EVMU_API evmuFlashUserDataBlocks(const EvmuFlash* pFlash, uint16_t* pBlocks);
EVMU_API evmuFlashBlockRootDebugDumpState(const EvmuFlash* pFlash);


EVMU_API evmuFatRoot(const EvmuFat* pFat, EvmuFatRootBlock** pRoot);


//===== Low-level Block API =======
EVMU_API evmuFatBlockCount(const EvmuFat* pFat, uint16_t* pCount);
EVMU_API evmuFatBlockDirectory(const EvmuFat* pFat, EvmuFatBlock* pBlock);
EVMU_API evmuFatBlockTable(const EvmuFat* pFat, EvmuFatBlock* pBlock);

EVMU_API evmuFatBlockData(const EvmuFat* pFat, EvmuFatBlock block, void** ppData);
EVMU_API evmuFatBlockTableEntry(const EvmuFat* pFat, EvmuFatBlock block, EvmuFatBlock** ppTableEntry);
EVMU_API evmuFatBlockNext(const EvmuFat* pFat, EvmuFatBlock block, EvmuFatBlock* pNext);
EVMU_API evmuFatBlockFree(const EvmuFat* pFat, EvmuFatBlock block);
EVMU_API evmuFatBlockAllocNext(const EvmuFat* pFat, EvmuFatBlock previous, EVMU_FAT_FILE_TYPE fileType, EvmuFatBlock* pNext);

//==== Mid-level Directory API ========

EVMU_API evmuFatDirEntryCount(const EvmuFat* pFat, uint16_t* pCount);
EVMU_API evmuFatDirEntryAt(const EvmuFat* pFat, uint16_t index, EvmuFatDirEntry** ppEntry);
EVMU_API evmuFatDirEntryFind(const EvmuFat* pFat, const char* pName, EvmuFatDirEntry** ppEntry);
EVMU_API evmuFatDirEntryGame(const EvmuFat* pFat, EvmuFatDirEntry** ppEntry);
//EVMU_API evmuFatDirEntryIconData(struct EvmuFat* pFat, EvmuFlashDirEntry** ppEntry);
//EVMU_API evmuFatDirEntryExtraBgPvr(struct EvmuFat* pFat, EvmuFlashDirEntry** ppEntry);

EVMU_API evmuFatDirEntryAlloc(const EvmuFat* pFat, EvmuFatDirEntry** ppEntry);

EVMU_API evmuFatDirEntryIndex(const EvmuFat* pFat, const EvmuFatDirEntry* pEntry, uint16_t* pIndex);
EVMU_API evmuFatDirEntryName(const EvmuFat* pFat, const EvmuFatDirEntry* pEntry, char *pBuffer, size_t* pSize);
EVMU_API evmuFatDirEntryVmsHeader(const EvmuFat* pFat, const EvmuFatDirEntry* pEntry, VMSFileInfo** ppVmsInfo);
EVMU_API evmuFatDirEntryDataNext(const EvmuFat* pFat, const EvmuFatDirEntry* pEntry, EvmuFatDirEntry** ppNext);
EVMU_API evmuFatDirEntryFree(const EvmuFat* pFat, EvmuFatDirEntry* pEntry);
EVMU_API evmuFatDirEntryPrint(const EvmuFat* pFat, const EvmuFatDirEntry* pEntry);



//Detect corruption? Wrong block sizes, nonzero unused bocks, etc?
//Fix corruption?



#ifdef __cplusplus
}
#endif

#endif // GYRO_EVMU_FLASH_H


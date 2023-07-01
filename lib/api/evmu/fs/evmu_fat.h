/*! \file
 *  \brief EvmuFat peripheral and FAT filesystem API
 *  \ingroup file_system
 *
 *  EvmuFat offers a mid-level API around the VMU's flash storage,
 *  which sits above the physical flash controller and below an
 *  actual filesystem API. The API operates at the block-level and
 *  also offers a low-level 8-bit FAT abstraction.
 *
 *  \test
 *  - Needs whole unit test suite
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_FAT_H
#define EVMU_FAT_H

#include "../hw/evmu_flash.h"
#include <gimbal/utils/gimbal_date_time.h>

/*! \defgroup file_system File System
 *  \brief    File System-related types and APIs
 *
 *  This module contains a collection of the various
 *  different headers, types, and components of the
 *  VMU's flash storage and internal file system.
 */

#define EVMU_FAT_TYPE                              (GBL_TYPEOF(EvmuFat))                        //!< UUID for EvmuFat type
#define EVMU_FAT(instance)                         (GBL_INSTANCE_CAST(instance, EvmuFat))       //!< Function-tyle GblInstance cast
#define EVMU_FAT_CLASS(klass)                      (GBL_CLASS_CAST(klass, EvmuFat))             //!< Function-style GblClass cast
#define EVMU_FAT_GET_CLASS(instance)               (GBL_INSTANCE_GET_CLASS(instance, EvmuFat))  //!< Get EvmuFatClass from GblInstance

#define EVMU_FAT_NAME                              "fat"    //!< EvmuFat GblObject name

#define EVMU_FAT_GAME_VMS_HEADER_OFFSET            0x200    //!< Offset of the VMS header from the file start for a GAME

#define EVMU_FAT_BLOCK_SIZE                        512      //!< Default block size for a standard VMU
#define EVMU_FAT_BLOCK_COUNT_DEFAULT               256      //!< Default block capacity for a standard VMU

#define EVMU_FAT_BLOCK_ROOT                        255      //!< Default root block number for a standard VMU
#define EVMU_FAT_BLOCK_ROOT_SIZE                   1        //!< Default root size for a standard VMU
#define EVMU_FAT_BLOCK_USERDATA_DEFAULT            0        //!< Default userdata start block for a standard VMU
#define EVMU_FAT_BLOCK_USERDATA_SIZE_DEFAULT       200      //!< Default number of userdata blocks for a standard VMU
#define EVMU_FAT_BLOCK_EXTRA_DEFAULT               200      //!< Default hidden start block for a standard VMU
#define EVMU_FAT_BLOCK_EXTRA_SIZE_DEFAULT          41       //!< Default number of hidden blocks for a standard VMU
#define EVMU_FAT_BLOCK_DIRECTORY_DEFAULT           253      //!< Default directory start block for a standard VMU
#define EVMU_FAT_BLOCK_DIRECTORY_SIZE_DEFAULT      13       //!< Default number of directory blocks for a standard VMU
#define EVMU_FAT_BLOCK_DIRECTORY_ENTRIES_DEFAULT   200      //!< Default number of directory entries for a standard-sized directory
#define EVMU_FAT_BLOCK_FAT_DEFAULT                 254      //!< Default FAT start block for a standard VMU
#define EVMU_FAT_BLOCK_FAT_SIZE_DEFAULT            1        //!< Default number of FAT blocks for a standard VMU
#define EVMU_FAT_BLOCK_FAT_COUNT_DEFAULT           256      //!< Default number of FAT entries for a standard-sized FAT

#define EVMU_FAT_BLOCK_FAT_UNALLOCATED             0xfffc   //!< FAT entry value signifying an unallocated block
#define EVMU_FAT_BLOCK_FAT_LAST_IN_FILE            0xfffa   //!< FAT entry value signifying the last block of a file
#define EVMU_FAT_BLOCK_FAT_DAMAGED                 0xffff   //!< FAT entry value signifying an unused, damaged block

#define EVMU_FAT_DIRECTORY_ENTRY_SIZE              32       //!< Size in bytes of a directory entry
#define EVMU_FAT_DIRECTORY_FILE_NAME_SIZE          12       //!< Maximum file name size in bytes for a directory entry (no NULL terminator)
#define EVMU_FAT_DIRECTORY_UNUSED_SIZE             4        //!< Size of unused region in a directory entry

#define EVMU_FAT_ROOT_BLOCK_FORMATTED_SIZE         16       //!< Size in bytes of the format string in the EvmuRootBlock structure
#define EVMU_FAT_ROOT_BLOCK_FORMATTED_BYTE         0x55     //!< Value string that must be preset in the EvmuRootBlock to signify a formatted card
#define EVMU_FAT_ROOT_BLOCK_VOLUME_LABEL_SIZE      32       //!< Size in bytes of the volume label in the EvmuRootBlock structure
#define EVMU_FAT_ROOT_BLOCK_ICON_SHAPE_MAX         123      //!< Maximum allowable value for icon shape in the EvmuRootBlock structure
#define EVMU_FAT_ROOT_BLOCK_RESERVED_SIZE          8        //!< Size in bytes of the first reserved field in the EvmuRootBlock structure
#define EVMU_FAT_ROOT_BLOCK_RESERVED2_SIZE         8        //!< Size in bytes of the second reserved field in the EvmuRootBlock structure

#define GBL_SELF_TYPE EvmuFat

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuNewFileInfo);
GBL_FORWARD_DECLARE_STRUCT(EvmuVms);
GBL_FORWARD_DECLARE_STRUCT(EvmuFat);

//! FAT block index
typedef uint16_t EvmuBlock;

//! Filesystem timestamp, stored in BCD
typedef struct EvmuTimestamp {
    uint8_t century;    //!< Date century (first two digits) (BCD)
    uint8_t year;       //!< Date year (last two digits) (BCD)
    uint8_t month;      //!< Date month (1-12) (BCD)
    uint8_t day;        //!< Date day (1-31) (BCD)
    uint8_t hour;       //!< Time hour (0-23) (BCD)
    uint8_t minute;     //!< Time minute (0-59) (BCD)
    uint8_t second;     //!< Time second (0-59) (BCD)
    uint8_t weekDay;    //!< Day of the week (0-6) (BCD)
} EvmuTimestamp;

//! Filesystem root FAT block
typedef struct EvmuRootBlock {
    //! Set to 0x55 to signify formatted device
    uint8_t formatted[EVMU_FAT_ROOT_BLOCK_FORMATTED_SIZE];
    union {
        struct {
            uint8_t customColor; //!< 1 if using color, 0 otherwise
            uint8_t b;           //!< Custom color blue channel (0-255)
            uint8_t g;           //!< Custom color green channel (0-255)
            uint8_t r;           //!< Custom color red channel (0-255)
            uint8_t a;           //!< Custom color alpha channel (0-255
        } vmu;
        //! Raw byte size of volume label (32 bytes, everything after vmu struct is unknown/unused)
        uint8_t 	bytes[EVMU_FAT_ROOT_BLOCK_VOLUME_LABEL_SIZE];
    } volumeLabel;
    EvmuTimestamp timestamp;     //!< Timestamp when device was formatted (BCD)
    //! Reserved or unused, all zeroes
    uint8_t       reserved[EVMU_FAT_ROOT_BLOCK_RESERVED_SIZE];
    uint16_t	  totalSize;     //!< Total size of partition in blocks
    uint16_t 	  partition;     //!< Partition number (default: 0)
    uint16_t	  rootBlock;     //!< Location of Root block (default: 255)
    uint16_t      fatBlock;      //!< Location of FAT table (default: 254)
    uint16_t      fatSize;       //!< Size of FAT table in blocks (default: 1)
    uint16_t      dirBlock;      //!< Location of Directory (default: 253)
    uint16_t      dirSize;       //!< Size of Directory in blocks (default: 13)
    uint8_t       iconShape;     //!< Icon type or shape (built into BIOS font: 0-253)
    uint8_t       sortFlag;      //!< Sort flag? (no fucking idea)
    uint16_t	  extraBlock;    //!< Location of Extra region (default: 200)
    uint16_t      extraSize;     //!< Size of Extra region in blocks (default: 41)
    uint16_t      gameBlock;     //!< Starting location for GAME file (default: 0)
    uint16_t      gameSize;      //!< Maximum size of GAME file (default: 128?)
    //! Reserved or unused, all zeroes
    uint8_t		  reserved2[EVMU_FAT_ROOT_BLOCK_RESERVED2_SIZE];
} EvmuRootBlock;

//! Type of file stored on the filesystem
typedef enum EVMU_FILE_TYPE {
    EVMU_FILE_TYPE_NONE = 0x00, //!< Not a file
    EVMU_FILE_TYPE_DATA = 0x33, //!< Save DATA file
    EVMU_FILE_TYPE_GAME = 0xcc  //!< Mini GAME file
} EVMU_FILE_TYPE;

//! Copy protection type byte
typedef enum EVMU_COPY_TYPE {
    EVMU_COPY_TYPE_OK        = 0x00, //!< Not copy protected
    EVMU_COPY_TYPE_PROTECTED = 0xff, //!< Copy protected
    EVMU_COPY_TYPE_UNKNOWN   = 0x01  //!< Unknown/Other
} EVMU_COPY_PROTECTION;

//! Represents a single entry into the FAT directory
typedef struct EvmuDirEntry {
    uint8_t       fileType;         //!< EVMU_FILE_TYPE for file
    uint8_t       copyProtection;   //!< EVMU_COPY_TYPE for file
    uint16_t      firstBlock;       //!< Location of first FAT block
    //! File name: Shift-JS encoding without NULL terminator
    char          fileName[EVMU_FAT_DIRECTORY_FILE_NAME_SIZE];
    EvmuTimestamp timestamp;        //!< File creation date timestamp
    uint16_t      fileSize;         //!< Size of file in blocks
    uint16_t      headerOffset;     //!< Offset of VMS header in blocks (1 for GAME, 0 for DATA)
    //! Unused/reserved bytes (all 0s)
    uint8_t       unused[EVMU_FAT_DIRECTORY_UNUSED_SIZE];
} EvmuDirEntry;

//! User callabck for iterating over all directory entries, return GBL_TRUE to break early
typedef GblBool (*EvmuDirEntryIterFn)(EvmuDirEntry* pEntry, void* pClosure);

GBL_STATIC_ASSERT(sizeof(EvmuDirEntry) == 32);

//! Struct used for querying current FAT block allocation status
typedef struct EvmuFlashUsage {
    uint16_t blocksUsed;    //!< Total number of allocated blocks
    uint16_t blocksFree;    //!< Number of available blocks
    uint16_t blocksHidden;  //!< Number of unavailable, hidden system blocks
    uint16_t blocksDamaged; //!< Number of unusable, damaged blocks
} EvmuFlashUsage;

//! Matches maple attributes for describing storage medium
typedef struct EvmuFatInfo {
    uint8_t  partitions;      //!< Number of partitions on the device
    uint16_t blockSize;       //!< Number of bytes within a single block
    struct {
        uint8_t reads:  4;    //!< Number of reads required to fetch whole block of data
        uint8_t writes: 4;    //!< Number of writes required to fetch whole block of data
    } accessesPerBlock;       //!< Number of accesses required to operate on a whole block of data
    struct {
        uint8_t removable: 1; //!< 1 if medium is a removable device, 0 if fixed
        uint8_t crcCheck:  1; //!< 1 if CRC calculation is required for read/writes, 0 otherwise
        uint8_t:           6; //!< Unused/reserved, set to 0s
    } config;                 //!< Additional configuration info
} EvmuFatInfo;

/*! \struct  EvmuFatClass
 *  \extends EvmuFlashClass
 *  \brief   GblClass structure for EvmuFat
 *
 *  Virtual table structure for EvmuFat, providing overridable
 *  virtual methods for returning the storage medium info. The
 *  default returns the expected configuration for a VMU.
 *
 *  \sa EvmuFat
 */
GBL_CLASS_DERIVE(EvmuFat, EvmuFlash)
    EVMU_RESULT (*pFnInfo)(GBL_CSELF, EvmuFatInfo* pInfo);      //!< Virtual method for fetching volume info
    EVMU_RESULT (*pFnRoot)(GBL_CSELF, EvmuRootBlock** ppRoot);  //!< Virtual method for fetching root block data
GBL_CLASS_END

/*! \struct  EvmuFat
 *  \extends EvmuFlash
 *  \ingroup file_system
 *  \brief   Peripheral providing 8-bit FAT API
 *
 *  EvmuFat provides the following 3 filesystem APIs:
 *  * Block-based reads/writes
 *  * FAT Table allocation and management
 *  * Directory allocation and management
 *
 *  \sa EvmuFatClass
 */
GBL_INSTANCE_DERIVE_EMPTY(EvmuFat, EvmuFlash)

//! \cond
GBL_PROPERTIES(EvmuFat,
    (color,         GBL_GENERIC, (READ, WRITE), GBL_UINT32_TYPE),
    (icon,          GBL_GENERIC, (READ, WRITE), GBL_UINT8_TYPE),
    (formatted,     GBL_GENERIC, (READ),        GBL_BOOL_TYPE),
    (fatSize,       GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (directorySize, GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (userSize,      GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (gameSize,      GBL_GENERIC, (READ),        GBL_UINT32_TYPE),
    (blocksUsed,    GBL_GENERIC, (READ),        GBL_UINT16_TYPE),
    (blocksFree,    GBL_GENERIC, (READ),        GBL_UINT16_TYPE),
    (blocksHidden,  GBL_GENERIC, (READ),        GBL_UINT16_TYPE),
    (blocksDamaged, GBL_GENERIC, (READ),        GBL_UINT16_TYPE)
)
//! \endcond

// ===== EvmuDirEntry utilities =======
//! @{
EVMU_EXPORT const char*    EvmuDirEntry_name         (const EvmuDirEntry* pSelf,
                                                      GblStringBuffer*    pStr)              GBL_NOEXCEPT;

EVMU_EXPORT void           EvmuDirEntry_setName      (EvmuDirEntry* pSelf, const char* pSt)  GBL_NOEXCEPT;
EVMU_EXPORT const char*    EvmuDirEntry_fileTypeStr  (const EvmuDirEntry* pSelf)             GBL_NOEXCEPT;
EVMU_EXPORT const char*    EvmuDirEntry_protectedStr (const EvmuDirEntry* pSelf)             GBL_NOEXCEPT;

EVMU_EXPORT GblDateTime*   EvmuTimestamp_dateTime    (const EvmuTimestamp* pSelf,
                                                      GblDateTime*         pDateTime)        GBL_NOEXCEPT;

EVMU_EXPORT void           EvmuTimestamp_setDateTime (EvmuTimestamp*     pSelf,
                                                      const GblDateTime* pDateTime)          GBL_NOEXCEPT;
//! @}

EVMU_EXPORT GblType        EvmuFat_type              (void)                                  GBL_NOEXCEPT;

// ===== Mid-level FAT API =======
//! @{
EVMU_EXPORT EvmuRootBlock* EvmuFat_root        (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT    EvmuFat_info        (GBL_CSELF, EvmuFatInfo* pInfo)         GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT    EvmuFat_format      (GBL_CSELF, const EvmuRootBlock* pRoot) GBL_NOEXCEPT;
EVMU_EXPORT GblBool        EvmuFat_isFormatted (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_capacity    (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_toBlocks    (GBL_CSELF, size_t bytes)               GBL_NOEXCEPT;
EVMU_EXPORT void           EvmuFat_usage       (GBL_CSELF, EvmuFlashUsage* pUsage)     GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_userBlocks  (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT void           EvmuFat_log         (GBL_CSELF)                             GBL_NOEXCEPT;

EVMU_EXPORT void           EvmuFat_logMemoryUsage    (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_seqFreeBlocks     (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_blockSize         (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_blockCount        (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT EvmuBlock      EvmuFat_blockTable        (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT EvmuBlock      EvmuFat_blockDirectory    (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT void*          EvmuFat_blockData         (GBL_CSELF, EvmuBlock block)            GBL_NOEXCEPT;
EVMU_EXPORT EvmuBlock      EvmuFat_blockNext         (GBL_CSELF, EvmuBlock block)            GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT    EvmuFat_blockLink         (GBL_CSELF, EvmuBlock b, EvmuBlock lnk) GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT    EvmuFat_blockFree         (GBL_CSELF, EvmuBlock block)            GBL_NOEXCEPT;
EVMU_EXPORT EvmuBlock      EvmuFat_blockAlloc        (GBL_CSELF,
                                         EvmuBlock      prev,
                                         EVMU_FILE_TYPE type)                   GBL_NOEXCEPT;
//! @}

//======== Mid-level Directory API (stays private) ========
//! @{
EVMU_EXPORT size_t         EvmuFat_dirEntryCount     (GBL_CSELF)                             GBL_NOEXCEPT;
EVMU_EXPORT EvmuDirEntry*  EvmuFat_dirEntry          (GBL_CSELF, size_t index)               GBL_NOEXCEPT;
EVMU_EXPORT size_t         EvmuFat_dirEntryIndex     (GBL_CSELF, const EvmuDirEntry* pEntry) GBL_NOEXCEPT;
EVMU_EXPORT EvmuDirEntry*  EvmuFat_dirEntryAlloc     (GBL_CSELF, EVMU_FILE_TYPE fileType)    GBL_NOEXCEPT;
EVMU_EXPORT void           EvmuFat_dirEntryLog       (GBL_CSELF, const EvmuDirEntry* pEntry) GBL_NOEXCEPT;
EVMU_EXPORT GblBool        EvmuFat_dirEntryForeach   (GBL_CSELF,
                                                      EvmuDirEntryIterFn pFnIt,
                                                      void*              pClosure)           GBL_NOEXCEPT;
//! @}


GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FAT_H

/*! \file
 *  \brief EvmuDirEntry and other filesystem utilities
 *  \ingroup file_system
 *
 *  This file contains accessors for EvmuDirEntry as
 *  well as other general-purpose filesystem related
 *  functionality.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_DIR_ENTRY_H
#define EVMU_DIR_ENTRY_H

#include "../evmu_api.h"
#include <gimbal/utils/gimbal_date_time.h>

/*! \name Directory Entry Info
 *  \brief Struct and field sizes for EvmuDirEntry
 *  @{
 */
#define EVMU_DIRECTORY_ENTRY_SIZE       32  //!< Size in bytes of a directory entry
#define EVMU_DIRECTORY_FILE_NAME_SIZE   12  //!< Maximum file name size in bytes for a directory entry (no NULL terminator)
#define EVMU_DIRECTORY_UNUSED_SIZE      4   //!< Size of unused region in a directory entry
//! @}

GBL_DECLS_BEGIN

//! Filesystem timestamp, stored in BCD format
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

/*! Represents a single entry into the FAT directory
 *  \ingroup file_system
 */
typedef struct EvmuDirEntry {
    uint8_t       fileType;         //!< EVMU_FILE_TYPE for file
    uint8_t       copyProtection;   //!< EVMU_COPY_TYPE for file
    uint16_t      firstBlock;       //!< Location of first FAT block
    //! File name: Shift-JS encoding without NULL terminator
    char          fileName[EVMU_DIRECTORY_FILE_NAME_SIZE];
    EvmuTimestamp timestamp;        //!< File creation date timestamp
    uint16_t      fileSize;         //!< Size of file in blocks
    uint16_t      headerOffset;     //!< Offset of VMS header in blocks (1 for GAME, 0 for DATA)
    //! Unused/reserved bytes (all 0s)
    uint8_t       unused[EVMU_DIRECTORY_UNUSED_SIZE];
} EvmuDirEntry;

//! User callabck for iterating over all directory entries, return GBL_TRUE to break early
typedef GblBool (*EvmuDirEntryIterFn)(EvmuDirEntry* pEntry, void* pClosure);

GBL_STATIC_ASSERT(sizeof(EvmuDirEntry) == 32);

/*! \name Accessor Methods
 *  \brief EvmuDirEntry read/write methods
 *  \relatesalso EvmuDirEntry
 *  @{
 */
//! Fills the buffer with the EvmuDirEntry::fileName field and returns a pointer to its internal C string
EVMU_EXPORT const char* EvmuDirEntry_name         (const EvmuDirEntry* pSelf,
                                                   GblStringBuffer*    pStr)  GBL_NOEXCEPT;
//! Writes the given string to the EvmuDirEntry::fileName field, returning number of bytes written
EVMU_EXPORT size_t      EvmuDirEntry_setName      (EvmuDirEntry* pSelf,
                                                   const char*   pStr)        GBL_NOEXCEPT;
//! Returns the a string representation of EvmuDirEntry::fileType
EVMU_EXPORT const char* EvmuDirEntry_fileTypeStr  (const EvmuDirEntry* pSelf) GBL_NOEXCEPT;
//! Returns the a string representation of EvmuDirEntry::copyProtection
EVMU_EXPORT const char* EvmuDirEntry_protectedStr (const EvmuDirEntry* pSelf) GBL_NOEXCEPT;
//! @}

/*! \name Conversion Methods
 *  \brief Methods for going to/from GblDateTime
 *  \relatesalso EvmuTimestamp
 *  @{
 */
//! Converts the given EvmuTimestamp into the given GblDateTime, also returning it
EVMU_EXPORT GblDateTime* EvmuTimestamp_dateTime    (const EvmuTimestamp* pSelf,
                                                    GblDateTime*         pDateTime) GBL_NOEXCEPT;
//! Converts the given GblDateTime into an EvmuTimestamp
EVMU_EXPORT void         EvmuTimestamp_setDateTime (EvmuTimestamp*     pSelf,
                                                    const GblDateTime* pDateTime)   GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#endif // EVMU_DIR_ENTRY_H

/*! \file
 *  \brief EvmuFileManager software peripheral
 *  \ingroup file_system
 *
 *  EvmuFileManager offers a high-level, file-oriented API
 *  above both EvmuFlash and EvmuFat. It's meant to be the
 *  main entry point for loading and exporting ROM images.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_FILE_MANAGER_H
#define EVMU_FILE_MANAGER_H

#include "evmu_fat.h"

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_FILE_MANAGER_TYPE              (GBL_TYPEID(EvmuFileManager))            //!< Type UUID for EvmuFileManager
#define EVMU_FILE_MANAGER(self)             (GBL_CAST(EvmuFileManager, self))        //!< Function-style GblInstance cast
#define EVMU_FILE_MANAGER_CLASS(klass)      (GBL_CLASS_CAST(EvmuFileManager, klass)) //!< Function-style GblClass cast
#define EVMU_FILE_MANAGER_GET_CLASS(self)   (GBL_CLASSOF(EvmuFileManager, self))     //!< Get EvmuFileManagerClass from GblInstance
//! @}

#define EVMU_FILE_MANAGER_NAME              "filemanager"   //!< EvmuFileManager GblObject name

#define GBL_SELF_TYPE EvmuFileManager

GBL_DECLS_BEGIN

/*! \defgroup file_formats File Formats
 *  \ingroup  file_system
 *  \brief    Supported file formats
 *
 *  This module contains all APIs pertaining to
 *  importing, exporting, encoding, or decoding
 *  the various VMU-specific file formats.
 */

GBL_FORWARD_DECLARE_STRUCT(EvmuFileManager)

/*! \struct  EvmuFileManagerClass
 *  \extends EvmuFatClass
 *  \brief   GblClass vtable structure for EvmuFileManager
 *
 *  \sa EvmuFileManager
 */
GBL_CLASS_DERIVE(EvmuFileManager, EvmuFat)
    EVMU_RESULT (*pFnLoad)  (GBL_SELF, const char* pPath);
    EVMU_RESULT (*pFnSave)  (GBL_SELF, const char* pPath);
    EVMU_RESULT (*pFnExport)(GBL_CSELF, const EvmuDirEntry* pEntry, const char* pPath);
GBL_CLASS_END

/*! \struct  EvmuFileManager
 *  \extends EvmuFat
 *  \ingroup file_system
 *  \brief   High-level file-oriented flash API
 *
 *  EvmuFileManager is the most high-level, user-friendly
 *  way to manage the VMU's filesystem and handle loading
 *  and saving both both individual ROM images and the
 *  entire flash filesystem.
 *
 *  \sa EvmuFileManagerClass
 */
GBL_INSTANCE_DERIVE_EMPTY(EvmuFileManager, EvmuFat)

//! \cond
GBL_SIGNALS(EvmuFileManager,
    (fileAdded,   (GBL_INSTANCE_TYPE, pReceiver), (GBL_POINTER_TYPE, pDirEntry)),
    (fileRemoved, (GBL_INSTANCE_TYPE, pReceiver), (GBL_POINTER_TYPE, pDirEntry))
)
//! \endcond

//! Returns the GblType UUID associated with EvmuFileManager
EVMU_EXPORT GblType EvmuFileManager_type (void) GBL_NOEXCEPT;

/*! \name Filesystem Operations
 *  \brief Methods for serializing and deserializing the filesystem
 *  \relatesalso EvmuFileManager
 *  @{
 */
//! Loads a generic image whose type is determined by its extension into flash
EVMU_EXPORT EVMU_RESULT EvmuFileManager_load   (GBL_SELF, const char* pPath)  GBL_NOEXCEPT;
//! Saves a binary image of the entire contents of flash to a file, whose format is determined by its extension
EVMU_EXPORT EVMU_RESULT EvmuFileManager_save   (GBL_CSELF, const char* pPath) GBL_NOEXCEPT;
//! Defragments the filesystem, consolidating all free blocks, storing all files contiguously
EVMU_EXPORT EVMU_RESULT EvmuFileManager_defrag (GBL_SELF)                     GBL_NOEXCEPT;
//! @}

/*! \name File Discovery
 *  \brief Methods for enumerating, looking-up, and retrieving files
 *  \relatesalso EvmuFileManager
 *  @{
 */
//! Returns the number of files which are currently loaded within the filesystem
EVMU_EXPORT size_t        EvmuFileManager_count    (GBL_CSELF)                    GBL_NOEXCEPT;
//! Returns the directory entry for the file at the given \p index, or NULL if there isn't one
EVMU_EXPORT EvmuDirEntry* EvmuFileManager_file     (GBL_CSELF, size_t index)      GBL_NOEXCEPT;
//! Returns the directory entry for the currently loaded GAME file, or NULL if there isn't one
EVMU_EXPORT EvmuDirEntry* EvmuFileManager_game     (GBL_CSELF)                    GBL_NOEXCEPT;
//! Returns the directory entry for the currently loaded ICONDATA.VMS file, or NULL if there isn't one
EVMU_EXPORT EvmuDirEntry* EvmuFileManager_iconData (GBL_CSELF)                    GBL_NOEXCEPT;
//! Searches for a directory entry with the given name, returning it if found or returning NULL if not found
EVMU_EXPORT EvmuDirEntry* EvmuFileManager_find     (GBL_CSELF, const char* pName) GBL_NOEXCEPT;
//! Iterates over directory entries, passing each with \p pClosure to \p pFnIt, returning early with GBL_TRUE if \p pFnIt returns GBL_TRUE
EVMU_EXPORT GblBool       EvmuFileManager_foreach  (GBL_CSELF,
                                                    EvmuDirEntryIterFn pFnIt,
                                                    void*              pClosure)  GBL_NOEXCEPT;
//! @}

/*! \name File Operations
 *  \brief Methods for operating on and with files
 *  \relatesalso EvmuFileManager
 *  @{
 */
//! Creates storage for a new file with the given info, copying its contents into the filesystem
EVMU_EXPORT EvmuDirEntry* EvmuFileManager_alloc  (GBL_SELF,
                                                  EvmuNewFileInfo* pInfo,
                                                  const void*      pData)      GBL_NOEXCEPT;
//! Destroys an existing file, releasing resources back to the filesystem
EVMU_EXPORT size_t        EvmuFileManager_free   (GBL_SELF,
                                                  EvmuDirEntry* pEntry)        GBL_NOEXCEPT;
//! Performs a generic read from an existing file, returning the number of bytes successfully read
EVMU_EXPORT size_t        EvmuFileManager_read   (GBL_CSELF,
                                                  const EvmuDirEntry* pEntry,
                                                  void*               pBuffer,
                                                  size_t              size,
                                                  size_t              offset,
                                                  GblBool             inclHdr) GBL_NOEXCEPT;
//! Performs a generic write from an existing file, returning the number of bytes successfully written
EVMU_EXPORT size_t        EvmuFileManager_write  (GBL_CSELF,
                                                  const EvmuDirEntry* pEntry,
                                                  const void*         pBuffer,
                                                  size_t              size,
                                                  size_t              offset)  GBL_NOEXCEPT;
//! Exports an existing file to the given path, with the file format automatically deduced from the extension type
EVMU_EXPORT EVMU_RESULT   EvmuFileManager_export (GBL_CSELF,
                                                  const EvmuDirEntry* pEntry,
                                                  const char*         pPath)   GBL_NOEXCEPT;
//! @}

/*! \name File Information
 *  \brief Methods for calculating and retrieving file info
 *  \relatesalso EvmuFileManager
 *  @{
 */
//! Returns the total byte size of the file on the filesystem, including the VMS header, icons, eyecatc, etc.
EVMU_EXPORT size_t   EvmuFileManager_bytes (GBL_CSELF,
                                            const EvmuDirEntry* pEntry) GBL_NOEXCEPT;
//! Returns the file index corresponding to a given directory entry for a file
EVMU_EXPORT size_t   EvmuFileManager_index (GBL_CSELF,
                                            const EvmuDirEntry* pEntry) GBL_NOEXCEPT;
//! Returns the VMS header segment (ONLY) for an existing file (not the entire VMS data with icons, eyecatch, etc)
EVMU_EXPORT EvmuVms* EvmuFileManager_vms   (GBL_CSELF,
                                            const EvmuDirEntry* pEntry) GBL_NOEXCEPT;
//! Calculates the 16-bit CRC for an existing file on the filesystem
EVMU_EXPORT uint16_t EvmuFileManager_crc   (GBL_CSELF,
                                            const EvmuDirEntry* pEntry) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FILE_MANAGER_H

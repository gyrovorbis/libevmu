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

#define EVMU_FILE_MANAGER_TYPE              (GBL_TYPEOF(EvmuFileManager))                   //!< Type UUID for EvmuFileManager
#define EVMU_FILE_MANAGER(self)             (GBL_INSTANCE_CAST(self, EvmuFileManager))      //!< Function-style GblInstance cast
#define EVMU_FILE_MANAGER_CLASS(klass)      (GBL_CLASS_CAST(klass, EvmuFileManager))        //!< Function-style GblClass cast
#define EVMU_FILE_MANAGER_GET_CLASS(self)   (GBL_INSTANCE_GET_CLASS(self, EvmuFileManager)) //!< Get EvmuFileManagerClass from GblInstance

#define EVMU_FILE_MANAGER_NAME              "filemanager"   //!< EvmuFileManager GblObject name

#define GBL_SELF_TYPE EvmuFileManager

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuFileManager)

/*! \struct  EvmuFileManagerClass
 *  \extends EvmuFatClass
 *  \brief   GblClass vtable structure for EvmuFileManager
 *
 *  \sa EvmuFileManager
 */
GBL_CLASS_DERIVE(EvmuFileManager, EvmuFat)

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
GBL_INSTANCE_DERIVE(EvmuFileManager, EvmuFat)

GBL_INSTANCE_END

//========= High-level File API =========
EVMU_EXPORT GblType       EvmuFileManager_type    (void)                         GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT   EvmuFileManager_defrag  (GBL_SELF)                     GBL_NOEXCEPT;

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_alloc   (GBL_SELF,
                                                   EvmuNewFileInfo* pInfo,
                                                   const void*      pData,
                                                   size_t           size)        GBL_NOEXCEPT;

EVMU_EXPORT size_t        EvmuFileManager_free    (GBL_SELF,
                                                   EvmuDirEntry* pEntry)         GBL_NOEXCEPT;

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_game    (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT size_t        EvmuFileManager_count   (GBL_CSELF)                    GBL_NOEXCEPT;
EVMU_EXPORT EvmuDirEntry* EvmuFileManager_file    (GBL_CSELF, size_t index)      GBL_NOEXCEPT;

EVMU_EXPORT size_t        EvmuFileManager_index   (GBL_CSELF,
                                                   const EvmuDirEntry* pEntry)   GBL_NOEXCEPT;

EVMU_EXPORT EvmuVms*      EvmuFileManager_vms     (GBL_CSELF,
                                                   const EvmuDirEntry* pEntry)   GBL_NOEXCEPT;

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_find    (GBL_CSELF, const char* pName) GBL_NOEXCEPT;

EVMU_EXPORT GblBool       EvmuFileManager_foreach (GBL_CSELF,
                                                   EvmuDirEntryIterFn pFnIt,
                                                   void*              pClosure)  GBL_NOEXCEPT;

EVMU_EXPORT size_t        EvmuFileManager_read    (GBL_CSELF,
                                                   const EvmuDirEntry* pEntry,
                                                   void*               pBuffer,
                                                   size_t              size,
                                                   size_t              offset,
                                                   GblBool             inclHdr)  GBL_NOEXCEPT;

EVMU_EXPORT size_t        EvmuFileManager_write   (GBL_CSELF,
                                                   EvmuDirEntry* pEntry,
                                                   const void*   pBuffer,
                                                   size_t        size,
                                                   size_t        offset)         GBL_NOEXCEPT;

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_import  (GBL_SELF,
                                                   const char* pPath,
                                                   GblFlags    flags)            GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT   EvmuFileManager_export  (GBL_CSELF,
                                                   const EvmuDirEntry* pEntry,
                                                   const char*         pPath)    GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT   EvmuFileManager_load    (GBL_SELF,
                                                   const char* pPath,
                                                   GblFlags    flags)            GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT   EvmuFileManager_save    (GBL_CSELF, const char* pPath) GBL_NOEXCEPT;
// Copy?
// Extra blocks
//

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FILE_MANAGER_H

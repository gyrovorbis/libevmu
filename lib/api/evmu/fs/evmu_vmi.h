/*! \file
 *  \ingroup file_system
 *  \brief   Decoding and encoding of the .VMI format
 *
 *  This file contains the structure and API around
 *  managing and working with the .VMI file format,
 *  whose data is represented by the EvmuVmi structure.
 *
 *  The .VMI format basically serves as a metadata file
 *  descriptor for .VMS files, and were used as the VMU
 *  file format with Dreamcast web browsers.
 *
 *  A web browser would follow a link to the .VMI file,
 *  which would then point to a corresponding .VMS file,
 *  and the two together would provide enough information
 *  to load the file onto the VMU.
 *
 *  \warning
 *  Because the .VMI file is only a metadata descriptor
 *  file, it is never standalone, as it has no real data
 *  payload of its own. It must have a corresponding VMS
 *  file.
 *
 *  \note
 *  At the low level, the .VMI format is essentially
 *  providing the filesystem with the same information
 *  as an \ref EvmuDirEntry, such as the size of the
 *  corresponding file, its filesystem name, file type,
 *  copy protection mode, etc.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 *
 *  \sa evmu_vms.h
 */
#ifndef EVMU_VMI_H
#define EVMU_VMI_H

#include "evmu_fat.h"

#include <gimbal/strings/gimbal_string_buffer.h>
#include <gimbal/utils/gimbal_date_time.h>

/*! \name  Sizes and Limits
 *  \brief Sizes for EvmuVmi and its fields
 * @{
*/
#define EVMU_VMI_FILE_SIZE              108 //!< Size of a .vmi file the EvmuVmi structure
#define EVMU_VMI_DESCRIPTION_SIZE       32  //!< Size of the EvmuVmi::description field
#define EVMU_VMI_COPYRIGHT_SIZE         32  //!< Size of the EvmuVmi::copyright field
#define EVMU_VMI_VMS_RESOURCE_SIZE      8   //!< Size of the EvmuVmi::vmsResourceName field
#define EVMU_VMI_VMS_NAME_SIZE          12  //!< Size of the EvmuVmi::fileNameOnVms field
//! @}

/*! \name  File Mode Flags
 *  \brief Bit positions and masks for EvmuVmi::fileMode fields
 * @{
*/
#define EVMU_VMI_GAME_POS               1   //!< Bit of the GAME flag within EvmuVmi::fileMode
#define EVMU_VMI_GAME_MASK              0x2 //!< Mask of the GAME flag within EvmuVmi::fileMode
#define EVMU_VMI_ROTECTED_POS           0   //!< Bit of the PROTECTED flag within EvmuVmi::fileMode
#define EVMU_VMI_PROTECTED_MASK         0x1 //!< Mask of the PROTECTED flag within EvmuVmi::fileMode
//! @}

#define EVMU_VMI_VERSION                0   //!< Value of the EvmuVmi::vmiVersion field

#define GBL_SELF_TYPE EvmuVmi

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuVms);

/*! Structure of the .VMI file format
 *  \ingroup file_system
 *
 *  EvmuVmi offers a structure and C API around
 *  the decoded .VMI file format.
 *
 *  \note
 *  All string fields are in Shift-JS format and
 *  are NOT NULL-terminated, hence the accessor
 *  utility methods.
 *
 *  \sa EvmuVms
 */
typedef struct EvmuVmi {
    uint32_t      checksum;                                     //!< Checksum value for entire structure
    char          description[EVMU_VMI_DESCRIPTION_SIZE];       //!< Description of VMI file string
    char          copyright[EVMU_VMI_COPYRIGHT_SIZE];           //!< Copyright information string
    EvmuTimestamp creationTimestamp;                            //!< File creation date
    uint16_t      vmiVersion;                                   //!< VMI version of the file, see \ref EVMU_VMI_VERSION
    uint16_t      fileNumber;                                   //!< File number in a series
    char          vmsResourceName[EVMU_VMI_VMS_RESOURCE_SIZE];  //!< File name of the corresponding VMS file, expected within the same directory
    char          fileNameOnVms[EVMU_VMI_VMS_NAME_SIZE];        //!< File name field within the .VMS file
    uint16_t      fileMode;                                     //!< File mode bitfield (GAME bit + PROTECTED bit)
    uint16_t      unknown;                                      //!< Unknown and undocumented (assumed to be 0)
    uint32_t      fileSize;                                     //!< File size of VMS (in bytes?)
} EvmuVmi;

GBL_STATIC_ASSERT(sizeof(EvmuVmi) == EVMU_VMI_FILE_SIZE)

/*! \name  Read Accessors
 *  \brief Methods for reading/Calculating fields
 *  \relatesalso EvmuVmi
 * @{
 */
//! Copies the EvmuVmi::description field to the given buffer
GBL_EXPORT const char*  EvmuVmi_description     (GBL_CSELF, GblStringBuffer* pBuff) GBL_NOEXCEPT;
//! Copies the EvmuVmi::copyright field to the given buffer
GBL_EXPORT const char*  EvmuVmi_copyright       (GBL_CSELF, GblStringBuffer* pBuff) GBL_NOEXCEPT;
//! Converts the EvmuVmi::creationTimestamp field to GblDateTime
GBL_EXPORT GblDateTime* EvmuVmi_creation        (GBL_CSELF, GblDateTime* pDateTime) GBL_NOEXCEPT;
//! Copies the EvmuVmi::vmsResourceName field to the given buffer
GBL_EXPORT const char*  EvmuVmi_vmsResource     (GBL_CSELF, GblStringBuffer* pBuff) GBL_NOEXCEPT;
//! Copies the EvmuVmi::fileNameOnVms field to the given buffer
GBL_EXPORT const char*  EvmuVmi_fileName        (GBL_CSELF, GblStringBuffer* pbuff) GBL_NOEXCEPT;
//! Returns whether the EvmuVmi::fileMode type field signifies a GAME file
GBL_EXPORT GblBool      EvmuVmi_isGame          (GBL_CSELF)                         GBL_NOEXCEPT;
//! Returns whether the EvmuVmi::fileMode protected field signifies copy protection
GBL_EXPORT GblBool      EvmuVmi_isProtected     (GBL_CSELF)                         GBL_NOEXCEPT;
//! Computes the checksum for the given VMI data
GBL_EXPORT uint32_t     EvmuVmi_computeChecksum (GBL_CSELF)                         GBL_NOEXCEPT;
//! @}

/*! \name  Write Accessors
 *  \brief Methods for writing to fields
 *  \relatesalso EvmuVmi
 * @{
 */
//! Populates the given structure from a VMS file
GBL_EXPORT EVMU_RESULT  EvmuVmi_fromVms         (GBL_SELF, const EvmuVms* pVms)     GBL_NOEXCEPT;
//! Sets the EvmuVmi::description field to the given string, returning the number of bytes copied
GBL_EXPORT size_t       EvmuVmi_setDescription  (GBL_SELF, const char* pStr)        GBL_NOEXCEPT;
//! Sets the EvmuVmi::copyright field to the given string, returning the number of bytes copied
GBL_EXPORT size_t       EvmuVmi_setCopyright    (GBL_SELF, const char* pStr)        GBL_NOEXCEPT;
//! Sets the EvmuVmi::creationTimestamp field to the given GblDateTime value
GBL_EXPORT void         EvmuVmi_setCreation     (GBL_SELF, const GblDateTime* pDt)  GBL_NOEXCEPT;
//! Sets the EvmuVmi::vmsResourceName field to the given string, returning the number of bytes copied
GBL_EXPORT size_t       EvmuVmi_setVmsResource  (GBL_SELF, const char* pStr)        GBL_NOEXCEPT;
//! Sets the EvmuVmi::fileNameOnVms field to the given string, returning the number of bytes copied
GBL_EXPORT size_t       EvmuVmi_setFileName     (GBL_SELF, const char* pStr)        GBL_NOEXCEPT;
//! Sets the EvmuVmi::fileMode type field signifying  whether or not file is a GAME
GBL_EXPORT void         EvmiVmi_setGame         (GBL_SELF, GblBool val)             GBL_NOEXCEPT;
//! Sets the EvmuVmi::fileMode protected filed to signify whether or not the file is copy protected
GBL_EXPORT void         EvmuVmi_setProtected    (GBL_SELF, GblBool val)             GBL_NOEXCEPT;
//! @}

/*! \name Utilities
 *  \brief Miscellaneous methods
 *  \relatesalso EvmuVmi
 * @{
 */
//! Populates the given structure by loading its contents from an external .VMI file
GBL_EXPORT EVMU_RESULT  EvmuVmi_load            (GBL_SELF, const char* pPath)       GBL_NOEXCEPT;
//! Logs the fields of the VMI file to libGimbal
GBL_EXPORT void         EvmuVmi_log             (GBL_CSELF)                         GBL_NOEXCEPT;
//! Writes the contens of the given structure to an external .VMI file
GBL_EXPORT EVMU_RESULT  EvmuVmi_save            (GBL_CSELF, const char* pPath)      GBL_NOEXCEPT;
//! @}

#if 0
void        gyVmuVmiFileInfoResourceNameGet(const VMIFileInfo* info, char* string);
uint32_t    gyVmuVMIChecksumGenerate(const VMIFileInfo* info);
void        gyVmuFlashPrintVMIFileInfo(const VMIFileInfo* info);
void        gyVmuVmiGenerateFromVms(VMIFileInfo* vmi, const struct VMSFileInfo* vms, size_t vmsFileSize, VMI_FILE_MODE_GAME fileType);
#endif

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_VMI_H


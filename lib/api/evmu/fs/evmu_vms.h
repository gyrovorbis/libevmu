/*! \file
 *  \ingroup    file_formats
 *  \brief      EvmuVms: Decoding and encoding of the .VMS format
 *
 *  This file contains the structure and API around managing
 *  and working with the .VMS file format, whose header is
 *  represented by the EvmuVms structure.
 *
 *  \warning
 *  When distributing a .VMS file, there should almost
 *  ALWAYS be a corresponding .VMI file providing its metadata
 *  information to Web Browsers and the filesystem. While EVMU
 *  can load GAME .VMS files without a corresponding .VMI file,
 *  this is not something typically supported. Make sure you have
 *  one if you choose to distribute a ROM file!
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 *
 *  \sa evmu_vmi.h
 */
#ifndef EVMU_VMS_H
#define EVMU_VMS_H

#include "evmu_fat.h"

#include <gimbal/strings/gimbal_string_buffer.h>
#include <gimbal/utils/gimbal_byte_array.h>

/*! \name  Sizes and Limits
 *  \brief Sizes for EvmuVms and its fields
 * @{
*/
#define EVMU_VMS_SIZE                   128 //!< Size of the VMS header and EvmuVms structure
#define EVMU_VMS_VMU_DESCRIPTION_SIZE   16  //!< Size of the EvmuVms::vmuDesc field
#define EVMU_VMS_DC_DESCRIPTION_SIZE    32  //!< Size of the EvmuVms::dcDesc field
#define EVMU_VMS_CREATOR_APP_SIZE       16  //!< Size of the EvmuVms::creatorApp field
#define EVMU_VMS_RESERVED_SIZE          20  //!< Size of the EvmuVms::reserved field
//! @}

/*! \name  Palette Colors
 *  \brief RGBA Bitfields for icon pallette color format
 * @{
*/
#define EVMU_VMS_ICON_PALETTE_BLUE_POS      0       //!< Bit positon of the blue color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_BLUE_MASK     0x000f  //!< Bit mask of the blue color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_GREEN_POS     4       //!< Bit position of the green color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_GREEN_MASK    0x00f0  //!< Bit mask of the green color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_RED_POS       8       //!< Bit position of the red color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_RED_MASK      0x0f00  //!< Bit mask of the red color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_ALPHA_POS     12      //!< Bit position of the alpha color channel within an EvmuVms::palette entry
#define EVMU_VMS_ICON_PALETTE_ALPHA_MASK    0xf000  //!< Bit mask of the alpha color channel withiin an EvmuVms::palette entry
//! @}

/*! \name  Icons
 *  \brief Sizes and static info for VMS icons
 * @{
*/
#define EVMU_VMS_ICON_COUNT_MAX     3   //!< Maximum number of icons within a VMS file
#define EVMU_VMS_ICON_PALETTE_SIZE  16  //!< Number of entries within a VMS icon palette
#define EVMU_VMS_ICON_BITMAP_WIDTH  32  //!< Width of a VMS icon
#define EVMU_VMS_ICON_BITMAP_HEIGHT 32  //!< Height of a VMS icon
#define EVMU_VMS_ICON_BITMAP_SIZE   512 //!< Size of a single icon in bytes
//! @}

/*! \name  Eyecatch
 *  \brief Sizes and static info for VMS eyecatches
 * @{
*/
#define EVMU_VMS_EYECATCH_BITMAP_WIDTH              72      //!< Width of a VMS eyecatch
#define EVMU_VMS_EYECATCH_BITMAP_HEIGHT             56      //!< Height of a VMS eyecatch
#define EVMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_256    512     //!< Palette size of a VMS eyecatch using 256 color mode
#define EVMU_VMS_EYECATCH_PALETTE_SIZE_COLOR_16     32      //!< Palette size of a VMS eyecatch using 16 color mode
#define EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_16BIT   8064    //!< Size of a VMS eyecatch using 16-bit colors
#define EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_256     4032    //!< Size of a VMS eyecatch using a 256-color palette
#define EVMU_VMS_EYECATCH_BITMAP_SIZE_COLOR_16      2016    //!< Size of a VMS eyecatch using a 16-color palette
//! @}

#define GBL_SELF_TYPE EvmuVms

GBL_DECLS_BEGIN

//! Types of different eyecatch formats
typedef enum EVMU_VMS_EYECATCH_TYPE {
    EVMU_VMS_EYECATCH_NONE,         //!< None
    EVMU_VMS_EYECATCH_16BIT,        //!< 16-bit Color
    EVMU_VMS_EYECATCH_PALETTE_256,  //!< 256-Entry Palleted
    EVMU_VMS_EYECATCH_PALETTE_16,   //!< 16-Entry Palleted
    EVMU_VMS_EYECATCH_COUNT         //!< Number of Formats
} EVMU_VMS_EYECATCH_TYPE;

/*! Structure of the .VMS file header
 *  \ingroup file_formats
 *
 *  EvmuVms offers a struct and object-oriented API
 *  around the .VMS file format. This strucure only
 *  represents the header; however, the API expects
 *  that there is additional data beyond this header.
 *
 *  \note
 *  Strings are JIS X 0201 encocded (included within
 *  Shift-JIS), and are not NULL-terminateed (hence
 *  the accessor methods).
 *
 *  \sa EvmuVmi
 */
typedef struct EvmuVms {
    char     vmuDesc[EVMU_VMS_VMU_DESCRIPTION_SIZE]; //!< File description in VMU BIOS
    char     dcDesc[EVMU_VMS_DC_DESCRIPTION_SIZE];   //!< File description in DC BIOS
    char     creatorApp[EVMU_VMS_CREATOR_APP_SIZE];  //!< App used to create VMS file
    uint16_t iconCount;                              //!< Number of icons in animation
    uint16_t animSpeed;                              //!< Number of frames to wait before advancing icon animation
    uint16_t eyecatchType;                           //!< Type of eyecatch graphic
    uint16_t crc;                                    //!< CRC for whole file
    uint32_t dataBytes;                              //!< Size of actual file data, without header, icons, and eyecatch (ignored for GAME files)
    char     reserved[EVMU_VMS_RESERVED_SIZE];       //!< Reserved/unknown. Set to 0.
    uint16_t palette[EVMU_VMS_ICON_PALETTE_SIZE];    //!< Palette color entries
} EvmuVms;

/*! \name  Read Accessors
 *  \brief Reading/calculating fields
 *  \relatesalso EvmuVms
 * @{
*/
//! Determine whether the given VMS header appears sane based on expected field values
EVMU_EXPORT GblBool     EvmuVms_isValid        (GBL_CSELF)                GBL_NOEXCEPT;
//! Returns the number of bytes of the VMS header, including graphics
EVMU_EXPORT size_t      EvmuVms_headerBytes    (GBL_CSELF)                GBL_NOEXCEPT;
//! Retruns the total calculated size of the VMS file, including header, icons, and eyecatch
EVMU_EXPORT size_t      EvmuVms_totalBytes     (GBL_CSELF)                GBL_NOEXCEPT;
#if 0
//! Computes the expected CRC for the entire VMS file
EVMU_EXPORT uint16_t    EvmuVms_computeCrc     (GBL_CSELF)                GBL_NOEXCEPT;
//! Returns a pointer to the raw icon image data for the given \p index
EVMU_EXPORT const void* EvmuVms_icon           (GBL_CSELF, size_t index)  GBL_NOEXCEPT;
//! Returns a pointer to the raw eyecatch image data
EVMU_EXPORT const void* EvmuVms_eyecatch       (GBL_CSELF)                GBL_NOEXCEPT;
#endif
//! Copies EvmuVms::vmuDesc to the given buffer
EVMU_EXPORT const char*    EvmuVms_vmuDescription  (GBL_CSELF,
                                                    GblStringBuffer* pBuffer) GBL_NOEXCEPT;
//! Copies EvmuVms::dcDesc to the given buffer
EVMU_EXPORT const char*    EvmuVms_dcDescription   (GBL_CSELF,
                                                    GblStringBuffer* pBuffer) GBL_NOEXCEPT;
//! Copies EvmuVms::creatorApp to the given buffer
EVMU_EXPORT const char*    EvmuVms_creatorApp      (GBL_CSELF,
                                                    GblStringBuffer* pBuffer) GBL_NOEXCEPT;
//! Returns a string representation of the EvmuVms::eyecatchType
EVMU_EXPORT const char*    EvmuVms_eyecatchTypeStr (GBL_CSELF)                GBL_NOEXCEPT;
//! Attempts to autodetect the type of file represented by the VMS, usually for when no VMI is present
EVMU_EXPORT EVMU_FILE_TYPE EvmuVms_guessFileType   (GBL_CSELF)                GBL_NOEXCEPT;
//! @}

/*! \name  Write Accessors
 *  \brief Writing to fields
 *  \relatesalso EvmuVms
 * @{
*/
//! Sets the EvmuVms::vmuDesc field to the given string, returning the number of bytes copied
EVMU_EXPORT size_t EvmuVms_setVmuDescription (GBL_SELF, const char* pStr) GBL_NOEXCEPT;
//! Sets the EvmuVms::dcDesc field to the given string, returning the number of bytes copied
EVMU_EXPORT size_t EvmuVms_setDcDescription  (GBL_SELF, const char* pStr) GBL_NOEXCEPT;
//! Sets the EvmuVms::creatorApp field to the given string, returning the number of bytes copied
EVMU_EXPORT size_t EvmuVms_setCreatorApp     (GBL_SELF, const char* pStr) GBL_NOEXCEPT;
//! @}

/*! \name  Utilities
 *  \brief Miscellaneous methods
 *  \relatesalso EvmuVms
 * @{
*/
//! Logs the properties of the VMS file to the libGimbal log system
EVMU_EXPORT void          EvmuVms_log                    (GBL_CSELF) GBL_NOEXCEPT;
//! Creates a GblRingList of GblByteArray instances containing ARGB444-encoded bitmaps for each icon
EVMU_EXPORT GblRingList*  EvmuVms_createIconsArgb4444    (GBL_CSELF) GBL_NOEXCEPT;
//! Creates a GblByteArray containing an ARG444 encoded bitmap for the eyecatch
EVMU_EXPORT GblByteArray* EvmuVms_createEyecatchArgb4444 (GBL_CSELF) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_VMS_H

/*! \file
 *  \brief EvmuIconData: Header for the ICONDATA_VMS reserved file
 *  \ingroup file_formats
 *
 *  ICONDATA_VMS is a reserved filename on the VMU filesystem that
 *  is treated differently by the Dreamcast's BIOS menu. Rather
 *  than having the regular EvmuVms header structure, it has
 *  a structure given by EvmuIconData.
 *
 *  An ICONDATA_VMS file provides the following:
 *  - Custom VMU icon which is drawn to the VMU in the DC BIOS
 *  - Custom DC icon which is drawn to the screen in the DC BIOS
 *  - Optional byte sequence for unlocking the secret 3D DC BIOS
 *
 *  The VMU icon is a simple monochrome bitmap of exactly the
 *  resolution of the VMU screen while the Dreamcast icon is a
 *  32x32 icon with a 16-bit, 16-entry color palette.
 *
 *  \note
 *  In order to unlock the secret 3D Dreamcast BIOS, a particular
 *  byte sequence must appear within the ICONDATA_VMS file.
 *  This sequence must appear starting at byte offset 0x2c0 with the
 *  following 16 byte values:
 *  |Byte|Value|Byte|Value|
 *  |----|-----|----|-----|
 *  |0   |0xda |8   |0x18 |
 *  |1   |0x69 |9   |0x92 |
 *  |2   |0xd0 |10  |0x79 |
 *  |3   |0xda |11  |0x68 |
 *  |4   |0xc7 |12  |0x2d |
 *  |5   |0x4e |13  |0xb5 |
 *  |6   |0xf8 |14  |0x30 |
 *  |7   |0x36 |15  |0x86 |
 *
 *  \todo
 *  - Set icondata icon if there is no game
 *  - data present instead of the ES logo/screensaver
 *  - good description of what IconData is
 *
 *  \bug
 *  - Loading VMU image with both a game and an icondata
 *    just shows icon instead of starting game
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_ICONDATA_H
#define EVMU_ICONDATA_H

#include "evmu_fat.h"
#include <gimbal/utils/gimbal_byte_array.h>

//! Reserved EvmuDirEntry::fileName value for EvmuIconData files
#define EVMU_ICONDATA_VMS_FILE_NAME     "ICONDATA_VMS"

/*! \name Field Sizes
 *  \brief Sizes and dimensions for icons
 *  @{
 */
#define EVMU_ICONDATA_DESCRIPTION_SIZE  16  //!< Number of bytes in the EvmuIconData::description string
#define EVMU_ICONDATA_ICON_WIDTH        32  //!< Width of the DC icon in pixels
#define EVMU_ICONDATA_ICON_HEIGHT		32  //!< Height of the DC icon in pixels
#define EVMU_ICONDATA_VMU_ICON_BYTES    128 //!< Total number of bytes for the VMU icon
#define EVMU_ICONDATA_DC_PALETTE_SIZE	16  //!< Number of entries in the DC icon color palette
#define EVMU_ICONDATA_DC_PALETTE_BYTES	32  //!< Total number of bytes for the DC icon color palette
#define EVMU_ICONDATA_DC_ICON_BYTES     512 //!< Total number of bytes for the DC icon
//! @}

/*! \name Secret Bios
 *  \brief Definitions for secret BIOS information
 *  @{
 */
#define EVMU_ICONDATA_BIOS_SECRET_OFFSET     0x2c0  //!< Offset of the secret BIOS byte sequence
#define EVMU_ICONDATA_BIOS_SECRET_BYTE_COUNT 16     //!< Number of bytes in the secret BIOS byte sequence
//! @}

/*! \name Palette Colors
 *  \brief Extracting and packet palette colors
 *  @{
 */
#define EVMU_ICONDATA_PALETTE_ENTRY_A(c) ((c >> 12) & 0xf)  //!< Extracts the alpha channel from DC icon a palette entry
#define EVMU_ICONDATA_PALETTE_ENTRY_R(c) ((c >> 8 ) & 0xf)  //!< Extracts the red channel from a DC icon palette entry
#define EVMU_ICONDATA_PALETTE_ENTRY_G(c) ((c >> 4 ) & 0xf)  //!< Extracts the green channel from a DC icon palette entry
#define EVMU_ICONDATA_PALETTE_ENTRY_B(c) ((c      ) & 0xf)  //!< Extracts the blue channel from a DC icon palette entry
//! Packs the channels given by \p a, \p r, \p g, \p b, into a single 16-bit palette entry color
#define EVMU_ICONDATA_PALETTE_ENTRY(a, r, g, b) \
            ((uint16_t)(((a & 0xf) << 12) | ((r & 0xf) <<  8) | ((g & 0xf) <<  4) | ((b & 0xf))))
//! @}

#define GBL_SELF_TYPE EvmuIconData

GBL_DECLS_BEGIN

/*! Header structure for the ICONDATA_VMS special VMU file
 *  \ingroup file_formats
 *
 *  EvmuIconData is the VMS file payload for the special
 *  ICONDATA_VMS reserved file. It doesn't have all of the
 *  regular VMS header fields and is treated differently
 *  from regular VMS headers.
 */
typedef struct EvmuIconData {
    //! VMU filesystem description of file
    char 		description[EVMU_ICONDATA_DESCRIPTION_SIZE];
    uint32_t	vmuIconOffset; //!< Byte offset of the VMU icon from the header
    uint32_t	dcIconOffset;  //!< Byte offset of the DC icon from the header
} EvmuIconData;

//! Returns the 16-byte ICONDATA sequence for unlocking the secret Dreamcast BIOS
EVMU_EXPORT const uint8_t* EvmuIconData_unlockSecretBiosBytes (void) GBL_NOEXCEPT;

/*! \name Read Accessors
 *  \brief Methods for fetching data
 *  \relatesalso EvmuIconData
 *  @{
 */
//! Fills the given GblStringBuffer with the EvmuIconData::description field, returning its C string
EVMU_EXPORT const char*    EvmuIconData_description        (GBL_CSELF,
                                                            GblStringBuffer* pBuff) GBL_NOEXCEPT;
//! Returns GBL_TRUE if the secret 3D Dreamcast BIOS has been unlocked, GBL_FALSE otherwise
EVMU_EXPORT GblBool        EvmuIconData_secretBiosUnlocked (GBL_CSELF)              GBL_NOEXCEPT;
//! Returns a raw data pointer to the VMU icon, which is a monochrome bitmap
EVMU_EXPORT const void*    EvmuIconData_vmuIcon            (GBL_CSELF)              GBL_NOEXCEPT;
//! Returns a raw data pointer to the DC icon, which is a 4-bit paletted image where each color is 16-bit
EVMU_EXPORT const void*    EvmuIconData_dcIcon             (GBL_CSELF)              GBL_NOEXCEPT;
//! Returns the 16-bit color palette entry at index \p idx for the DC icon
EVMU_EXPORT uint16_t       EvmuIconData_dcPaletteEntry     (GBL_CSELF, size_t idx)  GBL_NOEXCEPT;
//! Returns a pointer to the beginning of the byte sequence for unlocking the secret DC BIOS
EVMU_EXPORT const uint8_t* EvmuIconData_secretBiosBytes    (GBL_CSELF)              GBL_NOEXCEPT;
//! Returns the total size of the EvmuIconData file on the filesystem in bytes
EVMU_EXPORT size_t         EvmuIconData_totalBytes         (GBL_CSELF)              GBL_NOEXCEPT;
//! @}

/*! \name Write Accessors
 *  \brief Methods for storing data
 *  \relatesalso EvmuIconData
 *  @{
 */
//! Sets the EvmuIconData::description field to the string given by \p pString, returning the number of bytes copied
EVMU_EXPORT size_t EvmuIconData_setDescription     (GBL_SELF,
                                                    const char* pString)   GBL_NOEXCEPT;
//! Locks or unlocks the secret 3D additional Dreamcast BIOS by writing a secret bit sequence
EVMU_EXPORT void   EvmuIconData_unlockSecretBios   (GBL_SELF,
                                                    GblBool unlock)        GBL_NOEXCEPT;
//! Sets the 16-byte sequence for unlocking the secret BIOS to the data pointed to by \p pBytes
EVMU_EXPORT void   EvmuIconData_setSecretBiosBytes (GBL_SELF,
                                                    const uint8_t* pBytes) GBL_NOEXCEPT;
//! @}

/*! \name Utilities
 *  \brief Miscellaneous methods
 *  \relatesalso EvmuIconData
 *  @{
 */
//! Creates and returns a GblByteArray containing an ARGB4444 16-bit format texture for the VMU icon
EVMU_EXPORT GblByteArray* EvmuIconData_createVmuIconArgb4444 (GBL_CSELF) GBL_NOEXCEPT;
//! Creats and returns a GblByteArray containing an ARGB4444 16-bit format texture for the DC icon
EVMU_EXPORT GblByteArray* EvmuIconData_createDcIconArgb4444  (GBL_CSELF) GBL_NOEXCEPT;
//! Dumps all attribute and field information for EvmuIconData to the libGimbal log system
EVMU_EXPORT EVMU_RESULT   EvmuIconData_log                   (GBL_CSELF) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif


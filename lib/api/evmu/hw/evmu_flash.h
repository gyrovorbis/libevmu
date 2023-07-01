/*! \file
 *  \brief EvmuFlash peripheral, 8-bit FAT filesystem API
 *  \ingroup file_system
 *
 *  This file contains a very low-level, hardware
 *  abstraction around the VMU's flash chip. It
 *  models it at the raw byte level for programming.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
*/

#ifndef EVMU_FLASH_H
#define EVMU_FLASH_H

#include "../types/evmu_peripheral.h"

#define EVMU_FLASH_TYPE                     (GBL_TYPEOF(EvmuFlash))                         //!< Type UUID for EvmuFlash
#define EVMU_FLASH(instance)                (GBL_INSTANCE_CAST(instance, EvmuFlash))        //!< Function-style GblInstance cast
#define EVMU_FLASH_CLASS(klass)             (GBL_CLASS_CAST(klass, EvmuFlash))              //!< Function-style GblClass cast
#define EVMU_FLASH_GET_CLASS(instance)      (GBL_INSTANCE_GET_CLASS(instance, EvmuFlash))   //!< Get EvmuFlashClass from GblInstance

#define EVMU_FLASH_BANK_SIZE                65536                                       //!< Size of a single flash bank
#define EVMU_FLASH_BANKS                    2                                           //!< Number of flash banks
#define EVMU_FLASH_SIZE                     (EVMU_FLASH_BANK_SIZE * EVMU_FLASH_BANKS)   //!< Total flash size in bytes

/*! \name Programming Sequence
 *  \brief Sequence of target addresses and values for write programming
 *  @{
 */
#define EVMU_FLASH_PROGRAM_BYTE_COUNT       128     //!< Number of bytes software can write to flash once unlocked
#define EVMU_FLASH_PROGRAM_STATE_0_ADDRESS  0x5555  //!< First target address for programming flash
#define EVMU_FLASH_PROGRAM_STATE_0_VALUE    0xaa    //!< First value to write when programming flash
#define EVMU_FLASH_PROGRAM_STATE_1_ADDRESS  0x2aaa  //!< Second target address for programming flash
#define EVMU_FLASH_PROGRAM_STATE_1_VALUE    0x55    //!< Second value to write when programming flash
#define EVMU_FLASH_PROGRAM_STATE_2_ADDRESS  0x5555  //!< Third target address for programming flash
#define EVMU_FLASH_PROGRAM_STATE_2_VALUE    0xa0    //!< Third value to write when programming flash
//! @}

#define GBL_SELF_TYPE EvmuFlash

GBL_DECLS_BEGIN

GBL_DECLARE_ENUM(EVMU_FLASH_PROGRAM_STATE) {
    EVMU_FLASH_PROGRAM_STATE_0,
    EVMU_FLASH_PROGRAM_STATE_1,
    EVMU_FLASH_PROGRAM_STATE_2,
    EVMU_FLASH_PROGRAM_STATE_COUNT
};

/*! \struct  EvmuFlashClass
 *  \extends EvmuPeripheralClass
 *  \brief   GblClass vtable structure for EvmuFlash
 *
 *  No public methods.
 *
 *  \sa EvmuFlash
 */
GBL_CLASS_DERIVE_EMPTY   (EvmuFlash, EvmuPeripheral)

/*! \struct  EvmuFlash
 *  \extends EvmuPeripheral
 *  \ingroup file_system
 *
 *  EvmuFlash offers the lowest, hardware-level access to
 *  the VMU's flash storage. Unless you know what you're
 *  doing, it's advised to work with a higher level API.
 *
 * \sa EvmuFat, EvmuFileManager
 */
GBL_INSTANCE_DERIVE_EMPTY(EvmuFlash, EvmuPeripheral)

GBL_PROPERTIES(EvmuFlash,
    (programUnlocked, GBL_GENERIC, (READ, WRITE), GBL_BOOL_TYPE),
    (programState,    GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (programBytes,    GBL_GENERIC, (READ, WRITE), GBL_UINT8_TYPE),
    (targetAddress,   GBL_GENERIC, (READ, WRITE), GBL_UINT32_TYPE)
)

EVMU_EXPORT GblType     EvmuFlash_type           (void)                           GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuFlash_programAddress (EVMU_FLASH_PROGRAM_STATE state) GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuFlash_programValue   (EVMU_FLASH_PROGRAM_STATE state) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_FLASH_PROGRAM_STATE
                        EvmuFlash_programState   (GBL_CSELF)                      GBL_NOEXCEPT;
EVMU_EXPORT size_t      EvmuFlash_programBytes   (GBL_CSELF)                      GBL_NOEXCEPT;
EVMU_EXPORT size_t      EvmuFlash_programCycles  (GBL_CSELF)                      GBL_NOEXCEPT;

EVMU_EXPORT EvmuAddress EvmuFlash_targetAddress  (GBL_CSELF)                      GBL_NOEXCEPT;
EVMU_EXPORT GblBool     EvmuFlash_unlocked       (GBL_CSELF)                      GBL_NOEXCEPT;

EVMU_EXPORT EvmuWord    EvmuFlash_readByte       (GBL_CSELF, EvmuAddress address) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuFlash_readBytes      (GBL_CSELF,
                                                  EvmuAddress base,
                                                  void*       pData,
                                                  size_t*     pBytes)             GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuFlash_writeByte      (GBL_CSELF,
                                                  EvmuAddress address,
                                                  EvmuWord    value)              GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuFlash_writeBytes     (GBL_CSELF,
                                                  EvmuAddress base,
                                                  const void* pData,
                                                  size_t*    pBytes)              GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FLASH_H


/*! \file
 *  \brief EvmuFlash peripheral, 8-bit FAT filesystem API
 *  \ingroup peripherals
 *
 *  This file contains a very low-level, hardware
 *  abstraction around the VMU's flash chip. It
 *  models it at the raw byte-level for programming.
 *
 *  \note
 *  Unless you're byte-banging your own CPU core or really
 *  want to do raw byte operations on flash, you most likely
 *  want to check out the two EvmuFlash derived types:
 *  * EvmuFat
 *  * EvmuFileManager
 *
 *  \sa evmu_fat.h, evmu_file_manager.h
 *
 *  \todo
 *  - Implement flash program wait cycles
 *  - Add capacity
 *
 *  \test
 *  - Unit tests flexing truncated read/writes + verifying signals
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
*/

#ifndef EVMU_FLASH_H
#define EVMU_FLASH_H

#include "../types/evmu_peripheral.h"
#include "../types/evmu_imemory.h"

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_FLASH_TYPE             (GBL_TYPEID(EvmuFlash))            //!< Type UUID for EvmuFlash
#define EVMU_FLASH(self)            (GBL_CAST(EvmuFlash, self))        //!< Cast GblInstance to EvmuFlash
#define EVMU_FLASH_CLASS(klass)     (GBL_CLASS_CAST(EvmuFlash, klass)) //!< Cast GblClass to EvmuFlashClass
#define EVMU_FLASH_GET_CLASS(self)  (GBL_CLASSOF(EvmuFlash, self))     //!< Get EvmuFlashClass from GblInstance
//! @}

/*! \name  Sizes
 *  \brief Sizes of flash and its banks
 *  @{
 */
#define EVMU_FLASH_BANK_SIZE    65536                                       //!< Size of a single flash bank
#define EVMU_FLASH_BANKS        2                                           //!< Number of flash banks
#define EVMU_FLASH_SIZE         (EVMU_FLASH_BANK_SIZE * EVMU_FLASH_BANKS)   //!< Total flash size in bytes
//! @}

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

GBL_DECLARE_STRUCT(EvmuFlash);

//! Current state in the flash programming sequence to unlock writing
typedef enum EVMU_FLASH_PROGRAM_STATE {
    EVMU_FLASH_PROGRAM_STATE_0,     //!< First state
    EVMU_FLASH_PROGRAM_STATE_1,     //!< Second state
    EVMU_FLASH_PROGRAM_STATE_2,     //!< Third state
    EVMU_FLASH_PROGRAM_STATE_COUNT  //!< Number of states
} EVMU_FLASH_PROGRAM_STATE;

/*! \struct  EvmuFlashClass
 *  \extends EvmuPeripheralClass
 *  \implements EvmuIMemoryClass
 *  \brief   GblClass VTable structure for EvmuFlash
 *
 *  Virtual method table for providing actual low-level
 *  flash access logic. The default implementation is
 *  constructing a byte array internally and is reading and
 *  writing to it.
 *
 *  \note
 *  On a microcontroller, this could be subclassed and these
 *  methods could be doing actual flash chip accesses.
 *
 *  \sa EvmuFlash
 */
GBL_CLASS_DERIVE_EMPTY(EvmuFlash, EvmuPeripheral, EvmuIMemory)

/*! \struct  EvmuFlash
 *  \extends EvmuPeripheral
 *  \implements EvmuIMemory
 *  \ingroup peripherals
 *
 *  EvmuFlash offers the lowest, hardware-level access to
 *  the VMU's flash storage. Unless you know what you're
 *  doing, it's advised to work with a higher level API.
 *
 * \sa EvmuFat, EvmuFileManager
 */
GBL_INSTANCE_DERIVE(EvmuFlash, EvmuPeripheral)
    //! User toggle: will be set after a flash value changes, you can reset and poll for changes
    GblBool dataChanged;
GBL_INSTANCE_END

//! \cond
GBL_PROPERTIES(EvmuFlash,
    (dataChanged,     GBL_GENERIC, (READ, WRITE, OVERRIDE), GBL_BOOL_TYPE),
    (programUnlocked, GBL_GENERIC, (READ, WRITE          ), GBL_BOOL_TYPE),
    (programState,    GBL_GENERIC, (READ, WRITE          ), GBL_ENUM_TYPE),
    (programBytes,    GBL_GENERIC, (READ, WRITE          ), GBL_UINT8_TYPE),
    (targetAddress,   GBL_GENERIC, (READ, WRITE          ), GBL_UINT32_TYPE)
)

GBL_SIGNALS(EvmuFlash,
    (dataChanged, (GBL_UINT32_TYPE, baseAddress), (GBL_SIZE_TYPE, bytes), (GBL_POINTER_TYPE, value))
)
//! \endcond

/*! \name Utilities
 *  \brief Static utility methods
 *  @{
 */
//! Returns the GblType UUID associated with EvmuFlash, for use with the libGimbal type system
EVMU_EXPORT GblType     EvmuFlash_type           (void)                           GBL_NOEXCEPT;
//! Returns the target address corresponding to the given state in the flash programming sequence
EVMU_EXPORT EvmuAddress EvmuFlash_programAddress (EVMU_FLASH_PROGRAM_STATE state) GBL_NOEXCEPT;
//! Returns the target value corresponding to the given state in the flash programming sequence
EVMU_EXPORT EvmuWord    EvmuFlash_programValue   (EVMU_FLASH_PROGRAM_STATE state) GBL_NOEXCEPT;
//! @}

/*! \name Read Methods
 *  \brief Methods for reading or fetching state data
 *  \relatesalso EvmuFlash
 *  @{
 */
//! Returns the current state of the flash programming sequence for unlocking writes
EVMU_EXPORT EVMU_FLASH_PROGRAM_STATE
                        EvmuFlash_programState  (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the number of bytes remaining that can be written before reprogramming
EVMU_EXPORT size_t      EvmuFlash_programBytes  (GBL_CSELF) GBL_NOEXCEPT;
//! Returns the current target address of the next LDF or STF flash instruction
EVMU_EXPORT EvmuAddress EvmuFlash_targetAddress (GBL_CSELF) GBL_NOEXCEPT;
//! Returns whether or not flash is currently unlocked for writing
EVMU_EXPORT GblBool     EvmuFlash_unlocked      (GBL_CSELF) GBL_NOEXCEPT;
//! @}

/*! \name Memory Operations
 *  \brief Methods for read/write operations on flash memory
 *  \relatesalso EvmuFlash
 *  @{
 */
//! Reads a value from flash at the given address and returns its value
EVMU_EXPORT EvmuWord    EvmuFlash_readByte   (GBL_CSELF,
                                              EvmuAddress address) GBL_NOEXCEPT;
//! Reads the given number of bytes from flash into the buffer, returning the number successfully read
EVMU_EXPORT EVMU_RESULT EvmuFlash_readBytes  (GBL_CSELF,
                                              EvmuAddress base,
                                              void*       pData,
                                              size_t*     pBytes)  GBL_NOEXCEPT;
//! Writes a value to flash at the given address (bypassing unlock sequence)
EVMU_EXPORT EVMU_RESULT EvmuFlash_writeByte  (GBL_SELF,
                                              EvmuAddress address,
                                              EvmuWord    value)   GBL_NOEXCEPT;
//! Writes the given buffer to flash, returning nubmer of bytes written (bypassing unlock sequence)
EVMU_EXPORT EVMU_RESULT EvmuFlash_writeBytes (GBL_SELF,
                                              EvmuAddress base,
                                              const void* pData,
                                              size_t*     pBytes)  GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_FLASH_H


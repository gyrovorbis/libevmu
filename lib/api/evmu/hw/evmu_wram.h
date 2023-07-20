/*! \file
 *  \brief EvmuWram: Work-RAM peripheral memory API
 *  \ingroup peripherals
 *
 *  \author 2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_WRAM_H
#define EVMU_WRAM_H

#include "../types/evmu_peripheral.h"
#include "../types/evmu_imemory.h"

/*! \name Type System
 * \brief Type UUID and cast operators
 * @{
 */
#define EVMU_WRAM_TYPE                  (GBL_TYPEOF(EvmuWram))                       //!< Type UUID for EvmuWram
#define EVMU_WRAM(instance)             (GBL_INSTANCE_CAST(instance, EvmuWram))      //!< Function-style cast for GblInstance
#define EVMU_WRAM_CLASS(klass)          (GBL_CLASS_CAST(klass, EvmuWram))            //!< Function-style cast for GblClass
#define EVMU_WRAM_GET_CLASS(instance)   (GBL_INSTANCE_GET_CLASS(instance, EvmuWram)) //!< Get EvmuWramClass from GblInstance
//! @}

#define EVMU_WRAM_NAME          "wram"      //!< EvmuWram GblObject name

/*! \name Address Space
 *  \brief Region size and location definitions
 *@{
 */
#define EVMU_WRAM_BANK_COUNT    2                                             //!< Number of banks in WRAM
#define EVMU_WRAM_BANK_SIZE     256                                           //!< Size of each bank in WRAM
#define EVMU_WRAM_SIZE          (EVMU_WRAM_BANK_COUNT * EVMU_WRAM_BANK_SIZE)  //!< Total size of WRAM (both banks)
//! @}

#define GBL_SELF_TYPE EvmuWram

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuWram);

/*! \struct  EvmuWramClass
 *  \extends EvmuPeripheralClass
 *  \implements EvmuIMemoryClass
 *  \brief   GblClass structure for EvmuPeripheralClass
 *
 *  No public members.
 *
 *  \sa EvmuWram
 */
GBL_CLASS_DERIVE_EMPTY(EvmuWram, EvmuPeripheral, EvmuIMemory)

/*! \struct  EvmuWram
 *  \extends EvmuPeripheral
 *  \implements EvmuIMemory
 *  \ingroup peripherals
 *  \brief   GblInstance structure for EvmuWram
 *
 *  No public members.
 *
 *  \sa EvmuWramClass
 */
GBL_INSTANCE_DERIVE(EvmuWram, EvmuPeripheral)
    GblBool dataChanged; //!< User toggle: will be set after a WRAM value changes, you can reset and poll for changes
GBL_INSTANCE_END

//! \cond
GBL_PROPERTIES(EvmuWram,
    (dataChanged,    GBL_GENERIC, (READ, WRITE, OVERRIDE), GBL_BOOL_TYPE),
    (mode,           GBL_GENERIC, (READ, WRITE          ), GBL_ENUM_TYPE),
    (autoIncAddress, GBL_GENERIC, (READ, WRITE          ), GBL_BOOL_TYPE),
    (accessAddress,  GBL_GENERIC, (READ, WRITE          ), GBL_UINT16_TYPE),
    (transferring,   GBL_GENERIC, (READ),                  GBL_BOOL_TYPE)
)
//! \endcond

//! Returns the GblType UUID associated with EvmuWram
EVMU_EXPORT GblType EvmuWram_type (void) GBL_NOEXCEPT;

/*! \name Configuration Methods
 *  \brief Methods for querying or updating configuration
 *  \relatesalso EvmuWram
 *  @{
 */
//! Returns the target address created by using VRMAD1 the low byte, and VRMAD2 as the bank
EVMU_EXPORT EvmuAddress EvmuWram_accessAddress     (GBL_CSELF)                  GBL_NOEXCEPT;
//! Configures the values of VRMAD1 and VRMAD2 so that they point to the address given by \p addr
EVMU_EXPORT EVMU_RESULT EvmuWram_setAccessAddress  (GBL_SELF, EvmuAddress addr) GBL_NOEXCEPT;
//! Returns GBL_TRUE of a Maple transfer from the Dreamcast is in progress, disallowing VMU access
EVMU_EXPORT GblBool     EvmuWram_mapleTransferring (GBL_CSELF)                  GBL_NOEXCEPT;
//! @}

/*! \name Read/Write Accessors
 *  \brief Methods for reading and writing WRAM data
 *  \relatesalso EvmuWram
 *  @{
 */
//! Returns the byte value located at the given WRAM \p address
EVMU_EXPORT EvmuWord    EvmuWram_readByte   (GBL_CSELF,
                                             EvmuAddress address) GBL_NOEXCEPT;
//! Reads \p pSize bytes from WRAM into \p pData, starting at \p address, writing back the number of bytes read
EVMU_EXPORT EVMU_RESULT EvmuWram_readBytes  (GBL_CSELF,
                                             EvmuAddress address,
                                             void*       pData,
                                             size_t*     pSize)   GBL_NOEXCEPT;
//! Writes the \p byte value to the WRAM \p address
EVMU_EXPORT EVMU_RESULT EvmuWram_writeByte  (GBL_SELF,
                                             EvmuAddress address,
                                             EvmuWord    byte)    GBL_NOEXCEPT;
//! Writes \p pSize bytes to WRAM from \p pData, starting at \p address, writing back the number of bytes written
EVMU_EXPORT EVMU_RESULT EvmuWram_writeBytes (GBL_SELF,
                                             EvmuAddress address,
                                             const void* pData,
                                             size_t*     pSize)   GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_WRAM_H

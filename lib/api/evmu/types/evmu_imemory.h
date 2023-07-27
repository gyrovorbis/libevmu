/*! \file
 *  \brief EvmuIMemory interface for generic memory access
 *
 *  This file contains the interface definition and public
 *  API for EvmuIMemory, which is a polymorphic base class
 *  used to provide a common mechanism for interacting
 *  with the different memory peripherals of the VMU.
 *
 *  \author 2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_IMEMORY_H
#define EVMU_IMEMORY_H

#include "evmu_typedefs.h"
#include <gimbal/meta/instances/gimbal_object.h>
#include <gimbal/meta/signals/gimbal_signal.h>

/*! \name Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_IMEMORY_TYPE             (GBL_TYPEID(EvmuIMemory))             //!< Type UUID for EvmuIMemory
#define EVMU_IMEMORY(self)            (GBL_CAST(EvmuIMemory, self))         //!< Cast a GblInstance to EvmuIMemory
#define EVMU_IMEMORY_CLASS(klass)     (GBL_CLASS_CAST(EvmuIMemory, klass))  //!< Cast a GblClass to EvmuIMemoryClass
#define EVMU_IMEMORY_GET_CLASS(self)  (GBL_CLASSOF(EvmuIMemory, self))      //!< Get an EvmuIMemoryClass from a GblInstance
//! @}

#define GBL_SELF_TYPE EvmuIMemory

GBL_DECLS_BEGIN

/*! \struct EvmuIMemoryClass
 *  \extends GblInterface
 *  \brief   GblClass structure for EvmuIMemory
 *
 *  Provides virtual methods for reading and writing to
 *  some underlying memory space. The write method should
 *  also update the "dataChanged" property (required) as
 *  well as fire the "dataChaneg" signal.
 *
 *  \sa EvmuIMemory
 */
GBL_INTERFACE_DERIVE(EvmuIMemory)
    //! Virtual method for performing a flash read, storing to buffer, reporting number of bytes read
    EVMU_RESULT (*pFnRead) (GBL_CSELF,
                            EvmuAddress address,
                            void*       pBuffer,
                            size_t*     pBytes);
    //! Virtual method for performing a flash write from a buffer, reporting byes written, and emitting the change signal
    EVMU_RESULT (*pFnWrite)(GBL_SELF,
                            EvmuAddress address,
                            const void* pBuffer,
                            size_t*     pBytes);
    //! Byte size of memory space
    size_t capacity;
GBL_INTERFACE_END

/*! \struct EvmuIMemory
 *  \brief Interfaced type for generic read/write memory
 *
 *  EvmuIMemory provides a generic polymorphic
 *  interface for reading and writing from and to some
 *  external memory space.
 *
 *  \sa EvmuIMemoryClass
 */

GBL_PROPERTIES(EvmuIMemory,
    (dataChanged, GBL_GENERIC, (ABSTRACT, READ, WRITE), GBL_BOOL_TYPE)
)

GBL_SIGNALS(EvmuIMemory,
    (dataChange, (GBL_UINT32_TYPE, address), (GBL_UINT32_TYPE, bytes), (GBL_POINTER_TYPE, pData))
)

//! Returns the GblType UUID associated with EvmuIMemory
EVMU_EXPORT GblType EvmuIMemory_type(void) GBL_NOEXCEPT;

/*! \name Read Accessors
 *  \brief Methods for reading data and properties
 *  \relatesalso EvmuIMemory
 *  @{
 */
//! Retrives the capacity of the given interface, by grabbing it from its class
EVMU_EXPORT size_t      EvmuIMemory_capacity   (GBL_CSELF)           GBL_NOEXCEPT;
//! Reads a value from flash at the given address and returns its value
EVMU_EXPORT EvmuWord    EvmuIMemory_readByte   (GBL_CSELF,
                                                EvmuAddress address) GBL_NOEXCEPT;
//! Reads the given number of bytes from flash into the buffer, returning the number successfully read
EVMU_EXPORT EVMU_RESULT EvmuIMemory_readBytes  (GBL_CSELF,
                                                EvmuAddress base,
                                                void*       pData,
                                                size_t*     pBytes)  GBL_NOEXCEPT;
//! @}

/*! \name Write Accessors
 *  \brief Methods for writing data
 *  \relatesalso EvmuIMemory
 *  @{
 */
//! Writes a value to flash at the given address (bypassing unlock sequence)
EVMU_EXPORT EVMU_RESULT EvmuIMemory_writeByte  (GBL_SELF,
                                                EvmuAddress address,
                                                EvmuWord    value)  GBL_NOEXCEPT;
//! Writes the given buffer to flash, returning nubmer of bytes written (bypassing unlock sequence)
EVMU_EXPORT EVMU_RESULT EvmuIMemory_writeBytes (GBL_SELF,
                                                EvmuAddress base,
                                                const void* pData,
                                                size_t*     pBytes) GBL_NOEXCEPT;
//! Fills the given region with the specified bit pattern data, performing a series of batch writes
EVMU_EXPORT EVMU_RESULT EvmuIMemory_fillBytes  (GBL_SELF,
                                                EvmuAddress base,
                                                size_t      regionSize,
                                                const void* pData,
                                                size_t      dataBytes);
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_IMEMORY_H

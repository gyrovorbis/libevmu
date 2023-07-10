/*! \file
 *  \brief EvmuMemory top-level memory BUS entity
 *  \ingroup peripherals
 *
 *  \todo
 *  - Move ROM and WRAM to respective peripherals
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_MEMORY_H
#define EVMU_MEMORY_H

#include "../types/evmu_peripheral.h"
#include "../hw/evmu_sfr.h"
#include <gimbal/meta/signals/gimbal_signal.h>

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_MEMORY_TYPE                (GBL_TYPEOF(EvmuMemory))                        //!< Type UUID for EvmuMemory
#define EVMU_MEMORY(instance)           (GBL_INSTANCE_CAST(instance, EvmuMemory))       //!< Function-style cast for GblInstances
#define EVMU_MEMORY_CLASS(klass)        (GBL_CLASS_CAST(klass, EvmuMemory))             //!< Function-style cast for GblClasses
#define EVMU_MEMORY_GET_CLASS(instance) (GBL_INSTANCE_GET_CLASS(instance, EvmuMemory))  //!< Get EvmuMemoryClass from GblInstances
//! @}

#define EVMU_MEMORY_NAME                "memory"    //!< GblObject peripheral name

#define GBL_SELF_TYPE EvmuMemory

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuMemory);

GBL_DECLARE_ENUM(EVMU_PROGRAM_SRC) {
    EVMU_PROGRAM_SRC_ROM          = EVMU_SFR_EXT_ROM,
    EVMU_PROGRAM_SRC_FLASH_BANK_0 = EVMU_SFR_EXT_FLASH_BANK_0,
    EVMU_PROGRAM_SRC_FLASH_BANK_1 = EVMU_SFR_EXT_FLASH_BANK_1
};

/*! \struct  EvmuMemoryClass
 *  \extends EvmuPeripheralclass
 *  \brief   GblClass structure for EvmuPeripheral
 *
 *  Virtual table structure for EvmuPeripheral. Overridable for
 *  providing custom hooks for memory events or for custom
 *  address-space mapping.
 *
 *  \sa EvmuMemory
 */
GBL_CLASS_DERIVE(EvmuMemory, EvmuPeripheral)
    EVMU_RESULT (*pFnReadData)      (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnReadDataLatch) (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnWriteData)     (GBL_SELF,  EvmuAddress addr, EvmuWord value);
    EVMU_RESULT (*pFnWriteDataLatch)(GBL_SELF,  EvmuAddress addr, EvmuWord value);
    EVMU_RESULT (*pFnReadProgram)   (GBL_CSELF, EvmuAddress addr, EvmuWord* pValue);
    EVMU_RESULT (*pFnWriteProgram)  (GBL_SELF,  EvmuAddress addr, EvmuWord value);
GBL_CLASS_END

/*! \struct  EvmuMemory
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   GblInstance structure for EvmuPeripheral
 *
 *  Actual instantiable object for a "memory" peripheral.
 *  Public members are user r/w toggles which are used
 *  for the back-end to notify the client that a particular
 *  region of memory has changed. The client is then to
 *  reset the toggle of interest, in acknolwedgement, so it
 *  can be set again later.
 *
 *  \sa EvmuMemoryClass
 */
GBL_INSTANCE_DERIVE(EvmuMemory, EvmuPeripheral)
    uint32_t ramChanged   : 1;
    uint32_t sfrChanged   : 1;
    uint32_t xramChanged  : 1;
    uint32_t flashChanged : 1;
    uint32_t romChanged   : 1;
    uint32_t wramChanged  : 1;
GBL_INSTANCE_END

//! \cond
GBL_PROPERTIES(EvmuMemory,
    (ramBank,        GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (xramBank,       GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (programSource,  GBL_GENERIC, (READ, WRITE), GBL_ENUM_TYPE),
    (stackPointer,   GBL_GENERIC, (READ),        GBL_UINT8_TYPE)
)

GBL_SIGNALS(EvmuMemory,
    (ramValueChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address), (GBL_ENUM_TYPE, bank)),
    (ramBankChange,       (GBL_INSTANCE_TYPE, pReceiver), (GBL_ENUM_TYPE, bank)),
    (sfrValueChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address)),
    (xramValueChange,     (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT32_TYPE, address), (GBL_ENUM_TYPE, bank)),
    (xramBankChange,      (GBL_INSTANCE_TYPE, pReceiver), (GBL_ENUM_TYPE, bank)),
    (programSourceChange, (GBL_INSTANCE_TYPE, pReceiver), (GBL_BOOL_TYPE, flash)),
    (stackPush,           (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT8_TYPE, value)),
    (stackPop,            (GBL_INSTANCE_TYPE, pReceiver))
)
//! \endcond

//! Returns the GblType UUID associated with EvmuMemory
EVMU_EXPORT GblType EvmuMemory_type (void) GBL_NOEXCEPT;

/*! \name Address Modes
 *  \brief Method(s) for working with address modes
 *  \relatesalso EvmuMemory
 *  @{
 */
//! Returns the memory address pointed to by the given register indirect mode
EVMU_EXPORT EvmuAddress EvmuMemory_indirectAddress (GBL_CSELF, size_t mode) GBL_NOEXCEPT;
//! @}

/*! \name Data Memory
 *  \brief Methods for managing the data address space
 *  \relatesalso EvmuMemory
 *  @{
 */
//! Returns the register or port value located in data memory at the address given by \p addr
EVMU_EXPORT EvmuWord    EvmuMemory_readData       (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Returns the latch value located in data memory at the address given by \p addr
EVMU_EXPORT EvmuWord    EvmuMemory_readDataLatch  (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Returns the register or port value located in data memory at the given address WITHOUT triggering read side-effects
EVMU_EXPORT EvmuWord    EvmuMemory_viewData       (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Writes \p val to the register or port at the \p addr address in data memory
EVMU_EXPORT EVMU_RESULT EvmuMemory_writeData      (GBL_SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT;
//! Writes \p val to the latch located at the \p addr address in data memory
EVMU_EXPORT EVMU_RESULT EvmuMemory_writeDataLatch (GBL_SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT;
//! @}

/*! \name Program Memory
 *  \brief Methods for managing the program address space
 *  \relatesalso EvmuMemory
 *  @{
 */
//! Returns the current source memory chip for program data (ROM or Flash)
EVMU_EXPORT EVMU_PROGRAM_SRC EvmuMemory_programSrc    (GBL_CSELF)                                GBL_NOEXCEPT;
//! Sets the current source memory chip for program data to \p src (ROM or Flash)
EVMU_EXPORT EVMU_RESULT      EvmuMemory_setProgramSrc (GBL_SELF, EVMU_PROGRAM_SRC src)           GBL_NOEXCEPT;
//! Returns the value located in program memory at the address given by \p addr
EVMU_EXPORT EvmuWord         EvmuMemory_readProgram   (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Writes the \p val value to the location given by \p addr to program memory
EVMU_EXPORT EVMU_RESULT      EvmuMemory_writeProgram  (GBL_SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT;
//! @}

/*! \name Stack
 *  \brief Methods for managing the program stack
 *  \relatesalso EvmuMemory
 *  @{
 */
//! Returns the current depth of the program stack
EVMU_EXPORT int         EvmuMemory_stackDepth (GBL_CSELF)                GBL_NOEXCEPT;
//! Returns the value at the given stack depth (0 is current stack location)
EVMU_EXPORT EvmuWord    EvmuMemory_viewStack  (GBL_CSELF, size_t depth)  GBL_NOEXCEPT;
//! Pops the value from the top of the stack, returning it and updating the stack pointer
EVMU_EXPORT EvmuWord    EvmuMemory_popStack   (GBL_SELF)                 GBL_NOEXCEPT;
//! Pushes \p value onto the top of the stack, updating the stack pointer
EVMU_EXPORT EVMU_RESULT EvmuMemory_pushStack  (GBL_SELF, EvmuWord value) GBL_NOEXCEPT;
//! @}

// === FACTOR ME OUT ====

EVMU_EXPORT EvmuWord EvmuMemory_readWram(const EvmuMemory* pSelf, EvmuAddress addr) GBL_NOEXCEPT;

EVMU_EXPORT EVMU_RESULT EvmuMemory_writeWram(EvmuMemory* pSelf,
                                             EvmuAddress addr,
                                             EvmuWord    value) GBL_NOEXCEPT;

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_MEMORY_H


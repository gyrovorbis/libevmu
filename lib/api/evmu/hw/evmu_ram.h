/*! \file
 *  \brief EvmuRam top-level memory BUS entity
 *  \ingroup peripherals
 *
 *  \todo
 *  - Move ROM and WRAM to respective peripherals
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */
#ifndef EVMU_RAM_H
#define EVMU_RAM_H

#include "../types/evmu_peripheral.h"
#include "../hw/evmu_sfr.h"
#include "../types/evmu_imemory.h"

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_RAM_TYPE            (GBL_TYPEID(EvmuRam))            //!< Type UUID for EvmuRam
#define EVMU_RAM(self)           (GBL_CAST(EvmuRam, self))        //!< Function-style cast for GblInstances
#define EVMU_RAM_CLASS(klass)    (GBL_CLASS_CAST(EvmuRam, klass)) //!< Function-style cast for GblClasses
#define EVMU_RAM_GET_CLASS(self) (GBL_CLASSOF(EvmuRam, self))     //!< Get EvmuRamClass from GblInstances
//! @}

#define EVMU_RAM_NAME   "memory"    //!< GblObject peripheral name

#define GBL_SELF_TYPE   EvmuRam

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuRam);

//! Source memory space for program execution
typedef enum EVMU_PROGRAM_SRC {
    EVMU_PROGRAM_SRC_ROM          = EVMU_SFR_EXT_ROM,           //!< ROM
    EVMU_PROGRAM_SRC_FLASH_BANK_0 = EVMU_SFR_EXT_FLASH_BANK_0,  //!< Flash (Bank 0)
    EVMU_PROGRAM_SRC_FLASH_BANK_1 = EVMU_SFR_EXT_FLASH_BANK_1   //!< Flash (Bank 1)
} EVMU_PROGRAM_SRC;

/*! \struct  EvmuRamClass
 *  \extends EvmuPeripheralclass
 *  \implements EvmuIMemoryClass
 *  \brief   GblClass structure for EvmuPeripheral
 *
 *  Virtual table structure for EvmuPeripheral. Overridable for
 *  providing custom hooks for memory events or for custom
 *  address-space mapping.
 *
 *  \sa EvmuRam
 */
GBL_CLASS_DERIVE_EMPTY(EvmuRam, EvmuPeripheral, EvmuIMemory)

/*! \struct  EvmuRam
 *  \extends EvmuPeripheral
 *  \implements EvmuIMemory
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
 *  \sa EvmuRamClass
 */
GBL_INSTANCE_DERIVE(EvmuRam, EvmuPeripheral)
    GblBool dataChanged;
GBL_INSTANCE_END

//! \cond
GBL_PROPERTIES(EvmuRam,
    (dataChanged,   GBL_GENERIC, (READ, WRITE, OVERRIDE), GBL_BOOL_TYPE),
    (ramBank,       GBL_GENERIC, (READ, WRITE          ), GBL_ENUM_TYPE),
    (xramBank,      GBL_GENERIC, (READ, WRITE          ), GBL_ENUM_TYPE),
    (programSource, GBL_GENERIC, (READ, WRITE          ), GBL_ENUM_TYPE),
    (stackPointer,  GBL_GENERIC, (READ                 ), GBL_UINT8_TYPE)
)

GBL_SIGNALS(EvmuRam,
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

//! Returns the GblType UUID associated with EvmuRam
EVMU_EXPORT GblType EvmuRam_type (void) GBL_NOEXCEPT;

/*! \name Address Modes
 *  \brief Method(s) for working with address modes
 *  \relatesalso EvmuRam
 *  @{
 */
//! Returns the memory address pointed to by the given register indirect mode
EVMU_EXPORT EvmuAddress EvmuRam_indirectAddress (GBL_CSELF, size_t mode) GBL_NOEXCEPT;
//! @}

/*! \name Data Memory
 *  \brief Methods for managing the data address space
 *  \relatesalso EvmuRam
 *  @{
 */
//! Returns the register or port value located in data memory at the address given by \p addr
EVMU_EXPORT EvmuWord    EvmuRam_readData       (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Returns the latch value located in data memory at the address given by \p addr
EVMU_EXPORT EvmuWord    EvmuRam_readDataLatch  (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Returns the register or port value located in data memory at the given address WITHOUT triggering read side-effects
EVMU_EXPORT EvmuWord    EvmuRam_viewData       (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Writes \p val to the register or port at the \p addr address in data memory
EVMU_EXPORT EVMU_RESULT EvmuRam_writeData      (GBL_SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT;
//! Writes \p val to the latch located at the \p addr address in data memory
EVMU_EXPORT EVMU_RESULT EvmuRam_writeDataLatch (GBL_SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT;
//! @}

/*! \name Program Memory
 *  \brief Methods for managing the program address space
 *  \relatesalso EvmuRam
 *  @{
 */
//! Returns the current source memory chip for program data (ROM or Flash)
EVMU_EXPORT EVMU_PROGRAM_SRC EvmuRam_programSrc    (GBL_CSELF)                                GBL_NOEXCEPT;
//! Sets the current source memory chip for program data to \p src (ROM or Flash)
EVMU_EXPORT EVMU_RESULT      EvmuRam_setProgramSrc (GBL_SELF, EVMU_PROGRAM_SRC src)           GBL_NOEXCEPT;
//! Returns the value located in program memory at the address given by \p addr
EVMU_EXPORT EvmuWord         EvmuRam_readProgram   (GBL_CSELF, EvmuAddress addr)              GBL_NOEXCEPT;
//! Writes the \p val value to the location given by \p addr to program memory
EVMU_EXPORT EVMU_RESULT      EvmuRam_writeProgram  (GBL_SELF, EvmuAddress addr, EvmuWord val) GBL_NOEXCEPT;
//! @}

/*! \name Stack
 *  \brief Methods for managing the program stack
 *  \relatesalso EvmuRam
 *  @{
 */
//! Returns the current depth of the program stack
EVMU_EXPORT int         EvmuRam_stackDepth (GBL_CSELF)                GBL_NOEXCEPT;
//! Returns the value at the given stack depth (0 is current stack location)
EVMU_EXPORT EvmuWord    EvmuRam_viewStack  (GBL_CSELF, size_t depth)  GBL_NOEXCEPT;
//! Pops the value from the top of the stack, returning it and updating the stack pointer
EVMU_EXPORT EvmuWord    EvmuRam_popStack   (GBL_SELF)                 GBL_NOEXCEPT;
//! Pushes \p value onto the top of the stack, updating the stack pointer
EVMU_EXPORT EVMU_RESULT EvmuRam_pushStack  (GBL_SELF, EvmuWord value) GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_RAM_H


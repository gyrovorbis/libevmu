/*! \file
 *  \brief EvmuCpu: Sanyo LC86k CPU Core
 *  \ingroup peripherals
 *
 *  EvmuCpu models the main execution unit on the Potato.
 *  It provides an API for executing and fetching info on
 *  instructions.
 *
 *  Additionally, virtual methods within EvmuCpu can be
 *  overridden to provide custom CPU and opcode implementations.
 *
 *  \todo
 *      - secs per instruction in msec/nsec, not float
 *      - pull Rom/BIOS update out of CPU update path
 *      - implement/respect halted flags
 *      - ensure OV is set when divison by 0 occurs
 *
 *  \copyright 2023 Falco Girgis
 */
#ifndef EVMU_CPU_H
#define EVMU_CPU_H

#include "../types/evmu_peripheral.h"
#include "evmu_isa.h"

#include <gimbal/meta/signals/gimbal_signal.h>

/*! \name  Type System
 *  \brief Type UUID and cast operators
 *  @{
 */
#define EVMU_CPU_TYPE            (GBL_TYPEOF(EvmuCpu))                   //!< Type UUID for EvmuCpu
#define EVMU_CPU(self)           (GBL_INSTANCE_CAST(self, EvmuCpu))      //!< Cast GblInstance to EvmuCpu
#define EVMU_CPU_CLASS(klass)    (GBL_CLASS_CAST(klass, EvmuCpu))        //!< Cast GblClass to EvmuCpuClass
#define EVMU_CPU_GET_CLASS(self) (GBL_INSTANCE_GET_CLASS(self, EvmuCpu)) //!< Get EvmuCpuClass from GblInstance
//! @}

#define EVMU_CPU_NAME            "cpu"   //!< GblObject name for EvmCpu

#define GBL_SELF_TYPE EvmuCpu

GBL_DECLS_BEGIN

GBL_FORWARD_DECLARE_STRUCT(EvmuCpu);

//! Program counter for EvmuCpu instructions
typedef uint16_t EvmuPc;

/*! \struct  EvmuCpuClass
 *  \extends EvmuPeripheralClass
 *  \brief   Class for Sanyo LC86k CPU core
 *
 *  This structure is responsible for providing the actual
 *  implementation of the LC86k back-end logic. It can be
 *  overridden within a subclass to customize.
 *
 *  \sa EvmuCpu
 */
GBL_CLASS_DERIVE(EvmuCpu, EvmuPeripheral)
    //! Virtual method for fetching instructions
    EVMU_RESULT (*pFnFetch)  (GBL_SELF,
                              EvmuPc           pc,
                              EvmuInstruction* pEncoded);
    //! Virtual method for decoding instructions
    EVMU_RESULT (*pFnDecode) (GBL_SELF,
                              const EvmuInstruction*  pEncoded,
                              EvmuDecodedInstruction* pDecoded);
    //! Virtual method for executing instructions
    EVMU_RESULT (*pFnExecute)(GBL_SELF,
                              const EvmuDecodedInstruction* pInstr);
    //! Virtual method for top-level logic behind running a single instruction
    EVMU_RESULT (*pFnRunNext)(GBL_SELF);
GBL_CLASS_END

/*! \struct  EvmuCpu
 *  \extends EvmuPeripheral
 *  \ingroup peripherals
 *  \brief   Sanyo LC86k CPU core
 *
 *  EvmuCpu is an EvmuPeripheral which implements all of
 *  the logic behind and provides an API for the CPU core.
 *
 *  In addition to managing normal program flow, it allows
 *  you to feed in individual instructions and then query
 *  for their decoded operands and opcode. It also provides
 *  a signal for when the PC changes, which can be used to
 *  implement breakpoints and instruction tracing.
 *
 *  \sa EvmuCpuClass
 */
GBL_INSTANCE_DERIVE(EvmuCpu, EvmuPeripheral)
    uint32_t halted         : 1; //!< Halts CPU execution
    uint32_t haltAfterNext  : 1; //!< Halts the CPU execution after the next instruction
    uint32_t pcChanged      : 1; //!< User toggle (reset to false) which be set upon PC change
GBL_INSTANCE_END

//! \cond
GBL_PROPERTIES(EvmuCpu,
    (pc,       GBL_GENERIC, (READ, WRITE, LOAD, SAVE), GBL_UINT16_TYPE),
    (opcode,   GBL_GENERIC, (READ),                    GBL_UINT8_TYPE),
    (operand1, GBL_GENERIC, (READ),                    GBL_INT32_TYPE),
    (operand2, GBL_GENERIC, (READ),                    GBL_INT32_TYPE),
    (operand3, GBL_GENERIC, (READ),                    GBL_INT32_TYPE)
)

GBL_SIGNALS(EvmuCpu,
    (pcChange, (GBL_INSTANCE_TYPE, pReceiver), (GBL_UINT16_TYPE, pc))
)
//! \endcond

//! Returns the GblType UUID associated with EvmuCpu
EVMU_EXPORT GblType EvmuCpu_type (void) GBL_NOEXCEPT;

/*! \name Program Counter
 *  \brief Methods for reading and writing the PC
 *  \relatesalso EvmuCpu
 *  @{
 */
//! Returns the address for the program counter, which points to the next instruction
EVMU_EXPORT EvmuPc EvmuCpu_pc    (GBL_CSELF)                GBL_NOEXCEPT;
//! Sets the address of the program counter, so that it will be the next executed instruction
EVMU_EXPORT void   EvmuCpu_setPc (GBL_SELF, EvmuPc address) GBL_NOEXCEPT;
//! @}

/*! \name Instruction Info
 *  \brief Methods for querying current instruction info
 *  \relatesalso EvmuCpu
 *  @{
 */
//! Returns the number of seconds per instruction for the currently executing instruction
EVMU_EXPORT double   EvmuCpu_secs    (GBL_CSELF)             GBL_NOEXCEPT;
//! Returns the number of cycles per instruction for the currently executing instruction
EVMU_EXPORT size_t   EvmuCpu_cycles  (GBL_CSELF)             GBL_NOEXCEPT;
//! Returns the opcode of the currently executing instruction
EVMU_EXPORT EvmuWord EvmuCpu_opcode  (GBL_CSELF)             GBL_NOEXCEPT;
//! Returns the operand of the currently executing instruction at index \p idx
EVMU_EXPORT int32_t  EvmuCpu_operand (GBL_CSELF, size_t idx) GBL_NOEXCEPT;
//! @}

/*! \name Instruction Execution
 *  \brief Methods for executing instructions
 *  \relatesalso EvmuCpu
 *  @{
 */
//! Immediately executes an externally provided decoded instruction rather than fetching one from ROM or flash
EVMU_EXPORT EVMU_RESULT EvmuCpu_execute (GBL_SELF,
                                         const EvmuDecodedInstruction* pInstr) GBL_NOEXCEPT;
//! Fetches and executes the next instruction, which is located at the address pointed to by the program counter
EVMU_EXPORT EVMU_RESULT EvmuCpu_runNext (GBL_SELF)                             GBL_NOEXCEPT;
//! @}

GBL_DECLS_END

#undef GBL_SELF_TYPE

#endif // EVMU_CPU_H

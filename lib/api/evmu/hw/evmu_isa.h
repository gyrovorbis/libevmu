/*! \file
 *  \brief Instruction set, opcode, and operand info
 *
 * This file provides the API for everything relating to the
 * instruction set architecture of the Sanyo LC86K:
 *  * Opcodes
 *  * Operand Info
 *  * Encoded Instructions
 *  * Decoded Instructions
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
*/

#ifndef EVMU_ISA_H
#define EVMU_ISA_H

#include <stdint.h>
#include <string.h>
#include <gimbal/strings/gimbal_string_buffer.h>
#include "../types/evmu_typedefs.h"
#include "../evmu_api.h"

/*! \defgroup  opcodes Instruction Opcodes
 *  \brief First byte of all opcodes
 *
 *  Collection of defines used to identify the opcode of
 *  a particular instruction by its first byte.
 *
 *  \warning
 *  Some opcodes span a range of values and are not just
 *  a single byte value. Only use these values when
 *  encoding an instruction or when looking at a decoded
 *  instruction.
 *
 *  \subpage instruction_ref
 *
 * @{
 */
#define EVMU_OPCODE_NOP              0x00   //!< No operation
#define EVMU_OPCODE_BR               0x01   //!< Branch near relative address
#define EVMU_OPCODE_LD               0x02   //!< Load direct byte to accumulator
#define EVMU_OPCODE_LD_IND           0x04   //!< Load indirect byte to accumulator
#define EVMU_OPCODE_CALL             0x08   //!< Near absolute subroutine call
#define EVMU_OPCODE_CALLR            0x10   //!< Far relative subroutine call
#define EVMU_OPCODE_BRF              0x11   //!< Branch far relative address
#define EVMU_OPCODE_ST               0x12   //!< Store direct byte from accumulator
#define EVMU_OPCODE_ST_IND           0x14   //!< Store indirect byte from accumulator
#define EVMU_OPCODE_CALLF            0x20   //!< Far absolute subroutine call
#define EVMU_OPCODE_JMPF             0x21   //!< Jump far absolute address
#define EVMU_OPCODE_MOV              0x22   //!< Move direct data to indirect byte
#define EVMU_OPCODE_MOV_IND          0x24   //!< Move immediate data to indirect byte
#define EVMU_OPCODE_JMP              0x28   //!< Jump near absolute address
#define EVMU_OPCODE_MUL              0x30   //!< Multiply accumulator and C register by B register
#define EVMU_OPCODE_BEI              0x31   //!< Compare immediate byte to accumulator and branch near relative address if equal
#define EVMU_OPCODE_BE               0x32   //!< Compare direct byte to accumulator and branch near relativa address if equal
#define EVMU_OPCODE_BE_IND           0x34   //!< Compare indirect byte to accumulator and branch near relative address if equal
#define EVMU_OPCODE_DIV              0x40   //!< Divide accumulator and C register by B register
#define EVMU_OPCODE_BNEI             0x41   //!< Compare immediate byte to accumulator and branch near relative address if not zero
#define EVMU_OPCODE_BNE              0x42   //!< Compare direct byte to accumulator and branch near relative address if not zero
#define EVMU_OPCODE_BNE_IND          0x44   //!< Compare indirect byte to accumulator and branch near relative address if not zero
#define EVMU_OPCODE_BPC              0x48   //!< Branch near relative address if direct bit is one ("positive") and clear
#define EVMU_OPCODE_LDF              0x50   //!< Load from Flash
#define EVMU_OPCODE_STF              0x51   //!< Store to Flash
#define EVMU_OPCODE_DBNZ             0x52   //!< Decrement direct byte and branch near relative address if indirect byte is not zero
#define EVMU_OPCODE_DBNZ_IND         0x54   //!< Decrement indirect byte and branch near relative address if indirect byte is not zero
#define EVMU_OPCODE_PUSH             0x60   //!< Push direct byte to stack
#define EVMU_OPCODE_INC              0x62   //!< Increment direct byte
#define EVMU_OPCODE_INC_IND          0x64   //!< Increment indirect byte
#define EVMU_OPCODE_BP               0x68   //!< Branch near relative address if direct bit is one ("positive")
#define EVMU_OPCODE_POP              0x70   //!< Pop direct byte from stack
#define EVMU_OPCODE_DEC              0x72   //!< Decrement direct byte
#define EVMU_OPCODE_DEC_IND          0x74   //!< Decrement indirect byte
#define EVMU_OPCODE_BZ               0x80   //!< Branch near relative address if accumulator is zero
#define EVMU_OPCODE_ADDI             0x81   //!< Add immediate byte to accumulator
#define EVMU_OPCODE_ADD              0x82   //!< Add direct byte to accumulator
#define EVMU_OPCODE_ADD_IND          0x84   //!< Add indirect byte to accumulator
#define EVMU_OPCODE_BN               0x88   //!< Branch near relative address if direct bit is zero ("negative")
#define EVMU_OPCODE_BNZ              0x90   //!< Branch near relative address if accumulator is not zero
#define EVMU_OPCODE_ADDCI            0x91   //!< Add imediate byte and carry flag to accumulator
#define EVMU_OPCODE_ADDC             0x92   //!< Add direct byte and carry flag to accumulator
#define EVMU_OPCODE_ADDC_IND         0x94   //!< Add indirect byte and carry flag to accumualtor
#define EVMU_OPCODE_RET              0xa0   //!< Return from subroutine
#define EVMU_OPCODE_SUBI             0xa1   //!< Subtract immediate byte from accumulator
#define EVMU_OPCODE_SUB              0xa2   //!< Subtract direct byte from accumulator
#define EVMU_OPCODE_SUB_IND          0xa4   //!< Subtract indirect byte from accumulator
#define EVMU_OPCODE_NOT1             0xa8   //!< Not direct bit
#define EVMU_OPCODE_RETI             0xb0   //!< Return from interrupt
#define EVMU_OPCODE_SUBCI            0xb1   //!< Subtract immediate byte and carry flag from accumulator
#define EVMU_OPCODE_SUBC             0xb2   //!< Subtract direct byte and carry flag from accumulator
#define EVMU_OPCODE_SUBC_IND         0xb4   //!< Subtract indirect byte and carry flag from accumulator
#define EVMU_OPCODE_ROR              0xc0   //!< Rotate accumulator right
#define EVMU_OPCODE_LDC              0xc1   //!< Load code byte relative to TRR to accumulator
#define EVMU_OPCODE_XCH              0xc2   //!< Exchange direct byte with accumulator
#define EVMU_OPCODE_XCH_IND          0xc4   //!< Exchange indirect byte with accumulator
#define EVMU_OPCODE_CLR1             0xc8   //!< Clear direct bit
#define EVMU_OPCODE_RORC             0xd0   //!< Rotate accumulator right through the carry flag
#define EVMU_OPCODE_ORI              0xd1   //!< OR immediate byte to accumulator
#define EVMU_OPCODE_OR               0xd2   //!< OR direct byte to accumulator
#define EVMU_OPCODE_OR_IND           0xd4   //!< OR indirect byte to accumulator
#define EVMU_OPCODE_ROL              0xe0   //!< Rotate accumulator left
#define EVMU_OPCODE_ANDI             0xe1   //!< AND immediate byte to accumulator
#define EVMU_OPCODE_AND              0xe2   //!< AND direct byte to accumulator
#define EVMU_OPCODE_AND_IND          0xe4   //!< AND indirect byte to accumulator
#define EVMU_OPCODE_SET1             0xe8   //!< Set direct bit
#define EVMU_OPCODE_ROLC             0xf0   //!< Rotate accumulator left through carry flag
#define EVMU_OPCODE_XORI             0xf1   //!< XOR immediate byte to accumulator
#define EVMU_OPCODE_XOR              0xf2   //!< XOR direct byte to accumulator
#define EVMU_OPCODE_XOR_IND          0xf4   //!< XOR indirect byte to accumulator
//! @}

/*! \name Program Status Word Flags
 *  \brief Flags which may be modified by a particular instruction
 * @{
 */
#define EVMU_ISA_PSW_SYSTEM_POS     3       //!< System bit
#define EVMU_ISA_PSW_SYSTEM_MASK    0x8     //!< System mask
#define EVMU_ISA_PSW_CY_POS         2       //!< Carry bit
#define EVMU_ISA_PSW_CY_MASK        0x4     //!< Carry mask
#define EVMU_ISA_PSW_AC_POS         1       //!< Auxiliary carry bit
#define EVMU_ISA_PSW_AC_MASK        0x2     //!< Auxiliary carry mask
#define EVMU_ISA_PSW_OV_POS         0       //!< Overflow bit
#define EVMU_ISA_PSW_OV_MASK        0x1     //!< Overflow mask
#define EVMU_ISA_PSW_NONE           0x0     //!< No PSW flags effected
// @}

/*! \name Argument Packs
 *  \brief Macros for handling packed argument types
 * @{
 */
//! Packs the given argument types into an EvmuIsaArgFormat
#define EVMU_ISA_ARG_FORMAT_PACK(...) \
    GBL_VA_OVERLOAD_CALL(EVMU_ISA_ARG_FORMAT_PACK, GBL_VA_OVERLOAD_SUFFIXER_ARGC, __VA_ARGS__)

//! Unpacks the given field from the EvmuIsaArgFormat provided as argFormat
#define EVMU_ISA_ARG_FORMAT_UNPACK(argFormat, field) \
    ((argFormat >> (field * 8u)) & 0xff)

//! Returns the number of arguments encoded within an EvmuIsaArgFormat
#define EVMU_ISA_ARGC(argFmt)                                                                   \
    ((uint8_t)(EVMU_ISA_ARG_FORMAT_UNPACK(argFmt, EVMU_ISA_ARG1) != EVMU_ISA_ARG_TYPE_NONE) +  \
     (uint8_t)(EVMU_ISA_ARG_FORMAT_UNPACK(argFmt, EVMU_ISA_ARG2) != EVMU_ISA_ARG_TYPE_NONE) +  \
     (uint8_t)(EVMU_ISA_ARG_FORMAT_UNPACK(argFmt, EVMU_ISA_ARG3) != EVMU_ISA_ARG_TYPE_NONE))
//! @}

GBL_DECLS_BEGIN

//! Flags type for EvmuInstructionFormat::flags
typedef uint32_t EvmuIsaFlags;

//! Type for holding encoded instruction argument types in EvmuInstructionFormat::args
typedef uint32_t EvmuIsaArgFormat;

// ==== ARGS =======

//! Enumeration with every type of instruction argument
typedef enum EVMU_ISA_ARG_TYPE {
    EVMU_ISA_ARG_TYPE_NONE,         //!< No Argument
    EVMU_ISA_ARG_TYPE_RELATIVE_8,   //!< 8-bit relative address
    EVMU_ISA_ARG_TYPE_RELATIVE_16,  //!< 16-bit relative address
    EVMU_ISA_ARG_TYPE_IMMEDIATE_8,  //!< 8-bit immediate value
    EVMU_ISA_ARG_TYPE_DIRECT_9,     //!< 9-bit direct address
    EVMU_ISA_ARG_TYPE_INDIRECT_2,   //!< 2-bit indirection mode
    EVMU_ISA_ARG_TYPE_ABSOLUTE_12,  //!< 12-bit absolute address
    EVMU_ISA_ARG_TYPE_ABSOLUTE_16,  //!< 16-bit absolute address
    EVMU_ISA_ARG_TYPE_BIT_3,        //!< 3-bit bit position
    EVMU_ISA_ARG_TYPE_COUNT         //!< Number of different types
} EVMU_ISA_ARG_TYPE;

//! Enumeration for each argument position in an EvmuIaArgFormat
typedef enum EVMU_ISA_ARG {
    EVMU_ISA_ARG1,      //!< First argument
    EVMU_ISA_ARG2,      //!< Second argument
    EVMU_ISA_ARG3,      //!< Third argument
    EVMU_ISA_ARG_COUNT  //!< Size of argument pack
} EVMU_ISA_ARG;

/*! Structrure describing the format of each type of instruction
 *
 *  EvmuInstructionFormat describes for the EvmuCpu core what the
 *  layout of each type of instruction is. It also contains
 *  useful metadata for other tools.
 *
 *  \sa EvmuIsa_format()
 */
typedef struct EvmuInstructionFormat {
    const char*      pMnemonic;  //!< ASM instruction
    const char*      pDesc;      //!< Instruction description
    uint8_t          opcode;     //!< Opcode
    uint8_t          opBits;     //!< Bits for opcode (8 max)
    EvmuIsaArgFormat args;       //!< Operand arguments
    uint8_t          bytes;      //!< Bytes per instruction (1-3)
    uint8_t          cc;         //!< Clock cycles (1-7)
    EvmuIsaFlags     flags;      //!< Program status word modifiers (CY, AC, DV)
} EvmuInstructionFormat;

//! Enumeration containing each byte within an encoded EvmuInstruction
typedef enum EVMU_INSTRUCTION_BYTE {
    EVMU_INSTRUCTION_BYTE_OPCODE,   //!< First byte of an encoded instruction (opcode)
    EVMU_INSTRUCTION_BYTE_2,        //!< Second byte of an encoded instruction (if present)
    EVMU_INSTRUCTION_BYTE_3,        //!< Third byte of an encoded instruction (if present)
    EVMU_INSTRUCTION_BYTE_MAX = 0x4 //!< Maximum number of bytes in an encoded instruction (1 for padding)
} EVMU_INSTRUCTION_BYTE;

//! Contains a collection of bytes, representing an encoded instruction
typedef struct EvmuInstruction {
    uint8_t bytes[EVMU_INSTRUCTION_BYTE_MAX];   //!< Raw instruction byte data
    uint8_t byteCount;                          //!< Byte size of instruciton data
} EvmuInstruction;

//! Contains the decoded operands of an instruction
typedef struct EvmuOperands {
    union {
        uint16_t absolute;      //!< 12 or 16-bit absolute address
        uint16_t relative16;    //!< 16-bit relative address
        uint16_t direct;        //!< 9-bit direct address
    };
    uint8_t bit;                //!< 3-bit bit offset
    uint8_t indirect;           //!< 2-bit indirection mode
    int8_t  relative8;          //!< 8-bit relative address
    uint8_t immediate;          //!< 8-bit immediate data
} EvmuOperands;

//! Represents an instruction which has been decoded
typedef struct EvmuDecodedInstruction {
    EvmuOperands operands;  //!< Decoded operands
    EvmuWord     opcode;    //!< Opcode byte (see \ref opcodes)
} EvmuDecodedInstruction;

// ===== Public API =====

//! Fetches information on an instruction from the internal database
EVMU_EXPORT const EvmuInstructionFormat*
                        EvmuIsa_format (EvmuWord firstByte)               GBL_NOEXCEPT;

//! Fetches an encoded instruction from a buffer
EVMU_EXPORT EVMU_RESULT EvmuIsa_fetch  (EvmuInstruction* pEncoded,
                                        const void*      pBuffer,
                                        size_t*          pBytes)          GBL_NOEXCEPT;

//! Decodes an instruction into an EvmuDecodedInstruction
EVMU_EXPORT EVMU_RESULT EvmuIsa_decode (const EvmuInstruction*  pEncoded,
                                        EvmuDecodedInstruction* pDecoded) GBL_NOEXCEPT;

// ===== Implementation =====
//! \cond
#define EVMU_ISA_ARG_FORMAT_PACK_3(arg1, arg2, arg3)                                   \
    ((EvmuIsaArgFormat)                                                                \
     (((EvmuIsaArgFormat)arg1 & 0xffu) << ((EvmuIsaArgFormat)EVMU_ISA_ARG1 * 8))     | \
     (((EvmuIsaArgFormat)arg2 & 0xffu) << ((EvmuIsaArgFormat)EVMU_ISA_ARG2 * 8))     | \
     (((EvmuIsaArgFormat)arg3 & 0xffu) << ((EvmuIsaArgFormat)EVMU_ISA_ARG3 * 8)))

#define EVMU_ISA_ARG_FORMAT_PACK_2(a, b) (EVMU_ISA_ARG_FORMAT_PACK_3(a, b, EVMU_ISA_ARG_TYPE_NONE))

#define EVMU_ISA_ARG_FORMAT_PACK_1(a)    (EVMU_ISA_ARG_FORMAT_PACK_2(a, EVMU_ISA_ARG_TYPE_NONE))
//! \endcond

GBL_DECLS_END

#endif // EVMU_ISA_H

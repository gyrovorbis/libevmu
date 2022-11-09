#ifndef EVMU_ISA_H
#define EVMU_ISA_H

#include <stdint.h>
#include <string.h>
#include <gimbal/strings/gimbal_string_buffer.h>
#include "../types/evmu_typedefs.h"
#include "../evmu_api.h"

/* Terminology:
 * Opcode - First bit which is simply the unique identifier of the operation to perform
 * Instruction - Encoded, executable bytes containing an opcode + its operands
 * InstructionFormat - Description of instruction types, including opcode, operand types, cycle count, address modes, etc
 * InstructionSetArchitecture - Whole Instruction Set Format Map
 */

// Opcodes
#define EVMU_OPCODE_NOP              0x00
#define EVMU_OPCODE_BR               0x01
#define EVMU_OPCODE_LD               0x02
#define EVMU_OPCODE_LD_COUNT         2
#define EVMU_OPCODE_LD_IND           0x04
#define EVMU_OPCODE_LD_IND_COUNT     4
#define EVMU_OPCODE_CALL             0x08
#define EVMU_OPCODE_CALL_COUNT       8
#define EVMU_OPCODE_CALLR            0x10
#define EVMU_OPCODE_BRF              0x11
#define EVMU_OPCODE_ST               0x12
#define EVMU_OPCODE_ST_COUNT         2
#define EVMU_OPCODE_ST_IND           0x14
#define EVMU_OPCODE_ST_IND_COUNT     4
#define EVMU_OPCODE_CALLF            0x20
#define EVMU_OPCODE_JMPF             0x21
#define EVMU_OPCODE_MOV              0x22
#define EVMU_OPCODE_MOV_COUNT        2
#define EVMU_OPCODE_MOV_IND          0x24
#define EVMU_OPCODE_MOV_IND_COUNT    4
#define EVMU_OPCODE_JMP              0x28
#define EVMU_OPCODE_JMP_COUNT        8
#define EVMU_OPCODE_MUL              0x30
#define EVMU_OPCODE_BEI              0x31
#define EVMU_OPCODE_BE               0x32
#define EVMU_OPCODE_BE_COUNT         2
#define EVMU_OPCODE_BE_IND           0x34
#define EVMU_OPCODE_BE_IND_COUNT     4
#define EVMU_OPCODE_DIV              0x40
#define EVMU_OPCODE_BNEI             0x41
#define EVMU_OPCODE_BNE              0x42
#define EVMU_OPCODE_BNE_COUNT        2
#define EVMU_OPCODE_BNE_IND          0x44
#define EVMU_OPCODE_BNE_IND_COUNT    4
#define EVMU_OPCODE_BPC              0x48
#define EVMU_OPCODE_BPC_COUNT        8
#define EVMU_OPCODE_LDF              0x50
#define EVMU_OPCODE_LDF_COUNT        1
#define EVMU_OPCODE_STF              0x51
#define EVMU_OPCODE_STF_COUNT        2
#define EVMU_OPCODE_DBNZ             0x52
#define EVMU_OPCODE_DBNZ_COUNT       2
#define EVMU_OPCODE_DBNZ_IND         0x54
#define EVMU_OPCODE_DBNZ_IND_COUNT   4
#define EVMU_OPCODE_PUSH             0x60
#define EVMU_OPCODE_PUSH_COUNT       2
#define EVMU_OPCODE_INC              0x62
#define EVMU_OPCODE_INC_COUNT        2
#define EVMU_OPCODE_INC_IND          0x64
#define EVMU_OPCODE_INC_IND_COUNT    4
#define EVMU_OPCODE_BP               0x68
#define EVMU_OPCODE_BP_COUNT         8
#define EVMU_OPCODE_POP              0x70
#define EVMU_OPCODE_POP_COUNT        2
#define EVMU_OPCODE_DEC              0x72
#define EVMU_OPCODE_DEC_COUNT        2
#define EVMU_OPCODE_DEC_IND          0x74
#define EVMU_OPCODE_DEC_IND_COUNT    4
#define EVMU_OPCODE_BZ               0x80
#define EVMU_OPCODE_ADDI             0x81
#define EVMU_OPCODE_ADD              0x82
#define EVMU_OPCODE_ADD_COUNT        2
#define EVMU_OPCODE_ADD_IND          0x84
#define EVMU_OPCODE_ADD_IND_COUNT    4
#define EVMU_OPCODE_BN               0x88
#define EVMU_OPCODE_BN_COUNT         8
#define EVMU_OPCODE_BNZ              0x90
#define EVMU_OPCODE_ADDCI            0x91
#define EVMU_OPCODE_ADDC             0x92
#define EVMU_OPCODE_ADDC_COUNT       2
#define EVMU_OPCODE_ADDC_IND         0x94
#define EVMU_OPCODE_ADDC_IND_COUNT   4
#define EVMU_OPCODE_RET              0xa0
#define EVMU_OPCODE_SUBI             0xa1
#define EVMU_OPCODE_SUB              0xa2
#define EVMU_OPCODE_SUB_COUNT        2
#define EVMU_OPCODE_SUB_IND          0xa4
#define EVMU_OPCODE_SUB_IND_COUNT    4
#define EVMU_OPCODE_NOT1             0xa8
#define EVMU_OPCODE_NOT1_COUNT       8
#define EVMU_OPCODE_RETI             0xb0
#define EVMU_OPCODE_SUBCI            0xb1
#define EVMU_OPCODE_SUBC             0xb2
#define EVMU_OPCODE_SUBC_COUNT       2
#define EVMU_OPCODE_SUBC_IND         0xb4
#define EVMU_OPCODE_SUBC_IND_COUNT   4
#define EVMU_OPCODE_ROR              0xc0
#define EVMU_OPCODE_LDC              0xc1
#define EVMU_OPCODE_XCH              0xc2
#define EVMU_OPCODE_XCH_COUNT        2
#define EVMU_OPCODE_XCH_IND          0xc4
#define EVMU_OPCODE_XCH_IND_COUNT    4
#define EVMU_OPCODE_CLR1             0xc8
#define EVMU_OPCODE_CLR1_COUNT       8
#define EVMU_OPCODE_RORC             0xd0
#define EVMU_OPCODE_ORI              0xd1
#define EVMU_OPCODE_OR               0xd2
#define EVMU_OPCODE_OR_COUNT         2
#define EVMU_OPCODE_OR_IND           0xd4
#define EVMU_OPCODE_OR_IND_COUNT     4
#define EVMU_OPCODE_ROL              0xe0
#define EVMU_OPCODE_ANDI             0xe1
#define EVMU_OPCODE_AND              0xe2
#define EVMU_OPCODE_AND_COUNT        2
#define EVMU_OPCODE_AND_IND          0xe4
#define EVMU_OPCODE_AND_IND_COUNT    4
#define EVMU_OPCODE_SET1             0xe8
#define EVMU_OPCODE_SET1_COUNT       8
#define EVMU_OPCODE_ROLC             0xf0
#define EVMU_OPCODE_XORI             0xf1
#define EVMU_OPCODE_XOR              0xf2
#define EVMU_OPCODE_XOR_COUNT        2
#define EVMU_OPCODE_XOR_IND          0xf4
#define EVMU_OPCODE_XOR_IND_COUNT    4

#define EVMU_OPCODE_MAP_SIZE         256

//Program Status Word Opcode Fields
#define EVMU_ISA_PSW_SYSTEM_POS     3
#define EVMU_ISA_PSW_SYSTEM_MASK    0x8
#define EVMU_ISA_PSW_CY_POS         2
#define EVMU_ISA_PSW_CY_MASK        0x4
#define EVMU_ISA_PSW_AC_POS         1
#define EVMU_ISA_PSW_AC_MASK        0x2
#define EVMU_ISA_PSW_OV_POS         0
#define EVMU_ISA_PSW_OV_MASK        0x1
#define EVMU_ISA_PSW_NONE           0x0

#define EVMU_ISA_ARG_FORMAT_PACK_3(arg1, arg2, arg3)                                  \
    ((EvmuIsaArgFormat)                                                               \
    (((EvmuIsaArgFormat)arg1 & 0xffu) << ((EvmuIsaArgFormat)EVMU_ISA_ARG1 * 8))     | \
    (((EvmuIsaArgFormat)arg2 & 0xffu) << ((EvmuIsaArgFormat)EVMU_ISA_ARG2 * 8))     | \
    (((EvmuIsaArgFormat)arg3 & 0xffu) << ((EvmuIsaArgFormat)EVMU_ISA_ARG3 * 8)))

#define EVMU_ISA_ARG_FORMAT_PACK_2(a, b) (EVMU_ISA_ARG_FORMAT_PACK_3(a, b, EVMU_ISA_ARG_TYPE_NONE))

#define EVMU_ISA_ARG_FORMAT_PACK_1(a)    (EVMU_ISA_ARG_FORMAT_PACK2(a, EVMU_ISA_ARG_TYPE_NONE))

#define EVMU_ISA_ARG_FORMAT_PACK(...) \
    GBL_VA_OVERLOAD_CALL(EVMU_ISA_ARG_FORMAT_PACK, GBL_VA_OVERLOAD_SUFFIXER_ARGC, __VA_ARGS__)

#define EVMU_ISA_ARG_FORMAT_EXTRACT(argFormat, field) \
    ((argFormat >> (field * 8u)) & 0xff)

#define EVMU_ISA_ARGC(argFmt)                                                                   \
    ((uint8_t)(EVMU_ISA_ARG_FORMAT_EXTRACT(argFmt, EVMU_ISA_ARG1) != EVMU_ISA_ARG_TYPE_NONE) +  \
     (uint8_t)(EVMU_ISA_ARG_FORMAT_EXTRACT(argFmt, EVMU_ISA_ARG2) != EVMU_ISA_ARG_TYPE_NONE) +  \
     (uint8_t)(EVMU_ISA_ARG_FORMAT_EXTRACT(argFmt, EVMU_ISA_ARG3) != EVMU_ISA_ARG_TYPE_NONE))

GBL_DECLS_BEGIN

typedef enum EVMU_INSTRUCTION_BYTE {
    EVMU_INSTRUCTION_BYTE_OPCODE    =   0x0,
    EVMU_INSTRUCTION_BYTE_2         =   0x1,
    EVMU_INSTRUCTION_BYTE_3         =   0x2,
    EVMU_INSTRUCTION_BYTE_MAX       =   0x4
} EVMU_INSTRUCTION_BYTE;

typedef enum EVMU_ISA_CATEGORTY {
    EVMU_ISA_CATEGORY_ARITHMETIC,
    EVMU_ISA_CATEGORY_LOGICAL,
    EVMU_ISA_CATEGORY_DATA_TRANSFER,
    EVMU_ISA_CATEGORY_JUMP,
    EVMU_ISA_CATEGORY_CONDITIONAL_BRANCH,
    EVMU_ISA_CATEGORY_SUBROUTINE,
    EVMU_ISA_CATEGORY_BIT_MANIPULATION,
    EVMU_ISA_CATEGORY_OTHER,
    EVMU_ISA_CATEGORTY_COUNT
} EVMU_ISA_CATEGORY;

typedef uint32_t EvmuIsaFlags;

// ==== ARGS =======

typedef enum EVMU_ISA_ARG_TYPE {
    EVMU_ISA_ARG_TYPE_NONE,
    EVMU_ISA_ARG_TYPE_RELATIVE_8,
    EVMU_ISA_ARG_TYPE_RELATIVE_16,
    EVMU_ISA_ARG_TYPE_IMMEDIATE_8,
    EVMU_ISA_ARG_TYPE_DIRECT_9,
    EVMU_ISA_ARG_TYPE_INDIRECT_2,
    EVMU_ISA_ARG_TYPE_ABSOLUTE_12,
    EVMU_ISA_ARG_TYPE_ABSOLUTE_16,
    EVMU_ISA_ARG_TYPE_BIT_3,
    EVMU_ISA_ARG_TYPE_COUNT
} EVMU_ISA_ARG_TYPE;

typedef uint32_t EvmuIsaArgFormat;

typedef enum EVMU_ISA_ARG {
    EVMU_ISA_ARG1 = 0,
    EVMU_ISA_ARG2 = 1,
    EVMU_ISA_ARG3 = 2,
    EVMU_ISA_ARG_COUNT
} EVMU_ISA_ARG;

//Instruction data
typedef struct EvmuInstructionFormat {
    uint8_t             opcode;         // Byte identifier              (0x01)
    uint8_t             opcodeBits;     // Opcode size                  (2 bytes)
    EvmuIsaArgFormat    argFormat;      // Argument Format              (EVMU_ISA_ARG_TYPE_RELATIVE_8)
    uint8_t             byteCount;      // Instruction Size             (2)
    uint8_t             clockCycles;    // Cycle count                  (2)
    uint8_t             interruptCycle; // First interruptable cycle    (2)
    EvmuIsaFlags        flags;          // Additional properties        (NONE)
} EvmuInstructionFormat;

typedef struct EvmuInstructionInfo {
    const char*         pMnemonic;      // ASM Instruction              (BR r8)
    const char*         pName;          // Instruction Name             (Branch near relative address)
    const char*         pEncoding;      // Machine code                 (0 0 0 1 0 0 0 1 r7r6r5r4r3r2r1 (01H))
    const char*         pRtlFunction;   // RTL description              ((PC) <- (PC) + 2, (PC) <- (PC) + r8)
    const char*         pDescription;   // Description                  (Increments PC twice, increments the PC by r7 + r0)
    EVMU_ISA_CATEGORY   categorty;      // Type of instruction          (EVMU_ISA_CATEGORY_JUMP)
    // list of input registers, list of output registers
} EvmuInstructionInfo;

typedef struct EvmuInstruction {
    uint8_t bytes[EVMU_INSTRUCTION_BYTE_MAX];
    uint8_t byteCount;
} EvmuInstruction;

typedef struct EvmuOperands {
    union {
        uint16_t absolute;
        uint16_t relative16;
        uint16_t direct;
    };
    uint8_t bit;
    uint8_t indirect;
//    union {
//      uint8_t relativeU8;    don't see anywhere this is used!
        int8_t  relativeS8; // rename to relative8?
//    };
    uint8_t immediate;
} EvmuOperands;

typedef struct EvmuDecodedInstruction {
    uint8_t      opcode;
    EvmuOperands operands;
} EvmuDecodedInstruction;

EVMU_INLINE EVMU_RESULT EvmuIsa_fetch       (EvmuInstruction* pEncoded, const void* pBuffer, GblSize* pBytes)   GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuIsa_store       (const EvmuInstruction* pEncoded, void* pBuffer, GblSize* pBytes)   GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuIsa_decode      (const EvmuInstruction* pEncoded, EvmuDecodedInstruction* pDecoded) GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuIsa_encode      (const EvmuDecodedInstruction* pDecoded, EvmuInstruction* pEncoded) GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuIsa_assemble    (const char* pString, EvmuInstruction* pInstr)                      GBL_NOEXCEPT;
EVMU_EXPORT EVMU_RESULT EvmuIsa_disassemble (const EvmuDecodedInstruction* Decoded, GblStringBuffer* pString)   GBL_NOEXCEPT;
EVMU_EXPORT int32_t     EvmuIsa_operand     (const EvmuDecodedInstruction* pDecoded, EVMU_ISA_ARG arg)          GBL_NOEXCEPT;
EVMU_EXPORT EvmuWord    EvmuIsa_opcode      (uint8_t firstByte)                                                 GBL_NOEXCEPT;
EVMU_EXPORT const EvmuInstructionFormat*
                        EvmuIsa_format      (EvmuWord opcode)                                                   GBL_NOEXCEPT;
EVMU_EXPORT const EvmuInstructionInfo*
                        EvmuIsa_info        (EvmuWord opcode)                                                   GBL_NOEXCEPT;

//Create top-level LUT that's just a byte to opcode map so other arrays can be sparse

EVMU_INLINE EVMU_RESULT EvmuIsa_fetch(EvmuInstruction* pEncoded, const void* pBuffer, GblSize* pBytes) {
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_VERIFY_POINTER(pEncoded);
    GBL_CTX_VERIFY_POINTER(pBuffer);
    GBL_CTX_VERIFY_POINTER(pBytes);
    GBL_CTX_VERIFY_ARG(*pBytes);


    memset(pEncoded, 0, sizeof(EvmuInstruction));

    uint8_t opcode;
#ifndef EVMU_WIP
    GBL_CTX_CALL(evmuInstructionPeek(*(const uint8_t*)pBuffer, &opcode));
#endif
    EvmuInstructionFormat* pFormat;
    GBL_CTX_CALL(evmuInstructionFormat(opcode, &pFormat));

    GBL_CTX_VERIFY_EXPRESSION(*pBytes >= pFormat->byteCount,
                              "Opcode blah expects %u byte instruction!");

    memcpy(pEncoded->bytes, pBuffer, pFormat->byteCount);
    pEncoded->byteCount = pFormat->byteCount;
    GBL_CTX_END();
}

/*To assemble:
1) Build decoded from raw data
2) encode from decoded
*3) easy way to write encoded instructions to address?
  */

/* To disassemble:
 1) fetch instruction from bytes
 2) decode instruction
 3) fetch instruction format to build*/

// only handles single, only does one layer of translation... helper/util macros to chain?
//EVMU_EXPORT EVMU_RESULT evmuIsaAssemble(const char* pCodeText, GblSize* pSize, EvmuDecodedInstruction* pDecodedInstruction);
//EVMU_EXPORT EVMU_RESULT evmuIsaDisassemble(const EvmuDecodedInstruction* pDecodedInstruction, const char* pBuffer, GblSize* pSize);

#if 0
inline static int gyVmuInstrFetchByte(const uint8_t* buffer, VMUInstr* instr) {
    if(!instr->bytes) { //fetch first byte
        instr->bytes[INSTR_BYTE_OPCODE] = buffer[0];
        ++instr->bytes;
    } else if(instr->bytes < _instrMap[instr->instrBytes[INSTR_BYTE_OPCODE]]) {

    }
}
#endif


//void gyVmuInstrDecodeOperands(const VMUInstr* instr, VMUInstrOperands* operands);

//const extern VMUInstrAttr _instrMap[INSTR_MAP_SIZE];





#if 0 //MIGHT BE SWEET?
const extern ADDR_MODE  _instrArgAddrModes[INSTR_ARGS_COUNT];


inline static ADDR_MODE gyVmuInstrInstrArgAddrMode(INSTR_ARGS args, int arg);
#endif


#if 0

void runMyShit() {
    uint8_t* pMyBadassBufferOfInstructions;
    EvmuSize bytes = 256;

    // First you "fetch" an instruction from a byte stream:
    Instruction instruction;
    evmuInstructionFetch(&instruction, pMyBadassBuferofInstructions, &bytes);
    // You better give me a buffer with size, becuase the instruction sizes are variable on the VMU, and I want to return how any bytes we had to fetch and error out gracefully.

    // Kay, now you have a nice instruction that you can already send to the CPU and shit and serialize that's just a byte array. If you want to do interesting shit with it you gotta decode it:
    DecodedInstruction decodedInstr;
    evmuInstructionDecode(&decodedInstr, &instruction);

// K now if that succeeds you know everything there is to know about the fucker in that struct and have all of its operands decoded and readilly accessible as well to do what you please.
}
#endif

#if 0
EVMU_EXPORT EVMU_RESULT evmuDecodedInstructionDisassemble(const EvmuDecodedInstruction* pDecoded, char* pBuffer, GblSize* pSize) {
    EVMU_EXPORT EVMU_RESULT_BEGIN();
    EVMU_EXPORT EVMU_RESULT_VALIDATE_ARG(pDecoded);
    EVMU_EXPORT EVMU_RESULT_VALIDATE_ARG(pDecoded->pFormat);
    EVMU_EXPORT EVMU_RESULT_VALIDATE_ARG(pBuffer);
    EVMU_EXPORT EVMU_RESULT_VALIDATE_ARG(pSize && *pSize);

    char bufferScratch[1024] = { '/0' };

    const EvmuInstructionFormat* pFormat = pDecoded->pFormat;
    const unsigned     opCount = EVMU_ISA_ARG_FORMAT_COUNT(pFormat->argFormat);
    unsigned         opsWritten = 0;
    const char*     pMneumonic = pDecoded->pFormat->pMneumonic;
    const char*     pDelims = " ,"
    char*             pToken = strtok(pMneumonic, pDelims);
    const char*     pArgTypeToken = NULL;

    //Assuming we grabbed the opcode name!
    if(pToken != NULL) {
        strncat(bufferScratch, pToken, sizeof(bufferScratch));

        while((opsWritten < opCount) && (pToken = strtok(NULL, pDelims))) {
            EVMU_ISA_ARG_TYPE argType = EVMU_ISA_ARG_FORMAT_UNPACK(pFormat->argFormat, (EVMU_ISA_ARG_FORMAT_FIELD)opsWritten);

            EVMU_VALIDATE_CALL(evmuIsaArgTypeStringToken(argType, &pArgTypeToken));

            // Found what you were looking for!
            if(strcmp(pToken, pArgTypeToken) == 0) {
                char stringTempBuff[16] = {'\0'};
                int32_t value = 0;
                EVMU_VALIDATE_CALL(evmuDecodedInstructionOperandValue(pDecoded, (EVMU_ISA_ARG_FORMAT_FIELD)opsWritten, &value));
                snprintf(stringTempBuff, sizeof(stringTempBuff), argType == EVMU_ISA_ARG_FORMAT_FIELD_1? " %d" : ", %d", value);
                strncat(bufferScratch, stringTempBuff, sizeof(bufferScratch));
                ++opsWritten;
            } else {
                strncat(bufferScratch, " ? ");
            }

        }
        assert(opsWritten == opCount);
    }

    strncpy(pBuffer, bufferScratch, *pSize);

    EVMU_EXPORT EVMU_RESULT_END();

}
#endif

GBL_DECLS_END



#endif // EVMU_ISA_H
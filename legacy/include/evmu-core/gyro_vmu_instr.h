#ifndef GYRO_VMU_INSTR_H
#define GYRO_VMU_INSTR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

#define INSTR_BYTE_OPCODE       0x0
#define INSTR_BYTE_2            0x1
#define INSTR_BYTE_3            0x2
#define INSTR_BYTE_MAX          0x4     //even integer

#define INSTR_OPERAND_NONE      0x0
#define INSTR_OPERAND_IMM       0x1
#define INSTR_OPERAND_IMM_REL   0x2
#define INSTR_OPERAND_IMM_DIR   0x3
#define INSTR_OPERAND_IMM_INDIR 0x4
#define INSTR_OPERAND_REL_8BIT  0x5
#define INSTR_OPERAND_REL_16BIT 0x6
#define INSTR_OPERAND_DIR       0x7
#define INSTR_OPERAND_INDIR     0x8
#define INSTR_OPERAND_ABS_12BIT 0x9
#define INSTR_OPERAND_ABS_16BIT 0xa
#define INSTR_OPERAND_BIT_9D    0xb
#define INSTR_OPERAND_BIT_8REL  0xc
#define INSTR_OPERAND_TYPE_MAX  0xd

//Program Status Word Opcode Fields
#define OPCODE_PSW_CY_POS       2
#define OPCODE_PSW_CY_MASK      0x4
#define OPCODE_PSW_AC_POS       1
#define OPCODE_PSW_AC_MASK      0x2
#define OPCODE_PSW_OV_POS       0
#define OPCODE_PSW_OV_MASK      0x1
#define OPCODE_PSW_NONE         0x0

//Opcodes
#define OPCODE_NOP              0x00
#define OPCODE_BR               0x01
#define OPCODE_LD               0x02
#define OPCODE_LD_COUNT         2
#define OPCODE_LD_IND           0x04
#define OPCODE_LD_IND_COUNT     4
#define OPCODE_CALL             0x08
#define OPCODE_CALL_COUNT       8
#define OPCODE_CALLR            0x10
#define OPCODE_BRF              0x11
#define OPCODE_ST               0x12
#define OPCODE_ST_COUNT         2
#define OPCODE_ST_IND           0x14
#define OPCODE_ST_IND_COUNT     4
#define OPCODE_CALLF            0x20
#define OPCODE_JMPF             0x21
#define OPCODE_MOV              0x22
#define OPCODE_MOV_COUNT        2
#define OPCODE_MOV_IND          0x24
#define OPCODE_MOV_IND_COUNT    4
#define OPCODE_JMP              0x28
#define OPCODE_JMP_COUNT        8
#define OPCODE_MUL              0x30
#define OPCODE_BEI              0x31
#define OPCODE_BE               0x32
#define OPCODE_BE_COUNT         2
#define OPCODE_BE_IND           0x34
#define OPCODE_BE_IND_COUNT     4
#define OPCODE_DIV              0x40
#define OPCODE_BNEI             0x41
#define OPCODE_BNE              0x42
#define OPCODE_BNE_COUNT        2
#define OPCODE_BNE_IND          0x44
#define OPCODE_BNE_IND_COUNT    4
#define OPCODE_BPC              0x48
#define OPCODE_BPC_COUNT        8
#define OPCODE_LDF              0x50
#define OPCODE_LDF_COUNT        1
#define OPCODE_STF              0x51
#define OPCODE_STF_COUNT        1
#define OPCODE_DBNZ             0x52
#define OPCODE_DBNZ_COUNT       2
#define OPCODE_DBNZ_IND         0x54
#define OPCODE_DBNZ_IND_COUNT   4
#define OPCODE_PUSH             0x60
#define OPCODE_PUSH_COUNT       2
#define OPCODE_INC              0x62
#define OPCODE_INC_COUNT        2
#define OPCODE_INC_IND          0x64
#define OPCODE_INC_IND_COUNT    4
#define OPCODE_BP               0x68
#define OPCODE_BP_COUNT         8
#define OPCODE_POP              0x70
#define OPCODE_POP_COUNT        2
#define OPCODE_DEC              0x72
#define OPCODE_DEC_COUNT        2
#define OPCODE_DEC_IND          0x74
#define OPCODE_DEC_IND_COUNT    4
#define OPCODE_BZ               0x80
#define OPCODE_ADDI             0x81
#define OPCODE_ADD              0x82
#define OPCODE_ADD_COUNT        2
#define OPCODE_ADD_IND          0x84
#define OPCODE_ADD_IND_COUNT    4
#define OPCODE_BN               0x88
#define OPCODE_BN_COUNT         8
#define OPCODE_BNZ              0x90
#define OPCODE_ADDCI            0x91
#define OPCODE_ADDC             0x92
#define OPCODE_ADDC_COUNT       2
#define OPCODE_ADDC_IND         0x94
#define OPCODE_ADDC_IND_COUNT   4
#define OPCODE_RET              0xa0
#define OPCODE_SUBI             0xa1
#define OPCODE_SUB              0xa2
#define OPCODE_SUB_COUNT        2
#define OPCODE_SUB_IND          0xa4
#define OPCODE_SUB_IND_COUNT    4
#define OPCODE_NOT1             0xa8
#define OPCODE_NOT1_COUNT       8
#define OPCODE_RETI             0xb0
#define OPCODE_SUBCI            0xb1
#define OPCODE_SUBC             0xb2
#define OPCODE_SUBC_COUNT       2
#define OPCODE_SUBC_IND         0xb4
#define OPCODE_SUBC_IND_COUNT   4
#define OPCODE_ROR              0xc0
#define OPCODE_LDC              0xc1
#define OPCODE_XCH              0xc2
#define OPCODE_XCH_COUNT        2
#define OPCODE_XCH_IND          0xc4
#define OPCODE_XCH_IND_COUNT    4
#define OPCODE_CLR1             0xc8
#define OPCODE_CLR1_COUNT       8
#define OPCODE_RORC             0xd0
#define OPCODE_ORI              0xd1
#define OPCODE_OR               0xd2
#define OPCODE_OR_COUNT         2
#define OPCODE_OR_IND           0xd4
#define OPCODE_OR_IND_COUNT     4
#define OPCODE_ROL              0xe0
#define OPCODE_ANDI             0xe1
#define OPCODE_AND              0xe2
#define OPCODE_AND_COUNT        2
#define OPCODE_AND_IND          0xe4
#define OPCODE_AND_IND_COUNT    4
#define OPCODE_SET1             0xe8
#define OPCODE_SET1_COUNT       8
#define OPCODE_ROLC             0xf0
#define OPCODE_XORI             0xf1
#define OPCODE_XOR              0xf2
#define OPCODE_XOR_COUNT        2
#define OPCODE_XOR_IND          0xf4
#define OPCODE_XOR_IND_COUNT    4

#define INSTR_MAP_SIZE          256
#define INSTR_ARG_TYPE_MASK     0xf


#define ELYSIAN_LUA_PROXY_FIELD_GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define ELYSIAN_LUA_PROXY_FIELD(...) ELYSIAN_LUA_PROXY_FIELD_GET_MACRO(__VA_ARGS__, ELYSIAN_LUA_PROXY_FIELD_READ_WRITE, ELYSIAN_LUA_PROXY_FIELD_READ_ONLY)(__VA_ARGS__)


#define INSTR_ARG_TYPES_PACK_3(a, b, c) (unsigned int)(((unsigned int)a&0xffu) | (((unsigned int)b&0xffu)<<8) | (((unsigned int)c&0xffu)<<16))
#define INSTR_ARG_TYPES_PACK_2(a, b) (INSTR_ARG_TYPES_PACK_3(a, b, INSTR_ARG_TYPE_NONE))
#define INSTR_ARG_TYPES_PACK_1(a) (INSTR_ARG_TYPES_PACK_2(a, INSTR_ARG_TYPE_NONE))

#define INSTR_ARG_TYPES_PACK_GET_MACRO(_1, _2, _3, NAME, ...) NAME

#define INSTR_ARG_TYPES_PACK(...) INSTR_ARG_TYPES_PACK_GET_MACRO(__VA_ARGS__, INSTR_ARG_TYPES_PACK_3, INSTR_ARG_TYPES_PACK_2, INSTR_ARG_TYPES_PACK_1)(__VA_ARGS__)


//#define INSTR_ARG_TYPES_UNPACK(args, pos) (*((unsigned char*)&args)+pos)

inline static unsigned INSTR_ARG_TYPES_UNPACK(unsigned args, unsigned pos) {
    switch(pos) {
    default:
        case 0: return args&0xff;
    case 1: return (args>>8)&0xff;
    case 2: return (args>>16)&0xff;
    }
}

typedef enum ADDR_MODE {
    ADDR_MODE_NONE,
    ADDR_MODE_IMM,
    ADDR_MODE_DIR,
    ADDR_MODE_IND,
    ADDR_MODE_BIT,
    ADDR_MODE_ABS,
    ADDR_MODE_REL,
    ADDR_MODE_COUNT
} ADDR_MODE;

typedef enum INSTR_ARG_TYPE {
    INSTR_ARG_TYPE_NONE,
    INSTR_ARG_TYPE_R8,
    INSTR_ARG_TYPE_R16,
    INSTR_ARG_TYPE_I8,
    INSTR_ARG_TYPE_D9,
    INSTR_ARG_TYPE_N2, //N = INDIRECT
    INSTR_ARG_TYPE_A12,
    INSTR_ARG_TYPE_A16,
    INSTR_ARG_TYPE_B3,
    INSTR_ARG_TYPE_COUNT
} INSTR_ARG_TYPE;

//Instruction data
typedef struct VMUInstrAttr {
    const char*     mnemonic;   //ASM instruction
    const char*     desc;       //instruction description
    unsigned char   opcode;     //opcode
    unsigned char   opBits;     //bits for opcode (8 max)
    unsigned int    args;       //operand arguments
    unsigned char   bytes;      //bytes per instruction (1-3)
    unsigned char   cc;         //clock cycles (1-7)
    unsigned char   psw;        //program status word modifiers (CY, AC, DV)
} VMUInstrAttr;

typedef struct VMUInstr {
    union {
        uint8_t         instrBytes[INSTR_BYTE_MAX];
        uint32_t        instr;
    };
    unsigned char       bytes;
} VMUInstr;

typedef struct VMUInstrOperands {
    uint16_t    addrMode[ADDR_MODE_COUNT];
    int         addrRel;
} VMUInstrOperands;

#if 0
inline static int gyVmuInstrFetchByte(const uint8_t* buffer, VMUInstr* instr) {
    if(!instr->bytes) { //fetch first byte
        instr->bytes[INSTR_BYTE_OPCODE] = buffer[0];
        ++instr->bytes;
    } else if(instr->bytes < _instrMap[instr->instrBytes[INSTR_BYTE_OPCODE]]) {

    }
}
#endif


void gyVmuInstrDecodeOperands(const VMUInstr* instr, VMUInstrOperands* operands);

const extern VMUInstrAttr _instrMap[INSTR_MAP_SIZE];

inline static void gyVmuInstrFetch(const unsigned char* buffer, VMUInstr* instr) {
    memset(instr, 0, sizeof(VMUInstr));
    do {
        instr->instrBytes[instr->bytes] = buffer[instr->bytes];
        ++instr->bytes;
    } while(instr->bytes < _instrMap[instr->instrBytes[INSTR_BYTE_OPCODE]].bytes);
}

inline static unsigned instrArgsArgc(unsigned int args) {
    unsigned argc = 0;
    if(args&0x000000ff) {
        ++argc;
        if(args&0x0000ff00) {
            ++argc;
            if(args&0x00ff0000) ++argc;
        }
    }
    return argc;
}

#if 0 //MIGHT BE SWEET?
const extern ADDR_MODE  _instrArgAddrModes[INSTR_ARGS_COUNT];


inline static ADDR_MODE gyVmuInstrInstrArgAddrMode(INSTR_ARGS args, int arg);
#endif

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_INSTR_H


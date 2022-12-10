#include "gyro_vmu_instr.h"
#include <assert.h>

/* Read-only array mapping the LCD86000 instruction-set
 * into an 8-bit (256 element) intruction map that can be
 * indexed by the first byte of an instruction to fetch
 * information with contant time-complexity.
 */

const VMUInstrAttr _instrMap[INSTR_MAP_SIZE] = {
    [OPCODE_NOP] = {
        "NOP",
        "Stalls processor for one clock-cycle.",
        OPCODE_NOP,
        8,
        INSTR_ARG_TYPES_PACK(0),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_BR] = {
        "BR r8",
        "Branch unconditionally. The target address is specified using an 8-bit relative address. The signed 8-bit offset is added to the address of the instruction following the BR. No PSW flags are affected.",
        OPCODE_BR,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_R8),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_LD...OPCODE_LD+OPCODE_LD_COUNT-1] = {
        "LD d9",
        "Load the operand into the ACC register. No PSW flags are affected.",
        OPCODE_LD,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_LD_IND...OPCODE_LD_IND+OPCODE_LD_IND_COUNT-1] = {
        "LD @Ri",
        "Load the operand into the ACC register. No PSW flags are affected.",
        OPCODE_LD_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_CALL...OPCODE_CALL+0x7] = {
        "CALL a12",
        "Call function. The entry address of the function is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the CALL. The return address (the address of the instruction following the CALL instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        OPCODE_CALL,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_A12),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_CALLR] = {
        "CALLR r16",
        "Call function. The entry address of the function is specified using a 16-bit relative address. The unsigned 16-bit offset is added to the address of the instruction following the CALLR minus one to produce the target address. The addition is performed modulo 65536, which makes it possible to call a lower address as well. The return address (the address of the instruction following the CALLR instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        OPCODE_CALLR,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_R16),
        3,
        4,
        OPCODE_PSW_NONE
    },
    [OPCODE_BRF] = {
        "BRF r16",
        "Branch unconditionally. The target address is specified using a 16-bit relative address. The unsigned 16-bit offset is added to the address of the instruction following the BRF minus one to produce the target address. The addition is performed modulo 65536, which makes it possible to branch to a lower address as well. No PSW flags are affected.",
        OPCODE_BRF,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_R16),
        3,
        4,
        OPCODE_PSW_NONE
    },
    [OPCODE_ST...OPCODE_ST+OPCODE_ST_COUNT-1] = {
        "ST d9",
        "Store the contents of the ACC register into the operand address. No PSW flags are affected.",
        OPCODE_ST,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_ST_IND...OPCODE_ST_IND+OPCODE_ST_IND_COUNT-1] = {
        "ST @Ri",
        "Store the contents of the ACC register into the operand address. No PSW flags are affected.",
        OPCODE_ST_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_CALL + 0x10 ... OPCODE_CALL+0x17] = {
        "CALL a12",
        "Call function. The entry address of the function is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the CALL. The return address (the address of the instruction following the CALL instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        OPCODE_CALL,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_A12),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_CALLF] = {
        "CALLF a16",
        "Call function. The entry address of the function is specified using a full 16-bit absolute address. The return address (the address of the instruction following the CALLF instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        OPCODE_CALLF,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_A16),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_JMPF] = {
        "JMPF a16",
        "Jump unconditionally. The target address is specified using a full 16-bit absolute address. No PSW flags are affected.",
        OPCODE_JMPF,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_A16),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_MOV...OPCODE_MOV+OPCODE_MOV_COUNT-1] = {
        "MOV #i8, d9",
        "Set the contents of the operand to a constant value. No PSW flags are affected.",
        OPCODE_MOV,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_I8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_MOV_IND...OPCODE_MOV_IND+OPCODE_MOV_IND_COUNT-1] = {
        "MOV #i8, @Rj",
        "Set the contents of the operand to a constant value. No PSW flags are affected.",
        OPCODE_MOV_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2, INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_JMP...OPCODE_JMP+0x7] = {
        "JMP a12",
        "Jump unconditionally. The target address is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the JMP. No PSW flags are affected.",
        OPCODE_JMP,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_A12),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_MUL] = {
        "MUL",
        "Perform a multiplication. The ACC and C registers together form a 16-bit operand (ACC being the high 8 bits, and C being the low 8 bits) which is multiplied by the contents of the B register. The result is a 24-bit number that is stored in the ACC, C and B registers (the high 8 bits are stored in B, the middle 8 bits in ACC, and the low 8 bits in C). CY is cleared, and OV is set if the result is greater than 16 bits, otherwise cleared. AC is not affected.",
        OPCODE_MUL,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        7,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK
    },
    [OPCODE_BEI] = {
        "BE #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        OPCODE_BEI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_BE...OPCODE_BE+OPCODE_BE_COUNT-1] = {
        "BE d9, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        OPCODE_BE,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_BE_IND...OPCODE_BE_IND+OPCODE_BE_IND_COUNT-1] = {
        "BE @Rj, #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        OPCODE_BE_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2, INSTR_ARG_TYPE_I8, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_JMP+0x10 ... OPCODE_JMP+0x17] = {
        "JMP a12",
        "Jump unconditionally. The target address is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the JMP. No PSW flags are affected.",
        OPCODE_JMP,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_A12),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_DIV] = {
        "DIV",
        "Perform a division. The ACC and C registers together form a 16-bit operand (ACC being the high 8 bits, and C being the low 8 bits) which is divided by the contents of the B register. The result is a 16-bit quotient that is stored in ACC and C (the high 8 bits in ACC, and the low 8 bits in C), and an 8-bit remainder that is stored in B. CY is cleared, and OV is set if the remainder is zero, otherwise cleared. AC is not affected.",
        OPCODE_DIV,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        7,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK
    },
    [OPCODE_BNEI] = {
        "BNE #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are not equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        OPCODE_BNEI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_BNE...OPCODE_BNE+OPCODE_BNE_COUNT-1] = {
        "BNE d9, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are not equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        OPCODE_BNE,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_BNE_IND...OPCODE_BNE_IND+OPCODE_BNE_IND_COUNT-1] = {
        "BNE @Rj, #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are not equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        OPCODE_BNE_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2, INSTR_ARG_TYPE_I8, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_BPC ... OPCODE_BPC + 0x7] = {
        "BPC d9, b3, r8",
        "If the specified bit of the operand is set, clear the bit and branch. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BPC,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_LDF] = {
        "LDF",
        "Load a constant from Flash space into the ACC register. The Flash address is formed by taking the TRH and TRL registers viewed as a 16-bit value (TRH being the upper 8 bits, and TRL being the lower 8 bits). No PSW flags are affected.",
        OPCODE_LDF,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_STF] = {
        "STF",
        "Write to flash some fucking how.",
        OPCODE_STF,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_DBNZ...OPCODE_DBNZ+OPCODE_DBNZ_COUNT-1] = {
        "DBNZ d9, r8",
        "Decrement the operand by one, and branch if the result is not zero. See BR for address calculation. No PSW flags are affected.",
        OPCODE_DBNZ,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_DBNZ_IND...OPCODE_DBNZ_IND+OPCODE_DBNZ_IND_COUNT-1] = {
        "DBNZ @Ri, r8",
        "Decrement the operand by one, and branch if the result is not zero. See BR for address calculation. No PSW flags are affected.",
        OPCODE_DBNZ_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2, INSTR_ARG_TYPE_R8),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_BPC + 0x10 ... OPCODE_BPC + 0x17] = {
        "BPC d9, b3, r8",
        "If the specified bit of the operand is set, clear the bit and branch. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BPC,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_PUSH...OPCODE_PUSH+OPCODE_PUSH_COUNT-1] = {
        "PUSH d9",
        "Push the operand on the stack. The SP register is first incremented by one, and the operand value is then stored at the resulting stack position. No PSW flags are affected.",
        OPCODE_PUSH,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_INC...OPCODE_INC+OPCODE_INC_COUNT-1] = {
        "INC d9",
        "Increment the operand by one. No PSW flags are affected.",
        OPCODE_INC,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_INC_IND...OPCODE_INC_IND+OPCODE_INC_IND_COUNT-1] = {
        "INC @Ri",
        "Increment the operand by one. No PSW flags are affected.",
        OPCODE_INC_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_BP ... OPCODE_BP + 0x7] = {
        "BP d9, b3, r8",
        "Branch if the specified bit of the operand is set. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BP,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_POP...OPCODE_POP+OPCODE_POP_COUNT-1] = {
        "POP d9",
        "Pop the operand from the stack. The value is read from the stack position pointed out by the current value of the SP register, and SP is then decremented by one. No PSW flags are affected.",
        OPCODE_POP,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_DEC...OPCODE_DEC+OPCODE_DEC_COUNT-1] = {
        "DEC d9",
        "Decrement the operand by one. No PSW flags are affected.",
        OPCODE_DEC,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_DEC_IND...OPCODE_DEC_IND+OPCODE_DEC_IND_COUNT-1] = {
        "DEC @Ri",
        "Decrement the operand by one. No PSW flags are affected.",
        OPCODE_DEC_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_BP + 0x10 ... OPCODE_BP + 0x17] = {
        "BP d9, b3, r8",
        "Branch if the specified bit of the operand is set. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BP,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_BZ] = {
        "BZ r8",
        "Branch if the ACC register is zero. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BZ,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_R8),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_ADDI] = {
        "ADD #i8",
        "Add the operand to the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_ADDI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_ADD...OPCODE_ADD+OPCODE_ADD_COUNT-1] = {
        "ADD d9",
        "Add the operand to the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_ADD,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_ADD_IND...OPCODE_ADD_IND+OPCODE_ADD_IND_COUNT-1] = {
        "ADD @Ri",
        "Add the operand to the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_ADD_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_BN ... OPCODE_BN + 0x7] = {
        "BN d9, b3, r8",
        "Branch if the specified bit of the operand is not set. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BN,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_BNZ] = {
        "BNZ r8",
        "Branch if the ACC register is not zero. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BNZ,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_R8),
        2,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_ADDCI] = {
        "ADDC #i8",
        "Add the operand and the carry bit (CY) to the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_ADDCI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_ADDC...OPCODE_ADDC+OPCODE_ADDC_COUNT-1] = {
        "ADDC d9",
        "Add the operand and the carry bit (CY) to the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_ADDC,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_ADDC_IND...OPCODE_ADDC_IND+OPCODE_ADDC_IND_COUNT-1] = {
        "ADDC @Ri",
        "Add the operand and the carry bit (CY) to the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_ADDC_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_BN + 0x10 ... OPCODE_BN + 0x17] = {
        "BN d9, b3, r8",
        "Branch if the specified bit of the operand is not set. See BR for address calculation. No PSW flags are affected.",
        OPCODE_BN,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8),
        3,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_RET] = {
        "RET",
        "Return from function. The PC register is popped from the stack. The upper 8 bits are popped first, then the lower 8 bits. No PSW flags are affected.",
        OPCODE_RET,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_SUBI] = {
        "SUB #i8",
        "Subtract the operand from the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_SUBI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_SUB...OPCODE_SUB+OPCODE_SUB_COUNT-1] = {
        "SUB d9",
        "Subtract the operand from the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_SUB,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_SUB_IND...OPCODE_SUB_IND+OPCODE_SUB_IND_COUNT-1] = {
        "SUB @Ri",
        "Subtract the operand from the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_SUB_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_NOT1 ... OPCODE_NOT1 + 0x7] = {
        "NOT1 d9, b3",
        "Invert the specified bit in the operand. No PSW flags are affected.",
        OPCODE_NOT1,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_RETI] = {
        "RETI",
        "Return from interrupt. The PC register is popped from the stack. The upper 8 bits are popped first, then the lower 8 bits. No PSW flags are affected.",
        OPCODE_RETI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_SUBCI] = {
        "SUBC #i8",
        "Subtract the operand and the carry bit (CY) from the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_SUBCI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_SUBC...OPCODE_SUBC+OPCODE_SUBC_COUNT-1] = {
        "SUBC d9",
        "Subtract the operand and the carry bit (CY) from the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_SUBC,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_SUBC_IND...OPCODE_SUBC_IND+OPCODE_SUBC_IND_COUNT-1] = {
        "SUBC @Ri",
        "Subtract the operand and the carry bit (CY) from the ACC register. CY, AC and OV are set according to the result.",
        OPCODE_SUBC_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_CY_MASK | OPCODE_PSW_OV_MASK | OPCODE_PSW_AC_MASK
    },
    [OPCODE_NOT1 + 0x10 ... OPCODE_NOT1 + 0x17] = {
        "NOT1 d9, b3",
        "Invert the specified bit in the operand. No PSW flags are affected.",
        OPCODE_NOT1,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_ROR] = {
        "ROR",
        "Rotate the contents of the ACC register one bit to the right. The least signigicant bit will wrap immediately around to the most signigicant bit. No PSW flags are affected.",
        OPCODE_ROR,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_LDC] = {
        "LDC",
        "Load a constant from ROM space into the ACC register. The ROM address is formed by adding the old value of ACC to the contents of the TRH and TRL registers viewed as a 16-bit value (TRH being the upper 8 bits, and TRL being the lower 8 bits). No PSW flags are affected.",
        OPCODE_LDC,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        2,
        OPCODE_PSW_NONE
    },
    [OPCODE_XCH...OPCODE_XCH+OPCODE_XCH_COUNT-1] = {
        "XCH d9",
        "Exchange the contents of the operand with the contents of the ACC register. No PSW flags are affected.",
        OPCODE_XCH,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_XCH_IND...OPCODE_XCH_IND+OPCODE_XCH_IND_COUNT-1] = {
        "XCH @Ri",
        "Exchange the contents of the operand with the contents of the ACC register. No PSW flags are affected.",
        OPCODE_XCH_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_CLR1 ... OPCODE_CLR1 + 0x7] = {
        "CLR1 d9, b3",
        "Clear the specified bit in the operand. No PSW flags are affected.",
        OPCODE_CLR1,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_RORC] = {
        "RORC",
        "Rotate the contents of the ACC register one bit to the right. The least signigicant bit is copied to the CY flag, and the old value of CY will be place in the most signigicant bit. The AC and OV flags are unaffected.",
        OPCODE_RORC,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        1,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_ORI] = {
        "OR #i8",
        "Perform bitwise OR between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_ORI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_OR...OPCODE_OR+OPCODE_OR_COUNT-1] = {
        "OR d9",
        "Perform bitwise OR between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_OR,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_OR_IND...OPCODE_OR_IND+OPCODE_OR_IND_COUNT-1] = {
        "OR d9",
        "Perform bitwise OR between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_OR_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_CLR1 + 0x10 ... OPCODE_CLR1 + 0x17] = {
        "CLR1 d9, b3",
        "Clear the specified bit in the operand. No PSW flags are affected.",
        OPCODE_CLR1,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_ROL] = {
        "ROL",
        "Rotate the contents of the ACC register one bit to the left. The most signigicant bit will wrap immediately around to the least signigicant bit. No PSW flags are affected.",
        OPCODE_ROL,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_ANDI] = {
        "AND #i8",
        "Perform bitwise AND between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_ANDI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_AND...OPCODE_AND+OPCODE_AND_COUNT-1] = {
        "AND d9",
        "Perform bitwise AND between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_AND,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_AND_IND...OPCODE_AND_IND+OPCODE_AND_IND_COUNT-1] = {
        "AND @Ri",
        "Perform bitwise AND between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_AND_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_SET1 ... OPCODE_SET1 + 0x7] = {
        "SET1 d9, b3",
        "Set the specified bit in the operand. No PSW flags are affected.",
        OPCODE_SET1,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_ROLC] = {
        "ROLC",
        "Rotate the contents of the ACC register one bit to the left. The most signigicant bit is copied to the CY flag, and the old value of CY will be place in the least signigicant bit. The AC and OV flags are unaffected.",
        OPCODE_ROLC,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_NONE),
        1,
        1,
        OPCODE_PSW_CY_MASK
    },
    [OPCODE_XORI] = {
        "XOR #i8",
        "Perform bitwise XOR between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_XORI,
        8,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_I8),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_XOR...OPCODE_XOR+OPCODE_XOR_COUNT-1] = {
        "XOR d9",
        "Perform bitwise XOR between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_XOR,
        7,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_XOR_IND...OPCODE_XOR_IND+OPCODE_XOR_IND_COUNT-1] = {
        "XOR @Ri",
        "Perform bitwise XOR between the operand and the ACC register. No PSW flags are affected.",
        OPCODE_XOR_IND,
        6,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_N2),
        1,
        1,
        OPCODE_PSW_NONE
    },
    [OPCODE_SET1 + 0x10 ... OPCODE_SET1 + 0x17] = {
        "SET1 d9, b3",
        "Set the specified bit in the operand. No PSW flags are affected.",
        OPCODE_SET1,
        5,
        INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9),
        2,
        1,
        OPCODE_PSW_NONE
    },
};


void gyVmuInstrDecodeOperands(const VMUInstr* instrData, VMUInstrOperands* operands) {
    uint32_t instr = 0;
    //unsigned int types[3] = { 0 };
    memset(operands, 0, sizeof(VMUInstrOperands));

    const unsigned argc = instrArgsArgc(_instrMap[instrData->instrBytes[INSTR_BYTE_OPCODE]].args);

    if(argc == 0) return; //No additional operands, don't fucking bother

    //concatenate instruction bytes into single integer
    switch(_instrMap[instrData->instrBytes[INSTR_BYTE_OPCODE]].bytes) {
    case 1:
        instr = instrData->instrBytes[INSTR_BYTE_OPCODE];
        break;
    case 2:
        instr = instrData->instrBytes[INSTR_BYTE_2] | (instrData->instrBytes[INSTR_BYTE_OPCODE]<<8);
        break;
    case 3:
        instr = instrData->instrBytes[INSTR_BYTE_3] | (instrData->instrBytes[INSTR_BYTE_2]<<8) | (instrData->instrBytes[INSTR_BYTE_OPCODE]<<16);
        break;
    default: assert(0);
    }

    //Have to specifically check for the two fucktardedly encoded instruction types that make no fucking sense...
    switch(_instrMap[instrData->instrBytes[INSTR_BYTE_OPCODE]].args) {
    case INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9, INSTR_ARG_TYPE_R8):
        operands->addrMode[ADDR_MODE_REL] = (uint8_t)(instr&0xff);
        operands->addrRel = (instr&0xff);
        instr >>= 8;
    case INSTR_ARG_TYPES_PACK(INSTR_ARG_TYPE_B3, INSTR_ARG_TYPE_D9):
        operands->addrMode[ADDR_MODE_DIR] = (uint8_t)(instr&0xff);
        instr >>= 8;
        operands->addrMode[ADDR_MODE_BIT] = (uint8_t)(instr&0x7);
        instr >>= 3;
        operands->addrMode[ADDR_MODE_DIR] |= (instr&0x2)<<7; //MSB is not fucking contiguous here... YOU ASSHOLES, SANYO.
        break;
    default: //instructions that have been encoded without cocks in their mouths
        for(int i = (int)argc-1; i >= 0; --i) {
            unsigned argType = INSTR_ARG_TYPES_UNPACK(_instrMap[instrData->instrBytes[INSTR_BYTE_OPCODE]].args, (unsigned)i);
            switch(argType) {
            default:
            case INSTR_ARG_TYPE_NONE:
                goto doneloop;
                break;
            case INSTR_ARG_TYPE_R8:
                operands->addrMode[ADDR_MODE_REL] = (int8_t)(instr&0xff);
                operands->addrRel = (instr&0xff);
                instr >>= 8;
                break;
            case INSTR_ARG_TYPE_R16:
                operands->addrMode[ADDR_MODE_REL] = (uint16_t)((instr>>8)&0xff);
                operands->addrMode[ADDR_MODE_REL] |= (uint16_t)((instr&0xff)<<8);
                operands->addrRel = (uint16_t)((instr>>8)&0xff);
                operands->addrRel |= (uint16_t)((instr&0xff)<<8);
                instr >>= 16;
                break;
            case INSTR_ARG_TYPE_I8:
                operands->addrMode[ADDR_MODE_IMM] = (uint8_t)(instr&0xff);
                instr >>= 8;
                break;
            case INSTR_ARG_TYPE_D9:
                operands->addrMode[ADDR_MODE_DIR] = (uint16_t)(instr&0x1ff);
                instr >>= 9;
                break;
            case INSTR_ARG_TYPE_N2:
#if 0
                operands->addrMode[ADDR_MODE_IND]    = (gyVmuMemRead(dev,
                                                            (instr & 0x3) | //Bits 0-1 come from instruction
                                                            ((dev->sfr[EVMU_SFR_OFFSET(SFR_ADDR_PSW)]&(SFR_PSW_IRBK0_MASK|SFR_PSW_IRBK1_MASK))>>0x1u)) //Bits 2-3 come from PSW
                                               | (instr&0x2)<<0x7u); //MSB of pointer is bit 1 from instruction
#else
                operands->addrMode[ADDR_MODE_IND] = (instr & 0x3);
#endif
                instr >>= 2;
                break;
            case INSTR_ARG_TYPE_A12: //So fucking stupid... Not contiguous.
                operands->addrMode[ADDR_MODE_ABS] = (uint16_t)(instr&0x7ff);
                operands->addrMode[ADDR_MODE_ABS] |= (uint16_t)((instr&0x1000)>>1);
                instr >>= 11;
                break;
            case INSTR_ARG_TYPE_A16:
                operands->addrMode[ADDR_MODE_ABS] = (uint16_t)(instr&0xffff);
                instr >>= 16;
                break;
            case INSTR_ARG_TYPE_B3:
                operands->addrMode[ADDR_MODE_BIT] = (instr&0x7);
                instr >>= 3;
                break;
            }
        }
        break;
    }
    unsigned char opcode = _instrMap[instrData->instrBytes[INSTR_BYTE_OPCODE]].opcode;
    assert(instr < 256);
    unsigned char instrcode = _instrMap[instr<<(8-_instrMap[opcode].opBits)].opcode;
    assert(instrcode == opcode);

doneloop:
    return;

}



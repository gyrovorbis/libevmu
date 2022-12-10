#include <gimbal/preprocessor/gimbal_macro_utils.h>
#include <evmu/hw/evmu_isa.h>

static const EvmuInstructionFormat opcodeMap_[EVMU_OPCODE_MAP_SIZE] = {
    [EVMU_OPCODE_NOP] = {
        "NOP",
        "Stalls processor for one clock cycle.",
        EVMU_OPCODE_NOP,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(0),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BR] = {
        "BR r8",
        "Branch unconditionally. The target address is specified using an 8-bit relative address. The signed 8-bit offset is added to the address of the instruction following the BR. No PSW flags are affected.",
        EVMU_OPCODE_BR,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_RELATIVE_8),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_LD...EVMU_OPCODE_LD+EVMU_OPCODE_LD_COUNT-1] = {
        "LD d9",
        "Load the operand into the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_LD,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_LD_IND...EVMU_OPCODE_LD_IND+EVMU_OPCODE_LD_IND_COUNT-1] = {
        "LD @Ri",
        "Load the operand into the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_LD_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_CALL...EVMU_OPCODE_CALL+0x7] = {
        "CALL a12",
        "Call function. The entry address of the function is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the CALL. The return address (the address of the instruction following the CALL instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        EVMU_OPCODE_CALL,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_ABSOLUTE_12),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_CALLR] = {
        "CALLR r16",
        "Call function. The entry address of the function is specified using a 16-bit relative address. The unsigned 16-bit offset is added to the address of the instruction following the CALLR minus one to produce the target address. The addition is performed modulo 65536, which makes it possible to call a lower address as well. The return address (the address of the instruction following the CALLR instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        EVMU_OPCODE_CALLR,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_RELATIVE_16),
        3,
        4,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BRF] = {
        "BRF r16",
        "Branch unconditionally. The target address is specified using a 16-bit relative address. The unsigned 16-bit offset is added to the address of the instruction following the BRF minus one to produce the target address. The addition is performed modulo 65536, which makes it possible to branch to a lower address as well. No PSW flags are affected.",
        EVMU_OPCODE_BRF,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_RELATIVE_16),
        3,
        4,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ST...EVMU_OPCODE_ST+EVMU_OPCODE_ST_COUNT-1] = {
        "ST d9",
        "Store the contents of the ACC register into the operand address. No PSW flags are affected.",
        EVMU_OPCODE_ST,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ST_IND...EVMU_OPCODE_ST_IND+EVMU_OPCODE_ST_IND_COUNT-1] = {
        "ST @Ri",
        "Store the contents of the ACC register into the operand address. No PSW flags are affected.",
        EVMU_OPCODE_ST_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_CALL + 0x10 ... EVMU_OPCODE_CALL+0x17] = {
        "CALL a12",
        "Call function. The entry address of the function is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the CALL. The return address (the address of the instruction following the CALL instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        EVMU_OPCODE_CALL,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_ABSOLUTE_12),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_CALLF] = {
        "CALLF a16",
        "Call function. The entry address of the function is specified using a full 16-bit absolute address. The return address (the address of the instruction following the CALLF instruction) is pushed on the stack. The lower 8 bits of the address are pushed first, then the upper 8 bits. No PSW flags are affected.",
        EVMU_OPCODE_CALLF,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_ABSOLUTE_16),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_JMPF] = {
        "JMPF a16",
        "Jump unconditionally. The target address is specified using a full 16-bit absolute address. No PSW flags are affected.",
        EVMU_OPCODE_JMPF,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_ABSOLUTE_16),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_MOV...EVMU_OPCODE_MOV+EVMU_OPCODE_MOV_COUNT-1] = {
        "MOV #i8, d9",
        "Set the contents of the operand to a constant value. No PSW flags are affected.",
        EVMU_OPCODE_MOV,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_MOV_IND...EVMU_OPCODE_MOV_IND+EVMU_OPCODE_MOV_IND_COUNT-1] = {
        "MOV #i8, @Rj",
        "Set the contents of the operand to a constant value. No PSW flags are affected.",
        EVMU_OPCODE_MOV_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2, EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_JMP...EVMU_OPCODE_JMP+0x7] = {
        "JMP a12",
        "Jump unconditionally. The target address is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the JMP. No PSW flags are affected.",
        EVMU_OPCODE_JMP,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_ABSOLUTE_12),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_MUL] = {
        "MUL",
        "Perform a multiplication. The ACC and C registers together form a 16-bit operand (ACC being the high 8 bits, and C being the low 8 bits) which is multiplied by the contents of the B register. The result is a 24-bit number that is stored in the ACC, C and B registers (the high 8 bits are stored in B, the middle 8 bits in ACC, and the low 8 bits in C). CY is cleared, and OV is set if the result is greater than 16 bits, otherwise cleared. AC is not affected.",
        EVMU_OPCODE_MUL,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        7,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK
    },
    [EVMU_OPCODE_BEI] = {
        "BE #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        EVMU_OPCODE_BEI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_BE...EVMU_OPCODE_BE+EVMU_OPCODE_BE_COUNT-1] = {
        "BE d9, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        EVMU_OPCODE_BE,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_BE_IND...EVMU_OPCODE_BE_IND+EVMU_OPCODE_BE_IND_COUNT-1] = {
        "BE @Rj, #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        EVMU_OPCODE_BE_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2, EVMU_ISA_ARG_TYPE_IMMEDIATE_8, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_JMP+0x10 ... EVMU_OPCODE_JMP+0x17] = {
        "JMP a12",
        "Jump unconditionally. The target address is specified using a 12-bit absolute address, so the upper 4 bits of this address must be the same as for the instruction following the JMP. No PSW flags are affected.",
        EVMU_OPCODE_JMP,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_ABSOLUTE_12),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_DIV] = {
        "DIV",
        "Perform a division. The ACC and C registers together form a 16-bit operand (ACC being the high 8 bits, and C being the low 8 bits) which is divided by the contents of the B register. The result is a 16-bit quotient that is stored in ACC and C (the high 8 bits in ACC, and the low 8 bits in C), and an 8-bit remainder that is stored in B. CY is cleared, and OV is set if the remainder is zero, otherwise cleared. AC is not affected.",
        EVMU_OPCODE_DIV,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        7,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK
    },
    [EVMU_OPCODE_BNEI] = {
        "BNE #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are not equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        EVMU_OPCODE_BNEI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_BNE...EVMU_OPCODE_BNE+EVMU_OPCODE_BNE_COUNT-1] = {
        "BNE d9, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are not equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        EVMU_OPCODE_BNE,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_BNE_IND...EVMU_OPCODE_BNE_IND+EVMU_OPCODE_BNE_IND_COUNT-1] = {
        "BNE @Rj, #i8, r8",
        "Branch if the contents of the ACC register (or the indirect operand in the third form above) are not equal to the immediate or direct operand. See BR for address calculation. Additionally, CY is set to 1 if ACC (or the indirect operand) is strictly less than the immediate or direct operand. AC and OV are unaffected.",
        EVMU_OPCODE_BNE_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2, EVMU_ISA_ARG_TYPE_IMMEDIATE_8, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_BPC ... EVMU_OPCODE_BPC + 0x7] = {
        "BPC d9, b3, r8",
        "If the specified bit of the operand is set, clear the bit and branch. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BPC,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_LDF] = {
        "LDF",
        "Load a constant from Flash space into the ACC register. The Flash address is formed by taking the TRH and TRL registers viewed as a 16-bit value (TRH being the upper 8 bits, and TRL being the lower 8 bits). No PSW flags are affected.",
        EVMU_OPCODE_LDF,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_STF] = {
        "STF",
        "Write to flash some fucking how.",
        EVMU_OPCODE_STF,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_DBNZ...EVMU_OPCODE_DBNZ+EVMU_OPCODE_DBNZ_COUNT-1] = {
        "DBNZ d9, r8",
        "Decrement the operand by one, and branch if the result is not zero. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_DBNZ,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_DBNZ_IND...EVMU_OPCODE_DBNZ_IND+EVMU_OPCODE_DBNZ_IND_COUNT-1] = {
        "DBNZ @Ri, r8",
        "Decrement the operand by one, and branch if the result is not zero. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_DBNZ_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BPC + 0x10 ... EVMU_OPCODE_BPC + 0x17] = {
        "BPC d9, b3, r8",
        "If the specified bit of the operand is set, clear the bit and branch. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BPC,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_PUSH...EVMU_OPCODE_PUSH+EVMU_OPCODE_PUSH_COUNT-1] = {
        "PUSH d9",
        "Push the operand on the stack. The SP register is first incremented by one, and the operand value is then stored at the resulting stack position. No PSW flags are affected.",
        EVMU_OPCODE_PUSH,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_INC...EVMU_OPCODE_INC+EVMU_OPCODE_INC_COUNT-1] = {
        "INC d9",
        "Increment the operand by one. No PSW flags are affected.",
        EVMU_OPCODE_INC,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_INC_IND...EVMU_OPCODE_INC_IND+EVMU_OPCODE_INC_IND_COUNT-1] = {
        "INC @Ri",
        "Increment the operand by one. No PSW flags are affected.",
        EVMU_OPCODE_INC_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BP ... EVMU_OPCODE_BP + 0x7] = {
        "BP d9, b3, r8",
        "Branch if the specified bit of the operand is set. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BP,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_POP...EVMU_OPCODE_POP+EVMU_OPCODE_POP_COUNT-1] = {
        "POP d9",
        "Pop the operand from the stack. The value is read from the stack position pointed out by the current value of the SP register, and SP is then decremented by one. No PSW flags are affected.",
        EVMU_OPCODE_POP,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_DEC...EVMU_OPCODE_DEC+EVMU_OPCODE_DEC_COUNT-1] = {
        "DEC d9",
        "Decrement the operand by one. No PSW flags are affected.",
        EVMU_OPCODE_DEC,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_DEC_IND...EVMU_OPCODE_DEC_IND+EVMU_OPCODE_DEC_IND_COUNT-1] = {
        "DEC @Ri",
        "Decrement the operand by one. No PSW flags are affected.",
        EVMU_OPCODE_DEC_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BP + 0x10 ... EVMU_OPCODE_BP + 0x17] = {
        "BP d9, b3, r8",
        "Branch if the specified bit of the operand is set. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BP,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BZ] = {
        "BZ r8",
        "Branch if the ACC register is zero. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BZ,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_RELATIVE_8),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ADDI] = {
        "ADD #i8",
        "Add the operand to the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_ADDI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_ADD...EVMU_OPCODE_ADD+EVMU_OPCODE_ADD_COUNT-1] = {
        "ADD d9",
        "Add the operand to the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_ADD,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_ADD_IND...EVMU_OPCODE_ADD_IND+EVMU_OPCODE_ADD_IND_COUNT-1] = {
        "ADD @Ri",
        "Add the operand to the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_ADD_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_BN ... EVMU_OPCODE_BN + 0x7] = {
        "BN d9, b3, r8",
        "Branch if the specified bit of the operand is not set. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BN,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_BNZ] = {
        "BNZ r8",
        "Branch if the ACC register is not zero. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BNZ,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_RELATIVE_8),
        2,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ADDCI] = {
        "ADDC #i8",
        "Add the operand and the carry bit (CY) to the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_ADDCI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_ADDC...EVMU_OPCODE_ADDC+EVMU_OPCODE_ADDC_COUNT-1] = {
        "ADDC d9",
        "Add the operand and the carry bit (CY) to the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_ADDC,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_ADDC_IND...EVMU_OPCODE_ADDC_IND+EVMU_OPCODE_ADDC_IND_COUNT-1] = {
        "ADDC @Ri",
        "Add the operand and the carry bit (CY) to the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_ADDC_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_BN + 0x10 ... EVMU_OPCODE_BN + 0x17] = {
        "BN d9, b3, r8",
        "Branch if the specified bit of the operand is not set. See BR for address calculation. No PSW flags are affected.",
        EVMU_OPCODE_BN,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9, EVMU_ISA_ARG_TYPE_RELATIVE_8),
        3,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_RET] = {
        "RET",
        "Return from function. The PC register is popped from the stack. The upper 8 bits are popped first, then the lower 8 bits. No PSW flags are affected.",
        EVMU_OPCODE_RET,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_SUBI] = {
        "SUB #i8",
        "Subtract the operand from the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_SUBI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_SUB...EVMU_OPCODE_SUB+EVMU_OPCODE_SUB_COUNT-1] = {
        "SUB d9",
        "Subtract the operand from the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_SUB,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_SUB_IND...EVMU_OPCODE_SUB_IND+EVMU_OPCODE_SUB_IND_COUNT-1] = {
        "SUB @Ri",
        "Subtract the operand from the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_SUB_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_NOT1 ... EVMU_OPCODE_NOT1 + 0x7] = {
        "NOT1 d9, b3",
        "Invert the specified bit in the operand. No PSW flags are affected.",
        EVMU_OPCODE_NOT1,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_RETI] = {
        "RETI",
        "Return from interrupt. The PC register is popped from the stack. The upper 8 bits are popped first, then the lower 8 bits. No PSW flags are affected.",
        EVMU_OPCODE_RETI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_SUBCI] = {
        "SUBC #i8",
        "Subtract the operand and the carry bit (CY) from the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_SUBCI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_SUBC...EVMU_OPCODE_SUBC+EVMU_OPCODE_SUBC_COUNT-1] = {
        "SUBC d9",
        "Subtract the operand and the carry bit (CY) from the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_SUBC,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_SUBC_IND...EVMU_OPCODE_SUBC_IND+EVMU_OPCODE_SUBC_IND_COUNT-1] = {
        "SUBC @Ri",
        "Subtract the operand and the carry bit (CY) from the ACC register. CY, AC and OV are set according to the result.",
        EVMU_OPCODE_SUBC_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_CY_MASK | EVMU_ISA_PSW_OV_MASK | EVMU_ISA_PSW_AC_MASK
    },
    [EVMU_OPCODE_NOT1 + 0x10 ... EVMU_OPCODE_NOT1 + 0x17] = {
        "NOT1 d9, b3",
        "Invert the specified bit in the operand. No PSW flags are affected.",
        EVMU_OPCODE_NOT1,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ROR] = {
        "ROR",
        "Rotate the contents of the ACC register one bit to the right. The least signigicant bit will wrap immediately around to the most signigicant bit. No PSW flags are affected.",
        EVMU_OPCODE_ROR,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_LDC] = {
        "LDC",
        "Load a constant from ROM space into the ACC register. The ROM address is formed by adding the old value of ACC to the contents of the TRH and TRL registers viewed as a 16-bit value (TRH being the upper 8 bits, and TRL being the lower 8 bits). No PSW flags are affected.",
        EVMU_OPCODE_LDC,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        2,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_XCH...EVMU_OPCODE_XCH+EVMU_OPCODE_XCH_COUNT-1] = {
        "XCH d9",
        "Exchange the contents of the operand with the contents of the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_XCH,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_XCH_IND...EVMU_OPCODE_XCH_IND+EVMU_OPCODE_XCH_IND_COUNT-1] = {
        "XCH @Ri",
        "Exchange the contents of the operand with the contents of the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_XCH_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_CLR1 ... EVMU_OPCODE_CLR1 + 0x7] = {
        "CLR1 d9, b3",
        "Clear the specified bit in the operand. No PSW flags are affected.",
        EVMU_OPCODE_CLR1,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_RORC] = {
        "RORC",
        "Rotate the contents of the ACC register one bit to the right. The least signigicant bit is copied to the CY flag, and the old value of CY will be place in the most signigicant bit. The AC and OV flags are unaffected.",
        EVMU_OPCODE_RORC,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        1,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_ORI] = {
        "OR #i8",
        "Perform bitwise OR between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_ORI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_OR...EVMU_OPCODE_OR+EVMU_OPCODE_OR_COUNT-1] = {
        "OR d9",
        "Perform bitwise OR between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_OR,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_OR_IND...EVMU_OPCODE_OR_IND+EVMU_OPCODE_OR_IND_COUNT-1] = {
        "OR d9",
        "Perform bitwise OR between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_OR_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_CLR1 + 0x10 ... EVMU_OPCODE_CLR1 + 0x17] = {
        "CLR1 d9, b3",
        "Clear the specified bit in the operand. No PSW flags are affected.",
        EVMU_OPCODE_CLR1,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ROL] = {
        "ROL",
        "Rotate the contents of the ACC register one bit to the left. The most signigicant bit will wrap immediately around to the least signigicant bit. No PSW flags are affected.",
        EVMU_OPCODE_ROL,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ANDI] = {
        "AND #i8",
        "Perform bitwise AND between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_ANDI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_AND...EVMU_OPCODE_AND+EVMU_OPCODE_AND_COUNT-1] = {
        "AND d9",
        "Perform bitwise AND between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_AND,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_AND_IND...EVMU_OPCODE_AND_IND+EVMU_OPCODE_AND_IND_COUNT-1] = {
        "AND @Ri",
        "Perform bitwise AND between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_AND_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_SET1 ... EVMU_OPCODE_SET1 + 0x7] = {
        "SET1 d9, b3",
        "Set the specified bit in the operand. No PSW flags are affected.",
        EVMU_OPCODE_SET1,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_ROLC] = {
        "ROLC",
        "Rotate the contents of the ACC register one bit to the left. The most signigicant bit is copied to the CY flag, and the old value of CY will be place in the least signigicant bit. The AC and OV flags are unaffected.",
        EVMU_OPCODE_ROLC,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_NONE),
        1,
        1,
        EVMU_ISA_PSW_CY_MASK
    },
    [EVMU_OPCODE_XORI] = {
        "XOR #i8",
        "Perform bitwise XOR between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_XORI,
        8,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_IMMEDIATE_8),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_XOR...EVMU_OPCODE_XOR+EVMU_OPCODE_XOR_COUNT-1] = {
        "XOR d9",
        "Perform bitwise XOR between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_XOR,
        7,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_XOR_IND...EVMU_OPCODE_XOR_IND+EVMU_OPCODE_XOR_IND_COUNT-1] = {
        "XOR @Ri",
        "Perform bitwise XOR between the operand and the ACC register. No PSW flags are affected.",
        EVMU_OPCODE_XOR_IND,
        6,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_INDIRECT_2),
        1,
        1,
        EVMU_ISA_PSW_NONE
    },
    [EVMU_OPCODE_SET1 + 0x10 ... EVMU_OPCODE_SET1 + 0x17] = {
        "SET1 d9, b3",
        "Set the specified bit in the operand. No PSW flags are affected.",
        EVMU_OPCODE_SET1,
        5,
        EVMU_ISA_ARG_FORMAT_PACK(EVMU_ISA_ARG_TYPE_BIT_3, EVMU_ISA_ARG_TYPE_DIRECT_9),
        2,
        1,
        EVMU_ISA_PSW_NONE
    },
};

EVMU_EXPORT const EvmuInstructionFormat* EvmuIsa_format(EvmuWord firstByte) {
    return &opcodeMap_[firstByte];
}

EVMU_EXPORT EVMU_RESULT EvmuIsa_fetch(EvmuInstruction* pEncoded, const void* pBuffer, GblSize* pBytes) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_POINTER(pEncoded);
    GBL_CTX_VERIFY_POINTER(pBuffer);
    GBL_CTX_VERIFY_POINTER(pBytes);
    GBL_CTX_VERIFY_ARG(*pBytes);

    memset(pEncoded, 0, sizeof(EvmuInstruction));

    const EvmuInstructionFormat* pFormat = EvmuIsa_format(*(uint8_t*)pBuffer);
    GBL_ASSERT(pFormat);

    GBL_CTX_VERIFY_EXPRESSION(*pBytes >= pFormat->bytes,
                              "Opcode %s expects %u byte instruction!",
                              pFormat->pMnemonic,
                              pFormat->bytes);

    memcpy(pEncoded->bytes, pBuffer, pFormat->bytes);
    pEncoded->byteCount = pFormat->bytes;
    *pBytes = pFormat->bytes;

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuIsa_decode(const EvmuInstruction* pEncoded, EvmuDecodedInstruction* pDecoded) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY_POINTER(pEncoded);
    GBL_CTX_VERIFY_POINTER(pDecoded);

    // Local utility macros used to simplify individual operand extraction later.
#define EXTRACT_OPERAND_GENERIC(var, bitCount, srcBit, destBit, op)                          \
    GBL_STMT_START {                                                                         \
        pDecoded->operands.var op ((instrCode & GBL_BIT_MASK(bitCount, srcBit)) << destBit); \
        instrCode >>= bitCount;                                                              \
    } GBL_STMT_END

#define EXTRACT_OPERAND(var, bitCount) \
    EXTRACT_OPERAND_GENERIC(var, bitCount, 0u, 0u, =)

#define EXTRACT_OPERAND_APPEND(var, bitCount, srcBit, destBit) \
    EXTRACT_OPERAND_GENERIC(var, bitCount, srcBit, destBit, |=)

    // Retrieve detailed instruction format information
    const EvmuInstructionFormat* pFmt = EvmuIsa_format(pEncoded->bytes[EVMU_INSTRUCTION_BYTE_OPCODE]);

    // Initialize decoded instruction
    memset(pDecoded, 0, sizeof(EvmuDecodedInstruction));
    pDecoded->opcode = pFmt->opcode;

    // Fetch argument count
    const uint8_t argc = EVMU_ISA_ARGC(pFmt->args);

    if(!argc) GBL_CTX_DONE();

    // Concatenate bytes into a uint32 in reverse order for more easy processing.
    uint32_t instrCode = 0;
    for(uint32_t byte = 0; byte < pFmt->bytes; ++byte) {
        const uint32_t destByte = pFmt->bytes - 1 - byte;
        instrCode |= pEncoded->bytes[byte] << (8 * destByte);
    }

    // Begin instruction decoding.

    /* Check whether the instruction is one of the two special classes whose encoding
     * is totally disjoint from the rest and makes no sense...
     */
    if(EVMU_ISA_ARG_FORMAT_EXTRACT(pFmt->args, EVMU_ISA_ARG1) == EVMU_ISA_ARG_TYPE_BIT_3 &&
       EVMU_ISA_ARG_FORMAT_EXTRACT(pFmt->args, EVMU_ISA_ARG2) == EVMU_ISA_ARG_TYPE_DIRECT_9)
    {
        if(EVMU_ISA_ARG_FORMAT_EXTRACT(pFmt->args, EVMU_ISA_ARG3) == EVMU_ISA_ARG_TYPE_RELATIVE_8) {
            EXTRACT_OPERAND(relative8, 8u);
        }

        EXTRACT_OPERAND(direct, 8u);
        EXTRACT_OPERAND(bit, 3u);
        EXTRACT_OPERAND_APPEND(direct, 1u, 1u, 7u);    // Fuck this layout

    // Handle all other instruction types without dicks up their ass consistently.
    } else {

        for(int a = (int)argc-1; a >= 0; --a) {
            const EVMU_ISA_ARG_TYPE argType = EVMU_ISA_ARG_FORMAT_EXTRACT(pFmt->args, (unsigned)a);

            switch(argType) {
            case EVMU_ISA_ARG_TYPE_RELATIVE_8:
                EXTRACT_OPERAND(relative8, 8);
                break;
            case EVMU_ISA_ARG_TYPE_RELATIVE_16:
                // Bytes order is swapped here!
                EXTRACT_OPERAND_GENERIC(relative16, 8u, 0u, 8u, =);
                EXTRACT_OPERAND_APPEND(relative16, 8u, 0u, 0u);
                break;
            case EVMU_ISA_ARG_TYPE_IMMEDIATE_8:
                EXTRACT_OPERAND(immediate, 8u);
                break;
            case EVMU_ISA_ARG_TYPE_DIRECT_9:
                EXTRACT_OPERAND(direct,  9u);
                break;
            case EVMU_ISA_ARG_TYPE_INDIRECT_2:
                EXTRACT_OPERAND(indirect, 2u);
                break;
            case EVMU_ISA_ARG_TYPE_ABSOLUTE_12: //So fucking stupid... Not contiguous.
                EXTRACT_OPERAND(absolute, 11u);
                EXTRACT_OPERAND_APPEND(absolute, 1u, 1u, 10u);
                break;
            case EVMU_ISA_ARG_TYPE_ABSOLUTE_16:
                EXTRACT_OPERAND(absolute, 16u);
                break;
            case EVMU_ISA_ARG_TYPE_BIT_3:
                EXTRACT_OPERAND(bit, 3u);
                break;
            default:
            case EVMU_ISA_ARG_TYPE_NONE:
                goto exit_loop;
                break;
            }
        }
    }
exit_loop:

#undef EXTRACT_OPERAND
#undef EXTRACT_OPERAND_APPEND
#undef EXTRACT_OPERAND_GENERIC

    GBL_CTX_END();
}

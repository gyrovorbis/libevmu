#include "evmu_isa_test_suite.h"
#include <gimbal/test/gimbal_test_macros.h>

#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_address_space.h>

#define EVMU_ISA_TEST_SUITE_(self)  ((EvmuIsaTestSuite_*)GBL_INSTANCE_PRIVATE(self, EVMU_ISA_TEST_SUITE_TYPE))

#define GBL_TEST_SUITE_SELF EvmuIsaTestSuite

GBL_TEST_FIXTURE {
    EvmuInstruction instr;
};

GBL_TEST_INIT() {
    GBL_TEST_CASE_END;
}

GBL_TEST_FINAL() {

    GBL_TEST_CASE_END;
}

static void fill_(EvmuIsaTestSuite_* pFixture, unsigned count, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    pFixture->instr.byteCount = count;
    pFixture->instr.bytes[0] = byte1;
    pFixture->instr.bytes[1] = byte2;
    pFixture->instr.bytes[2] = byte3;
}

GBL_RESULT verifyDecode_(GblTestSuite* pSelf,
                         const char encoded[static 3],
                         const EvmuDecodedInstruction* pDecoded)
{
    GBL_CTX_BEGIN(pSelf);
    EvmuIsaTestSuite_* pFixture = EVMU_ISA_TEST_SUITE_(pSelf);
    EvmuDecodedInstruction decoded;

    fill_(pFixture, 3, encoded[0], encoded[1], encoded[2]);

    GBL_CTX_VERIFY_CALL(EvmuIsa_decode(&pFixture->instr, &decoded));

    const int diff = memcmp(pDecoded, &decoded, sizeof(EvmuDecodedInstruction));
    GBL_TEST_COMPARE(diff, 0);

    GBL_CTX_END();
}

GBL_TEST_CASE(nop) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { 0x0, 0x0, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_NOP
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ld) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_LD|0x3, 0xff, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_LD,
                .operands = {
                    .direct = 0x1ff
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ldInd) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_LD_IND|0x3, 0x0, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_LD_IND,
                .operands = {
                    .indirect = 0x3
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(mov) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { 0x23, 0x00, 0x02 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_MOV,
                .operands = {
                    .direct = EVMU_ADDRESS_SFR_ACC,
                    .immediate = 0x2
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(movInd) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_MOV_IND|0x2, 0xfe, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_MOV_IND,
                .operands = {
                    .indirect = 0x2,
                    .immediate = 0xfe
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ldc) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_LDC, 0x0, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_LDC
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(jmp) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_JMP|0x3f, 0x0e, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_JMP,
                .operands = {
                    .absolute = 0x0f0e
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(jmpf) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_JMPF|0x21, 0x7f, 0x0e },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_JMPF,
                .operands = {
                    .absolute = 0x7f0e
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(br) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_BR|0x01, 0x3f, 0x00 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_BR,
                .operands = {
                    .relative8 = 0x3f
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(brf) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_BRF, 0x3f, 0x53 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_BRF,
                .operands = {
                    .relative16 = 0x533f
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bp) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_BP|0x10|0xd, 0xab, 0xfe },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_BP,
                .operands = {
                    .direct = 0x1ab,
                    .bit = 0x5,
                    .relative8 = 0xfe,
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clr1) {
    GBL_TEST_CALL(
        verifyDecode_(pSelf,
            (const char[]) { EVMU_OPCODE_CLR1|0x10|0xd, 0xab, 0x0 },
            &(const EvmuDecodedInstruction) {
                .opcode = EVMU_OPCODE_CLR1,
                .operands = {
                    .direct = 0x1ab,
                    .bit = 0x5,
                }
            }));

    GBL_TEST_CASE_END;
}

GBL_TEST_REGISTER(nop,
                  ld,
                  ldInd,
                  mov,
                  movInd,
                  ldc,
                  jmp,
                  jmpf,
                  br,
                  brf,
                  bp,
                  clr1);

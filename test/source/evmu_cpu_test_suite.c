#include "evmu_cpu_test_suite.h"
#include <gimbal/test/gimbal_test_macros.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_address_space.h>
#include <evmu/hw/evmu_pic.h>

#define EVMU_CPU_TEST_SUITE_(instance)  ((EvmuCpuTestSuite_*)GBL_INSTANCE_PRIVATE(instance, EVMU_CPU_TEST_SUITE_TYPE))

#define GBL_TEST_SUITE_SELF EvmuCpuTestSuite

GBL_TEST_FIXTURE {
    EvmuDevice* pDevice;
    EvmuCpu*    pCpu;
    EvmuMemory* pMemory;
};

GBL_TEST_INIT() {
    pFixture->pDevice = GBL_OBJECT_NEW(EvmuDevice);
    pFixture->pCpu = pFixture->pDevice->pCpu;
    pFixture->pMemory = pFixture->pDevice->pMemory;

    EvmuMemory_setExtSource(pFixture->pMemory, EVMU_MEMORY_EXT_SRC_FLASH_BANK_0);
    GBL_TEST_CASE_END;
}

GBL_TEST_FINAL() {
    GBL_BOX_UNREF(pFixture->pDevice);
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(nop) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_NOP,
                                        }));
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ld) {
    EvmuMemory_writeInt(pFixture->pDevice->pMemory, 0x2, 27);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_LD,
                                            .operands = {
                                                .direct = 0x2
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pDevice->pMemory, EVMU_ADDRESS_SFR_ACC), 27);

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(ldInd) {
    const EvmuAddress ind = EvmuMemory_indirectAddress(pFixture->pMemory, 3);

    EvmuMemory_writeInt(pFixture->pMemory, ind, 0xab);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_LD_IND,
                                            .operands = {
                                                .indirect = 3
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pDevice->pMemory,
                                        EVMU_ADDRESS_SFR_ACC), 0xab);
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(st) {
    EvmuMemory_writeInt(pFixture->pDevice->pMemory, EVMU_ADDRESS_SFR_ACC, 128);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ST,
                                            .operands = {
                                                .direct = 3
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pDevice->pMemory, 3), 128);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(stInd) {
    EvmuMemory_writeInt(pFixture->pDevice->pMemory, EVMU_ADDRESS_SFR_ACC, 129);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ST_IND,
                                            .operands = {
                                                .indirect = 2
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory,
                                        EvmuMemory_indirectAddress(pFixture->pMemory, 2)), 129);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(mov) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_MOV,
                                            .operands = {
                                                .direct = 4,
                                                .immediate = 255
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pDevice->pMemory, 4), 255);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(movInd) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_MOV_IND,
                                            .operands = {
                                                .indirect  = 3,
                                                .immediate = 245
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory,
                                        EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 245);

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(push) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_PUSH,
                                            .operands = {
                                                .direct = 3,
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 0), 128);
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 1);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(pop) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_POP,
                                            .operands = {
                                                .direct = 5,
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_viewInt(pFixture->pDevice->pMemory, 5), 128);
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 0);

    GBL_TEST_CASE_END;
}

// TEST INVALID
GBL_TEST_CASE(br) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BR,
                                            .operands = {
                                                .relative8 = 5
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc+5);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(brf) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BRF,
                                            .operands = {
                                                .relative16 = 0x10ab
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc+0x10ab-1);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(jmp) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_JMP,
                                            .operands = {
                                                .absolute = 0xabc
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), 0x1abc);

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(jmpf) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_JMPF,
                                            .operands = {
                                                .absolute = 0xabc
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), 0xabc);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(call) {
    EvmuCpu_setPc(pFixture->pDevice->pCpu, 0xbabe);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_CALL,
                                            .operands = {
                                                .absolute = 0xdead,
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 2);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 1), 0xbe);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 0), 0xba);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0xbead);

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(callr) {
    EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_CALLR,
                                            .operands = {
                                                .relative16 = 0x1f1
                                            }
                                        }));
    pc += 0x1f1-1;
    pc %= UINT16_MAX;

    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 4);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 1), 0xad);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 0), 0xbe);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), pc);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(callf) {
    EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_CALLF,
                                            .operands = {
                                                .absolute = 0x00a
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 6);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 1), pc&0xff);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pFixture->pDevice->pMemory, 0), (pc&0xff00)>>8);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0x00a);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ret) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 4);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 2);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0xbead);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pFixture->pDevice->pMemory), 0);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0xbabe);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bei) {
    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 44);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), 0xbabe - 17);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 33);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), 0xbabe - 17);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(be) {
    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pFixture->pMemory, 0xad, 80);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 80);

    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 22);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 60);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 22);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(beInd) {
    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);
    EvmuMemory_writeInt(pFixture->pMemory,
                        EvmuMemory_indirectAddress(pFixture->pMemory, 3), 77);

    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 77,
                                                .relative8 = -128
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 128);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 78,
                                                .relative8 = -128
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 128);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bnei) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 43);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 17);
    GBL_TEST_VERIFY(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 44);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));

    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 17);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bne) {
    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pFixture->pMemory, 0xad, 80);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 79);

    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 22);
    GBL_TEST_VERIFY(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 80);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 22);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bneInd) {
    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);
    EvmuMemory_writeInt(pFixture->pMemory,
                        EvmuMemory_indirectAddress(pFixture->pMemory, 3), 76);

    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 77,
                                                .relative8 = -128
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 128);
    GBL_TEST_VERIFY(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 76,
                                                .relative8 = -128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 128);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bp) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuMemory_writeInt(pFixture->pMemory, 0x3, 0xf8);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BP,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BP,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 1,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bpc) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuMemory_writeInt(pFixture->pMemory, 0x3, 0xff);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BPC,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x3), 0x7f);


    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BPC,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x3), 0x7f);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bn) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuMemory_writeInt(pFixture->pMemory, 0x3, 0x7f);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BN,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BN,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 6,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bz) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BZ,
                                            .operands = {
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x1);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BZ,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 6,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(dbnz) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuMemory_writeInt(pFixture->pMemory, 0x3, 0x2);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ,
                                            .operands = {
                                                .direct    = 0x3,
                                                .relative8 = 11
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 11);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ,
                                            .operands = {
                                                .direct    = 0x3,
                                                .relative8 = 11
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 11);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(dbnzInd) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuMemory_writeInt(pFixture->pMemory,
                        EvmuMemory_indirectAddress(pFixture->pMemory, 2), 0x2);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ_IND,
                                            .operands = {
                                                .indirect  = 2,
                                                .relative8 = 12
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 12);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ_IND,
                                            .operands = {
                                                .indirect  = 2,
                                                .relative8 = 12
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 12);

    GBL_TEST_CASE_END;
}

static GBL_RESULT clearPswFlags_(GblTestSuite* pSelf) {
    GBL_CTX_BEGIN(pSelf);
    EvmuCpuTestSuite_* pFixture = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    psw &= ~(EVMU_SFR_PSW_CY_MASK|EVMU_SFR_PSW_AC_MASK|EVMU_SFR_PSW_OV_MASK);
    GBL_TEST_CALL(EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, psw));

    GBL_CTX_END();
}

static GBL_RESULT testPswFlags_(GblTestSuite* pSelf, GblBool cy, GblBool ac, GblBool ov) {
    GBL_CTX_BEGIN(pSelf);
    EvmuCpuTestSuite_* pFixture = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW);
    GBL_TEST_COMPARE(((psw & EVMU_SFR_PSW_CY_MASK) >> EVMU_SFR_PSW_CY_POS), cy);
    GBL_TEST_COMPARE(((psw & EVMU_SFR_PSW_AC_MASK) >> EVMU_SFR_PSW_AC_POS), ac);
    GBL_TEST_COMPARE(((psw & EVMU_SFR_PSW_OV_MASK) >> EVMU_SFR_PSW_OV_POS), ov);

    GBL_CTX_END();
}

GBL_TEST_CASE(addi) {
    // ACC initial value
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);

    // Add immediate with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0x13
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Add immediate with AC set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0xa
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Add immediate with AC and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Add immediate with CY and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0x80
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(add) {
    // ACC initial value
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x13);

    // Add direct with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Add direct with AC set
    EvmuMemory_writeInt(pFixture->pMemory, 0x69, 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x69
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Add direct with AC and OV set
    EvmuMemory_writeInt(pFixture->pMemory, 0x70, 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x70
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Add direct with CY and OV set
    EvmuMemory_writeInt(pFixture->pMemory, 0x71, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x71
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(addInd) {
    // ACC initial value
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x13);

    // Add indirect with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Add indirect with AC set
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Add indirect with AC and OV set
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Add indirect with CY and OV set
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(addci) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    // ACC initial value
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);

    // Addc immediate with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0x13
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Addc immediate with AC set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0xa
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Addc immediate with AC and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Addc immediate with CY and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0x80
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));


    // Addc immediate with CY in
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(addc) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    // ACC initial value
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x13);

    // Addc with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Addc with AC set
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Addc with AC and OV set
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Addc with CY and OV set
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    // addc 0x68: accum: 0x1, CY = 1 => mem[0x68] = 3
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(addcInd) {    // ACC initial value
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x13);

    // Addc indirect with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Addc indirect with AC set
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Addc indirect with AC and OV set
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Addc indirect with CY and OV set
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));


    // Addc indirect with CY in
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subi) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0xc
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0x2
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0x95
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(sub) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subInd) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subci) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0xc
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0x2
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0x95
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subc) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuMemory_writeInt(pFixture->pMemory, 0x68, 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subcInd) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(mul) {
    // Case 1
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, 0xc4);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x11);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C,   0x23);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B,   0x52);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_MUL
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x7d);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C),   0x36);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B),   0x5);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Case 2
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, 0xc4);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x7);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C,   0x5);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B,   0x10);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_MUL
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x70);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C),   0x50);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B),   0x00);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(div) {
    // Case 1
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, 0xc4);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x79);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C,   0x5);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B,   0x7);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DIV
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x11);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C),   0x49);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B),   0x6);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Case 2
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_PSW, 0xc0);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x7);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C,   0x10);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B,   0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DIV
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_C),   0x10);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_B),   0x00);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(andi) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ANDI,
                                      .operands = {
                                          .immediate = 0x55
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ANDI,
                                      .operands = {
                                          .immediate = 0xaa
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(and) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0xff);
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x55);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xaa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(andInd) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0xff);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x55);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xaa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ori) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0x3
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0xc
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf);


    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0x30
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3f);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0xc0
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(or) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x3);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xc);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x30);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3f);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xc0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(orInd) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x3);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xc);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0x30);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3f);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xc0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xori) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf0
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf0
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xor) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xf);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf0);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xorInd) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xf);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xf0);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(rol) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROL
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xaa);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROL
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x55);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(rolc) {
    clearPswFlags_(pSelf);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x60);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xc0);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x80);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));


    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ror) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x1);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROR
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x80);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROR
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x40);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(rorc) {
    clearPswFlags_(pSelf);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x6);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x80);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));


    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xc0);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(inc) {
    clearPswFlags_(pSelf);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x1);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0xf1);


    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xff);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(incInd) {
    clearPswFlags_(pSelf);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3), 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory,
                                        EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 0x1);

    EvmuMemory_writeInt(pFixture->pMemory,
                        EvmuMemory_indirectAddress(pFixture->pMemory, 3), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory,
                                        EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 0xf1);


    EvmuMemory_writeInt(pFixture->pMemory,
                        EvmuMemory_indirectAddress(pFixture->pMemory, 3), 0xff);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory,
                                        EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 0x0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(dec) {
    clearPswFlags_(pSelf);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x1);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0xef);


    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(decInd) {
    clearPswFlags_(pSelf);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3), 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 0x1);

    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 0xef);


    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3), 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 3)), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xch) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x33);
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XCH,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x33);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xchInd) {
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x33);
    EvmuMemory_writeInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1), 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XCH_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0xff);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EvmuMemory_indirectAddress(pFixture->pMemory, 1)), 0x33);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clr1) {
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_CLR1,
                                      .operands = {
                                          .bit = 0x0,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x0);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_CLR1,
                                      .operands = {
                                          .bit = 0x7,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(set1) {
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SET1,
                                      .operands = {
                                          .bit = 0x0,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x1);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x00);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SET1,
                                      .operands = {
                                          .bit = 0x7,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x80);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(not1) {
    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_NOT1,
                                      .operands = {
                                          .bit = 0x0,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x1);

    EvmuMemory_writeInt(pFixture->pMemory, 0x23, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_NOT1,
                                      .operands = {
                                          .bit = 0x7,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, 0x23), 0x0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ldc) {
    EvmuMemory_setExtSource(pFixture->pMemory, EVMU_MEMORY_EXT_SRC_FLASH_BANK_0);
    EvmuMemory_writeExt(pFixture->pMemory, 0x123, 0x77);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, 0x1);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, 0x23);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_LDC,
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x77);

    EvmuMemory_setExtSource(pFixture->pMemory, EVMU_MEMORY_EXT_SRC_ROM);
    EvmuMemory_writeExt(pFixture->pMemory, 0x123, 0x33);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, 0x1);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, 0x23);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_LDC,
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x33);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(reti) {
    EvmuPic_raiseIrq(pFixture->pDevice->pPic, EVMU_IRQ_EXT_INT3_TBASE);
    EvmuPic_update(pFixture->pDevice->pPic);

    GBL_TEST_COMPARE(EvmuPic_irqsActiveDepth(pFixture->pDevice->pPic), 1);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RETI,
                                  }));

    GBL_TEST_COMPARE(EvmuPic_irqsActiveDepth(pFixture->pDevice->pPic), 0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ldf) {
    EvmuMemory_writeFlash(pFixture->pMemory, 0x1abcd, 0x89);

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_FPR, 0x1);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, 0xab);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, 0xcd);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_LDF,
                                  }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC), 0x89);

    GBL_TEST_CASE_END;
}

static GBL_RESULT stfUnlockToState_(GblTestSuite* pSelf, EVMU_FLASH_PROGRAM_STATE state) {
    GBL_CTX_BEGIN(pSelf);
    EvmuCpuTestSuite_* pFixture = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuWord oldTrh = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH);
    const EvmuWord oldTrl = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL);
    const EvmuWord oldAcc = EvmuMemory_readInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC);

    if(state >= EVMU_FLASH_PROGRAM_STATE_0) {
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, (EVMU_FLASH_PROGRAM_STATE_0_ADDRESS & 0xff00)>>8);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, EVMU_FLASH_PROGRAM_STATE_0_ADDRESS & 0xff);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, EVMU_FLASH_PROGRAM_STATE_0_VALUE);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));
    }

    if(state >= EVMU_FLASH_PROGRAM_STATE_1) {
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, (EVMU_FLASH_PROGRAM_STATE_1_ADDRESS & 0xff00)>>8);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, EVMU_FLASH_PROGRAM_STATE_1_ADDRESS & 0xff);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, EVMU_FLASH_PROGRAM_STATE_1_VALUE);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));
    }

    if(state >= EVMU_FLASH_PROGRAM_STATE_2) {
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, (EVMU_FLASH_PROGRAM_STATE_2_ADDRESS & 0xff00)>>8);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, EVMU_FLASH_PROGRAM_STATE_2_ADDRESS & 0xff);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, EVMU_FLASH_PROGRAM_STATE_2_VALUE);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));
    }

    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, oldTrh);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, oldTrl);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, oldAcc);

    GBL_CTX_END();
}

GBL_TEST_CASE(stf) {
    // Write initial value to flash (so we can check if we overrode it)
    EvmuMemory_writeFlash(pFixture->pMemory, 0x1ab00+129, 0x76);

    // Configure flash address registers
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_FPR, 0x1);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRH, 0xab);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, 0xcd);

    // Write without being in system mode
    EvmuMemory_setExtSource(pFixture->pMemory, EVMU_MEMORY_EXT_SRC_FLASH_BANK_0);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write without unlocking
    EvmuMemory_setExtSource(pFixture->pMemory, EVMU_MEMORY_EXT_SRC_FLASH_BANK_0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write at state 0
    EvmuMemory_setExtSource(pFixture->pMemory, EVMU_MEMORY_EXT_SRC_ROM);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_FPR, 0x1|EVMU_SFR_FPR_UNLOCK_MASK);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write at state 1
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_0));
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write at state 2
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_1));
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write when done but still unlocked
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_2));
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write when done + unlocked but invalid start address
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_2));
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_FPR, 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    // Write successfully for 128 bytes
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_FPR, 0x1|EVMU_SFR_FPR_UNLOCK_MASK);
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_2));
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_FPR, 0x1);
    for(GblSize b = 0; b < 128; ++b) {
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, b);
        EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, b);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));

        GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+b), b);
    }

    // Ensure 129th write FAILS
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_TRL, 129);
    EvmuMemory_writeInt(pFixture->pMemory, EVMU_ADDRESS_SFR_ACC, 129);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuMemory_readFlash(pFixture->pMemory, 0x1ab00+129), 0x76);

    GBL_TEST_CASE_END;
}

GBL_TEST_REGISTER(nop,
                  ld,
                  ldInd,
                  st,
                  stInd,
                  mov,
                  movInd,
                  push,
                  pop,
                  br,
                  brf,
                  jmp,
                  jmpf,
                  call,
                  callr,
                  callf,
                  ret,
                  bei,
                  be,
                  beInd,
                  bnei,
                  bne,
                  bneInd,
                  bp,
                  bpc,
                  bn,
                  bz,
                  dbnz,
                  dbnzInd,
                  addi,
                  add,
                  addInd,
                  addci,
                  addc,
                  addcInd,
                  subi,
                  sub,
                  subInd,
                  subci,
                  subc,
                  subcInd,
                  mul,
                  div,
                  andi,
                  and,
                  andInd,
                  ori,
                  or,
                  orInd,
                  xori,
                  xor,
                  xorInd,
                  rol,
                  rolc,
                  ror,
                  rorc,
                  inc,
                  incInd,
                  dec,
                  decInd,
                  xch,
                  xchInd,
                  clr1,
                  set1,
                  not1,
                  ldc,
                  reti,
                  ldf,
                  stf);

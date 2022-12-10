#include "evmu_cpu_test_suite.h"
#include <gimbal/test/gimbal_test_macros.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_address_space.h>

#define EVMU_CPU_TEST_SUITE_(instance)  ((EvmuCpuTestSuite_*)GBL_INSTANCE_PRIVATE(instance, EVMU_CPU_TEST_SUITE_TYPE))

GBL_DECLARE_STRUCT(EvmuCpuTestSuite_) {
    EvmuDevice* pDevice;
    EvmuCpu*    pCpu;
    EvmuMemory* pMemory;
};

GBL_RESULT EvmuCpuTestSuite_init_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);
    pSelf_->pDevice = GBL_OBJECT_NEW(EvmuDevice);
    pSelf_->pCpu = pSelf_->pDevice->pCpu;
    pSelf_->pMemory = pSelf_->pDevice->pMemory;

    EvmuMemory_writeInt(pSelf_->pDevice->pMemory, EVMU_ADDRESS_SFR_EXT, EVMU_SFR_EXT_MASK);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_final_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);
    GBL_BOX_UNREF(pSelf_->pDevice);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_nop_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_NOP,
                                        }));

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_ld_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuMemory_writeInt(pSelf_->pDevice->pMemory, 0x2, 27);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_LD,
                                            .operands = {
                                                .direct = 0x2
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pDevice->pMemory, EVMU_ADDRESS_SFR_ACC), 27);

    GBL_CTX_END();
}


GBL_RESULT EvmuCpuTestSuite_ldInd_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress ind = EvmuMemory_indirectAddress(pSelf_->pMemory, 1);

    EvmuMemory_writeInt(pSelf_->pMemory, ind, 0xab);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_LD_IND,
                                            .operands = {
                                                .indirect = 1
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pDevice->pMemory,
                                        EVMU_ADDRESS_SFR_ACC), 0xab);
    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_st_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuMemory_writeInt(pSelf_->pDevice->pMemory, EVMU_ADDRESS_SFR_ACC, 128);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ST,
                                            .operands = {
                                                .direct = 3
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pDevice->pMemory, 3), 128);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_stInd_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuMemory_writeInt(pSelf_->pDevice->pMemory, EVMU_ADDRESS_SFR_ACC, 129);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ST_IND,
                                            .operands = {
                                                .indirect = 2
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory,
                                        EvmuMemory_indirectAddress(pSelf_->pMemory, 2)), 129);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_mov_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_MOV,
                                            .operands = {
                                                .direct = 4,
                                                .immediate = 255
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pDevice->pMemory, 4), 255);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_movInd_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_MOV_IND,
                                            .operands = {
                                                .indirect  = 3,
                                                .immediate = 245
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory,
                                        EvmuMemory_indirectAddress(pSelf_->pMemory, 3)), 245);

    GBL_CTX_END();
}


GBL_RESULT EvmuCpuTestSuite_push_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_PUSH,
                                            .operands = {
                                                .direct = 3,
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 0), 128);
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 1);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_pop_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_POP,
                                            .operands = {
                                                .direct = 5,
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_viewInt(pSelf_->pDevice->pMemory, 5), 128);
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 0);

    GBL_CTX_END();
}

// TEST INVALID
GBL_RESULT EvmuCpuTestSuite_br_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BR,
                                            .operands = {
                                                .relative8 = 5
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc+5);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_brf_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BRF,
                                            .operands = {
                                                .relative16 = 0x10ab
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc+0x10ab-1);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_jmp_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_JMP,
                                            .operands = {
                                                .absolute = 0xabc
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), 0x1abc);

    GBL_CTX_END();
}


GBL_RESULT EvmuCpuTestSuite_jmpf_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_JMPF,
                                            .operands = {
                                                .absolute = 0xabc
                                            }
                                         }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), 0xabc);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_call_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuCpu_setPc(pSelf_->pDevice->pCpu, 0xbabe);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_CALL,
                                            .operands = {
                                                .absolute = 0xdead,
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 2);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 1), 0xbe);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 0), 0xba);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pDevice->pCpu), 0xbead);

    GBL_CTX_END();
}


GBL_RESULT EvmuCpuTestSuite_callr_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_CALLR,
                                            .operands = {
                                                .relative16 = 0x1f1
                                            }
                                        }));
    pc += 0x1f1-1;
    pc %= UINT16_MAX;

    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 4);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 1), 0xad);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 0), 0xbe);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pDevice->pCpu), pc);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_callf_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_CALLF,
                                            .operands = {
                                                .absolute = 0x00a
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 6);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 1), pc&0xff);
    GBL_TEST_COMPARE(EvmuMemory_viewStack(pSelf_->pDevice->pMemory, 0), (pc&0xff00)>>8);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pDevice->pCpu), 0x00a);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_ret_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);


    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 4);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 2);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pDevice->pCpu), 0xbead);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuMemory_stackDepth(pSelf_->pDevice->pMemory), 0);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pDevice->pCpu), 0xbabe);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bei_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 44);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), 0xbabe - 17);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 33);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), 0xbabe - 17);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_be_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pSelf_->pMemory, 0xad, 80);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 80);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 22);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 60);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 22);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_beInd_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);
    EvmuMemory_writeInt(pSelf_->pMemory,
                        EvmuMemory_indirectAddress(pSelf_->pMemory, 3), 77);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 77,
                                                .relative8 = -128
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc - 128);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 78,
                                                .relative8 = -128
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc - 128);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bnei_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuWord psw = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 43);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc - 17);
    GBL_TEST_VERIFY(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 44);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));

    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc - 17);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bne_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pSelf_->pMemory, 0xad, 80);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 79);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 22);
    GBL_TEST_VERIFY(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 80);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 22);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bneInd_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);
    EvmuMemory_writeInt(pSelf_->pMemory,
                        EvmuMemory_indirectAddress(pSelf_->pMemory, 3), 76);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 77,
                                                .relative8 = -128
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc - 128);
    GBL_TEST_VERIFY(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 76,
                                                .relative8 = -128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc - 128);
    GBL_TEST_VERIFY(!(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bp_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuMemory_writeInt(pSelf_->pMemory, 0x3, 0xf8);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BP,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BP,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 1,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bpc_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuMemory_writeInt(pSelf_->pMemory, 0x3, 0xff);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BPC,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, 0x3), 0x7f);


    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BPC,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, 0x3), 0x7f);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bn_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuMemory_writeInt(pSelf_->pMemory, 0x3, 0x7f);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BN,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 7,
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BN,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 6,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_bz_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 0x0);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BZ,
                                            .operands = {
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 0x1);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BZ,
                                            .operands = {
                                                .direct    = 0x3,
                                                .bit       = 6,
                                                .relative8 = 128
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 127);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_dbnz_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuMemory_writeInt(pSelf_->pMemory, 0x3, 0x2);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ,
                                            .operands = {
                                                .direct    = 0x3,
                                                .relative8 = 11
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 11);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ,
                                            .operands = {
                                                .direct    = 0x3,
                                                .relative8 = 11
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 11);

    GBL_CTX_END();
}

GBL_RESULT EvmuCpuTestSuite_dbnzInd_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuAddress pc = EvmuCpu_pc(pSelf_->pCpu);

    EvmuMemory_writeInt(pSelf_->pMemory,
                        EvmuMemory_indirectAddress(pSelf_->pMemory, 2), 0x2);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ_IND,
                                            .operands = {
                                                .indirect  = 2,
                                                .relative8 = 12
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 12);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ_IND,
                                            .operands = {
                                                .indirect  = 2,
                                                .relative8 = 12
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 12);

    GBL_CTX_END();
}


GBL_RESULT EvmuCpuTestSuite_addi_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuCpuTestSuite_* pSelf_ = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_ACC, 3);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ADDI,
                                            .operands = {
                                                .immediate = 253
                                            }
                                        }));



    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pSelf_->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_DBNZ_IND,
                                            .operands = {
                                                .indirect  = 2,
                                                .relative8 = 12
                                            }
                                        }));
//    GBL_TEST_COMPARE(EvmuCpu_pc(pSelf_->pCpu), pc + 12);

    GBL_CTX_END();
}

GBL_EXPORT GblType EvmuCpuTestSuite_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTestCase cases[] = {
        { "nop",     EvmuCpuTestSuite_nop_    },
        { "ld",      EvmuCpuTestSuite_ld_     },
        { "ldInd",   EvmuCpuTestSuite_ldInd_  },
        { "st",      EvmuCpuTestSuite_st_     },
        { "stInd",   EvmuCpuTestSuite_stInd_  },
        { "mov",     EvmuCpuTestSuite_mov_    },
        { "moveInd", EvmuCpuTestSuite_movInd_ },
        { "push",    EvmuCpuTestSuite_push_   },
        { "pop",     EvmuCpuTestSuite_pop_    },
        { "br",      EvmuCpuTestSuite_br_     },
        { "brf",     EvmuCpuTestSuite_brf_    },
        { "jmp",     EvmuCpuTestSuite_jmp_    },
        { "jmpf",    EvmuCpuTestSuite_jmpf_   },
        { "call",    EvmuCpuTestSuite_call_   },
        { "callr",   EvmuCpuTestSuite_callr_  },
        { "callf",   EvmuCpuTestSuite_callf_  },
        { "ret",     EvmuCpuTestSuite_ret_    },
        { "bei",     EvmuCpuTestSuite_bei_    },
        { "be",      EvmuCpuTestSuite_be_     },
        { "beInd",   EvmuCpuTestSuite_beInd_  },
        { "bnei",    EvmuCpuTestSuite_bnei_   },
        { "bne",     EvmuCpuTestSuite_bne_    },
        { "bneInd",  EvmuCpuTestSuite_bneInd_ },
        { "bp",      EvmuCpuTestSuite_bp_     },
        { "bpc",     EvmuCpuTestSuite_bpc_    },
        { "bn",      EvmuCpuTestSuite_bn_     },
        { "bz",      EvmuCpuTestSuite_bz_     },
        { "dbnz",    EvmuCpuTestSuite_dbnz_   },
        { "dbnzInd", EvmuCpuTestSuite_dbnzInd_},
        { NULL,      NULL                     }
    };

    const static GblTestSuiteClassVTable vTable = {
        .pFnSuiteInit   = EvmuCpuTestSuite_init_,
        .pFnSuiteFinal  = EvmuCpuTestSuite_final_,
        .pCases         = cases
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);

        type = GblTestSuite_register(GblQuark_internStringStatic("EvmuCpuTestSuite"),
                                     &vTable,
                                     sizeof(EvmuCpuTestSuite),
                                     sizeof(EvmuCpuTestSuite_),
                                     GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();

        GBL_CTX_END_BLOCK();
    }

    return type;
}

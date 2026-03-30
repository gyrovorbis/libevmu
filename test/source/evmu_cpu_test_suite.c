#include "evmu_cpu_test_suite.h"
#include <gimbal/algorithms/gimbal_hash.h>
#include <gimbal/containers/gimbal_ring_list.h>
#include <gimbal/test/gimbal_test_macros.h>
#include <sys/stat.h>
#include <unistd.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_address_space.h>
#include <evmu/hw/evmu_pic.h>
#include <evmu/hw/evmu_flash.h>
#include <evmu/hw/evmu_buzzer.h>
#include <evmu/hw/evmu_wave.h>
#include <evmu/fs/evmu_fs_utils.h>
#include <evmu/fs/evmu_vmi.h>
#include <evmu/fs/evmu_vms.h>
#include "../../legacy/include/evmu-core/gyro_vmu_flash.h"
#include "../../lib/source/hw/evmu_cpu_.h"
#include "../../lib/source/hw/evmu_device_.h"
#include "../../lib/source/hw/evmu_pic_.h"
#include "../../lib/source/hw/evmu_timers_.h"

#define EVMU_CPU_TEST_SUITE_(instance)  (GBL_PRIVATE(EvmuCpuTestSuite, instance))

#define GBL_SELF_TYPE EvmuCpuTestSuite

GBL_TEST_FIXTURE {
    EvmuDevice* pDevice;
    EvmuCpu*    pCpu;
    EvmuRam*    pRam;
    EvmuFlash*  pFlash;
};

GBL_TEST_INIT() {
    pFixture->pDevice = GBL_OBJECT_NEW(EvmuDevice);
    pFixture->pCpu    = pFixture->pDevice->pCpu;
    pFixture->pRam = pFixture->pDevice->pRam;
    pFixture->pFlash  = pFixture->pDevice->pFlash;

    EvmuRam_setProgramSrc(pFixture->pRam, EVMU_PROGRAM_SRC_FLASH_BANK_0);
    GBL_TEST_CASE_END;
}

GBL_TEST_FINAL() {
    GBL_UNREF(pFixture->pDevice);
    GBL_TEST_CASE_END;
}

static GBL_RESULT byteArrayUnref_(void* pValue, void* pClosure) {
    (void)pClosure;
    GblByteArray_unref((GblByteArray*)pValue);
    return GBL_RESULT_SUCCESS;
}

static void setNopInstruction_(EvmuCpu* pCpu) {
    EvmuCpu_* pCpu_ = EVMU_CPU_(pCpu);
    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_NOP;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);
}

static void stepBaseTimer_(EvmuDevice* pDevice, unsigned steps) {
    setNopInstruction_(pDevice->pCpu);

    for(unsigned i = 0; i < steps; ++i)
        EvmuTimers_update(pDevice->pTimers);
}

static void baseTimerDefaultHalfSecondQuartz_(GblTestFixture* pFixture);
static void baseTimerCycleClockModes_(GblTestFixture* pFixture);
static void baseTimerStopClearsCounter_(GblTestFixture* pFixture);
static void baseTimerT0PrescalerSource_(GblTestFixture* pFixture);

GBL_TEST_CASE(nop) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_NOP,
                                        }));
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ld) {
    EvmuRam_writeData(pFixture->pDevice->pRam, 0x2, 27);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_LD,
                                            .operands = {
                                                .direct = 0x2
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pDevice->pRam, EVMU_ADDRESS_SFR_ACC), 27);

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(ldInd) {
    const EvmuAddress ind = EvmuRam_indirectAddress(pFixture->pRam, 3);

    EvmuRam_writeData(pFixture->pRam, ind, 0xab);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_LD_IND,
                                            .operands = {
                                                .indirect = 3
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pDevice->pRam,
                                        EVMU_ADDRESS_SFR_ACC), 0xab);
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(st) {
    EvmuRam_writeData(pFixture->pDevice->pRam, EVMU_ADDRESS_SFR_ACC, 128);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ST,
                                            .operands = {
                                                .direct = 3
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pDevice->pRam, 3), 128);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(stInd) {
    EvmuRam_writeData(pFixture->pDevice->pRam, EVMU_ADDRESS_SFR_ACC, 129);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_ST_IND,
                                            .operands = {
                                                .indirect = 2
                                            }
                                        }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam,
                                        EvmuRam_indirectAddress(pFixture->pRam, 2)), 129);

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

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pDevice->pRam, 4), 255);

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

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam,
                                        EvmuRam_indirectAddress(pFixture->pRam, 3)), 245);

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

    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 0), 128);
    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 1);

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

    GBL_TEST_COMPARE(EvmuRam_viewData(pFixture->pDevice->pRam, 5), 128);
    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 0);

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

    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 2);
    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 1), 0xbe);
    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 0), 0xba);
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

    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 4);
    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 1), 0xad);
    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 0), 0xbe);
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

    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 6);
    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 1), pc&0xff);
    GBL_TEST_COMPARE(EvmuRam_viewStack(pFixture->pDevice->pRam, 0), (pc&0xff00)>>8);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0x00a);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ret) {
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 4);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 2);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0xbead);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_RET
                                        }));
    GBL_TEST_COMPARE(EvmuRam_stackDepth(pFixture->pDevice->pRam), 0);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), 0xbabe);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bei) {
    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 44);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), 0xbabe - 17);
    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 33);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), 0xbabe - 17);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(be) {
    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuRam_writeData(pFixture->pRam, 0xad, 80);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 80);

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
    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 60);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 22);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(beInd) {
    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);
    EvmuRam_writeData(pFixture->pRam,
                        EvmuRam_indirectAddress(pFixture->pRam, 3), 77);

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
    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BE_IND,
                                            .operands = {
                                                .indirect = 3,
                                                .immediate = 78,
                                                .relative8 = -128
                                            }
                                        }));
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 128);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bnei) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 43);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 17);
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 44);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNEI,
                                            .operands = {
                                                .immediate = 44,
                                                .relative8 = -17
                                            }
                                        }));

    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc - 17);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bne) {
    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);

    EvmuRam_writeData(pFixture->pRam, 0xad, 80);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 79);

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
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 80);

    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BNE,
                                            .operands = {
                                                .direct = 0xad,
                                                .relative8 = 22
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 22);
    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bneInd) {
    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw&~EVMU_SFR_PSW_CY_MASK);
    EvmuRam_writeData(pFixture->pRam,
                        EvmuRam_indirectAddress(pFixture->pRam, 3), 76);

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
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK);

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
    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW) & EVMU_SFR_PSW_CY_MASK));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bp) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuRam_writeData(pFixture->pRam, 0x3, 0xf8);
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

    EvmuRam_writeData(pFixture->pRam, 0x3, 0xff);
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
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x3), 0x7f);


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
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x3), 0x7f);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(bn) {
    const EvmuAddress pc = EvmuCpu_pc(pFixture->pCpu);

    EvmuRam_writeData(pFixture->pRam, 0x3, 0x7f);
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

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);
    GBL_CTX_VERIFY_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                        &(const EvmuDecodedInstruction) {
                                            .opcode = EVMU_OPCODE_BZ,
                                            .operands = {
                                                .relative8 = 127
                                            }
                                        }));
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pCpu), pc + 127);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x1);
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

    EvmuRam_writeData(pFixture->pRam, 0x3, 0x2);
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

    EvmuRam_writeData(pFixture->pRam,
                        EvmuRam_indirectAddress(pFixture->pRam, 2), 0x2);

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

    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    psw &= ~(EVMU_SFR_PSW_CY_MASK|EVMU_SFR_PSW_AC_MASK|EVMU_SFR_PSW_OV_MASK);
    GBL_TEST_CALL(EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, psw));

    GBL_CTX_END();
}

static GBL_RESULT testPswFlags_(GblTestSuite* pSelf, GblBool cy, GblBool ac, GblBool ov) {
    GBL_CTX_BEGIN(pSelf);
    EvmuCpuTestSuite_* pFixture = EVMU_CPU_TEST_SUITE_(pSelf);

    EvmuWord psw = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW);
    GBL_TEST_COMPARE(((psw & EVMU_SFR_PSW_CY_MASK) >> EVMU_SFR_PSW_CY_POS), cy);
    GBL_TEST_COMPARE(((psw & EVMU_SFR_PSW_AC_MASK) >> EVMU_SFR_PSW_AC_POS), ac);
    GBL_TEST_COMPARE(((psw & EVMU_SFR_PSW_OV_MASK) >> EVMU_SFR_PSW_OV_POS), ov);

    GBL_CTX_END();
}

GBL_TEST_CASE(addi) {
    // ACC initial value
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);

    // Add immediate with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0x13
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Add immediate with AC set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0xa
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Add immediate with AC and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Add immediate with CY and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDI,
                                      .operands = {
                                          .immediate = 0x80
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(add) {
    // ACC initial value
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, 0x68, 0x13);

    // Add direct with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Add direct with AC set
    EvmuRam_writeData(pFixture->pRam, 0x69, 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x69
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Add direct with AC and OV set
    EvmuRam_writeData(pFixture->pRam, 0x70, 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x70
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Add direct with CY and OV set
    EvmuRam_writeData(pFixture->pRam, 0x71, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD,
                                      .operands = {
                                          .direct = 0x71
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(addInd) {
    // ACC initial value
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x13);

    // Add indirect with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Add indirect with AC set
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Add indirect with AC and OV set
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Add indirect with CY and OV set
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADD_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(addci) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    // ACC initial value
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);

    // Addc immediate with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0x13
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Addc immediate with AC set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0xa
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Addc immediate with AC and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Addc immediate with CY and OV set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0x80
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));


    // Addc immediate with CY in
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDCI,
                                      .operands = {
                                          .immediate = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(addc) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    // ACC initial value
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, 0x68, 0x13);

    // Addc with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Addc with AC set
    EvmuRam_writeData(pFixture->pRam, 0x68, 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Addc with AC and OV set
    EvmuRam_writeData(pFixture->pRam, 0x68, 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Addc with CY and OV set
    EvmuRam_writeData(pFixture->pRam, 0x68, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    // addc 0x68: accum: 0x1, CY = 1 => mem[0x68] = 3
    EvmuRam_writeData(pFixture->pRam, 0x68, 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}


GBL_TEST_CASE(addcInd) {    // ACC initial value
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x13);

    // Addc indirect with no flags set
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x68);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    // Addc indirect with AC set
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x72);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Addc indirect with AC and OV set
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x81);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Addc indirect with CY and OV set
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));


    // Addc indirect with CY in
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ADDC_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subi) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0xc
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0x2
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBI,
                                      .operands = {
                                          .immediate = 0x95
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(sub) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, 0x68, 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuRam_writeData(pFixture->pRam, 0x68, 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuRam_writeData(pFixture->pRam, 0x68, 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuRam_writeData(pFixture->pRam, 0x68, 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subInd) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUB_IND,
                                      .operands = {
                                          .indirect = 1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subci) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0xc
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0x2
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBCI,
                                      .operands = {
                                          .immediate = 0x95
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subc) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, 0x68, 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuRam_writeData(pFixture->pRam, 0x68, 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuRam_writeData(pFixture->pRam, 0x68, 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuRam_writeData(pFixture->pRam, 0x68, 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC,
                                      .operands = {
                                          .direct = 0x68
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(subcInd) {
    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xc);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x49);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x68);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(clearPswFlags_(pSelf));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x80);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7e);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x95);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SUBC_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xe9);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(mul) {
    // Case 1
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, 0xc4);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x11);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_C,   0x23);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_B,   0x52);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_MUL
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x7d);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_C),   0x36);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_B),   0x5);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    // Case 2
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, 0xc4);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x7);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_C,   0x5);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_B,   0x10);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_MUL
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x70);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_C),   0x50);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_B),   0x00);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(div) {
    // Case 1
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, 0xc4);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x79);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_C,   0x5);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_B,   0x7);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DIV
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x11);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_C),   0x49);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_B),   0x6);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Case 2
    // Exact division clears OV when the divisor is non-zero.
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, 0xc0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_C,   0x10);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_B,   0x4);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DIV
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x0);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_C),   0x4);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_B),   0x0);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_FALSE));

    // Case 3
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_PSW, 0xc0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x7);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_C,   0x10);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_B,   0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DIV
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_C),   0x10);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_B),   0x00);

    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_TRUE, GBL_TRUE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(andi) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ANDI,
                                      .operands = {
                                          .immediate = 0x55
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ANDI,
                                      .operands = {
                                          .immediate = 0xaa
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(and) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0xff);
    EvmuRam_writeData(pFixture->pRam, 0x23, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x55);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xaa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(andInd) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0xff);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x55);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xaa);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_AND_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ori) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0x3
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0xc
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf);


    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0x30
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3f);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ORI,
                                      .operands = {
                                          .immediate = 0xc0
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(or) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuRam_writeData(pFixture->pRam, 0x23, 0x3);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xc);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0x30);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3f);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xc0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(orInd) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x3);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xc);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0x30);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3f);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xc0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_OR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xori) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf0
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XORI,
                                      .operands = {
                                          .immediate = 0xf0
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xor) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuRam_writeData(pFixture->pRam, 0x23, 0xf);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf0);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xorInd) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xf);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xf);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xf0);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XOR_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(rol) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x55);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROL
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xaa);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROL
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x55);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(rolc) {
    clearPswFlags_(pSelf);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x60);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xc0);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x80);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));


    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROLC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ror) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x1);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROR
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x80);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_ROR
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x40);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(rorc) {
    clearPswFlags_(pSelf);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x6);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x3);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x1);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x80);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_TRUE, GBL_FALSE, GBL_FALSE));


    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_RORC
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xc0);
    GBL_TEST_CALL(testPswFlags_(pSelf, GBL_FALSE, GBL_FALSE, GBL_FALSE));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(inc) {
    clearPswFlags_(pSelf);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x1);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0xf1);


    EvmuRam_writeData(pFixture->pRam, 0x23, 0xff);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(incInd) {
    clearPswFlags_(pSelf);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3), 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam,
                                        EvmuRam_indirectAddress(pFixture->pRam, 3)), 0x1);

    EvmuRam_writeData(pFixture->pRam,
                        EvmuRam_indirectAddress(pFixture->pRam, 3), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam,
                                        EvmuRam_indirectAddress(pFixture->pRam, 3)), 0xf1);


    EvmuRam_writeData(pFixture->pRam,
                        EvmuRam_indirectAddress(pFixture->pRam, 3), 0xff);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_INC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam,
                                        EvmuRam_indirectAddress(pFixture->pRam, 3)), 0x0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(dec) {
    clearPswFlags_(pSelf);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x1);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0xef);


    EvmuRam_writeData(pFixture->pRam, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(decInd) {
    clearPswFlags_(pSelf);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3), 0x2);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3)), 0x1);

    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3), 0xf0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3)), 0xef);


    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3), 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_DEC_IND,
                                      .operands = {
                                          .indirect = 0x3
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 3)), 0xff);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xch) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x33);
    EvmuRam_writeData(pFixture->pRam, 0x23, 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XCH,
                                      .operands = {
                                          .direct = 0x23
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x33);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(xchInd) {
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x33);
    EvmuRam_writeData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1), 0xff);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_XCH_IND,
                                      .operands = {
                                          .indirect = 0x1
                                      }
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0xff);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EvmuRam_indirectAddress(pFixture->pRam, 1)), 0x33);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clr1) {
    EvmuRam_writeData(pFixture->pRam, 0x23, 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_CLR1,
                                      .operands = {
                                          .bit = 0x0,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x0);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_CLR1,
                                      .operands = {
                                          .bit = 0x7,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(set1) {
    EvmuRam_writeData(pFixture->pRam, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SET1,
                                      .operands = {
                                          .bit = 0x0,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x1);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0x00);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_SET1,
                                      .operands = {
                                          .bit = 0x7,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x80);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(not1) {
    EvmuRam_writeData(pFixture->pRam, 0x23, 0x0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_NOT1,
                                      .operands = {
                                          .bit = 0x0,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x1);

    EvmuRam_writeData(pFixture->pRam, 0x23, 0x80);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_NOT1,
                                      .operands = {
                                          .bit = 0x7,
                                          .direct = 0x23
                                      }
                                  }));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, 0x23), 0x0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(ldc) {
    EvmuRam_setProgramSrc(pFixture->pRam, EVMU_PROGRAM_SRC_FLASH_BANK_0);
    EvmuRam_writeProgram(pFixture->pRam, 0x123, 0x77);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, 0x1);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, 0x23);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_LDC,
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x77);

    EvmuRam_setProgramSrc(pFixture->pRam, EVMU_PROGRAM_SRC_ROM);
    EvmuRam_writeProgram(pFixture->pRam, 0x123, 0x33);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, 0x1);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, 0x23);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_LDC,
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x33);

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
    EvmuFlash_writeByte(pFixture->pFlash, 0x1abcd, 0x89);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_FPR, 0x1);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, 0xab);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, 0xcd);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_LDF,
                                  }));

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC), 0x89);

    GBL_TEST_CASE_END;
}

static GBL_RESULT stfUnlockToState_(GblTestSuite* pSelf, EVMU_FLASH_PROGRAM_STATE state) {
    GBL_CTX_BEGIN(pSelf);
    EvmuCpuTestSuite_* pFixture = EVMU_CPU_TEST_SUITE_(pSelf);

    const EvmuWord oldTrh = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH);
    const EvmuWord oldTrl = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL);
    const EvmuWord oldAcc = EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC);

    if(state >= EVMU_FLASH_PROGRAM_STATE_0) {
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, (EVMU_FLASH_PROGRAM_STATE_0_ADDRESS & 0xff00)>>8);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, EVMU_FLASH_PROGRAM_STATE_0_ADDRESS & 0xff);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, EVMU_FLASH_PROGRAM_STATE_0_VALUE);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));
    }

    if(state >= EVMU_FLASH_PROGRAM_STATE_1) {
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, (EVMU_FLASH_PROGRAM_STATE_1_ADDRESS & 0xff00)>>8);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, EVMU_FLASH_PROGRAM_STATE_1_ADDRESS & 0xff);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, EVMU_FLASH_PROGRAM_STATE_1_VALUE);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));
    }

    if(state >= EVMU_FLASH_PROGRAM_STATE_2) {
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, (EVMU_FLASH_PROGRAM_STATE_2_ADDRESS & 0xff00)>>8);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, EVMU_FLASH_PROGRAM_STATE_2_ADDRESS & 0xff);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, EVMU_FLASH_PROGRAM_STATE_2_VALUE);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));
    }

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, oldTrh);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, oldTrl);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, oldAcc);

    GBL_CTX_END();
}

GBL_TEST_CASE(stf) {
    // Write initial value to flash (so we can check if we overrode it)
    EvmuFlash_writeByte(pFixture->pFlash, 0x1ab00+129, 0x76);

    // Configure flash address registers
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_FPR, 0x1);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRH, 0xab);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, 0xcd);

    // Write without being in system mode
    EvmuRam_setProgramSrc(pFixture->pRam, EVMU_PROGRAM_SRC_FLASH_BANK_0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 0x0);

    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write without unlocking
    EvmuRam_setProgramSrc(pFixture->pRam, EVMU_PROGRAM_SRC_FLASH_BANK_0);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write at state 0
    EvmuRam_setProgramSrc(pFixture->pRam, EVMU_PROGRAM_SRC_ROM);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_FPR, 0x1|EVMU_SFR_FPR_UNLOCK_MASK);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write at state 1
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_0));
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write at state 2
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_1));
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write when done but still unlocked
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_2));
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write when done + unlocked but invalid start address
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_2));
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_FPR, 0x1);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    // Write successfully for 128 bytes
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_FPR, 0x1|EVMU_SFR_FPR_UNLOCK_MASK);
    GBL_TEST_CALL(stfUnlockToState_(pSelf, EVMU_FLASH_PROGRAM_STATE_2));
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_FPR, 0x1);
    for(size_t b = 0; b < 128; ++b) {
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, b);
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, b);
        GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                      &(const EvmuDecodedInstruction) {
                                          .opcode = EVMU_OPCODE_STF,
                                      }));

        GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+b), b);
    }

    // Ensure 129th write FAILS
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_TRL, 129);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ACC, 129);
    GBL_TEST_CALL(EvmuCpu_execute(pFixture->pDevice->pCpu,
                                  &(const EvmuDecodedInstruction) {
                                      .opcode = EVMU_OPCODE_STF,
                                  }));
    GBL_TEST_COMPARE(EvmuFlash_readByte(pFixture->pFlash, 0x1ab00+129), 0x76);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(timerStartDelay16) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pFixture->pDevice);
    EvmuCpu_*    pCpu_    = EVMU_CPU_(pFixture->pCpu);

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_MOV;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_MOV);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0PRR, 0xff);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0LR,  0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0HR,  0x00);
    pDevice_->pTimers->timer0.base.tl = 0x00;
    pDevice_->pTimers->timer0.base.th = 0x00;
    EvmuRam_writeData(pFixture->pRam,
                      EVMU_ADDRESS_SFR_T0CNT,
                      EVMU_SFR_T0CNT_P0LONG_MASK |
                      EVMU_SFR_T0CNT_P0LRUN_MASK |
                      EVMU_SFR_T0CNT_P0HRUN_MASK);

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T0L), 0x01);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T0H), 0x00);

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_NOP;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T0L), 0x02);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T0H), 0x00);

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_MOV;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_MOV);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LR,  0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1HR,  0x00);
    pDevice_->pTimers->timer1.base.tl = 0x00;
    pDevice_->pTimers->timer1.base.th = 0x00;
    EvmuRam_writeData(pFixture->pRam,
                      EVMU_ADDRESS_SFR_T1CNT,
                      EVMU_SFR_T1CNT_T1LONG_MASK |
                      EVMU_SFR_T1CNT_T1LRUN_MASK |
                      EVMU_SFR_T1CNT_T1HRUN_MASK);

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L), 0x01);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H), 0x00);

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_NOP;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L), 0x02);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H), 0x00);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(timerCounterWriteStopped) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pFixture->pDevice);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0CNT, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT, 0x00);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0L, 0x12);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0H, 0x34);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L, 0x56);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H, 0x78);

    GBL_TEST_COMPARE(pDevice_->pTimers->timer0.base.tl, 0x12);
    GBL_TEST_COMPARE(pDevice_->pTimers->timer0.base.th, 0x34);
    GBL_TEST_COMPARE(pDevice_->pTimers->timer1.base.tl, 0x56);
    GBL_TEST_COMPARE(pDevice_->pTimers->timer1.base.th, 0x78);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T0L), 0x12);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T0H), 0x34);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L), 0x56);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H), 0x78);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clockTicksPerCycle) {
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR7_MASK |
                          EVMU_SFR_OCR_OCR5_MASK |
                          EVMU_SFR_OCR_OCR4_MASK |
                          EVMU_SFR_OCR_OCR1_MASK |
                          EVMU_SFR_OCR_OCR0_MASK),
                     EVMU_SFR_OCR_OCR7_MASK |
                     EVMU_SFR_OCR_OCR5_MASK |
                     EVMU_SFR_OCR_OCR1_MASK |
                     EVMU_SFR_OCR_OCR0_MASK);
    GBL_TEST_COMPARE(EvmuClock_systemTicksPerCycle(pFixture->pDevice->pClock),
                     EVMU_CLOCK_OSC_QUARTZ_TCYC_1_6);

    const struct {
        EvmuWord  ocr;
        EvmuTicks ticks;
    } cases[] = {
        { 0x00,                                      EVMU_CLOCK_OSC_RC_TCYC_1_12     },
        { EVMU_SFR_OCR_OCR7_MASK,                    EVMU_CLOCK_OSC_RC_TCYC_1_6      },
        { EVMU_SFR_OCR_OCR5_MASK,                    EVMU_CLOCK_OSC_QUARTZ_TCYC_1_12 },
        { EVMU_SFR_OCR_OCR5_MASK | EVMU_SFR_OCR_OCR7_MASK,
                                                    EVMU_CLOCK_OSC_QUARTZ_TCYC_1_6  },
        { EVMU_SFR_OCR_OCR4_MASK,                    EVMU_CLOCK_OSC_CF_TCYC_1_12     },
        { EVMU_SFR_OCR_OCR4_MASK | EVMU_SFR_OCR_OCR7_MASK,
                                                    EVMU_CLOCK_OSC_CF_TCYC_1_6      },
        { EVMU_SFR_OCR_OCR5_MASK | EVMU_SFR_OCR_OCR4_MASK,
                                                    EVMU_CLOCK_OSC_CF_TCYC_1_12     },
        { EVMU_SFR_OCR_OCR5_MASK | EVMU_SFR_OCR_OCR4_MASK |
          EVMU_SFR_OCR_OCR7_MASK,                    EVMU_CLOCK_OSC_CF_TCYC_1_6      }
    };

    for(size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        const EvmuTicks ticks = cases[i].ticks;
        EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR, cases[i].ocr);

        GBL_TEST_COMPARE(EvmuClock_systemTicksPerCycle(pFixture->pDevice->pClock),
                         ticks);
        GBL_TEST_COMPARE((EvmuTicks)(EvmuClock_systemSecsPerCycle(pFixture->pDevice->pClock) *
                                     1000000000.0 + 0.5),
                         ticks);
    }

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clockOcrReadbackHighBits) {
    const EvmuWord ocrControlBits =
        EVMU_SFR_OCR_OCR7_MASK | EVMU_SFR_OCR_OCR0_MASK;

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR, ocrControlBits);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR),
                     0xcd);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR7_MASK |
                          EVMU_SFR_OCR_OCR5_MASK |
                          EVMU_SFR_OCR_OCR4_MASK |
                          EVMU_SFR_OCR_OCR1_MASK |
                          EVMU_SFR_OCR_OCR0_MASK),
                     ocrControlBits);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clockApiReflectsAndUpdatesRegisters) {
    EvmuClock* pClock = pFixture->pDevice->pClock;
    EVMU_OSCILLATOR source = EVMU_OSCILLATOR_CF;
    EVMU_CLOCK_DIVIDER divider = EVMU_CLOCK_DIVIDER_1;
    GBL_RESULT result = GBL_RESULT_SUCCESS;

    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_QUARTZ));

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setOscillatorActive(pClock, EVMU_OSCILLATOR_RC, GBL_FALSE)));
    GBL_TEST_VERIFY(!EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_RC));
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setOscillatorActive(pClock, EVMU_OSCILLATOR_CF, GBL_FALSE)));
    GBL_TEST_VERIFY(!EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_CF));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR1_MASK | EVMU_SFR_OCR_OCR0_MASK),
                     EVMU_SFR_OCR_OCR1_MASK | EVMU_SFR_OCR_OCR0_MASK);

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setOscillatorActive(pClock, EVMU_OSCILLATOR_RC, GBL_TRUE)));
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setOscillatorActive(pClock, EVMU_OSCILLATOR_CF, GBL_TRUE)));
    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_RC));
    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_CF));
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR1_MASK | EVMU_SFR_OCR_OCR0_MASK),
                     0);

    GBL_TEST_EXPECT_ERROR();
    result = EvmuClock_setOscillatorActive(pClock, EVMU_OSCILLATOR_QUARTZ, GBL_FALSE);
    GBL_TEST_VERIFY(!GBL_RESULT_SUCCESS(result));
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_INVALID_OPERATION);
    GBL_CTX_CLEAR_LAST_RECORD();
    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_QUARTZ));

    GBL_TEST_COMPARE(EvmuClock_systemState(pClock), EVMU_CLOCK_SYSTEM_STATE_RUNNING);
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemState(pClock, EVMU_CLOCK_SYSTEM_STATE_HALT)));
    GBL_TEST_COMPARE(EvmuClock_systemState(pClock), EVMU_CLOCK_SYSTEM_STATE_HALT);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PCON),
                     EVMU_SFR_PCON_HALT_MASK);

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemState(pClock, EVMU_CLOCK_SYSTEM_STATE_HOLD)));
    GBL_TEST_COMPARE(EvmuClock_systemState(pClock), EVMU_CLOCK_SYSTEM_STATE_HOLD);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PCON),
                     EVMU_SFR_PCON_HOLD_MASK);
    GBL_TEST_VERIFY(!EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_QUARTZ));
    GBL_TEST_VERIFY(!EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_RC));
    GBL_TEST_VERIFY(!EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_CF));

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemState(pClock, EVMU_CLOCK_SYSTEM_STATE_RUNNING)));
    GBL_TEST_COMPARE(EvmuClock_systemState(pClock), EVMU_CLOCK_SYSTEM_STATE_RUNNING);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PCON), 0);
    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_QUARTZ));
    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_RC));
    GBL_TEST_VERIFY(EvmuClock_oscillatorActive(pClock, EVMU_OSCILLATOR_CF));

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemConfig(pClock,
                                                                 EVMU_OSCILLATOR_QUARTZ,
                                                                 EVMU_CLOCK_DIVIDER_6)));
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_systemConfig(pClock, &source, &divider)));
    GBL_TEST_COMPARE(source, EVMU_OSCILLATOR_QUARTZ);
    GBL_TEST_COMPARE(divider, EVMU_CLOCK_DIVIDER_6);

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemConfig(pClock,
                                                                 EVMU_OSCILLATOR_CF,
                                                                 EVMU_CLOCK_DIVIDER_12)));
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_systemConfig(pClock, &source, &divider)));
    GBL_TEST_COMPARE(source, EVMU_OSCILLATOR_CF);
    GBL_TEST_COMPARE(divider, EVMU_CLOCK_DIVIDER_12);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR7_MASK | EVMU_SFR_OCR_OCR5_MASK | EVMU_SFR_OCR_OCR4_MASK),
                     EVMU_SFR_OCR_OCR4_MASK);

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemConfig(pClock,
                                                                 EVMU_OSCILLATOR_RC,
                                                                 EVMU_CLOCK_DIVIDER_6)));
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_systemConfig(pClock, &source, &divider)));
    GBL_TEST_COMPARE(source, EVMU_OSCILLATOR_RC);
    GBL_TEST_COMPARE(divider, EVMU_CLOCK_DIVIDER_6);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR7_MASK | EVMU_SFR_OCR_OCR5_MASK | EVMU_SFR_OCR_OCR4_MASK),
                     EVMU_SFR_OCR_OCR7_MASK);

    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_setSystemConfig(pClock,
                                                                 EVMU_OSCILLATOR_QUARTZ,
                                                                 EVMU_CLOCK_DIVIDER_12)));
    GBL_TEST_VERIFY(GBL_RESULT_SUCCESS(EvmuClock_systemConfig(pClock, &source, &divider)));
    GBL_TEST_COMPARE(source, EVMU_OSCILLATOR_QUARTZ);
    GBL_TEST_COMPARE(divider, EVMU_CLOCK_DIVIDER_12);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR) &
                         (EVMU_SFR_OCR_OCR7_MASK | EVMU_SFR_OCR_OCR5_MASK | EVMU_SFR_OCR_OCR4_MASK),
                     EVMU_SFR_OCR_OCR5_MASK);

    GBL_TEST_EXPECT_ERROR();
    result = EvmuClock_setSystemConfig(pClock,
                                       EVMU_OSCILLATOR_CF,
                                       EVMU_CLOCK_DIVIDER_1);
    GBL_TEST_VERIFY(!GBL_RESULT_SUCCESS(result));
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_INVALID_ARG);
    GBL_CTX_CLEAR_LAST_RECORD();

    baseTimerDefaultHalfSecondQuartz_(pFixture);
    baseTimerCycleClockModes_(pFixture);
    baseTimerStopClearsCounter_(pFixture);
    baseTimerT0PrescalerSource_(pFixture);

    GBL_TEST_CASE_END;
}

static void baseTimerDefaultHalfSecondQuartz_(GblTestFixture* pFixture) {
    GBL_CTX_BEGIN(NULL);

    EvmuPic_* pPic_ = EVMU_PIC_(pFixture->pDevice->pPic);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    pPic_->intReq = 0;
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_OCR,
                      EVMU_SFR_OCR_OCR7_MASK |
                      EVMU_SFR_OCR_OCR5_MASK |
                      EVMU_SFR_OCR_OCR1_MASK |
                      EVMU_SFR_OCR_OCR0_MASK);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ISL, 0xc0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT0_REQ_EN_MASK);

    stepBaseTimer_(pFixture->pDevice, 2730);

    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     16380);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT0_SRC_MASK,
                     0);
    GBL_TEST_COMPARE(pPic_->intReq & (1u << EVMU_IRQ_EXT_INT3_TBASE), 0);

    stepBaseTimer_(pFixture->pDevice, 1);

    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     2);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT0_SRC_MASK,
                     EVMU_SFR_BTCR_INT0_SRC_MASK);
    GBL_TEST_COMPARE(pPic_->intReq & (1u << EVMU_IRQ_EXT_INT3_TBASE),
                     (EvmuIrqMask)(1u << EVMU_IRQ_EXT_INT3_TBASE));

    GBL_CTX_END_BLOCK();
}

static void baseTimerCycleClockModes_(GblTestFixture* pFixture) {
    GBL_CTX_BEGIN(NULL);

    EvmuPic_* pPic_ = EVMU_PIC_(pFixture->pDevice->pPic);
    const EvmuWord islCycleClock = 0xd0;
    const EvmuWord btcrInt1Fast2 =
        EVMU_SFR_BTCR_OP_CTRL_MASK |
        EVMU_SFR_BTCR_INT1_REQ_EN_MASK |
        EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK |
        0x20u;
    const EvmuWord btcrInt1Fast8 =
        EVMU_SFR_BTCR_OP_CTRL_MASK |
        EVMU_SFR_BTCR_INT1_REQ_EN_MASK |
        EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK |
        0x30u;

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    pPic_->intReq = 0;
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ISL, islCycleClock);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT1_REQ_EN_MASK);

    stepBaseTimer_(pFixture->pDevice, 31);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     30);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     31);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     32);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     EVMU_SFR_BTCR_INT1_SRC_MASK);
    GBL_TEST_COMPARE(pPic_->intReq & (1u << EVMU_IRQ_EXT_INT3_TBASE),
                     (EvmuIrqMask)(1u << EVMU_IRQ_EXT_INT3_TBASE));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    pPic_->intReq = 0;
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT1_REQ_EN_MASK |
                      EVMU_SFR_BTCR_INT1_CYCLE_CTRL_MASK);

    stepBaseTimer_(pFixture->pDevice, 127);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     128);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     EVMU_SFR_BTCR_INT1_SRC_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT0_REQ_EN_MASK |
                      EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK);
    pPic_->intReq = 0;

    stepBaseTimer_(pFixture->pDevice, 63);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT0_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT0_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     64);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT0_SRC_MASK,
                     EVMU_SFR_BTCR_INT0_SRC_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, btcrInt1Fast2);
    pPic_->intReq = 0;

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     2);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     EVMU_SFR_BTCR_INT1_SRC_MASK);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, btcrInt1Fast8);
    pPic_->intReq = 0;

    stepBaseTimer_(pFixture->pDevice, 7);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     8);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     EVMU_SFR_BTCR_INT1_SRC_MASK);

    GBL_CTX_END_BLOCK();
}

static void baseTimerStopClearsCounter_(GblTestFixture* pFixture) {
    GBL_CTX_BEGIN(NULL);

    const EvmuWord islCycleClock = 0xd0;

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ISL, islCycleClock);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT1_REQ_EN_MASK);

    stepBaseTimer_(pFixture->pDevice, 20);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     20);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     0);

    stepBaseTimer_(pFixture->pDevice, 40);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     0);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         (EVMU_SFR_BTCR_INT0_SRC_MASK | EVMU_SFR_BTCR_INT1_SRC_MASK),
                     0);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT1_REQ_EN_MASK);
    stepBaseTimer_(pFixture->pDevice, 33);

    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     32);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     EVMU_SFR_BTCR_INT1_SRC_MASK);

    GBL_CTX_END_BLOCK();
}

static void baseTimerT0PrescalerSource_(GblTestFixture* pFixture) {
    GBL_CTX_BEGIN(NULL);

    const EvmuWord islT0Prescaler = 0xf0;

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR, 0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0PRR, 0xfd);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_ISL, islT0Prescaler);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR,
                      EVMU_SFR_BTCR_OP_CTRL_MASK |
                      EVMU_SFR_BTCR_INT1_REQ_EN_MASK);

    stepBaseTimer_(pFixture->pDevice, 95);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     31);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     0);

    stepBaseTimer_(pFixture->pDevice, 1);
    GBL_TEST_COMPARE(EVMU_TIMERS_(pFixture->pDevice->pTimers)->baseTimer.counter,
                     32);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_BTCR) &
                         EVMU_SFR_BTCR_INT1_SRC_MASK,
                     EVMU_SFR_BTCR_INT1_SRC_MASK);

    GBL_CTX_END_BLOCK();
}

GBL_TEST_CASE(clockHoldStopsCpuAndTimers) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pFixture->pDevice);
    EvmuLcd* pLcd = pFixture->pDevice->pLcd;
    const EvmuPc startPc = EvmuCpu_pc(pFixture->pDevice->pCpu);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0PRR, 0xff);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0LR, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0HR, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T0CNT,
                      EVMU_SFR_T0CNT_P0LRUN_MASK |
                      EVMU_SFR_T0CNT_P0HRUN_MASK);

    pDevice_->pTimers->timer0.base.tl = 0x12;
    pDevice_->pTimers->timer0.base.th = 0x34;

    EvmuLcd_setPixel(pLcd, 0, 0, GBL_TRUE);
    EvmuLcd_setPixel(pLcd, 1, 0, GBL_FALSE);
    EvmuIBehavior_update(EVMU_IBEHAVIOR(pLcd),
                         EvmuLcd_refreshRateTicks(pLcd) * EVMU_LCD_SCREEN_REFRESH_DIVISOR);
    GBL_TEST_VERIFY(EvmuLcd_decoratedPixel(pLcd, 0, 0) !=
                    EvmuLcd_decoratedPixel(pLcd, 1, 0));

    pLcd->screenChanged = GBL_FALSE;
    EvmuClock_setSystemState(pFixture->pDevice->pClock,
                             EVMU_CLOCK_SYSTEM_STATE_HOLD);
    EvmuIBehavior_update(EVMU_IBEHAVIOR(pFixture->pDevice->pCpu),
                         EVMU_CLOCK_OSC_QUARTZ_TCYC_1_6 * 16);

    GBL_TEST_COMPARE(pLcd->screenChanged, GBL_TRUE);
    GBL_TEST_COMPARE(EvmuCpu_pc(pFixture->pDevice->pCpu), startPc);
    GBL_TEST_COMPARE(pDevice_->pTimers->timer0.base.tl, 0x12);
    GBL_TEST_COMPARE(pDevice_->pTimers->timer0.base.th, 0x34);
    GBL_TEST_COMPARE(EvmuLcd_pixel(pLcd, 0, 0), GBL_TRUE);
    GBL_TEST_COMPARE(EvmuLcd_pixel(pLcd, 1, 0), GBL_FALSE);
    GBL_TEST_COMPARE(EvmuLcd_decoratedPixel(pLcd, 0, 0), 255);
    GBL_TEST_COMPARE(EvmuLcd_decoratedPixel(pLcd, 1, 0), 255);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PCON),
                     EVMU_SFR_PCON_HOLD_MASK);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(clockHoldCancelsOnPort3Input) {
    EvmuGamepad* pGamepad = pFixture->pDevice->pGamepad;

    EvmuClock_setSystemState(pFixture->pDevice->pClock,
                             EVMU_CLOCK_SYSTEM_STATE_HOLD);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P3INT,
                      EVMU_SFR_P3INT_P32INT_MASK);
    pFixture->pDevice->pLcd->screenChanged = GBL_FALSE;

    pGamepad->a = GBL_TRUE;
    EvmuIBehavior_update(EVMU_IBEHAVIOR(pGamepad), 0);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_PCON),
                     0);
    GBL_TEST_COMPARE(pFixture->pDevice->pLcd->screenChanged, GBL_TRUE);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_P3INT) &
                         EVMU_SFR_P3INT_P31INT_MASK,
                     EVMU_SFR_P3INT_P31INT_MASK);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(waveTransitions) {
    EvmuWave wave = EVMU_WAVE_0_0;

    EvmuWave_reset(&wave);
    GBL_TEST_COMPARE(wave, EVMU_WAVE_X_X);
    GBL_TEST_COMPARE(EvmuWave_logicPrevious(&wave), EVMU_LOGIC_X);
    GBL_TEST_COMPARE(EvmuWave_logicCurrent(&wave), EVMU_LOGIC_X);
    GBL_TEST_VERIFY(EvmuWave_hasStayedUnknown(&wave));

    EvmuWave_update(&wave, EVMU_LOGIC_0);
    GBL_TEST_COMPARE(wave, EVMU_WAVE_X_0);
    GBL_TEST_COMPARE(EvmuWave_logicPrevious(&wave), EVMU_LOGIC_X);
    GBL_TEST_COMPARE(EvmuWave_logicCurrent(&wave), EVMU_LOGIC_0);
    GBL_TEST_VERIFY(EvmuWave_wasLogicUnknown(&wave));
    GBL_TEST_VERIFY(EvmuWave_isLogicLow(&wave));
    GBL_TEST_VERIFY(EvmuWave_hasChangedKnown(&wave));
    GBL_TEST_VERIFY(EvmuWave_hasChangedValid(&wave));

    EvmuWave_update(&wave, EVMU_LOGIC_1);
    GBL_TEST_COMPARE(wave, EVMU_WAVE_0_1);
    GBL_TEST_VERIFY(EvmuWave_hasChanged(&wave));
    GBL_TEST_VERIFY(EvmuWave_hasChangedEdge(&wave));
    GBL_TEST_VERIFY(EvmuWave_hasChangedEdgeRising(&wave));
    GBL_TEST_VERIFY(EvmuWave_wasLogicLow(&wave));
    GBL_TEST_VERIFY(EvmuWave_isLogicHigh(&wave));

    EvmuWave_update(&wave, EVMU_LOGIC_1);
    GBL_TEST_COMPARE(wave, EVMU_WAVE_1_1);
    GBL_TEST_VERIFY(EvmuWave_hasStayed(&wave));
    GBL_TEST_VERIFY(EvmuWave_hasStayedHigh(&wave));
    GBL_TEST_VERIFY(!EvmuWave_hasChanged(&wave));

    EvmuWave_update(&wave, EVMU_LOGIC_0);
    GBL_TEST_COMPARE(wave, EVMU_WAVE_1_0);
    GBL_TEST_VERIFY(EvmuWave_hasChangedEdge(&wave));
    GBL_TEST_VERIFY(EvmuWave_hasChangedEdgeFalling(&wave));

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(fileReadWithoutHeaderSpansBlocks) {
    enum {
        kIconCount    = 3,
        kPayloadBytes = 96
    };

    EvmuNewFileInfo info;
    unsigned char fileData[EVMU_FAT_BLOCK_SIZE * 4] = {0};
    unsigned char payload[kPayloadBytes];
    unsigned char readData[kPayloadBytes];
    EvmuVms* pVms = (EvmuVms*)fileData;

    const size_t headerBytes = sizeof(EvmuVms) + kIconCount * EVMU_VMS_ICON_BITMAP_SIZE;
    EvmuDirEntry* pEntry = NULL;

    GBL_TEST_VERIFY(headerBytes > EVMU_FAT_BLOCK_SIZE);

    pVms->iconCount = kIconCount;
    pVms->animSpeed = 1;
    pVms->eyecatchType = EVMU_VMS_EYECATCH_NONE;
    pVms->crc = 1;
    pVms->dataBytes = kPayloadBytes;

    memset(fileData + sizeof(EvmuVms), 0xa5, headerBytes - sizeof(EvmuVms));

    for(size_t i = 0; i < kPayloadBytes; ++i) {
        payload[i] = (unsigned char)(0x40 + i);
        fileData[headerBytes + i] = payload[i];
    }

    EvmuNewFileInfo_init(&info,
                         "HDRSPAN",
                         headerBytes + kPayloadBytes,
                         EVMU_FILE_TYPE_DATA,
                         EVMU_COPY_ALLOWED);

    pEntry = EvmuFileManager_alloc(pFixture->pDevice->pFileMgr, &info, fileData);
    GBL_TEST_VERIFY(pEntry);
    GBL_TEST_VERIFY(pEntry->firstBlock != 0);
    GBL_TEST_COMPARE(EvmuFileManager_visibleBytes(pFixture->pDevice->pFileMgr,
                                                  pEntry,
                                                  GBL_TRUE),
                     pEntry->fileSize * EVMU_FAT_BLOCK_SIZE);
    GBL_TEST_COMPARE(EvmuFileManager_visibleBytes(pFixture->pDevice->pFileMgr,
                                                  pEntry,
                                                  GBL_FALSE),
                     pEntry->fileSize * EVMU_FAT_BLOCK_SIZE - headerBytes);

    memset(readData, 0, sizeof(readData));
    GBL_TEST_COMPARE(EvmuFileManager_read(pFixture->pDevice->pFileMgr,
                                          pEntry,
                                          readData,
                                          sizeof(readData),
                                          0,
                                          GBL_FALSE),
                     sizeof(readData));
    GBL_TEST_COMPARE(memcmp(readData, payload, sizeof(payload)), 0);

    memset(readData, 0, sizeof(readData));
    GBL_TEST_COMPARE(EvmuFileManager_read(pFixture->pDevice->pFileMgr,
                                          pEntry,
                                          readData,
                                          16,
                                          7,
                                          GBL_FALSE),
                     16);
    GBL_TEST_COMPARE(memcmp(readData, payload + 7, 16), 0);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(dciExportRoundTripsDataFile) {
    unsigned char fileData[EVMU_FAT_BLOCK_SIZE] = {0};
    unsigned char importedData[EVMU_FAT_BLOCK_SIZE] = {0};
    EvmuNewFileInfo info;
    EvmuDirEntry* pEntry = NULL;
    EvmuDevice* pImportedDevice = NULL;
    EvmuDirEntry* pImportedEntry = NULL;
    VMU_LOAD_IMAGE_STATUS status = VMU_LOAD_IMAGE_SUCCESS;
    struct stat fileStat = {0};
    char pathTemplate[] = "/tmp/libevmu_dci_export_XXXXXX";
    int fd = -1;
    const size_t dataSize = sizeof(EvmuDirEntry) + sizeof(fileData);
    const size_t bytesToWrite = dataSize + (dataSize % 4? 4 - dataSize % 4 : 0);

    for(size_t i = 0; i < sizeof(fileData); ++i)
        fileData[i] = (unsigned char)(0x30 + i);

    EvmuNewFileInfo_init(&info,
                         "DCIEXP",
                         sizeof(fileData),
                         EVMU_FILE_TYPE_DATA,
                         EVMU_COPY_ALLOWED);

    pEntry = EvmuFileManager_alloc(pFixture->pDevice->pFileMgr, &info, fileData);
    GBL_TEST_VERIFY(pEntry);

    fd = mkstemp(pathTemplate);
    GBL_TEST_VERIFY(fd >= 0);
    if(fd >= 0)
        close(fd);
    unlink(pathTemplate);

    GBL_TEST_VERIFY(gyVmuFlashExportDci(pFixture->pDevice, pEntry, pathTemplate));
    GBL_TEST_VERIFY(stat(pathTemplate, &fileStat) == 0);
    GBL_TEST_COMPARE((size_t)fileStat.st_size, bytesToWrite);

    pImportedDevice = GBL_OBJECT_NEW(EvmuDevice);
    GBL_TEST_VERIFY(pImportedDevice);

    pImportedEntry = gyVmuFlashLoadImageDci(pImportedDevice, pathTemplate, &status);
    GBL_TEST_VERIFY(pImportedEntry);
    GBL_TEST_COMPARE(status, VMU_LOAD_IMAGE_SUCCESS);
    GBL_TEST_COMPARE(EvmuFileManager_read(pImportedDevice->pFileMgr,
                                          pImportedEntry,
                                          importedData,
                                          sizeof(importedData),
                                          0,
                                          GBL_TRUE),
                     sizeof(importedData));
    GBL_TEST_COMPARE(memcmp(importedData, fileData, sizeof(fileData)), 0);

    unlink(pathTemplate);
    if(pImportedDevice)
        GBL_UNREF(pImportedDevice);
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(vmsComputeCrcDoesNotMutate) {
    enum { kDataBytes = 32 };

    unsigned char fileBytes[sizeof(EvmuVms) + kDataBytes] = {0};
    unsigned char original[sizeof(fileBytes)];
    EvmuVms* pVms = (EvmuVms*)fileBytes;
    EvmuVms headerCopy;
    uint16_t expected = 0;

    pVms->iconCount = 0;
    pVms->animSpeed = 0;
    pVms->eyecatchType = EVMU_VMS_EYECATCH_NONE;
    pVms->crc = 0x1234;
    pVms->dataBytes = kDataBytes;

    for(size_t i = 0; i < kDataBytes; ++i)
        fileBytes[sizeof(EvmuVms) + i] = (unsigned char)(0x40 + i);

    memcpy(original, fileBytes, sizeof(fileBytes));

    headerCopy = *pVms;
    headerCopy.crc = 0;
    expected = gblHashCrc16BitPartial(&headerCopy, sizeof(headerCopy), &expected);
    expected = gblHashCrc16BitPartial(fileBytes + sizeof(EvmuVms), kDataBytes, &expected);

    GBL_TEST_COMPARE(EvmuVms_computeCrc(pVms), expected);
    GBL_TEST_COMPARE(memcmp(fileBytes, original, sizeof(fileBytes)), 0);
    GBL_TEST_COMPARE(pVms->crc, 0x1234);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(vmsGuessFileTypeRejectsImpossibleDataSize) {
    EvmuVms vms = {0};

    vms.iconCount = 0;
    vms.animSpeed = 0;
    vms.eyecatchType = EVMU_VMS_EYECATCH_NONE;
    vms.crc = 1;
    vms.dataBytes =
        EVMU_FAT_BLOCK_USERDATA_SIZE_DEFAULT * EVMU_FAT_BLOCK_SIZE + 1;

    GBL_TEST_COMPARE(EvmuVms_guessFileType(&vms), EVMU_FILE_TYPE_NONE);

    vms.dataBytes = 32;
    GBL_TEST_COMPARE(EvmuVms_guessFileType(&vms), EVMU_FILE_TYPE_DATA);

    vms.crc = 0;
    GBL_TEST_COMPARE(EvmuVms_guessFileType(&vms), EVMU_FILE_TYPE_GAME);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(vmiFromVmsFileRejectsTruncatedInput) {
    unsigned char shortBytes[64] = {0};
    unsigned char oneBlock[EVMU_FAT_BLOCK_SIZE] = {0};
    EvmuVms* pInvalidHeader = (EvmuVms*)oneBlock;
    EvmuVmi vmi;

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_VERIFY(!GBL_RESULT_SUCCESS(EvmuVmi_fromVmsFile(&vmi,
                                                            shortBytes,
                                                            sizeof(shortBytes))));
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), EVMU_RESULT_ERROR_INVALID_FILE);
    GBL_CTX_CLEAR_LAST_RECORD();

    pInvalidHeader->eyecatchType = EVMU_VMS_EYECATCH_COUNT;

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_VERIFY(!GBL_RESULT_SUCCESS(EvmuVmi_fromVmsFile(&vmi,
                                                            oneBlock,
                                                            sizeof(oneBlock))));
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), EVMU_RESULT_ERROR_INVALID_FILE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(vmsCreateIconsArgb4444AllocatesDestSize) {
    unsigned char fileBytes[sizeof(EvmuVms) + EVMU_VMS_ICON_BITMAP_SIZE] = {0};
    EvmuVms* pVms = (EvmuVms*)fileBytes;
    GblRingList* pIcons = NULL;
    GblByteArray* pIcon = NULL;

    pVms->iconCount = 1;
    pVms->animSpeed = 0;
    pVms->eyecatchType = EVMU_VMS_EYECATCH_NONE;
    pVms->palette[0] = 0xffff;

    pIcons = EvmuVms_createIconsArgb4444(pVms);
    GBL_TEST_VERIFY(pIcons);
    GBL_TEST_COMPARE(GblRingList_size(pIcons), 1);

    pIcon = (GblByteArray*)GblRingList_front(pIcons);
    GBL_TEST_VERIFY(pIcon);
    GBL_TEST_COMPARE(pIcon->size,
                     sizeof(uint16_t) *
                     EVMU_VMS_ICON_BITMAP_WIDTH *
                     EVMU_VMS_ICON_BITMAP_HEIGHT);

    GblRingList_unref(pIcons, byteArrayUnref_, NULL);
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(vmsCreateIconsArgb4444WritesSequentialPixels) {
    unsigned char fileBytes[sizeof(EvmuVms) + EVMU_VMS_ICON_BITMAP_SIZE] = {0};
    EvmuVms* pVms = (EvmuVms*)fileBytes;
    GblRingList* pIcons = NULL;
    GblByteArray* pIcon = NULL;
    uint16_t pixels[2] = {0};

    pVms->iconCount = 1;
    pVms->animSpeed = 0;
    pVms->eyecatchType = EVMU_VMS_EYECATCH_NONE;
    pVms->palette[1] = 0x1111;
    pVms->palette[2] = 0x2222;

    // First packed byte contains palette indices 1 then 2.
    fileBytes[sizeof(EvmuVms)] = 0x12;

    pIcons = EvmuVms_createIconsArgb4444(pVms);
    GBL_TEST_VERIFY(pIcons);

    pIcon = (GblByteArray*)GblRingList_front(pIcons);
    GBL_TEST_VERIFY(pIcon);
    GBL_CTX_VERIFY_CALL(GblByteArray_read(pIcon, 0, sizeof(pixels), pixels));

    GBL_TEST_COMPARE(pixels[0], 0x1111);
    GBL_TEST_COMPARE(pixels[1], 0x2222);

    GblRingList_unref(pIcons, byteArrayUnref_, NULL);
    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(lcdBlankWhenDisabled) {
    EvmuLcd* pLcd = pFixture->pDevice->pLcd;

    EvmuLcd_setPixel(pLcd, 0, 0, GBL_TRUE);
    EvmuLcd_setPixel(pLcd, 1, 0, GBL_FALSE);

    EvmuIBehavior_update(EVMU_IBEHAVIOR(pLcd),
                         EvmuLcd_refreshRateTicks(pLcd) * EVMU_LCD_SCREEN_REFRESH_DIVISOR);

    GBL_TEST_COMPARE(EvmuLcd_pixel(pLcd, 0, 0), GBL_TRUE);
    GBL_TEST_COMPARE(EvmuLcd_pixel(pLcd, 1, 0), GBL_FALSE);
    GBL_TEST_VERIFY(EvmuLcd_decoratedPixel(pLcd, 0, 0) !=
                    EvmuLcd_decoratedPixel(pLcd, 1, 0));

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_VCCR, 0x00);

    GBL_TEST_COMPARE(EvmuLcd_screenEnabled(pLcd), GBL_FALSE);
    GBL_TEST_COMPARE(EvmuLcd_pixel(pLcd, 0, 0), GBL_TRUE);
    GBL_TEST_COMPARE(EvmuLcd_pixel(pLcd, 1, 0), GBL_FALSE);
    GBL_TEST_COMPARE(EvmuLcd_decoratedPixel(pLcd, 0, 0), 255);
    GBL_TEST_COMPARE(EvmuLcd_decoratedPixel(pLcd, 1, 0), 255);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(buzzerMode1ComparatorLatchesPerCycle) {
    EvmuCpu_* pCpu_ = EVMU_CPU_(pFixture->pCpu);
    uint16_t period = 0;
    uint8_t invPulseLength = 0;

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_NOP;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P1, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P1DDR, EVMU_SFR_P1DDR_P17DDR_MASK);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P1FCR, EVMU_SFR_P1FCR_P17FCR_MASK);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LR, 0xf0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LC, 0xf1);
    EvmuRam_writeData(pFixture->pRam,
                      EVMU_ADDRESS_SFR_T1CNT,
                      EVMU_SFR_T1CNT_ELDT1C_MASK |
                      EVMU_SFR_T1CNT_T1LRUN_MASK);

    EvmuBuzzer_tone(pFixture->pDevice->pBuzzer, &period, &invPulseLength);
    GBL_TEST_COMPARE(period, 16);
    GBL_TEST_COMPARE(invPulseLength, 1);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LC, 0xf8);

    EvmuBuzzer_tone(pFixture->pDevice->pBuzzer, &period, &invPulseLength);
    GBL_TEST_COMPARE(period, 16);
    GBL_TEST_COMPARE(invPulseLength, 1);

    for(size_t i = 0; i < 32; ++i) {
        if(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
           EVMU_SFR_T1CNT_T1LOVF_MASK)
            break;

        EvmuTimers_update(pFixture->pDevice->pTimers);
    }

    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
                    EVMU_SFR_T1CNT_T1LOVF_MASK);

    EvmuBuzzer_tone(pFixture->pDevice->pBuzzer, &period, &invPulseLength);
    GBL_TEST_COMPARE(period, 16);
    GBL_TEST_COMPARE(invPulseLength, 8);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(buzzerMode1Eldt1cGatesComparatorUpdates) {
    EvmuCpu_* pCpu_ = EVMU_CPU_(pFixture->pCpu);
    uint16_t period = 0;
    uint8_t invPulseLength = 0;

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_NOP;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P1, 0x00);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P1DDR, EVMU_SFR_P1DDR_P17DDR_MASK);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_P1FCR, EVMU_SFR_P1FCR_P17FCR_MASK);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LR, 0xf0);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LC, 0xf1);
    EvmuRam_writeData(pFixture->pRam,
                      EVMU_ADDRESS_SFR_T1CNT,
                      EVMU_SFR_T1CNT_T1LRUN_MASK);
    EvmuBuzzer_tone(pFixture->pDevice->pBuzzer, &period, &invPulseLength);
    GBL_TEST_COMPARE(period, 16);
    GBL_TEST_COMPARE(invPulseLength, 1);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LC, 0xf8);

    for(size_t i = 0; i < 32; ++i) {
        if(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
           EVMU_SFR_T1CNT_T1LOVF_MASK)
            break;

        EvmuTimers_update(pFixture->pDevice->pTimers);
    }

    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
                    EVMU_SFR_T1CNT_T1LOVF_MASK);

    EvmuBuzzer_tone(pFixture->pDevice->pBuzzer, &period, &invPulseLength);
    GBL_TEST_COMPARE(period, 16);
    GBL_TEST_COMPARE(invPulseLength, 1);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT, 0x00);
    EvmuRam_writeData(pFixture->pRam,
                      EVMU_ADDRESS_SFR_T1CNT,
                      EVMU_SFR_T1CNT_T1LRUN_MASK);

    EvmuBuzzer_tone(pFixture->pDevice->pBuzzer, &period, &invPulseLength);
    GBL_TEST_COMPARE(period, 16);
    GBL_TEST_COMPARE(invPulseLength, 8);

    GBL_TEST_CASE_END;
}

GBL_TEST_CASE(timer1ReloadCascade16) {
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pFixture->pDevice);
    EvmuCpu_*    pCpu_    = EVMU_CPU_(pFixture->pCpu);

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_MOV;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_MOV);

    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1LR,  0xfe);
    EvmuRam_writeData(pFixture->pRam, EVMU_ADDRESS_SFR_T1HR,  0xfe);
    pDevice_->pTimers->timer1.base.tl = 0xfe;
    pDevice_->pTimers->timer1.base.th = 0xfe;
    EvmuRam_writeData(pFixture->pRam,
                      EVMU_ADDRESS_SFR_T1CNT,
                      EVMU_SFR_T1CNT_T1LONG_MASK |
                      EVMU_SFR_T1CNT_T1LRUN_MASK |
                      EVMU_SFR_T1CNT_T1HRUN_MASK);
    EVMU_DEVICE_(pFixture->pDevice)->pTimers->timer1.startDelayCycles = 0;

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L), 0xfe);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H), 0xff);
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
                    EVMU_SFR_T1CNT_T1LOVF_MASK);
    GBL_TEST_VERIFY(!(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
                      EVMU_SFR_T1CNT_T1HOVF_MASK));

    pCpu_->curInstr.encoded.bytes[EVMU_INSTRUCTION_BYTE_OPCODE] = EVMU_OPCODE_NOP;
    pCpu_->curInstr.pFormat = EvmuIsa_format(EVMU_OPCODE_NOP);

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L), 0xff);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H), 0xff);

    EvmuTimers_update(pFixture->pDevice->pTimers);

    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1L), 0xfe);
    GBL_TEST_COMPARE(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1H), 0xfe);
    GBL_TEST_VERIFY(EvmuRam_readData(pFixture->pRam, EVMU_ADDRESS_SFR_T1CNT) &
                    EVMU_SFR_T1CNT_T1HOVF_MASK);

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
                  stf,
                  timerStartDelay16,
                  timerCounterWriteStopped,
                  clockTicksPerCycle,
                  clockApiReflectsAndUpdatesRegisters,
                  waveTransitions,
                  fileReadWithoutHeaderSpansBlocks,
                  dciExportRoundTripsDataFile,
                  lcdBlankWhenDisabled,
                  buzzerMode1ComparatorLatchesPerCycle,
                  buzzerMode1Eldt1cGatesComparatorUpdates,
                  timer1ReloadCascade16);

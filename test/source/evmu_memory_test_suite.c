#include "evmu_memory_test_suite.h"
#include <gimbal/test/gimbal_test_macros.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_address_space.h>
#include <evmu/hw/evmu_wram.h>

#define EVMU_RAM_TEST_SUITE_(instance)  ((EvmuRamTestSuite_*)GBL_INSTANCE_PRIVATE(instance, EVMU_RAM_TEST_SUITE_TYPE))

GBL_DECLARE_STRUCT(EvmuRamTestSuite_) {
    EvmuDevice* pDevice;
    EvmuRam*    pRam;
    EvmuWram*   pWram;
};

GBL_RESULT EvmuRamTestSuite_init_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    pSelf_->pDevice = GBL_OBJECT_NEW(EvmuDevice);
    pSelf_->pRam = pSelf_->pDevice->pRam;
    pSelf_->pWram   = pSelf_->pDevice->pWram;

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_final_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    GBL_BOX_UNREF(pSelf_->pDevice);

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_readIntInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, 0xffff), 0);
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_writeIntInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuRam_writeData(pSelf_->pRam, 0xffff, 0xff),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);

    GBL_CTX_CLEAR_LAST_RECORD();
    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_readProgramInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuRam_readProgram(pSelf_->pRam, EVMU_FLASH_SIZE), 0x0);
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_writeProgramInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuRam_writeData(pSelf_->pRam, EVMU_FLASH_SIZE, 0xff),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();
    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_ramBankChange_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    const EvmuWord psw = EvmuRam_viewData(pSelf_->pRam, EVMU_ADDRESS_SFR_PSW);

    EvmuRam_writeData(pSelf_->pRam, 0x0, 0x1);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_PSW, psw & ~EVMU_SFR_PSW_RAMBK0_MASK);
    GBL_TEST_COMPARE(EvmuRam_viewData(pSelf_->pRam, 0x0), 0);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_PSW, psw | EVMU_SFR_PSW_RAMBK0_MASK);
    GBL_TEST_COMPARE(EvmuRam_viewData(pSelf_->pRam, 0x0), 1);

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_extChange_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    EvmuRam_writeProgram(pSelf_->pRam, 0x1, 0x1);
    EvmuRam_setProgramSrc(pSelf_->pRam, EVMU_PROGRAM_SRC_ROM);
    GBL_TEST_COMPARE(EvmuRam_readProgram(pSelf_->pRam, 0x1), 0);
    EvmuRam_setProgramSrc(pSelf_->pRam, EVMU_PROGRAM_SRC_FLASH_BANK_0);
    GBL_TEST_COMPARE(EvmuRam_readProgram(pSelf_->pRam, 0x1), 1);

    GBL_CTX_END();
}
#if 0
static EVMU_RESULT setIrbk_(EvmuRam* pSelf, GblBool irbk0, GblBool irbk1) {
    EvmuWord psw = EvmuRam_readData(pSelf, EVMU_ADDRESS_SFR_PSW);

    psw &= ~(EVMU_SFR_PSW_IRBK0_MASK|EVMU_SFR_PSW_IRBK1_MASK);
    if(irbk0) psw |= EVMU_SFR_PSW_IRBK0_MASK;
    if(irbk1) psw |= EVMU_SFR_PSW_IRBK1_MASK;

    return EvmuRam_writeData(pSelf, EVMU_ADDRESS_SFR_PSW, psw);
}
GBL_RESULT EvmuRamTestSuite_indirectAddressInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    for(uint8_t addr = 0; addr < 0xf; ++addr) {
        EvmuRam_writeData(pSelf_->pRam, addr, addr);

    }

    GBL_CTX_VERIFY_CALL(setIrbk_(pSelf, GBL_FALSE, GBL_FALSE));
    GBL_TEST_COMPARE(EvmuRam_indirectAddress(pSelf, 0), 0x

    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}
#endif


GBL_RESULT EvmuRamTestSuite_wramReadInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_COMPARE(EvmuWram_readByte(pSelf_->pWram, 0xffff), 0);
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_wramRead_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    const EvmuWord vsel = EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VSEL);

    // Test with auto-increment + overflow
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VSEL, vsel|EVMU_SFR_VSEL_INCE_MASK);
    GBL_CTX_VERIFY_CALL(EvmuWram_writeByte(pSelf_->pWram, 0x1ff, 0xab));
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2, 0x1);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1, 0xff);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VTRBF), 0xab);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1), 0x0);

    // Test without auto increment
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VSEL, vsel&~EVMU_SFR_VSEL_INCE_MASK);
    GBL_CTX_VERIFY_CALL(EvmuWram_writeByte(pSelf_->pWram, 0x00, 0xac));
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VTRBF), 0xac);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1), 0x0);

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_wramWriteInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_COMPARE(EvmuWram_writeByte(pSelf_->pWram, 0xffff, 0),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_wramWrite_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);
    const EvmuWord vsel = EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VSEL);

    // Test with auto-increment + overflow
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VSEL, vsel|EVMU_SFR_VSEL_INCE_MASK);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2, 0x1);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1, 0xff);
    GBL_CTX_VERIFY_CALL(EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VTRBF, 0xcd));
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1), 0x0);
    GBL_TEST_COMPARE(EvmuWram_readByte(pSelf_->pWram, 0x1ff), 0xcd);

    // Test without auto-increment
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VSEL, vsel&~EVMU_SFR_VSEL_INCE_MASK);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2, 0x0);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1, 0x0);
    GBL_CTX_VERIFY_CALL(EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_VTRBF, 0xef));
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SFR_VRMAD1), 0x0);
    GBL_TEST_COMPARE(EvmuWram_readByte(pSelf_->pWram, 0x0), 0xef);

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_xramBankChangeInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_COMPARE(EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_XBNK, 3),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuRamTestSuite_xramBankChange_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuRamTestSuite_* pSelf_ = EVMU_RAM_TEST_SUITE_(pSelf);

    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_XBNK, 0);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SEGMENT_XRAM_BASE, 1);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_XBNK, 1);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SEGMENT_XRAM_BASE), 0);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SEGMENT_XRAM_BASE, 2);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_XBNK, 2);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SEGMENT_XRAM_BASE), 0);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SEGMENT_XRAM_BASE, 3);
    EvmuRam_writeData(pSelf_->pRam, EVMU_ADDRESS_SFR_XBNK, 0);
    GBL_TEST_COMPARE(EvmuRam_readData(pSelf_->pRam, EVMU_ADDRESS_SEGMENT_XRAM_BASE), 1);

    GBL_CTX_END();
}


GBL_EXPORT GblType EvmuRamTestSuite_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTestCase cases[] = {
        { "readIntInvalid",        EvmuRamTestSuite_readIntInvalid_        },
        { "writeIntInvalid",       EvmuRamTestSuite_writeIntInvalid_       },
        { "readProgramInvalid",    EvmuRamTestSuite_readProgramInvalid_    },
        { "writeProgramInvalid",   EvmuRamTestSuite_writeProgramInvalid_   },
        { "ramBankChange",         EvmuRamTestSuite_ramBankChange_         },
        { "extChanged",            EvmuRamTestSuite_extChange_             },
        { "wramReadInvalid",       EvmuRamTestSuite_wramReadInvalid_       },
        { "wramRead",              EvmuRamTestSuite_wramRead_              },
        { "wramWriteInvalid",      EvmuRamTestSuite_writeIntInvalid_       },
        { "wramWrite",             EvmuRamTestSuite_wramWrite_             },
        { "xramBankChangeInvalid", EvmuRamTestSuite_xramBankChangeInvalid_ },
        { "xramBankChange",        EvmuRamTestSuite_xramBankChange_        },
        { NULL,                    NULL                                       },
    };

    const static GblTestSuiteClassVTable vTable = {
        .pFnSuiteInit   = EvmuRamTestSuite_init_,
        .pFnSuiteFinal  = EvmuRamTestSuite_final_,
        .pCases         = cases
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);

        type = GblTestSuite_register(GblQuark_internStringStatic("EvmuRamTestSuite"),
                                     &vTable,
                                     sizeof(EvmuRamTestSuite),
                                     sizeof(EvmuRamTestSuite_),
                                     GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();

        GBL_CTX_END_BLOCK();
    }

    return type;
}

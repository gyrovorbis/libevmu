#include "evmu_memory_test_suite.h"
#include <gimbal/test/gimbal_test_macros.h>
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_isa.h>
#include <evmu/hw/evmu_sfr.h>
#include <evmu/hw/evmu_address_space.h>

#define EVMU_MEMORY_TEST_SUITE_(instance)  ((EvmuMemoryTestSuite_*)GBL_INSTANCE_PRIVATE(instance, EVMU_MEMORY_TEST_SUITE_TYPE))

GBL_DECLARE_STRUCT(EvmuMemoryTestSuite_) {
    EvmuDevice* pDevice;
    EvmuMemory* pMemory;
};

GBL_RESULT EvmuMemoryTestSuite_init_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    pSelf_->pDevice = GBL_OBJECT_NEW(EvmuDevice);
    pSelf_->pMemory = pSelf_->pDevice->pMemory;

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_final_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    GBL_BOX_UNREF(pSelf_->pDevice);

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_readIntInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, 0xffff), 0);
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_writeIntInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuMemory_writeInt(pSelf_->pMemory, 0xffff, 0xff),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);

    GBL_CTX_CLEAR_LAST_RECORD();
    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_readExtInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuMemory_readExt(pSelf_->pMemory, EVMU_FLASH_SIZE), 0x0);
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_writeExtInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    GBL_TEST_EXPECT_ERROR();

    GBL_TEST_COMPARE(EvmuMemory_writeInt(pSelf_->pMemory, EVMU_FLASH_SIZE, 0xff),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();
    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_ramBankChange_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    const EvmuWord psw = EvmuMemory_viewInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW);

    EvmuMemory_writeInt(pSelf_->pMemory, 0x0, 0x1);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw & ~EVMU_SFR_PSW_RAMBK0_MASK);
    GBL_TEST_COMPARE(EvmuMemory_viewInt(pSelf_->pMemory, 0x0), 0);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_PSW, psw | EVMU_SFR_PSW_RAMBK0_MASK);
    GBL_TEST_COMPARE(EvmuMemory_viewInt(pSelf_->pMemory, 0x0), 1);

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_extChange_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    EvmuMemory_writeExt(pSelf_->pMemory, 0x1, 0x1);
    EvmuMemory_setExtSource(pSelf_->pMemory, EVMU_MEMORY_EXT_SRC_ROM);
    GBL_TEST_COMPARE(EvmuMemory_readExt(pSelf_->pMemory, 0x1), 0);
    EvmuMemory_setExtSource(pSelf_->pMemory, EVMU_MEMORY_EXT_SRC_FLASH_BANK_0);
    GBL_TEST_COMPARE(EvmuMemory_readExt(pSelf_->pMemory, 0x1), 1);

    GBL_CTX_END();
}
#if 0
static EVMU_RESULT setIrbk_(EvmuMemory* pSelf, GblBool irbk0, GblBool irbk1) {
    EvmuWord psw = EvmuMemory_readInt(pSelf, EVMU_ADDRESS_SFR_PSW);

    psw &= ~(EVMU_SFR_PSW_IRBK0_MASK|EVMU_SFR_PSW_IRBK1_MASK);
    if(irbk0) psw |= EVMU_SFR_PSW_IRBK0_MASK;
    if(irbk1) psw |= EVMU_SFR_PSW_IRBK1_MASK;

    return EvmuMemory_writeInt(pSelf, EVMU_ADDRESS_SFR_PSW, psw);
}
GBL_RESULT EvmuMemoryTestSuite_indirectAddressInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    for(uint8_t addr = 0; addr < 0xf; ++addr) {
        EvmuMemory_writeInt(pSelf_->pMemory, addr, addr);

    }

    GBL_CTX_VERIFY_CALL(setIrbk_(pSelf, GBL_FALSE, GBL_FALSE));
    GBL_TEST_COMPARE(EvmuMemory_indirectAddress(pSelf, 0), 0x

    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}
#endif


GBL_RESULT EvmuMemoryTestSuite_wramReadInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_COMPARE(EvmuMemory_readWram(pSelf_->pMemory, 0xffff), 0);
    GBL_TEST_COMPARE(GBL_CTX_LAST_RESULT(), GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_wramRead_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    const EvmuWord vsel = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VSEL);

    // Test with auto-increment + overflow
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VSEL, vsel|EVMU_SFR_VSEL_INCE_MASK);
    GBL_CTX_VERIFY_CALL(EvmuMemory_writeWram(pSelf_->pMemory, 0x1ff, 0xab));
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2, 0x1);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1, 0xff);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VTRBF), 0xab);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1), 0x0);

    // Test without auto increment
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VSEL, vsel&~EVMU_SFR_VSEL_INCE_MASK);
    GBL_CTX_VERIFY_CALL(EvmuMemory_writeWram(pSelf_->pMemory, 0x00, 0xac));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VTRBF), 0xac);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1), 0x0);

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_wramWriteInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_COMPARE(EvmuMemory_writeWram(pSelf_->pMemory, 0xffff, 0),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_wramWrite_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);
    const EvmuWord vsel = EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VSEL);

    // Test with auto-increment + overflow
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VSEL, vsel|EVMU_SFR_VSEL_INCE_MASK);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2, 0x1);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1, 0xff);
    GBL_CTX_VERIFY_CALL(EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VTRBF, 0xcd));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1), 0x0);
    GBL_TEST_COMPARE(EvmuMemory_readWram(pSelf_->pMemory, 0x1ff), 0xcd);

    // Test without auto-increment
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VSEL, vsel&~EVMU_SFR_VSEL_INCE_MASK);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2, 0x0);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1, 0x0);
    GBL_CTX_VERIFY_CALL(EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VTRBF, 0xef));
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD2), 0xfe);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_VRMAD1), 0x0);
    GBL_TEST_COMPARE(EvmuMemory_readWram(pSelf_->pMemory, 0x0), 0xef);

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_xramBankChangeInvalid_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    GBL_TEST_EXPECT_ERROR();
    GBL_TEST_COMPARE(EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_XBNK, 3),
                     GBL_RESULT_ERROR_OUT_OF_RANGE);
    GBL_CTX_CLEAR_LAST_RECORD();

    GBL_CTX_END();
}

GBL_RESULT EvmuMemoryTestSuite_xramBankChange_(GblTestSuite* pSelf, GblContext* pCtx) {
    GBL_CTX_BEGIN(pCtx);

    EvmuMemoryTestSuite_* pSelf_ = EVMU_MEMORY_TEST_SUITE_(pSelf);

    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_XBNK, 0);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SEGMENT_XRAM_BASE, 1);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_XBNK, 1);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SEGMENT_XRAM_BASE), 0);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SEGMENT_XRAM_BASE, 2);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_XBNK, 2);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SEGMENT_XRAM_BASE), 0);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SEGMENT_XRAM_BASE, 3);
    EvmuMemory_writeInt(pSelf_->pMemory, EVMU_ADDRESS_SFR_XBNK, 0);
    GBL_TEST_COMPARE(EvmuMemory_readInt(pSelf_->pMemory, EVMU_ADDRESS_SEGMENT_XRAM_BASE), 1);

    GBL_CTX_END();
}


GBL_EXPORT GblType EvmuMemoryTestSuite_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTestCase cases[] = {
        { "readIntInvalid",        EvmuMemoryTestSuite_readIntInvalid_        },
        { "writeIntInvalid",       EvmuMemoryTestSuite_writeIntInvalid_       },
        { "readExtInvalid",        EvmuMemoryTestSuite_readExtInvalid_        },
        { "writeExtInvalid",       EvmuMemoryTestSuite_writeExtInvalid_       },
        { "ramBankChange",         EvmuMemoryTestSuite_ramBankChange_         },
        { "extChanged",            EvmuMemoryTestSuite_extChange_             },
        { "wramReadInvalid",       EvmuMemoryTestSuite_wramReadInvalid_       },
        { "wramRead",              EvmuMemoryTestSuite_wramRead_              },
        { "wramWriteInvalid",      EvmuMemoryTestSuite_writeIntInvalid_       },
        { "wramWrite",             EvmuMemoryTestSuite_wramWrite_             },
        { "xramBankChangeInvalid", EvmuMemoryTestSuite_xramBankChangeInvalid_ },
        { "xramBankChange",        EvmuMemoryTestSuite_xramBankChange_        },
        { NULL,                    NULL                                       },
    };

    const static GblTestSuiteClassVTable vTable = {
        .pFnSuiteInit   = EvmuMemoryTestSuite_init_,
        .pFnSuiteFinal  = EvmuMemoryTestSuite_final_,
        .pCases         = cases
    };

    if(type == GBL_INVALID_TYPE) {
        GBL_CTX_BEGIN(NULL);

        type = GblTestSuite_register(GblQuark_internStringStatic("EvmuMemoryTestSuite"),
                                     &vTable,
                                     sizeof(EvmuMemoryTestSuite),
                                     sizeof(EvmuMemoryTestSuite_),
                                     GBL_TYPE_FLAGS_NONE);
        GBL_CTX_VERIFY_LAST_RECORD();

        GBL_CTX_END_BLOCK();
    }

    return type;
}

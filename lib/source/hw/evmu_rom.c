#include "evmu_rom_.h"
#include "evmu_memory_.h"
#include "evmu_device_.h"
#include <gimbal/utils/gimbal_date_time.h>

EVMU_EXPORT GblBool EvmuRom_biosActive(const EvmuRom* pSelf) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    return pSelf_->pMemory->pExt == pSelf_->pMemory->rom;
}

EVMU_EXPORT EVMU_BIOS_TYPE EvmuRom_biosType(const EvmuRom* pSelf) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    return pSelf_->eBiosType;
}

EVMU_EXPORT EVMU_BIOS_MODE EvmuRom_biosMode(const EvmuRom* pSelf) {
    if(EvmuRom_biosType(pSelf) == EVMU_BIOS_TYPE_EMULATED)
        return EVMU_BIOS_MODE_UNKNOWN;

    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    return (EVMU_BIOS_MODE)pSelf_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MODE];

}

EVMU_EXPORT EVMU_RESULT EvmuRom_setDateTime(EvmuRom* pSelf, const GblDateTime* pDateTime) {
    GBL_CTX_BEGIN(NULL);

    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    EvmuMemory_* pMemory = pSelf_->pMemory;

    GBL_CTX_VERIFY_ARG(pDateTime);
    GBL_CTX_VERIFY(GblDateTime_isValid(pDateTime),
                   GBL_RESULT_ERROR_INVALID_DATE_TIME);

    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB_BCD]  = GBL_BCD_BYTE_PACK(pDateTime->date.year / 100);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB_BCD]  = GBL_BCD_BYTE_PACK(pDateTime->date.year % 100);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH_BCD]     = GBL_BCD_BYTE_PACK(pDateTime->date.month + 1);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY_BCD]       = GBL_BCD_BYTE_PACK(pDateTime->date.day);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR_BCD]      = GBL_BCD_BYTE_PACK(pDateTime->time.hours);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE_BCD]    = GBL_BCD_BYTE_PACK(pDateTime->time.minutes);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_SEC_BCD]       = GBL_BCD_BYTE_PACK(pDateTime->time.seconds);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB]      = pDateTime->date.year >> 8;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB]      = pDateTime->date.year & 0xff;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH]         = pDateTime->date.month;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY]           = pDateTime->date.day;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR]          = pDateTime->time.hours;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE]        = pDateTime->time.minutes;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_SEC]           = pDateTime->time.seconds;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HALF_SEC]      = 0;
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_LEAP_YEAR]     = GblDate_isLeapYear(pDateTime->date.year);
    pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DATE_SET]      = 0xff;

    GBL_CTX_END();
}

EVMU_EXPORT GblDateTime* EvmuRom_dateTime(const EvmuRom* pSelf, GblDateTime* pDateTime) {
    GBL_CTX_BEGIN(NULL);

    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    EvmuMemory_* pMemory = pSelf_->pMemory;

    GBL_CTX_VERIFY_ARG(pDateTime);

    pDateTime->date.year    = (pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_MSB] << 8)
                            | (pMemory->ram[0][EVMU_ADDRESS_SYSTEM_YEAR_LSB] & 0xff);
    pDateTime->date.month   = pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MONTH] + 1;
    pDateTime->date.day     = pMemory->ram[0][EVMU_ADDRESS_SYSTEM_DAY];
    pDateTime->time.hours   = pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HOUR];
    pDateTime->time.minutes = pMemory->ram[0][EVMU_ADDRESS_SYSTEM_MINUTE];
    pDateTime->time.seconds = pMemory->ram[0][EVMU_ADDRESS_SYSTEM_SEC];

    GBL_CTX_VERIFY(GblDateTime_normalize(pDateTime) != NULL,
                   GBL_RESULT_ERROR_INVALID_DATE_TIME);

    GBL_CTX_END_BLOCK();

    return GBL_RESULT_SUCCESS(GBL_CTX_RESULT())?
                pDateTime : NULL;
}


EVMU_EXPORT EVMU_RESULT EvmuRom_skipBiosSetup(EvmuRom* pSelf, GblBool enabled) {
    EVMU_LOG_VERBOSE("%s BIOS Setup Skip", enabled ? "Enabling" : "Disabling");
    EVMU_LOG_PUSH();

    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);

    switch(pSelf_->eBiosType){
        case EVMU_BIOS_TYPE_AMERICAN_IMAGE_V1_05:
        case EVMU_BIOS_TYPE_JAPANESE_IMAGE_V1_04:
            pSelf_->bSetupSkipEnabled = enabled;
            break;
        case EVMU_BIOS_TYPE_EMULATED:
        case EVMU_BIOS_TYPE_UNKNOWN_IMAGE:
            EVMU_LOG_WARNING("Setup Skip Unavailable due to Unknown/Emulated BIOS");
            break;
    }

    EVMU_LOG_POP(1);
    return GBL_RESULT_SUCCESS;
}

EVMU_EXPORT EVMU_RESULT EvmuRom_loadBios(EvmuRom* pSelf, const char* pPath) {
    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuRom, pFnLoadBios, pSelf, pPath);
    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuRom_unloadBios(EvmuRom* pSelf) {
    GBL_CTX_BEGIN(NULL);
    EVMU_LOG_VERBOSE("Unloading BIOS");
    EVMU_LOG_PUSH();

    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    memset(pSelf_->pMemory->rom, 0, sizeof(EvmuWord) * EVMU_ROM_SIZE);
    pSelf_->eBiosType = EVMU_BIOS_TYPE_EMULATED;

    EVMU_LOG_POP(1);
    GBL_CTX_END();
}

EVMU_EXPORT EvmuAddress EvmuRom_callBios(EvmuRom* pSelf, EvmuAddress entry) {
    EvmuAddress returnPc = 0;

    GBL_CTX_BEGIN(NULL);
    GBL_INSTANCE_VCALL(EvmuRom, pFnCallBios, pSelf, entry, &returnPc);
    GBL_CTX_END_BLOCK();

    return returnPc;
}

///\todo DEREEST ME!!!!
static void biosWriteFlashRom_(EvmuRom_* pSelf_) {
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(EVMU_ROM_PUBLIC_(pSelf_)));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);
    VMUDevice* dev = EVMU_DEVICE_REEST(pDevice);

    int i, a = ((pDevice_->pMemory->ram[1][0x7d]<<16)|(pDevice_->pMemory->ram[1][0x7e]<<8)|pDevice_->pMemory->ram[1][0x7f])&0x1ffff;
    VMUFlashDirEntry* pEntry = gyVmuFlashDirEntryGame(dev);
    if(!pEntry ||  a >= pEntry->fileSize * VMU_FLASH_BLOCK_SIZE)
        EvmuMemory_writeData(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory, 0x100, 0xff);
    else {
        EvmuMemory_writeData(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory, 0x100, 0x00);
        for(i=0; i<0x80; i++) {
            const uint16_t flashAddr = (a&~0xff)|((a+i)&0xff);
            pDevice_->pMemory->flash[flashAddr] = pDevice_->pMemory->ram[1][i+0x80];
            if(dev->pFnFlashChange)
                dev->pFnFlashChange(dev, flashAddr);
        }
    }
}

EVMU_EXPORT EVMU_RESULT EvmuRom_loadBios_(EvmuRom* pSelf, const char* path) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);

    FILE* file = NULL;

    EVMU_LOG_VERBOSE("Loading BIOS image from file [%s].", path);
    EVMU_LOG_PUSH();

    if (!(file = fopen(path, "rb"))) {
        EVMU_LOG_ERROR("Could not open file!");
        EVMU_LOG_POP(1);
        return 0;
    }

    //Clear ROM
    memset(pSelf_->pMemory->rom, 0, sizeof(pSelf_->pMemory->rom));

    size_t bytesRead   = 0;
    size_t bytesTotal  = 0;

    while(bytesTotal < sizeof(pSelf_->pMemory->rom)) {
        if((bytesRead = fread(pSelf_->pMemory->rom+bytesTotal, 1, sizeof(pSelf_->pMemory->rom)-bytesTotal, file))) {
            bytesTotal += bytesRead;
        } else break;
    }

    fclose(file);

    EVMU_LOG_VERBOSE("Read %d bytes.", bytesTotal);

    GBL_ASSERT(bytesTotal >= 0);

    const GblHash biosHash = gblHashCrc(pSelf_->pMemory->rom, EVMU_ROM_SIZE);
    switch (biosHash) {
        case EVMU_BIOS_TYPE_AMERICAN_IMAGE_V1_05:
            EVMU_LOG_VERBOSE("Detected American V1.05 BIOS");
            pSelf_->eBiosType = EVMU_BIOS_TYPE_AMERICAN_IMAGE_V1_05;
            break;
        case EVMU_BIOS_TYPE_JAPANESE_IMAGE_V1_04:
            EVMU_LOG_VERBOSE("Detected Japanese V1.04 BIOS");
            pSelf_->eBiosType = EVMU_BIOS_TYPE_JAPANESE_IMAGE_V1_04;
            break;
        default:
            EVMU_LOG_WARNING("Unknown BIOS CRC: 0x%X", biosHash);
            pSelf_->eBiosType = EVMU_BIOS_TYPE_UNKNOWN_IMAGE;
            break;
    }

    EVMU_LOG_POP(1);
    return GBL_RESULT_SUCCESS;
}

static EVMU_RESULT EvmuRom_callBios_(EvmuRom* pSelf, EvmuAddress pc, EvmuAddress* pRetPc) {
    GBL_CTX_BEGIN(NULL);

    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);

    switch(pc) {
    case EVMU_BIOS_SUBROUTINE_RESET:
        *pRetPc = 0x0;
        break;
    case EVMU_BIOS_SUBROUTINE_FM_WRT_EX: //fm_wrt_ex(ORG 0100H)
        biosWriteFlashRom_(pSelf_);
        *pRetPc = 0x105;
        break;
    case EVMU_BIOS_SUBROUTINE_FM_WRTA_EX: //fm_vrf_ex(ORG 0108H)
        biosWriteFlashRom_(pSelf_);
        *pRetPc = 0x10b;
        break;
    case EVMU_BIOS_SUBROUTINE_FM_VRF_EX: { //fm_vrf_ex(ORG 0110H)
        int i, a = ((pDevice_->pMemory->ram[1][0x7d]<<16)|(pDevice_->pMemory->ram[1][0x7e]<<8)|pDevice_->pMemory->ram[1][0x7f])&0x1ffff;
        int r = 0;
        for(i=0; i<0x80; i++)
            if((r = (pDevice_->pMemory->flash[(a&~0xff)|((a+i)&0xff)] ^ pDevice_->pMemory->ram[1][i+0x80])) != 0)
                break;
        EvmuMemory_writeData(pDevice->pMemory, 0x100, r);
        *pRetPc = 0x115;
        break;
    }
    case EVMU_BIOS_SUBROUTINE_FM_PRD_EX: { //fm_prd_ex(ORG 0120H)
        int i, a = ((pDevice_->pMemory->ram[1][0x7d]<<16)|(pDevice_->pMemory->ram[1][0x7e]<<8)|pDevice_->pMemory->ram[1][0x7f])&0x1ffff;
        for(i=0; i<0x80; i++) {
            pDevice_->pMemory->ram[1][i+0x80] = pDevice_->pMemory->flash[(a&~0xff)|((a+i)&0xff)];
        }
        *pRetPc = 0x125;
        break;
    }
    case EVMU_BIOS_SUBROUTINE_TIMER_EX: //timer_ex fm_prd_ex(ORG 0130H)
        if(!((pDevice_->pMemory->ram[0][EVMU_ADDRESS_SYSTEM_HALF_SEC]^=1)&1)) {
            GblDateTime curTime;
            EvmuRom_setDateTime(pSelf, GblDateTime_addSeconds(EvmuRom_dateTime(pSelf, &curTime), 1));
        }
        *pRetPc = 0x139;
        break;
    case EVMU_BIOS_SUBROUTINE_SLEEP_EX:
        EVMU_LOG_WARNING("Entered firmware at SLEEP mode address! Unimplemented!");
        *pRetPc = 0;
        break;
    default:
        EVMU_LOG_ERROR("Entering firmware at unknown address! [%x]", pc);
        *pRetPc =  0;
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuRom_GblObject_setProperty_(GblObject* pObject, const GblProperty* pProp, GblVariant* pValue) {
    GBL_UNUSED(pObject, pProp, pValue);
    GBL_CTX_BEGIN(NULL);

    switch(pProp->id) {
    default:
        GBL_CTX_RECORD_SET(GBL_RESULT_ERROR_INVALID_PROPERTY,
                           "Attempt to write unknown EvmuRom property: [%s]",
                           GblProperty_nameString(pProp));
        break;
    }
    GBL_CTX_END();
}


static GBL_RESULT EvmuRom_GblObject_property_(const GblObject* pObject, const GblProperty* pProp, GblVariant* pValue) {
    GBL_CTX_BEGIN(NULL);

    EvmuRom* pSelf = EVMU_ROM(pObject);

    switch(pProp->id) {
    case EvmuRom_Property_Id_biosActive:
        GblVariant_setBool(pValue, EvmuRom_biosActive(pSelf));
        break;
    case EvmuRom_Property_Id_biosType:
        GblVariant_setBool(pValue, EvmuRom_biosType(pSelf));
        break;
    case EvmuRom_Property_Id_biosMode:
        GblVariant_setBool(pValue, EvmuRom_biosMode(pSelf));
        break;
    case EvmuRom_Property_Id_dateTime: {
        GblDateTime dt;
        GblStringBuffer* pBuffer = GBL_STRING_BUFFER_ALLOCA(GBL_DATE_TIME_ISO8601_STRING_SIZE);
        GblVariant_setString(pValue, GblDateTime_toIso8601(EvmuRom_dateTime(pSelf, &dt), pBuffer));
        break;
    }
    default:
        GBL_CTX_RECORD_SET(GBL_RESULT_ERROR_INVALID_PROPERTY,
                           "Attempt to read unknown EvmuRom property: [%s]",
                           GblProperty_nameString(pProp));
        break;
    }

    GBL_CTX_END();
}

static GBL_RESULT EvmuRom_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_ROM_NAME);

    EvmuRom*  pRom   = EVMU_ROM(pSelf);
    EvmuRom_* pRom_  = EVMU_ROM_(pRom);
    pRom_->eBiosType = EVMU_BIOS_TYPE_EMULATED;
    pRom_->bSetupSkipEnabled = GBL_FALSE;

    GBL_CTX_END();
}

static GBL_RESULT EvmuRomClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_OBJECT_CLASS(pClass)->pFnConstructed = EvmuRom_GblObject_constructed_;
    GBL_OBJECT_CLASS(pClass)->pFnProperty    = EvmuRom_GblObject_property_;
    GBL_OBJECT_CLASS(pClass)->pFnSetProperty = EvmuRom_GblObject_setProperty_;
    EVMU_ROM_CLASS(pClass)  ->pFnCallBios    = EvmuRom_callBios_;
    EVMU_ROM_CLASS(pClass)  ->pFnLoadBios    = EvmuRom_loadBios_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuRom_type(void) {
    static GblType type = GBL_INVALID_TYPE;

    const static GblTypeInfo info = {
        .classSize              = sizeof(EvmuRomClass),
        .pFnClassInit           = EvmuRomClass_init_,
        .instanceSize           = sizeof(EvmuRom),
        .instancePrivateSize    = sizeof(EvmuRom_)
    };

    if(!GblType_verify(type)) {
        GBL_CTX_BEGIN(NULL);
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuRom"),
                                      EVMU_PERIPHERAL_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);
        GBL_CTX_VERIFY_LAST_RECORD();
        GBL_CTX_END_BLOCK();
    }

    return type;
}



/*// Call this whole fucker "Bios"
// Call the thing that MUXes Flash + Bios "Rom"


// use system clock?
 *
 *
 * TimeShooter.VMS is entering FM at unknown address!!!!! Marcus's emulator can't handle it either.
 *
 * WHOLE ADDRESS SPACE: BIOS
 * Standalone Utility Functions: Firmware/OS routines
 * Random routines mid-BIOS: system routines
 * All of this shit: subroutines
 *
 */

/* FLASH MEMORY VARIABLES USED WITH BIOS FW CALL
 *  THIS IS IN RAM BANK 1, IN APP-SPACE!!!
 *  0x7d Fmbank - Specify flash bank to use (guess bit 0 or 1)
 *  0x7e Fmadd_h - Flash memory address (upper 8 bits)
 *  0x7f Fmadd_l - Flash memory address (lower 8 bits)
 */

/*
Document all known static metadata regions in the BIOS
1) BIOS version
2) Maple information
3) Font characters

4) Known harness-able utility functions that can be used via stack return attacks

add a public API that allows you to query and extract this info.

Present at 0x14BE in the BIOS, alongside some build info.
0x14BE JAP BIOS version info
0xAA7 US BIOS version info

Visual Memory Produced By or Under License From SEGA ENTERPRISES,LTD.
Version 1.004,1998/09/30,315-6208-01,SEGA Visual Memory System BIOS Produced by Sue
*/

/* Need a list of BIOS initialization registers with default values!
 *
 */



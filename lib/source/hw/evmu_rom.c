#include "evmu_rom_.h"
#include "evmu_memory_.h"
#include "evmu_device_.h"
#include <gyro_system_api.h>
#include <gyro_file_api.h>

EVMU_EXPORT GblBool EvmuRom_biosLoaded(const EvmuRom* pSelf) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    return pSelf_->pMemory->rom[0]? GBL_TRUE : GBL_FALSE;
}

EVMU_EXPORT GblBool EvmuRom_biosActive(const EvmuRom* pSelf) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    return pSelf_->pMemory->pExt == pSelf_->pMemory->rom;
}

static inline int monthDays_(const EvmuRom_* pSelf_) {
    int m = pSelf_->pMemory->ram[0][0x19];
    if(m==2) {
        int y = pSelf_->pMemory->ram[0][0x18] | (pSelf_->pMemory->ram[0][0x17] << 8);
        if(y&3)
          return 28;
        if(!(y%4000))
          return 29;
        if(!(y%1000))
          return 28;
        if(!(y%400))
          return 29;
        if(!(y%100))
          return 28;
        return 29;
    } else return (m>7? ((m&1)? 30:31) : ((m&1)? 31:30));
}

EVMU_EXPORT EVMU_RESULT EvmuRom_loadBios(EvmuRom* pSelf, const char* path) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);

    GYFile* file;

    _gyLog(GY_DEBUG_VERBOSE, "Loading BIOS image from file [%s].", path);
    _gyPush();

    if (!gyFileOpen(path, "rb", &file)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open file!");
        _gyPop(0);
        return 0;
    }

    //Clear ROM
    memset(pSelf_->pMemory->rom, 0, sizeof(pSelf_->pMemory->rom));

    size_t bytesRead   = 0;
    size_t bytesTotal  = 0;

    while(bytesTotal < sizeof(pSelf_->pMemory->rom)) {
        if(gyFileRead(file, pSelf_->pMemory->rom+bytesTotal, sizeof(pSelf_->pMemory->rom)-bytesTotal, &bytesRead)) {
            bytesTotal += bytesRead;
        } else break;
    }

    gyFileClose(&file);

    _gyLog(GY_DEBUG_VERBOSE, "Read %d bytes.", bytesTotal);

    //assert(bytesRead <= 0);       //Didn't read shit
    assert(bytesTotal >= 0);

    _gyPop(0);
    return 1;

}


static void biosWriteFlashRom_(EvmuRom_* pSelf_) {
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(EVMU_ROM_PUBLIC_(pSelf_)));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);
    VMUDevice* dev = EVMU_DEVICE_REEST(pDevice);

    int i, a = ((pDevice_->pMemory->ram[1][0x7d]<<16)|(pDevice_->pMemory->ram[1][0x7e]<<8)|pDevice_->pMemory->ram[1][0x7f])&0x1ffff;
    VMUFlashDirEntry* pEntry = gyVmuFlashDirEntryGame(dev);
    if(!pEntry ||  a >= pEntry->fileSize * VMU_FLASH_BLOCK_SIZE)
        EvmuMemory_writeInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory, 0x100, 0xff);
    else {
        EvmuMemory_writeInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory, 0x100, 0x00);
        for(i=0; i<0x80; i++) {
            const uint16_t flashAddr = (a&~0xff)|((a+i)&0xff);
            pDevice_->pMemory->flash[flashAddr] = pDevice_->pMemory->ram[1][i+0x80];
            if(dev->pFnFlashChange)
                dev->pFnFlashChange(dev, flashAddr);
        }
    }
}

/*
 * TimeShooter.VMS is entering FM at unknown address!!!!! Marcus's emulator can't handle it either.
 */
EVMU_EXPORT EvmuAddress EvmuRom_callBios(EvmuRom* pSelf) {
    EvmuRom_* pSelf_ = EVMU_ROM_(pSelf);
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuDevice_* pDevice_ = EVMU_DEVICE_(pDevice);
    VMUDevice* dev = EVMU_DEVICE_REEST(pDevice);

    switch(EvmuCpu_pc(pDevice->pCpu)) {
    case EVMU_BIOS_ADDRESS_FM_WRT_EX: //fm_wrt_ex(ORG 0100H)
        biosWriteFlashRom_(pSelf_);
        return 0x105;
    case EVMU_BIOS_ADDRESS_FM_WRTA_EX: //fm_vrf_ex(ORG 0108H)
        biosWriteFlashRom_(pSelf_);
        return 0x10b;
    case EVMU_BIOS_ADDRESS_FM_VRF_EX: { //fm_vrf_ex(ORG 0110H)
        int i, a = ((pDevice_->pMemory->ram[1][0x7d]<<16)|(pDevice_->pMemory->ram[1][0x7e]<<8)|pDevice_->pMemory->ram[1][0x7f])&0x1ffff;
        int r = 0;
        for(i=0; i<0x80; i++)
        if((r = (pDevice_->pMemory->flash[(a&~0xff)|((a+i)&0xff)] ^ pDevice_->pMemory->ram[1][i+0x80])) != 0)
        break;
        //writemem(0x100, r);
        //printf("READ FLASH[%x] = %d\n", 0, r);
        EvmuMemory_writeInt(EVMU_DEVICE_PRISTINE_PUBLIC(dev)->pMemory, 0x100, r);
        return 0x115;
    }
    case EVMU_BIOS_ADDRESS_FM_PRD_EX: { //fm_prd_ex(ORG 0120H)
        int i, a = ((pDevice_->pMemory->ram[1][0x7d]<<16)|(pDevice_->pMemory->ram[1][0x7e]<<8)|pDevice_->pMemory->ram[1][0x7f])&0x1ffff;
        for(i=0; i<0x80; i++) {
        pDevice_->pMemory->ram[1][i+0x80] = pDevice_->pMemory->flash[(a&~0xff)|((a+i)&0xff)];
                //printf("READ FLASH[%x] = %d\n", (a&~0xff)|((a+i)&0xff), pDevice_->pMemory->ram[1][i+0x80]);
        }


        /*
        fprintf(stderr, "ROM read @ %05x\n", a);
        */
        return 0x125;
    }
    case EVMU_BIOS_ADDRESS_TIMER_EX: //timer_ex fm_prd_ex(ORG 0130H)
        if(!((pDevice_->pMemory->ram[0][0x1e]^=1)&1))
            if(++pDevice_->pMemory->ram[0][0x1d]>=60) {
                pDevice_->pMemory->ram[0][0x1d] = 0;
                if(++pDevice_->pMemory->ram[0][0x1c]>=60) {
                    pDevice_->pMemory->ram[0][0x1c] = 0;
                    if(++pDevice_->pMemory->ram[0][0x1b]>=24) {
                        pDevice_->pMemory->ram[0][0x1b] = 0;
                        if(++pDevice_->pMemory->ram[0][0x1a] > monthDays_(pSelf_)) {
                            pDevice_->pMemory->ram[0][0x1a] = 1;
                            if(++pDevice_->pMemory->ram[0][0x19]>=13) {
                                pDevice_->pMemory->ram[0][0x19] = 1;
                                if(pDevice_->pMemory->ram[0][0x18]==0xff) {
                                    pDevice_->pMemory->ram[0][0x18]=0;
                                    pDevice_->pMemory->ram[0][0x17]++;
                                } else
                                    pDevice_->pMemory->ram[0][0x18]++;
                            }
                        }
                    }
                }
            }
        return 0x139;
    case EVMU_BIOS_ADDRESS_SLEEP_EX: //fm_prd_ex(ORG 0140H)
        _gyLog(GY_DEBUG_WARNING, "Entered firmare at SLEEP mode address! Unimplemented!");
        return 0;
    default:
        //assert(0);
        _gyLog(GY_DEBUG_ERROR, "Entering firmware at unknown address! [%x]",
               EvmuCpu_pc(pDevice->pCpu));
        return 0;
    }
}

static GBL_RESULT EvmuRom_GblObject_constructed_(GblObject* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuPeripheral, base.pFnConstructed, pSelf);
    GblObject_setName(pSelf, EVMU_ROM_NAME);

    GBL_CTX_END();
}

static GBL_RESULT EvmuRom_IBehavior_reset_(EvmuIBehavior* pSelf) {
    GBL_CTX_BEGIN(NULL);

    GBL_INSTANCE_VCALL_DEFAULT(EvmuIBehavior, pFnReset, pSelf);

    EvmuRom*  pRom   = EVMU_ROM(pSelf);
    EvmuRom_* pRom_  = EVMU_ROM_(pRom);
    GBL_UNUSED(pRom_);

    GBL_CTX_END();
}

static GBL_RESULT EvmuRomClass_init_(GblClass* pClass, const void* pUd, GblContext* pCtx) {
    GBL_UNUSED(pUd);
    GBL_CTX_BEGIN(pCtx);

    GBL_OBJECT_CLASS(pClass)    ->pFnConstructed = EvmuRom_GblObject_constructed_;
    EVMU_IBEHAVIOR_CLASS(pClass)->pFnReset       = EvmuRom_IBehavior_reset_;

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




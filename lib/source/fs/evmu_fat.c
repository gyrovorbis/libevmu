#include <evmu/fs/evmu_fat.h>
#include <evmu/hw/evmu_device.h>
#include "../hw/evmu_ram_.h"
#include "evmu_fat_.h"
#include "../hw/evmu_flash_.h"

#include <gimbal/utils/gimbal_date_time.h>
#include <gimbal/preprocessor/gimbal_macro_utils.h>
#include <gimbal/strings/gimbal_string_buffer.h>

#include <stdlib.h>

EVMU_EXPORT EvmuRootBlock* EvmuFat_root(const EvmuFat* pSelf) {
    EvmuRootBlock* pRoot = NULL;

    pSelf->pClass->pFnRoot(pSelf, &pRoot);

    return pRoot;
}

EVMU_EXPORT EVMU_RESULT EvmuFat_info(const EvmuFat* pSelf, EvmuFatInfo* pInfo) {
    return pSelf->pClass->pFnInfo(pSelf, pInfo);
}

EVMU_EXPORT size_t EvmuFat_blockSize(const EvmuFat* pSelf) {
    EvmuFatInfo info = { 0 };

    EvmuFat_info(pSelf, &info);

    return info.blockSize;
}

EVMU_EXPORT EVMU_RESULT EvmuFat_format(const EvmuFat* pSelf, const EvmuRootBlock* pRoot) {
    const static EvmuRootBlock defaultRoot = {
        .volumeLabel.vmu.a = 255,
        .volumeLabel.vmu.r = 255,
        .volumeLabel.vmu.g = 255,
        .volumeLabel.vmu.b = 255,
        .totalSize         = 256,
        .partition         = 0,
        .rootBlock         = EVMU_FAT_BLOCK_ROOT,
        .fatBlock          = EVMU_FAT_BLOCK_FAT_DEFAULT,
        .fatSize           = EVMU_FAT_BLOCK_FAT_SIZE_DEFAULT,
        .dirBlock          = EVMU_FAT_BLOCK_DIRECTORY_DEFAULT,
        .dirSize           = EVMU_FAT_BLOCK_DIRECTORY_SIZE_DEFAULT,
        .extraBlock        = EVMU_FAT_BLOCK_EXTRA_DEFAULT,
        .extraSize         = EVMU_FAT_BLOCK_EXTRA_SIZE_DEFAULT
    };

    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Formatting Flash");
    EVMU_LOG_PUSH();

    EvmuDevice*  pDevice  = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));
    EvmuRam_* pRam_ = EVMU_RAM_(pDevice->pRam);

    if(!pRoot) pRoot = &defaultRoot;

    EVMU_LOG_DEBUG("Zeroing flash");
    memset(pRam_->pFlash->pStorage->pData, 0, pRoot->totalSize * EvmuFat_blockSize(pSelf));

    EVMU_LOG_DEBUG("Copying root block config");
    EvmuRootBlock* pDstRoot = EvmuFat_root(pSelf);
    GBL_CTX_VERIFY(pDstRoot,
                   EVMU_RESULT_ERROR_INVALID_BLOCK,
                   "Failed to retrieve root block: [%u]",
                   EVMU_FAT_BLOCK_ROOT);
    memcpy(pDstRoot, pRoot, sizeof(EvmuRootBlock));

    EVMU_LOG_DEBUG("Initializing FAT table");
    for(size_t b = 0; b < pDstRoot->totalSize; ++b) {
        GBL_CTX_VERIFY_CALL(EvmuFat_blockLink(pSelf, b, EVMU_FAT_BLOCK_FAT_UNALLOCATED));
    }

    EVMU_LOG_DEBUG("Writing formatted byte sequence");
    memset(pDstRoot->formatted, EVMU_FAT_ROOT_BLOCK_FORMATTED_BYTE, EVMU_FAT_ROOT_BLOCK_FORMATTED_SIZE);

    EVMU_LOG_DEBUG("Writing timestamp");
    GblDateTime dateTime;
    EvmuTimestamp_setDateTime(&pDstRoot->timestamp, GblDateTime_nowLocal(&dateTime));

    EVMU_LOG_DEBUG("Allocating FAT Table block");
    const EvmuBlock fatTableBlock = EvmuFat_blockTable(pSelf);
    GBL_CTX_VERIFY_CALL(EvmuFat_blockLink(pSelf, fatTableBlock, EVMU_FAT_BLOCK_FAT_LAST_IN_FILE));

    EVMU_LOG_DEBUG("Allocating Root block");
    GBL_CTX_VERIFY_CALL(EvmuFat_blockLink(pSelf, EvmuFat_root(pSelf)->rootBlock, EVMU_FAT_BLOCK_FAT_LAST_IN_FILE));

    EVMU_LOG_DEBUG("Allocating Directory blocks");
    const EvmuBlock dirLast = pDstRoot->dirBlock - pDstRoot->dirSize + 1;
    for(EvmuBlock b = pDstRoot->dirBlock;
        b >= dirLast;
        --b)
    {
        GBL_CTX_VERIFY_CALL(EvmuFat_blockLink(pSelf, b, b == dirLast? EVMU_FAT_BLOCK_FAT_LAST_IN_FILE : b-1));
    }

    GBL_CTX_END();
}

EVMU_EXPORT GblBool EvmuFat_isFormatted(const EvmuFat* pSelf) {
    const EvmuRootBlock* pRoot = EvmuFat_root(pSelf);

    for(size_t i = 0; i < EVMU_FAT_ROOT_BLOCK_FORMATTED_SIZE; ++i)
        if(pRoot->formatted[i] != EVMU_FAT_ROOT_BLOCK_FORMATTED_BYTE)
            return GBL_FALSE;

    return GBL_TRUE;
}

EVMU_EXPORT size_t EvmuFat_capacity(const EvmuFat* pSelf) {
    const EvmuRootBlock* pRoot = EvmuFat_root(pSelf);
    if(!pRoot) return 0;
    return pRoot->totalSize * EvmuFat_blockSize(pSelf);
}

EVMU_EXPORT size_t EvmuFat_toBlocks(const EvmuFat* pSelf, size_t bytes) {
    GBL_UNUSED(pSelf);

    const div_t result = div(bytes, EvmuFat_blockSize(pSelf));
    return result.quot + (result.rem? 1 : 0);
}

EVMU_EXPORT size_t EvmuFat_seqFreeBlocks(const EvmuFat* pSelf) {
    size_t contiguousBlocks = 0;
    size_t userDataBlocks   = EvmuFat_userBlocks(pSelf);

    for(size_t b = 0; b < userDataBlocks; ++b) {
        if(EvmuFat_blockNext(pSelf, b) != EVMU_FAT_BLOCK_FAT_UNALLOCATED)
            break;
        else
            ++contiguousBlocks;
    }

    return contiguousBlocks;
}

EVMU_EXPORT void EvmuFat_usage(const EvmuFat* pSelf, EvmuFatUsage* pUsage) {
    memset(pUsage, 0, sizeof(EvmuFatUsage));

    if(EvmuFat_isFormatted(pSelf)) {
        for(EvmuBlock b = 0; b < EvmuFat_userBlocks(pSelf); ++b) {
            const EvmuBlock fatEntry = EvmuFat_blockNext(pSelf, b);

            switch(fatEntry) {
            case EVMU_FAT_BLOCK_FAT_UNALLOCATED:
                ++pUsage->blocksFree;
                break;
            case EVMU_FAT_BLOCK_FAT_DAMAGED:
                ++pUsage->blocksDamaged;
                break;
            default:
                ++pUsage->blocksUsed;
            }
        }
#if 0
        const EvmuRootBlock* pRoot = EvmuFat_root(pSelf);
        pUsage->blocksHidden = pRoot->extraSize;
        GBL_ASSERT(pUsage->blocksDamaged +
                   pUsage->blocksFree    +
                   pUsage->blocksHidden  +
                   pUsage->blocksUsed    +
                   pRoot->dirSize        +
                   pRoot->fatSize        +
                   1 == pRoot->totalSize,
                   "Memory statistics totals make no sense!");
#endif
    }
}

EVMU_EXPORT size_t EvmuFat_userBlocks(const EvmuFat* pSelf) {
    return EvmuFat_root(pSelf)->extraBlock;
}

EVMU_EXPORT void EvmuFat_logRoot(const EvmuFat* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("FAT Root Block Information:");
    EVMU_LOG_PUSH();

    const EvmuRootBlock* pRoot   = EvmuFat_root(pSelf);
    GblStringBuffer      strBuff;
    GblDateTime          dt;

    GblStringBuffer_construct(&strBuff);

    GBL_CTX_VERIFY(pRoot,
                   EVMU_RESULT_ERROR_INVALID_BLOCK,
                   "Failed to retrieve root block!");

    EVMU_LOG_INFO("%-25s: %40s", "Formatted",       EvmuFat_isFormatted(pSelf)? "YES" : "NO");
    EVMU_LOG_INFO("%-25s: %40s", "Custom Color",    pRoot->volumeLabel.vmu.customColor? "YES" : "NO");
    EVMU_LOG_INFO("%-25s: %40s", "Color",           GblStringBuffer_printf(&strBuff, "<%u, %u, %u, %u>",
                                                           pRoot->volumeLabel.vmu.r, pRoot->volumeLabel.vmu.g,
                                                           pRoot->volumeLabel.vmu.b, pRoot->volumeLabel.vmu.a));
    EVMU_LOG_INFO("%-25s: %40s", "Format Date",     GblDateTime_toIso8601(
                                                           EvmuTimestamp_dateTime(&pRoot->timestamp, &dt),
                                                           &strBuff));
    EVMU_LOG_INFO("%-25s: %40u", "Total Blocks",    pRoot->totalSize);
    EVMU_LOG_INFO("%-25s: %40u", "Partition",       pRoot->partition);
    EVMU_LOG_INFO("%-25s: %40u", "Root Block",      pRoot->rootBlock);
    EVMU_LOG_INFO("%-25s: %40u", "FAT Block",       pRoot->fatBlock);
    EVMU_LOG_INFO("%-25s: %40u", "FAT Size",        pRoot->fatSize);
    EVMU_LOG_INFO("%-25s: %40u", "Directory Block", pRoot->dirBlock);
    EVMU_LOG_INFO("%-25s: %40u", "Directory Size",  pRoot->dirSize);
    EVMU_LOG_INFO("%-25s: %40u", "Extra Block",     pRoot->dirBlock);
    EVMU_LOG_INFO("%-25s: %40u", "Extra Size",      pRoot->dirSize);
    EVMU_LOG_INFO("%-25s: %40u", "Game Block",      pRoot->gameBlock);
    EVMU_LOG_INFO("%-25s: %40u", "Game Size",       pRoot->gameSize);
    EVMU_LOG_INFO("%-25s: %40u", "Icon Shape",      pRoot->iconShape);
    EVMU_LOG_INFO("%-25s: %40u", "Sort Flag",       pRoot->sortFlag);

    // Scan reserved range for any unknown values
    for(size_t b = 0; b < EVMU_FAT_ROOT_BLOCK_RESERVED_SIZE; ++b) {
        if(pRoot->reserved[b] != 0) {
            EVMU_LOG_WARN("Unknown Value: Root.reserved[%zu] = %x", b, pRoot->reserved[b]);
        }
    }

    // Scan second reserved range + remainder of block for any unknown values
    const uint8_t* pRootData = (uint8_t*)pRoot;
    for(size_t b = offsetof(EvmuRootBlock, reserved2); b < EvmuFat_blockSize(pSelf); ++b) {
        if(pRootData[b] != 0) {
            EVMU_LOG_WARN("Unknown Value: Root[%zu] = %x", b, pRootData[b]);
        }
    }

    GBL_CTX_END_BLOCK();

    GblStringBuffer_destruct(&strBuff);
}

EVMU_EXPORT void EvmuFat_logTable(const EvmuFat* pSelf) {
    EVMU_LOG_INFO("FAT Table Information");
    EVMU_LOG_PUSH();

    struct {
        GblStringBuffer strBuff;
        char            stackBuff[256];
    } stackStr;

    GblStringBuffer_construct(&stackStr.strBuff, GBL_STRV(""), sizeof(stackStr));

    for(size_t b = 0; b < EvmuFat_blockCount(pSelf); ++b) {
        size_t i;

        GblStringBuffer_clear(&stackStr.strBuff);

        for(i = 0; i < 5 && b + i < EvmuFat_blockCount(pSelf); ++i) {
            const EvmuBlock  entry = EvmuFat_blockNext(pSelf, b + i);
            const size_t     block  = b + i;

            switch(entry) {
            case EVMU_FAT_BLOCK_FAT_UNALLOCATED:
                GblStringBuffer_appendPrintf(&stackStr.strBuff, "[%03u] = %4s", block, "FREE");
                break;
            case EVMU_FAT_BLOCK_FAT_LAST_IN_FILE:
                GblStringBuffer_appendPrintf(&stackStr.strBuff, "[%03u] = %4s", block, "EOF");
                break;
            case EVMU_FAT_BLOCK_FAT_DAMAGED:
                GblStringBuffer_appendPrintf(&stackStr.strBuff, "[%03u] = %4s", block, "DMG");
                break;
            default:
                GblStringBuffer_appendPrintf(&stackStr.strBuff, "[%03u] = %4u", block, entry);
            }

            GblStringBuffer_appendPrintf(&stackStr.strBuff, " ");
        }
        b += i - 1;

        EVMU_LOG_INFO(GblStringBuffer_cString(&stackStr.strBuff));
    }

    GblStringBuffer_destruct(&stackStr.strBuff);

    EVMU_LOG_POP(1);
}

EVMU_EXPORT void EvmuFat_logDirectory(const EvmuFat* pSelf) {
    EVMU_LOG_INFO("Directory Information");
    EVMU_LOG_PUSH();

    const size_t dirEntries = EvmuFat_dirEntryCount(pSelf);
    for(size_t d = 0; d < dirEntries; ++d) {
        EvmuFat_dirEntryLog(pSelf, EvmuFat_dirEntry(pSelf, d));
    }

    EVMU_LOG_POP(1);
}

EVMU_EXPORT void EvmuFat_logMemoryUsage(const EvmuFat* pSelf) {
    EvmuFatUsage usage;

    EvmuFat_usage(pSelf, &usage);

    EVMU_LOG_INFO("Memory Usage Information");
    EVMU_LOG_PUSH();

    EVMU_LOG_INFO("%-20s: %40u", "Blocks Used",    usage.blocksUsed);
    EVMU_LOG_INFO("%-20s: %40u", "Blocks Free",    usage.blocksFree);
    EVMU_LOG_INFO("%-20s: %40u", "Blocks Hidden",  usage.blocksHidden);
    EVMU_LOG_INFO("%-20s: %40u", "Blocks Damaged", usage.blocksDamaged);

    EVMU_LOG_POP(1);
}

EVMU_EXPORT void EvmuFat_log(const EvmuFat* pSelf) {
    EVMU_LOG_INFO("Filesystem Info");
    EVMU_LOG_PUSH();

    EvmuFat_logRoot(pSelf);
    EvmuFat_logTable(pSelf);
    EvmuFat_logDirectory(pSelf);
    EvmuFat_logMemoryUsage(pSelf);

    EVMU_LOG_POP(1);
}

EVMU_EXPORT size_t EvmuFat_blockCount(const EvmuFat* pSelf) {
    return EvmuFat_root(pSelf)->totalSize;
}

EVMU_EXPORT EvmuBlock EvmuFat_blockTable(const EvmuFat* pSelf) {
    return EvmuFat_root(pSelf)->fatBlock;
}

EVMU_EXPORT EvmuBlock EvmuFat_blockDirectory(const EvmuFat* pSelf) {
    return EvmuFat_root(pSelf)->dirBlock;
}

EVMU_EXPORT const void* EvmuFat_blockData(const EvmuFat* pSelf, EvmuBlock block) {
    const void*  pData     = NULL;
    size_t       flashByte = block * EvmuFat_blockSize(pSelf);

    if(flashByte < EvmuFat_capacity(pSelf))
        pData = &EVMU_FLASH_(pSelf)->pStorage->pData[flashByte];

    return pData;
}

EVMU_EXPORT EvmuBlock EvmuFat_blockNext(const EvmuFat* pSelf, EvmuBlock block) {
    const EvmuBlock  tableBlock = EvmuFat_blockTable(pSelf);
    const EvmuBlock* pFatTable  = EvmuFat_blockData(pSelf, tableBlock);

    GBL_ASSERT(pFatTable);

    return pFatTable[block];
}

EVMU_EXPORT EVMU_RESULT EvmuFat_blockLink(const EvmuFat* pSelf, EvmuBlock block, EvmuBlock next) {
    GBL_CTX_BEGIN(NULL);

    GBL_CTX_VERIFY(block < EvmuFat_root(pSelf)->fatSize * EvmuFat_blockSize(pSelf) / sizeof(uint16_t),
                   EVMU_RESULT_ERROR_INVALID_BLOCK,
                   "Tried to link invalid block [%u] to %u.",
                   block, next);

    const EvmuBlock tableBlock = EvmuFat_blockTable(pSelf);
    EvmuBlock*      pFatTable  = EvmuFat_blockData(pSelf, tableBlock);

    GBL_CTX_VERIFY(pFatTable,
                   EVMU_RESULT_ERROR_INVALID_BLOCK,
                   "Could not fetch fat table from fat block: [%u]",
                   tableBlock);

    pFatTable[block] = next;

    GBL_CTX_END();
}

EVMU_EXPORT EVMU_RESULT EvmuFat_blockFree(const EvmuFat* pSelf, EvmuBlock block) {
    GBL_CTX_BEGIN(NULL);
    GBL_CTX_VERIFY_CALL(EvmuFat_blockLink(pSelf, block, EVMU_FAT_BLOCK_FAT_UNALLOCATED));
    GBL_CTX_END();
}

EVMU_EXPORT EvmuBlock EvmuFat_blockAlloc(const EvmuFat* pSelf, EvmuBlock prev, EVMU_FILE_TYPE type) {
    EvmuBlock block = EVMU_FAT_BLOCK_FAT_UNALLOCATED;
    GBL_CTX_BEGIN(NULL);

    EvmuBlock* pFatTable = EvmuFat_blockData(pSelf, EvmuFat_blockTable(pSelf));

    int firstBlock;
    int endBlock;
    int blockDelta;

    // Determine loop boundaries and direction based on file type
    if(type == EVMU_FILE_TYPE_GAME) {
        firstBlock  = 0;
        endBlock    = EvmuFat_userBlocks(pSelf);
        blockDelta  = 1;
    } else if(type == EVMU_FILE_TYPE_DATA) {
        firstBlock  = EvmuFat_userBlocks(pSelf) - 1;
        endBlock    = -1;
        blockDelta  = -1;
    } else GBL_CTX_VERIFY(GBL_FALSE,
                          GBL_RESULT_ERROR_INVALID_ARG,
                          "Invalid file type: [%u]",
                           type);

    //Iterate until unallocated block is found.
    for(EvmuBlock i = firstBlock; i != endBlock; i += blockDelta) {
        if(pFatTable[i] == EVMU_FAT_BLOCK_FAT_UNALLOCATED) {
            //Claim block
            block = i;
            /* Assume this is the last block in the sequence,
             * until it's passed to the alloc function later
             * as the previous block of a new allocation.
             */
            pFatTable[i] = EVMU_FAT_BLOCK_FAT_LAST_IN_FILE;
            //Zero out contents of block
            memset(EvmuFat_blockData(pSelf, block), 0, EvmuFat_blockSize(pSelf));
            //Update fat entry if not first block in series
            if(prev != EVMU_FAT_BLOCK_FAT_UNALLOCATED &&
               prev != EVMU_FAT_BLOCK_FAT_LAST_IN_FILE)
                pFatTable[prev] = block;
            break;
        }
    }

    GBL_CTX_END_BLOCK();
    return block;
}

EVMU_EXPORT size_t EvmuFat_dirEntryCount(const EvmuFat* pSelf) {
    return EvmuFat_root(pSelf)->dirSize *
           EvmuFat_blockSize(pSelf) /
           EVMU_DIRECTORY_ENTRY_SIZE;
}


EVMU_EXPORT EvmuDirEntry* EvmuFat_dirEntry(const EvmuFat* pSelf, size_t index) {
    const EvmuRootBlock*  pRoot    = EvmuFat_root(pSelf);
    const EvmuBlock       dirBlock = pRoot->dirBlock - pRoot->dirSize - 1;
    EvmuDirEntry*         pEntry   = EvmuFat_blockData(pSelf, dirBlock);

    return index < EvmuFat_dirEntryCount(pSelf)? &pEntry[index] : NULL;
}

EVMU_EXPORT EvmuDirEntry* EvmuFat_dirEntryAlloc(const EvmuFat* pSelf, EVMU_FILE_TYPE fileType) {
    for(uint16_t e = 0; e < EvmuFat_dirEntryCount(pSelf); ++e) {
        EvmuDirEntry* pEntry = EvmuFat_dirEntry(pSelf, e);
        if(pEntry && pEntry->fileType == EVMU_FILE_TYPE_NONE) {
            memset(pEntry, 0, sizeof(EvmuDirEntry));
            pEntry->fileType = fileType;
            return pEntry;
        }
    }

    return NULL;
}

EVMU_EXPORT void EvmuFat_dirEntryLog(const EvmuFat* pSelf, const EvmuDirEntry* pEntry) {
    GblStringBuffer strBuff;
    GblDateTime     dt;

    GblStringBuffer_construct(&strBuff);

    EVMU_LOG_INFO("Directory Entry[%u]: %s",
                  EvmuFat_dirEntryIndex(pSelf, pEntry),
                  EvmuDirEntry_name(pEntry, &strBuff));

    if(pEntry->fileType != EVMU_FILE_TYPE_NONE) {
        EVMU_LOG_PUSH();

        EVMU_LOG_INFO("%-25s: %40s", "Type",            EvmuDirEntry_fileTypeStr(pEntry));
        EVMU_LOG_INFO("%-25s: %40s", "Copy Protection", EvmuDirEntry_protectedStr(pEntry));
        EVMU_LOG_INFO("%-25s: %40s", "Creation Date",   GblDateTime_toIso8601(
                                                            EvmuTimestamp_dateTime(&pEntry->timestamp, &dt),
                                                            &strBuff));
        EVMU_LOG_INFO("%-25s, %40u", "First Block",     pEntry->firstBlock);
        EVMU_LOG_INFO("%-25s, %40u", "Size (Blocks)",   pEntry->fileSize);
        EVMU_LOG_INFO("%-25s, %40u", "Header Block",    pEntry->headerOffset);
        EVMU_LOG_INFO("%-25s, %40x", "Unused",          pEntry->unused[0]       |
                                                        pEntry->unused[1] << 8  |
                                                        pEntry->unused[2] << 16 |
                                                        pEntry->unused[3] << 24);

        EVMU_LOG_POP(1);
    }


    GblStringBuffer_destruct(&strBuff);
}

EVMU_EXPORT size_t EvmuFat_dirEntryIndex(const EvmuFat* pSelf, const EvmuDirEntry* pEntry) {
    const EvmuDirEntry* pDirectory = EvmuFat_blockData(pSelf, EvmuFat_blockDirectory(pSelf) -
                                                              EvmuFat_root(pSelf)->dirSize  - 1);
    return pEntry - pDirectory;
}

static EVMU_RESULT EvmuFat_root_(const EvmuFat* pSelf, EvmuRootBlock** ppRoot) {
    const size_t    blockSize = EvmuFat_blockSize(pSelf);

    *ppRoot = (EvmuRootBlock*)&EVMU_FLASH_(pSelf)->pStorage->pData[EVMU_FAT_BLOCK_ROOT * blockSize];

    return GBL_RESULT_SUCCESS;
}

static EVMU_RESULT EvmuFat_info_(const EvmuFat* pSelf, EvmuFatInfo* pInfo) {
    pInfo->partitions              = 1;
    pInfo->blockSize               = EVMU_FAT_BLOCK_SIZE;
    pInfo->accessesPerBlock.reads  = 1;
    pInfo->accessesPerBlock.writes = 1;
    pInfo->config.crcCheck         = 0;
    pInfo->config.removable        = 1;

    return GBL_RESULT_SUCCESS;
}

static EVMU_RESULT EvmuFat_GblObject_constructed_(GblObject* pObject) {
    GBL_CTX_BEGIN(NULL);

    GblObject_setName(pObject, EVMU_FAT_NAME);

    GBL_CTX_END();
}

static EVMU_RESULT EvmuFatClass_init_(GblClass* pClass, const void* pUd) {
    GBL_UNUSED(pUd);

    GBL_CTX_BEGIN(NULL);

    if(!GblType_classRefCount(EVMU_FAT_TYPE)) {
        GBL_PROPERTIES_REGISTER(EvmuFat);
    }

    GBL_OBJECT_CLASS(pClass)->pFnConstructed = EvmuFat_GblObject_constructed_;
    EVMU_FAT_CLASS(pClass)  ->pFnInfo        = EvmuFat_info_;
    EVMU_FAT_CLASS(pClass)  ->pFnRoot        = EvmuFat_root_;

    GBL_CTX_END();
}

EVMU_EXPORT GblType EvmuFat_type(void) {
    const static GblTypeInfo info = {
        .pFnClassInit        = EvmuFatClass_init_,
        .classSize           = sizeof(EvmuFatClass),
        .instanceSize        = sizeof(EvmuFat),
        .instancePrivateSize = sizeof(EvmuFat_)
    };

    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) {
        type = GblType_register(GblQuark_internStatic("EvmuFat"),
                                      EVMU_FLASH_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);

    }

    return type;

}

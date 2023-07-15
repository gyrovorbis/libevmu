#include <evmu/fs/evmu_file_manager.h>
#include <gimbal/strings/gimbal_string_buffer.h>
#include "gyro_vmu_vms.h"
#include "evmu_fat_.h"
#include "hw/evmu_ram_.h"
#include "gyro_vmu_flash.h"

EVMU_EXPORT size_t EvmuFileManager_count(const EvmuFileManager* pSelf) {
    size_t              count = 0;
    const EvmuDirEntry* entry = NULL;
    EvmuFat*            pFat  = EVMU_FAT(pSelf);

    for(ssize_t d = EvmuFat_dirEntryCount(pFat) - 1; d >= 0; --d) {
        entry = EvmuFat_dirEntry(pFat, d);

        if(entry && (entry->fileType == EVMU_FILE_TYPE_DATA ||
                      entry->fileType == EVMU_FILE_TYPE_GAME))
        {
            ++count;
        }
    }

    return count;
}

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_file(const EvmuFileManager* pSelf, size_t index) {
    EvmuDirEntry* pEntry = NULL;
    size_t        count  = 0;
    EvmuFat*      pFat   = EVMU_FAT(pSelf);

    const size_t dirEntryCount = EvmuFat_dirEntryCount(pFat);
    for(size_t d = 0; d < dirEntryCount; ++d) {
        pEntry = EvmuFat_dirEntry(pFat, d);
        GBL_ASSERT(pEntry);

        if(pEntry->fileType != EVMU_FILE_TYPE_NONE) {
            if(count++ == index) {
                break;
            }
        }
    }

    return pEntry;
}

EVMU_EXPORT size_t EvmuFileManager_free(EvmuFileManager* pSelf, EvmuDirEntry* pEntry) {
    size_t blocksFreed = 0;
    struct {
        GblStringBuffer buff;
        char            stackData[EVMU_DIRECTORY_FILE_NAME_SIZE + 1];
    } str;

    GblStringBuffer_construct(&str.buff, GBL_STRV(""), sizeof(str));

    EVMU_LOG_INFO("Deleting file from flash: [%s]", EvmuDirEntry_name(pEntry, &str.buff));
    EVMU_LOG_PUSH();
    GBL_CTX_BEGIN(NULL);

    EvmuBlock    block      = pEntry->firstBlock;
    const size_t blockCount = pEntry->fileSize;
    EvmuFat*     pFat       = EVMU_FAT(pSelf);

    while(block != EVMU_FAT_BLOCK_FAT_LAST_IN_FILE) {
        EVMU_LOG_VERBOSE("[%zu] Freeing block: [%u]", ++blocksFreed, block);

        const EvmuBlock next = EvmuFat_blockNext(pFat, block);
        GBL_CTX_VERIFY(next != EVMU_FAT_BLOCK_FAT_UNALLOCATED &&
                           next != EVMU_FAT_BLOCK_FAT_DAMAGED     &&
                           (next != EVMU_FAT_BLOCK_FAT_UNALLOCATED || blocksFreed < blockCount),
                       EVMU_RESULT_ERROR_INVALID_BLOCK,
                       "Invalid block in the middle of the chain: [%u]", next);

        GBL_CTX_VERIFY_CALL(EvmuFat_blockFree(pFat, block));
        block = next;
    }

    GBL_CTX_VERIFY(blocksFreed == blockCount,
                   EVMU_RESULT_ERROR_INVALID_FILE,
                   "Unexpected file size: [%zu blocks freed of %zu total]",
                   blocksFreed,
                   blockCount);

    memset(pEntry, 0, sizeof(EvmuDirEntry));
    pEntry->fileType = EVMU_FILE_TYPE_NONE;

    GBL_CTX_END_BLOCK();
    EVMU_LOG_POP(1);
    GblStringBuffer_destruct(&str.buff);
    return blocksFreed;
}

EVMU_EXPORT size_t EvmuFileManager_read(const EvmuFileManager* pSelf,
                                        const EvmuDirEntry*    pEntry,
                                        void*                  pData,
                                        size_t                 bytes,
                                        size_t                 offset,
                                        GblBool                includeHeader)
{
    size_t bytesRead = 0;
    GBL_CTX_BEGIN(NULL);

    EvmuFat* pFat = EVMU_FAT(pSelf);

    assert(pEntry && pData);

    const size_t blockSize = EvmuFat_blockSize(pFat);

    const VMSFileInfo* vmsHeader = (const VMSFileInfo*)EvmuFileManager_vms(pSelf, pEntry);
    if(!vmsHeader) return bytesRead;
    //else if(!includeHeader) startOffset += gyVmuVmsFileInfoHeaderSize(vmsHeader);

    size_t startBlockNum = offset / blockSize;
    uint16_t curBlock = pEntry->firstBlock;

    //Seek to startBlock
    for(unsigned b = 0; b < startBlockNum; ++b) {
        curBlock = EvmuFat_blockNext(pFat, curBlock);
        if(curBlock == EVMU_FAT_BLOCK_FAT_UNALLOCATED) return bytesRead; //FUCKED
    }

    //Read start block (starting at offset)
    if(includeHeader || pEntry->headerOffset != curBlock) {
        size_t startBlockByteOffset = offset % blockSize;
        size_t startBlockBytesLeft = blockSize - startBlockByteOffset;
        size_t startBlockBytes = (bytes < startBlockBytesLeft)? bytes : startBlockBytesLeft;
        const unsigned char* firstBlockData = EvmuFat_blockData(pFat, curBlock);
        if(!firstBlockData) return bytesRead;
        memcpy(pData, firstBlockData+startBlockByteOffset, startBlockBytes);
        bytesRead += startBlockBytes;
    }
    curBlock = EvmuFat_blockNext(pFat, curBlock);

    //Read each block beyond the start block
    //for(unsigned b = 1; b < numBlocks; ++b) {
    while(bytesRead < bytes) {
        const unsigned char* data = EvmuFat_blockData(pFat, curBlock);
        if(!data) break; // Jesus fucking CHRIST

        if(includeHeader || curBlock != pEntry->headerOffset) {
            const size_t bytesLeft = bytes - bytesRead;
            const size_t byteCount = (EVMU_FAT_BLOCK_SIZE < bytesLeft)?
                                         EVMU_FAT_BLOCK_SIZE : bytesLeft;

            memcpy(&pData[bytesRead], data, byteCount);
            bytesRead += byteCount;
        }

        curBlock = EvmuFat_blockNext(pFat, curBlock);
        if(curBlock == EVMU_FAT_BLOCK_FAT_UNALLOCATED) break;
    }

    GBL_CTX_END_BLOCK();
    return bytesRead;
}

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_game(const EvmuFileManager* pSelf) {
    EvmuFat*     pFat          = EVMU_FAT(pSelf);

    const size_t dirEntryCount = EvmuFat_dirEntryCount(pFat);
    for(size_t e = 0; e < dirEntryCount; ++e) {
        EvmuDirEntry* pEntry = EvmuFat_dirEntry(pFat, e);
        GBL_ASSERT(pEntry);

        if(pEntry->fileType == EVMU_FILE_TYPE_GAME)
            return pEntry;
    }

    return NULL;
}

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_find(const EvmuFileManager* pSelf, const char* pName) {
    EvmuFat* pFat = EVMU_FAT(pSelf);

    const size_t dirEntryCount = EvmuFat_dirEntryCount(pFat);
    for(uint16_t e = 0; e < dirEntryCount; ++e) {
        EvmuDirEntry* pEntry = EvmuFat_dirEntry(pFat, e);
        GBL_ASSERT(pEntry);

        if(pEntry->fileType != EVMU_FILE_TYPE_NONE)
            if(strncmp(pName, pEntry->fileName, EVMU_DIRECTORY_FILE_NAME_SIZE) == 0)
                return pEntry;
    }

    return NULL;
}

EVMU_EXPORT const EvmuVms* EvmuFileManager_vms(const EvmuFileManager* pSelf, const EvmuDirEntry* pEntry) {
    EvmuBlock block = pEntry->firstBlock;
    EvmuFat*  pFat  = EVMU_FAT(pSelf);

    for(size_t b = 0; b < pEntry->headerOffset; ++b)
        block = EvmuFat_blockNext(pFat, block);

    return EvmuFat_blockData(pFat, block);
}


EVMU_EXPORT EVMU_RESULT EvmuFileManager_defrag(EvmuFileManager* pSelf) {
    GBL_CTX_BEGIN(NULL);

    EvmuDevice* pTempDevice = NULL;

    EVMU_LOG_VERBOSE("Defragmenting VMU Flash Storage");
    EVMU_LOG_PUSH();

    GBL_CTX_VERIFY(EvmuFat_isFormatted(EVMU_FAT(pSelf)),
                   EVMU_RESULT_ERROR_UNFORMATTED,
                   "Cannot defrag and unformatted card!");
    {
        EvmuFat*       pFat         = EVMU_FAT(pSelf);
        EvmuFat_*      pFat_        = EVMU_FAT_(pFat);
        EvmuFlash_*    pFlash_      = EVMU_FLASH_(pFat);
        EvmuFlash_*    pTempFlash   = NULL;
        const size_t   fileCount    = EvmuFileManager_count(pSelf);
        EvmuFatUsage origMemUsage;

        // Cache flash metrics before defrag to validate post-defrag later
        EvmuFat_usage(pFat, &origMemUsage);
        if(fileCount) {
            // Create temporary device
            pTempDevice = GBL_NEW(EvmuDevice,
                                  "name", "tempDefragDevice");

            pTempFlash  = EVMU_FLASH_(pTempDevice->pFlash);

            // Create temporary back-up of flash to restore at any point if we fail
            GblByteArray_copy(pTempFlash->pStorage, pFlash_->pStorage);

            EVMU_LOG_VERBOSE("Uninstalling all files.");
            EVMU_LOG_PUSH();

            const size_t  dirEntryCount = EvmuFat_dirEntryCount(pFat);
            EvmuDirEntry* dirEntries[dirEntryCount];
            memset(dirEntries, 0, sizeof(EvmuDirEntry*) * dirEntryCount);

            // Iterate through all files, building a cache of their directory entries
            for(size_t f = 0; f < fileCount; ++f) {
                dirEntries[f] = EvmuFileManager_file(pSelf, f);

                GBL_CTX_VERIFY(dirEntries[f],
                               EVMU_RESULT_ERROR_INVALID_FILE,
                               "Failed to retrieve directory entry for file: [%zu]",
                               f);
            }

            size_t beginBlocks = 0;
            size_t endBlocks   = 0;

            // Iterate through all files in cache, erasing each
            for(size_t f = 0; f < fileCount; ++f) {
                EvmuDirEntry* pEntry = dirEntries[f];

                beginBlocks += pEntry->fileSize;
                endBlocks   += EvmuFileManager_free(pSelf, pEntry);
                GBL_CTX_VERIFY_LAST_RECORD();
            }

            // Ensure sanity of newly cleaned card
            size_t newFileCount = EvmuFileManager_count(pSelf);
            GBL_CTX_VERIFY(!newFileCount,
                           GBL_RESULT_ERROR_INTERNAL,
                           "Failed to uninstall all files: [%zu remaining]",
                           newFileCount);

            GBL_CTX_VERIFY(beginBlocks == endBlocks,
                           GBL_RESULT_ERROR_INTERNAL,
                           "Formatting erased incorrect number of blocks: [%zu actual, %zu expected]",
                           endBlocks,
                           beginBlocks);

            EvmuFatUsage memUsage;
            EvmuFat_usage(pFat, &memUsage);

            GBL_CTX_VERIFY(memUsage.blocksFree >= endBlocks,
                           GBL_RESULT_ERROR_INTERNAL,
                           "Formatted card didn't have the expected number of free blocks: [%u]",
                           memUsage.blocksFree);

            // Log all remaining FAT entries for post-mortem debugging if anything still goes wrong
            for(size_t b = 0; b < EvmuFat_blockCount(pFat); ++b)
                EVMU_LOG_DEBUG("FAT[%x] = %u", b, EvmuFat_blockNext(pFat, b));

            EVMU_LOG_POP(1);

            EVMU_LOG_VERBOSE("Reinstalling all files.");
            EVMU_LOG_PUSH();

            for(size_t f = 0; f < fileCount; ++f) {
                const size_t capacity = EvmuFat_capacity(pFat);
                uint8_t tempImage[capacity? capacity : 1];

                //Read file from temporary device to temporary buffer
                const EvmuDirEntry* pEntry = EvmuFileManager_file(pTempDevice->pFileMgr, f);
                GBL_CTX_VERIFY(pEntry,
                               EVMU_RESULT_ERROR_INVALID_FILE,
                               "Failed to retrieve dirEntry on tempDevice for file: [%zu]",
                               f);


                GBL_CTX_VERIFY(gyVmuFlashFileRead(pTempDevice, pEntry, tempImage, 1),
                               GBL_RESULT_ERROR_FILE_READ,
                               "Failed to extract file from temporary device: [file %d]",
                               f);

                //Write file from buffer to device
                VMU_LOAD_IMAGE_STATUS status;
                VMUFlashNewFileProperties fileProperties;
                gyVmuFlashNewFilePropertiesFromDirEntry(&fileProperties, pEntry);
                pEntry = gyVmuFlashFileCreate(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf)),
                                              &fileProperties,
                                              tempImage,
                                              &status);
                GBL_CTX_VERIFY(pEntry && status == VMU_LOAD_IMAGE_SUCCESS,
                               GBL_RESULT_ERROR_FILE_WRITE,
                               "Failed to write file back to device: [file %d]",
                               f);
            }

            //Verify that everything has been reinstalled.
            newFileCount = EvmuFileManager_count(pSelf);
            GBL_CTX_VERIFY(newFileCount == fileCount,
                           EVMU_RESULT_ERROR_INVALID_FILE_SYSTEM,
                           "Final file count does not match initial file count! [%zu != %zu]",
                           fileCount,
                           newFileCount);

            EvmuFat_usage(pFat, &memUsage);
            GBL_CTX_VERIFY(memUsage.blocksUsed == origMemUsage.blocksUsed,
                           EVMU_RESULT_ERROR_INVALID_FILE_SYSTEM,
                           "Final used block count does not match initial! [%zu != %zu]",
                           memUsage.blocksUsed,
                           origMemUsage.blocksUsed);

            EVMU_LOG_POP(1);

        }
    }
    GBL_CTX_END_BLOCK();

    EvmuDevice_unref(pTempDevice);
    EVMU_LOG_POP(1);
    return GBL_CTX_RESULT();
}


EVMU_EXPORT GblType EvmuFileManager_type(void) {
    const static GblTypeInfo info = {
        .classSize           = sizeof(EvmuFileManagerClass),
        .instanceSize        = sizeof(EvmuFileManager)
    };

    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) {
        type = GblType_registerStatic(GblQuark_internStringStatic("EvmuFileManager"),
                                      EVMU_FAT_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);

    }

    return type;

}

#include <evmu/fs/evmu_file_manager.h>
#include <evmu/fs/evmu_vms.h>
#include <evmu/fs/evmu_icondata.h>
#include <evmu/fs/evmu_vmi.h>
#include <gimbal/strings/gimbal_string_buffer.h>
#include <gimbal/strings/gimbal_string.h>
#include <gyro_vmu_icondata.h>
#include "gyro_vmu_vms.h"
#include "evmu_fat_.h"
#include "../hw/evmu_flash_.h"
#include "gyro_vmu_flash.h"

static EVMU_RESULT EvmuFileManager_loadFlash_(EvmuFileManager* pSelf, const char* pPath);

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

    GblStringBuffer_construct(&str.buff, "", 0, sizeof(str));

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

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_iconData(const EvmuFileManager* pSelf) {
    return EvmuFileManager_find(pSelf, EVMU_ICONDATA_VMS_FILE_NAME);
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

EVMU_EXPORT GblBool EvmuFileManager_foreach(const EvmuFileManager* pSelf, EvmuDirEntryIterFn pFnIt, void* pClosure) {
    EvmuFat* pFat = EVMU_FAT(pSelf);

    for(uint16_t e = 0; e < EvmuFat_dirEntryCount(pFat); ++e) {
        EvmuDirEntry* pEntry = EvmuFat_dirEntry(pFat, e);
        GBL_ASSERT(pEntry);

        if(pEntry->fileType != EVMU_FILE_TYPE_NONE)
            if(pFnIt(pEntry, pClosure))
                return GBL_TRUE;
    }

    return GBL_FALSE;
}

EVMU_EXPORT EvmuVms* EvmuFileManager_vms(const EvmuFileManager* pSelf, const EvmuDirEntry* pEntry) {
    EvmuBlock block = pEntry->firstBlock;
    EvmuFat*  pFat  = EVMU_FAT(pSelf);

    for(size_t b = 0; b < pEntry->headerOffset; ++b)
        block = EvmuFat_blockNext(pFat, block);

    return (EvmuVms*)EvmuFat_blockData(pFat, block);
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
                pEntry = EvmuFileManager_alloc(EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf))->pFileMgr,
                                              &fileProperties,
                                              tempImage);
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

EVMU_EXPORT size_t EvmuFileManager_bytes(const EvmuFileManager* pSelf, const EvmuDirEntry* pDirEntry) {
    if(pDirEntry->fileType == EVMU_FILE_TYPE_DATA) {
        const EvmuVms* pVms = EvmuFileManager_vms(pSelf, pDirEntry);
        return EvmuVms_totalBytes(pVms);
    } else {
        return pDirEntry->fileSize * EvmuFat_blockSize(EVMU_FAT(pSelf));
    }
}

EVMU_EXPORT uint16_t EvmuFileManager_crc(const EvmuFileManager* pSelf, const EvmuDirEntry* pDirEntry) {
    // GAME files don't use the CRC...
    if(pDirEntry->fileType == EVMU_FILE_TYPE_GAME)
        return 0;

    const EvmuFat* pFat      = EVMU_FAT(pSelf);
    EvmuVms*       pVms      = EvmuFileManager_vms(pSelf, pDirEntry);
    const size_t   blockSize = EvmuFat_blockSize(pFat);
    uint16_t       crc       = 0;
    size_t         bytesLeft = EvmuFileManager_bytes(pSelf, pDirEntry);

    //have to set this equal to 0 to get the right CRC!
    const uint16_t prevCrc = pVms->crc;
    pVms->crc = 0;

    for(uint16_t block = pDirEntry->firstBlock;
         block != EVMU_FAT_BLOCK_FAT_LAST_IN_FILE;
         block = EvmuFat_blockNext(pFat, block))
    {
        const size_t bytes = (bytesLeft > blockSize)?
                                blockSize :
                                bytesLeft;

        const uint8_t* pData = EvmuFat_blockData(pFat, block);
        GBL_ASSERT(pData, "No block data found!?");

        crc = gblHashCrc16BitPartial(pData, bytes, &crc);
        bytesLeft -= bytes;

    }

    GBL_ASSERT(!bytesLeft, "Failed to consume all bytes when calculating CRC!");

    pVms->crc = prevCrc;

    return crc;
}

EVMU_EXPORT EvmuDirEntry* EvmuFileManager_alloc(EvmuFileManager* pSelf,
                                                EvmuNewFileInfo* pInfo,
                                                const void*      pData)
{
    EvmuFat*      pFat   = EVMU_FAT(pSelf);
    EvmuDirEntry* pEntry = NULL;

    int blocks[EvmuFat_userBlocks(pFat)];

    struct {
        GblStringBuffer buff;
        char            stackBytes[128];
    } str;

    GBL_CTX_BEGIN(NULL);

    GblStringBuffer_construct(&str.buff, "", 0, sizeof(str));
    memset(blocks, -1, sizeof(int) * EvmuFat_userBlocks(pFat));

    EVMU_LOG_VERBOSE("VMU Flash - Creating file [%s].", EvmuNewFileInfo_name(pInfo, &str.buff));
    EVMU_LOG_PUSH();

    //=== 1 - Check if we're creating a GAME file while one already exists. ===
    GBL_CTX_VERIFY(pInfo->type != EVMU_FILE_TYPE_GAME || !EvmuFileManager_game(pSelf),
                   EVMU_RESULT_ERROR_EXISTING_GAME,
                   "Only one GAME file can be present at a time!");

#if 0
    //=== 2 - Make sure we don't already have a file with the same name. ===
    GBL_CTX_VERIFY(EvmuFileSystem_find(pSelf, GblStringBuffer_cString(&str.buff),
                   EVMU_RESULT_ERROR_FILE_DUPLICATE,
                   "File with the same name already existed!");
#endif

    //=== 3 - Check whether there are enough free blocks available for the file. ===
    const size_t blocksRequired  = EvmuFat_toBlocks(pFat, pInfo->bytes);
    EvmuFatUsage memUsage;

    EvmuFat_usage(pFat, &memUsage);
    GBL_CTX_VERIFY(memUsage.blocksFree >= blocksRequired,
                   EVMU_RESULT_ERROR_NOT_ENOUGH_SPACE,
                   "Not enough free blocks! [Required: %zu, Available: %zu",
                   blocksRequired,
                   memUsage.blocksFree);

    /* Game data must all be stored contiguously starting at block 0,
     * so check whether memory card requires defrag.
     */
    if(pInfo->type == EVMU_FILE_TYPE_GAME) {
        // Defragment card if we couldn't find enough contiguous blocks.
        if(EvmuFat_seqFreeBlocks(pFat) < blocksRequired) {
            EVMU_LOG_WARN("Not enough contiguous blocks available for GAME file [%d/%d]. Defrag required.",
                          EvmuFat_seqFreeBlocks(pFat),
                          blocksRequired);

            GBL_CTX_VERIFY_CALL(EvmuFileManager_defrag(pSelf));

            GBL_CTX_VERIFY(EvmuFat_seqFreeBlocks(pFat) >= blocksRequired,
                           EVMU_RESULT_ERROR_DEFRAG_FAILED,
                           "Still not enough contiguous blocks available [%d/%d], Defrag must have failed!",
                           EvmuFat_seqFreeBlocks(pFat),
                           blocksRequired);
        }
    }

    //=== 4 - Create Flash Directory Entry for file. ===
    pEntry = EvmuFat_dirEntryAlloc(pFat, pInfo->type);
    GBL_CTX_VERIFY(pEntry,
                   EVMU_RESULT_ERROR_TOO_MANY_FILES,
                   "Could not allocate entry in Flash Directory (too many files present: %zu).",
                   EvmuFileManager_count(pSelf));

    EVMU_LOG_VERBOSE("Creating Flash Directory Entry [index: %zu]", EvmuFat_dirEntryIndex(pFat, pEntry));
    EVMU_LOG_PUSH();

    //Fill in Flash Directory Entry for file
    memcpy(pEntry->fileName, pInfo->name, EVMU_DIRECTORY_FILE_NAME_SIZE);
    pEntry->copyProtection  = pInfo->copy;
    pEntry->fileSize        = blocksRequired;
    pEntry->headerOffset    = (pInfo->type == EVMU_FILE_TYPE_DATA)? 0 : 1;

    //Add timestamp to directory
    GblDateTime dt;
    EvmuTimestamp_setDateTime(&pEntry->timestamp, GblDateTime_nowLocal(&dt));

    EVMU_LOG_POP(1);

    //=== 5 - Allocate FAT Blocks for File ===
    EVMU_LOG_VERBOSE("Allocating FAT Blocks for file [Blocks: %d].", blocksRequired);
    EVMU_LOG_PUSH();

    // Allocate FAT blocks to hold the image, chain them together
    for(size_t b = 0; b < blocksRequired; ++b) {
        blocks[b] = EvmuFat_blockAlloc(pFat,
                                       (b > 0)? blocks[b - 1] : EVMU_FAT_BLOCK_FAT_UNALLOCATED,
                                       pInfo->type);

        GBL_CTX_VERIFY(blocks[b] != EVMU_FAT_BLOCK_FAT_UNALLOCATED,
                       EVMU_RESULT_ERROR_INVALID_BLOCK,
                       "Failed to allocate FAT block: [%d/%d]",
                       b,
                       blocksRequired);
    }

    pEntry->firstBlock = blocks[0];
    EvmuFat_dirEntryLog(pFat, pEntry);

    EVMU_LOG_POP(1);

    //=== 6 - Write VMS File  ===
    EVMU_LOG_VERBOSE("Writing VMS File Data.");
    EVMU_LOG_PUSH();

    size_t bytesWritten = 0;
    GBL_CTX_VERIFY((bytesWritten = EvmuFileManager_write(pSelf, pEntry, pData, pInfo->bytes, 0))
                       == pInfo->bytes,
                   GBL_RESULT_ERROR_FILE_WRITE,
                   "Failed to write entirety of file: [%zu/%zu]",
                   bytesWritten,
                   pInfo->bytes);


    GBL_CTX_VERIFY(EvmuFileManager_vms(pSelf, pEntry),
                   EVMU_RESULT_ERROR_INVALID_FILE,
                   "Failed to fetch VMS header after write!");

    if(EvmuFileManager_iconData(pSelf) != pEntry)
        EvmuVms_log(EvmuFileManager_vms(pSelf, pEntry));
    else
        gyVmuIconDataPrint((IconDataFileInfo*)EvmuFileManager_vms(pSelf, pEntry));

    EVMU_LOG_POP(2);

    GBL_CTX_END_BLOCK();

    // Check if we have an error
    if(GBL_RESULT_ERROR(GBL_CTX_RESULT())) {
        // Free up the FAT entries
        for(size_t b = 0; b < EvmuFat_userBlocks(pFat); ++b) {
            if(blocks[b] == -1) break;
            EvmuFat_blockFree(pFat, blocks[b]);
        }
        // Flag directory entry as reested
        if(pEntry)
            pEntry->fileType = EVMU_FILE_TYPE_NONE;
    }

    GblStringBuffer_destruct(&str.buff);

    return pEntry;
}

EVMU_EXPORT size_t EvmuFileManager_write(const EvmuFileManager* pSelf,
                                         const EvmuDirEntry*    pEntry,
                                         const void*            pBuffer,
                                         size_t                 size,
                                         size_t                 offset)
{
    size_t bytesWritten = 0;
    struct {
        GblStringBuffer buff;
        char            stackBytes[128];
    } str;

    GBL_CTX_BEGIN(NULL);

    const EvmuFat* pFat      = EVMU_FAT(pSelf);
    EvmuFlash*     pFlash    = EVMU_FLASH(pSelf);
    const size_t   blockSize = EvmuFat_blockSize(pFat);

    GblStringBuffer_construct(&str.buff, "", 0, sizeof(str));

    /* NOT THAT THIS IS ROUNDED UP TO A FULL BLOCK, SO FOR DATA FILES
     * THIS RANGE CAN ACTUALLY BE LARGER THAN THE ACTUAL BYTE SIZE OF
     * THE FILE. */
    const size_t fileSize = pEntry->fileSize * EvmuFat_blockSize(pFat);
    GBL_CTX_VERIFY(offset + size <= fileSize,
                   GBL_RESULT_ERROR_OUT_OF_RANGE,
                   "Out-of-range write attempt to file [%s]: "
                   "[offset: %zu, count: %zu, actual size: %zu]",
                   EvmuDirEntry_name(pEntry, &str.buff),
                   offset,
                   size,
                   fileSize);

    const size_t blockCount    = EvmuFat_toBlocks(pFat, fileSize);
    const size_t startBlockIdx = offset / blockSize;

    // Iterate over blocks to find the starting block
    EvmuBlock startBlock = pEntry->firstBlock;
    for(size_t b = 1; b < startBlockIdx; ++b)
        startBlock = EvmuFat_blockNext(pFat, startBlock);

    // Iterate over blocks to find the ending block
    EvmuBlock endBlock = startBlock;
    for(size_t b = 1; b < blockCount; ++b)
        endBlock = EvmuFat_blockNext(pFat, endBlock);

    // Iterate from start to end block, writing block-sized chunks
    EvmuBlock curBlock     = startBlock;
    size_t    remaining    = size;
    for(size_t b = 0; b < blockCount; ++b) {
        size_t writeBytes = (remaining < blockSize)?
                             remaining : blockSize;

        const size_t blockOffset = b * blockSize + (b? 0 : offset);

        GBL_CTX_CALL(EvmuFlash_writeBytes(pFlash,
                                          blockOffset,
                                          &pBuffer[bytesWritten],
                                          &writeBytes));

        bytesWritten += writeBytes;
        remaining    -= writeBytes;
        curBlock      = EvmuFat_blockNext(pFat, curBlock);
    }

    GBL_CTX_VERIFY(!remaining,
                   GBL_RESULT_ERROR_FILE_WRITE,
                   "Failed to write all bytes: [%zu/%zu remaining]",
                   remaining,
                   size);

    GBL_CTX_END_BLOCK();
    GblStringBuffer_destruct(&str.buff);

    return bytesWritten;
}

EVMU_EXPORT EVMU_RESULT EvmuFileManager_load(EvmuFileManager* pSelf, const char* pPath) {
    GblStringList* pStrList = NULL;
    VMU_LOAD_IMAGE_STATUS status;
    EvmuDirEntry* pEntry = NULL;
    EvmuDevice* pDevice = EvmuPeripheral_device(EVMU_PERIPHERAL(pSelf));

    struct {
        GblStringBuffer buff;
        char            stackBytes[FILENAME_MAX];
    } str;

    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Loading file: [%s]", pPath);
    EVMU_LOG_PUSH();

    GblStringBuffer_construct(&str.buff, "", 0, sizeof(str));

    pStrList = GblStringList_createSplit(pPath, "/\\");

    if(!gblStrCaseCmp(GblStringList_back(pStrList),
                       EVMU_ICONDATA_VMS_FILE_NAME) ||
       !gblStrCaseCmp(GblStringList_back(pStrList),
                       "ICONDATA.VMS"))
    {
        pEntry =
            gyVmuFlashLoadIconDataVms(pDevice,
                                      pPath,
                                      &status);
    } else {
        GblStringList_unref(pStrList);
        pStrList = GblStringList_createSplit(pPath, ".");
        GblStringRef* pExt = GblStringList_back(pStrList);

        GBL_CTX_VERIFY(GblStringList_size(pStrList) > 1,
                       EVMU_RESULT_ERROR_INVALID_FILE,
                       "No extension found, cannot deduce file type!");

        if(!gblStrCaseCmp(pExt, "bin") || !gblStrCaseCmp(pExt, "vmu")) {
            GBL_CTX_VERIFY_CALL(EvmuFileManager_loadFlash_(pSelf, pPath));
            pEntry = (EvmuDirEntry*)0xFFF;
            status = VMU_LOAD_IMAGE_SUCCESS;
        } else if(!gblStrCaseCmp(pExt, "dcm"))
            pEntry = gyVmuFlashLoadImageDcm(pDevice, pPath, &status);
        else if(!gblStrCaseCmp(pExt, "dci"))
            pEntry = gyVmuFlashLoadImageDci(pDevice, pPath, &status);
        else if(!gblStrCaseCmp(pExt, "vmi")) {
            GBL_CTX_VERIFY(EvmuVmi_findVmsPath(NULL, pPath, &str.buff),
                           EVMU_RESULT_ERROR_INVALID_FILE,
                           "No VMS file could be found for VMI!");
            pEntry =
                gyVmuFlashLoadImageVmiVms(pDevice,
                                          pPath,
                                          GblStringBuffer_cString(&str.buff),
                                          &status);
        } else if(!gblStrCaseCmp(pExt, "vms")) {
           EvmuVms_findVmiPath(pPath, &str.buff);

            pEntry =
                    gyVmuFlashLoadImageVmiVms(pDevice,
                                              GblStringBuffer_cString(&str.buff),
                                              pPath,
                                              &status);

        } else GBL_CTX_RECORD_SET(EVMU_RESULT_ERROR_INVALID_FILE,
                                  "Unknown file extension: [%s]",
                                  pExt);
    }
    EVMU_LOG_POP(1);

    GBL_CTX_VERIFY(pEntry && status == VMU_LOAD_IMAGE_SUCCESS,
                   EVMU_RESULT_ERROR_INVALID_FILE,
                   "Failed to load file due to legacy libGyro reest: %s",
                   gyVmuFlashLastErrorMessage());

    GBL_CTX_END_BLOCK();

    GblStringList_unref(pStrList);
    GblStringBuffer_destruct(&str.buff);
    return GBL_CTX_RESULT();
}

static EVMU_RESULT EvmuFileManager_loadFlash_(EvmuFileManager* pSelf, const char* pPath) {
    EvmuFlash*   pFlash   = EVMU_FLASH(pSelf);
    EvmuIMemory* pIMemory = EVMU_IMEMORY(pSelf);
    EvmuFat*     pFat     = EVMU_FAT(pSelf);
    FILE*        pFile    = NULL;
    const size_t capacity = EvmuIMemory_capacity(pIMemory);

    GBL_CTX_BEGIN(NULL);

    EVMU_LOG_INFO("Loading Raw Flash image from file: [%s]", pPath);
    EVMU_LOG_PUSH();

    GBL_CTX_VERIFY((pFile = fopen(pPath, "rb")),
                   GBL_RESULT_ERROR_FILE_OPEN);

    EvmuWord fillBuffer[EVMU_FAT_BLOCK_SIZE] = { 0 };
#if 0
    EVMU_LOG_VERBOSE("Zeroing flash");
    EvmuIMemory_fillBytes(pIMemory,
                          0,
                          capacity,
                          fillBuffer,
                          EVMU_FAT_BLOCK_SIZE);
#endif
    size_t fileLen = 0;
    fseek(pFile, 0, SEEK_END); // seek to end of file
    fileLen = ftell(pFile);    // get current file pointer
    fseek(pFile, 0, SEEK_SET); // seek back to beginning of file

    if(fileLen != capacity) {
        EVMU_LOG_WARN("File size does not match flash size. Probaly not a legitimate image: "
                      "[File Size: %zu, Flash Size: %zu]",
                      fileLen,
                      capacity);
    }

    const size_t toRead = fileLen < capacity?
                         fileLen : capacity;

    size_t read = 0;
    while(read < toRead) {
        const size_t chunkSize =
            toRead > EVMU_FAT_BLOCK_SIZE?
            EVMU_FAT_BLOCK_SIZE : toRead;

        size_t retVal =
            fread(fillBuffer, 1, chunkSize, pFile);

        if(retVal == chunkSize) {
            GBL_CTX_VERIFY_CALL(
                EvmuFlash_writeBytes(pFlash, 0, fillBuffer, &retVal)
            );
            read += chunkSize;
        } else {
            GBL_CTX_VERIFY(!feof(pFile),
                           GBL_RESULT_ERROR_FILE_READ,
                           "Unexpected end of file! [%zu/%zu bytes read]",
                           read,
                           toRead);

            GBL_CTX_VERIFY(!ferror(pFile),
                           GBL_RESULT_ERROR_FILE_READ,
                           "File error [%s]: [%zu/%zu bytes read]",
                           strerror(errno),
                           read,
                           toRead);
        }
    }

    EvmuFat_log(pFat);

    GBL_CTX_VERIFY(EvmuFat_isFormatted(pFat),
                   EVMU_RESULT_ERROR_UNFORMATTED);

    GBL_CTX_END_BLOCK();

    EVMU_LOG_POP(1);
    if(pFile) fclose(pFile);

    return GBL_CTX_RESULT();
}

EVMU_EXPORT GblType EvmuFileManager_type(void) {
    const static GblTypeInfo info = {
        .classSize    = sizeof(EvmuFileManagerClass),
        .instanceSize = sizeof(EvmuFileManager)
    };

    static GblType type = GBL_INVALID_TYPE;

    if(type == GBL_INVALID_TYPE) GBL_UNLIKELY {
        type = GblType_register(GblQuark_internStatic("EvmuFileManager"),
                                      EVMU_FAT_TYPE,
                                      &info,
                                      GBL_TYPE_FLAG_TYPEINFO_STATIC);

    }

    return type;

}

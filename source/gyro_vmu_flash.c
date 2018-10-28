#include "gyro_vmu_flash.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_vmi.h"
#include "gyro_vmu_vms.h"
#include "gyro_vmu_lcd.h"
#include "gyro_vmu_util.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <gyro_system_api.h>
#include <libGyro/gyro_defines.h>
#include <gyro_file_api.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

static char _loadImageErrorMsg[VMU_FLASH_LOAD_IMAGE_ERROR_MESSAGE_SIZE] = { '\0' };


const char* gyVmuFlashLoadImageLastErrorMessage(void) {
    return _loadImageErrorMsg;
}

size_t gyVmuFlashBytes(const struct VMUDevice* dev) {
    return FLASH_SIZE;
}

static inline int tobcd(int n) {
  return ((n/10)<<4)|(n%10);
}

int gyVmuFlashBytesToBlocks(int bytes) {
    int blocks = bytes/VMU_FLASH_BLOCK_SIZE;
    if(bytes % VMU_FLASH_BLOCK_SIZE) ++blocks;
    return blocks;
}


int gyVmuFlashLoadDCM(VMUDevice *dev, const char *path) {

}




//FAKE FLASH IMAGE
void gyfakeflash(VMUDevice* dev, const char *filename, int sz) {
    unsigned char *root, *fat, *dir;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    int i;
    char *fn2;

    if((fn2 = strrchr(filename, '/'))!=NULL)
    filename = fn2+1;
    else if((fn2 = strrchr(filename, '\\'))!=NULL)
    filename = fn2+1;
    memset(dev->flash+241*512, 0, sizeof(dev->flash)-241*512);
    root = dev->flash+255*512;
    fat = dev->flash+254*512;
    dir = dev->flash+253*512;
    sz = ((sz+511)>>9);
    for(i=0; i<256*2; i+=2) {
    fat[i] = 0xfc;
    fat[i+1] = 0xff;
    }
    for(i=0; i<sz; i++) {
    fat[2*i] = i+1;
    fat[2*i+1] = 0;
    }
    if((--i)>=0) {
    fat[2*i] = 0xfa;
    fat[2*i+1] = 0xff;
    }
    fat[254*2] = 0xfa;
    fat[254*2+1] = 0xff;
    fat[255*2] = 0xfa;
    fat[255*2+1] = 0xff;
    for(i=253; i>241; --i) {
    fat[2*i] = i-1;
    fat[2*i+1] = 0;
    }
    fat[241*2] = 0xfa;
    fat[241*2+1] = 0xff;
    dir[0] = 0xcc;
    strncpy((char*)dir+4, filename, 12);
    for(i=strlen(filename); i<12; i++)
    dir[4+i]=' ';
    dir[0x10] = tobcd(tm->tm_year/100+19);
    dir[0x11] = tobcd(tm->tm_year%100);
    dir[0x12] = tobcd(tm->tm_mon+1);
    dir[0x13] = tobcd(tm->tm_mday);
    dir[0x14] = tobcd(tm->tm_hour);
    dir[0x15] = tobcd(tm->tm_min);
    dir[0x16] = tobcd(tm->tm_sec);
    dir[0x17] = tobcd(tm->tm_wday);
    dir[0x18] = sz&0xff;
    dir[0x19] = sz>>8;
    dir[0x1a] = 1;
    memset(root, 0x55, 16);
    root[0x10] = 1;
    memcpy(root+0x30, dir+0x10, 8);
    root[0x44] = 255;
    root[0x46] = 254;
    root[0x48] = 1;
    root[0x4a] = 253;
    root[0x4c] = 13;
    root[0x50] = 200;
}

void gycheck_gamesize(VMUDevice* dev)
{
  unsigned char *root, *fat, *dir;
  int i, fatblk, dirblk, dircnt;
  root = dev->flash+255*512;
  fatblk = (root[0x47]<<8)|root[0x46];
  dirblk = (root[0x4b]<<8)|root[0x4a];
  dircnt = (root[0x4d]<<8)|root[0x4c];
  if(fatblk>=256 || dircnt>=256 || !dircnt)
    return;
  fat = dev->flash+fatblk*512;
  while(dircnt-- && dirblk<256) {
    dir = dev->flash+dirblk*512;
    dirblk = (fat[2*dirblk+1]<<8)|fat[2*dirblk];
    for(i=0; i<16; i++)
      if(dir[i*32] == 0xcc) {
    int sz = (dir[i*32+0x19]<<8)|dir[i*32+0x18];
    if(sz>1 && sz<=200) {
      dev->gameSize = sz*512;
      return;
    }
      }
  }
}


static inline size_t _gyread(GYFile* fd, void* buff, size_t bytes) {
    size_t bytesRead;
    gyFileRead(fd, buff, bytes, &bytesRead);
    return bytesRead;
}

int gyinitflash(VMUDevice* dev, GYFile* fd) {
    int r=0, t=0;
    memset(dev->flash, 0, sizeof(dev->flash));

    while(t<(int)sizeof(dev->flash) && (r=_gyread(fd, dev->flash+t, sizeof(dev->flash)-t))>0)
    t += r;
    if(r<0 || t<0x480)
    return 0;
    return t;
}


int gyVmuFlashLoadVMI(VMIFileInfo* info, const char *path) {
    int success = 0;
    size_t bytesRead;

    _gyLog(GY_DEBUG_VERBOSE, "Loading VMI Info [%s].", path);
    _gyPush();

    memset(info, 0, sizeof(VMIFileInfo));

    GYFile* file;
    gyFileOpen(path, "rb", &file);

    if(!file) {
        _gyLog(GY_DEBUG_ERROR, "Could not open binary file for reading!");

    } else {

        gyFileRead(file, info, sizeof(VMIFileInfo), &bytesRead);
        gyFileClose(&file);

        //BETTER BE TRUE OR COMPILER PACKED OUR FUCKING STRUCT WRONG!!!!
        assert(sizeof(VMIFileInfo) == VMU_VMI_FILE_SIZE);

       if(!bytesRead) {
            _gyLog(GY_DEBUG_ERROR, "Could not read from file!");
        } else {
            if(bytesRead != VMU_VMI_FILE_SIZE) {
                _gyLog(GY_DEBUG_WARNING, "File size didn't exactly match expected VMI size, but continuing anyway [%d/%d].", bytesRead, VMU_VMI_FILE_SIZE);
            }
            success = 1;
        }
    }

    _gyPop(1);

    return success;

}

int gyVmuFlashLoadVMS(VMUDevice* dev, const char *filename) {
    _gyLog(GY_DEBUG_VERBOSE, "Loading VMS image into flash [%s].", filename);
    _gyPush();

  int r;
  GYFile* fd;
  gyFileOpen(filename, "rb", &fd);

  if(fd) {
    if(!(r=gyinitflash(dev, fd))) {
        gyFileClose(&fd);
        _gyLog(GY_DEBUG_ERROR, "Could not load ROM!");
        _gyPop(1);
      return 0;
    }
    gyFileClose(&fd);
    dev->gameSize = r;

    _gyLog(GY_DEBUG_VERBOSE, "Raw image size - %d", r);
    gyVmuPrintVMSFileInfo((VMSFileInfo*)&dev->flash[0x200]);

    if(r<241*512)
      gyfakeflash(dev, filename, r);
    gycheck_gamesize(dev);

    _gyPop(1);
    return 1;
  } else {
    _gyLog(GY_DEBUG_ERROR, "Could not open file handle!");
    _gyPop(1);
    return 0;
  }


}

int gyinitbios(VMUDevice* dev, GYFile* fd) {
    int r=0, t=0;
    memset(dev->rom, 0, sizeof(dev->rom));
    while(t<(int)sizeof(dev->rom) && (r=_gyread(fd, dev->rom+t, sizeof(dev->rom)-t))>0)
    t += r;
    if(r<0 || t<0x200)
    return 0;
    return 1;
}

int gyloadbios(VMUDevice* dev, const char *filename)
{
  GYFile* fd;

  gyFileOpen(filename, "rb", &fd);

  if(fd) {
    if(!gyinitbios(dev, fd)) {
      gyFileClose(&fd);
      _gyLog(GY_DEBUG_ERROR, "%s: can't load BIOS image", filename);
      return 0;
    } else {
        _gyLog(GY_DEBUG_VERBOSE, "Bios image [%s] loaded successfully!", filename);
    }
    gyFileClose(&fd);
    dev->biosLoaded = 1;
    return 1;
  } else {
    //perror(filename);
    return 0;
  }
}





//=== FLASH BLOCK API ===

VMUFlashRootBlock* gyVmuFlashBlockRoot(struct VMUDevice* dev) {
    return (VMUFlashRootBlock*)(dev->flash + VMU_FLASH_BLOCK_SIZE*VMU_FLASH_BLOCK_ROOT);
}

uint16_t gyVmuFlashBlockDirectory(const struct VMUDevice* dev) {
    return gyVmuFlashBlockRoot((VMUDevice*)dev)->dirBlock;
}

uint16_t gyVmuFlashBlockFat(const struct VMUDevice* dev) {
    return gyVmuFlashBlockRoot((VMUDevice*)dev)->fatBlock;
}

uint16_t gyVmuFlashBlockCount(const struct VMUDevice* dev) {
    return (gyVmuFlashBlockRoot((VMUDevice*)dev)->fatSize*VMU_FLASH_BLOCK_SIZE/sizeof(uint16_t));
}

unsigned char* gyVmuFlashBlock(struct VMUDevice* dev, uint16_t block) {
    return (block >= gyVmuFlashBlockCount(dev))? NULL : &dev->flash[block*VMU_FLASH_BLOCK_SIZE];
}

uint16_t* gyVmuFlashBlockFATEntry(VMUDevice* dev, uint16_t block) {
    return (block >= gyVmuFlashBlockCount(dev))? NULL : (((uint16_t*)gyVmuFlashBlock(dev, gyVmuFlashBlockFat(dev)))+block);
}

uint16_t gyVmuFlashDirEntryCount(const struct VMUDevice* dev) {
    return gyVmuFlashBlockRoot((VMUDevice*)dev)->dirSize*VMU_FLASH_BLOCK_SIZE/sizeof(VMUFlashDirEntry);
}


uint16_t gyVmuFlashBlockNext(const VMUDevice* dev, uint16_t block) {
    const uint16_t* fatEntry = gyVmuFlashBlockFATEntry(dev, block);
    return fatEntry? *fatEntry : VMU_FLASH_BLOCK_FAT_UNALLOCATED;
}

/* DATA file blocks are allocated as the first available free block starting at the highest address.
 * GAME file blocks are allocated contiguously starting at address 0.
 */
uint16_t gyVmuFlashBlockAlloc(VMUDevice* dev, uint16_t previous, int fileType) {
    uint16_t    block = VMU_FLASH_BLOCK_FAT_UNALLOCATED;
    uint16_t*   fat   = (uint16_t*)gyVmuFlashBlock(dev, gyVmuFlashBlockFat(dev));

    uint16_t firstBlock;
    uint16_t lastBlock;
    if(fileType == VMU_FLASH_FILE_TYPE_GAME) {
        firstBlock  = 0;
        lastBlock   = gyVmuFlashUserDataBlocks(dev)-1;
    } else if(fileType == VMU_FLASH_FILE_TYPE_DATA) {
        firstBlock  = gyVmuFlashUserDataBlocks(dev)-1;
        lastBlock   = 0;
    } else assert(0); //NO file type? What the shit you trying to pull?

    //Iterate until unallocated block is found.
    int i = firstBlock;
    for(;;) {
        if(fat[i] == VMU_FLASH_BLOCK_FAT_UNALLOCATED) {
            //Claim block
            block = i;
            /* Assume this is the last block in the sequence,
             * until it's passed to the alloc function later
             * as the previous block of a new allocation.
             */
            fat[i] = VMU_FLASH_BLOCK_FAT_LAST_IN_FILE;
            //Zero out contents of block
            memset(gyVmuFlashBlock(dev, block), 0, VMU_FLASH_BLOCK_SIZE);
            //Update fat entry if not first block in series
            if(previous != VMU_FLASH_BLOCK_FAT_UNALLOCATED &&
               previous != VMU_FLASH_BLOCK_FAT_LAST_IN_FILE)
                fat[previous] = block;
            break;
        }


        if(i == lastBlock) break;

        if(fileType == VMU_FLASH_FILE_TYPE_GAME) {
            ++i;
        } else {
            --i;
        }
    }

    return block;
}

/* It's not recommended that you just delete individual blocks randomly.
 * Whatever the previous block was that was pointing to this current block
 * is now a dangling pointer to an unallocated block once it's freed.
 */
void gyVmuFlashBlockFree(VMUDevice* dev, uint16_t block) {
    assert(block < gyVmuFlashBlockCount(dev));
    memset(gyVmuFlashBlock(dev, block), 0, VMU_FLASH_BLOCK_SIZE);
    *gyVmuFlashBlockFATEntry(dev, block) = VMU_FLASH_BLOCK_FAT_UNALLOCATED;
}






//=== FLASH DIRECTORY API ====


VMUFlashDirEntry* gyVmuFlashDirEntryByIndex(VMUDevice* dev, uint16_t index) {
    return ((VMUFlashDirEntry*)gyVmuFlashBlock(dev, gyVmuFlashBlockDirectory(dev))+index);
}

VMUFlashDirEntry* gyVmuFlashDirEntryFind(struct VMUDevice* dev, const char* name) {
    VMUFlashDirEntry* entry = NULL;
    uint16_t block          = 0;
    const size_t len        = strlen(name);
    int match = 0;

    for(int b = 0; b < gyVmuFlashFileCount(dev); ++b) {
        entry = gyVmuFlashDirEntryByIndex(dev, b);

        if(entry->fileType != VMU_FLASH_FILE_TYPE_DATA ||
           entry->fileType != VMU_FLASH_FILE_TYPE_GAME)
                continue;

        match = 1;

        for(unsigned i = 0; i < VMU_FLASH_DIRECTORY_FILE_NAME_SIZE && i < len; ++i) {
            if(entry->fileName[i] != name[i]) {
                match = 0;
                entry = NULL;
                break;
            }
        }

        if(match) break;
    }

    return match? entry : NULL;
}

VMUFlashDirEntry* gyVmuFlashDirEntryGame(struct VMUDevice* dev) {
    VMUFlashDirEntry* entry = NULL;
    for(unsigned i = 0; i < gyVmuFlashBlockRoot(dev)->dirSize; ++i) {
        entry = gyVmuFlashDirEntryByIndex(dev, i);
        if(entry && entry->fileType == VMU_FLASH_FILE_TYPE_GAME)
            return entry;
    }

    return NULL;
}


uint8_t gyVmuFlashDirEntryIndex(const struct VMUDevice* dev, const VMUFlashDirEntry* entry) {
    const unsigned char* dir = gyVmuFlashBlock(dev, gyVmuFlashBlockDirectory(dev));
    const ptrdiff_t diff = (const unsigned char*)entry - dir;
    assert(diff % sizeof(VMUFlashDirEntry) == 0);
    return diff/sizeof(VMUFlashDirEntry);
}

VMUFlashDirEntry* gyVmuFlashDirEntryDataNext(struct VMUDevice* dev, const VMUFlashDirEntry* prev) {
    const VMUFlashDirEntry*     entry       = NULL;
    uint16_t                    dirIndex    = prev? gyVmuFlashDirEntryIndex(dev, prev) : 0;

    if(prev) {
        //Check if we're at the end
        if(dirIndex == gyVmuFlashDirEntryCount(dev)-1)
            return NULL; //There is no next entry
    } else { //No previous entry, beginning iteration
        //Grab first entry
        prev = gyVmuFlashDirEntryByIndex(dev, 0);
        //Return first entry if it's valid
        if(prev && prev->fileType == VMU_FLASH_FILE_TYPE_DATA) {
            return prev;
        } else return NULL;
    }

    //Start iterating from the next index, looking for the next data file
    for(dirIndex = dirIndex + 1; dirIndex < gyVmuFlashDirEntryCount(dev); ++dirIndex) {
        prev = gyVmuFlashDirEntryByIndex(dev, dirIndex);
        //Found next data file, break
        if(prev->fileType == VMU_FLASH_FILE_TYPE_DATA) {
            entry = prev;
            break;
        }
    }

    return entry; //Should still be NULL if next data file was never found.
}

/* Don't need to update FAT entries, since it should always have the directory
 * as allocated, regardless of whether or not directory entries are currently free.
 */
VMUFlashDirEntry* gyVmuFlashDirEntryAlloc(struct VMUDevice* dev) {
    for(unsigned i = 0; i < gyVmuFlashDirEntryCount(dev); ++i) {
        VMUFlashDirEntry* entry = gyVmuFlashDirEntryByIndex(dev, i);
        if(entry->fileType == VMU_FLASH_FILE_TYPE_NONE) {
            return entry;
        }
    }
    return NULL;
}

//Don't have to update FAT table either, data is still ALLOCATED to directory, even if entry is blank.
void gyVmuFlashDirEntryFree(struct VMUFlashDirEntry* entry) {
    memset(entry, 0, sizeof(VMUFlashDirEntry));
}

void gyVmuFlashDirEntryPrint(const VMUDevice* dev, const VMUFlashDirEntry* entry) {

    _gyLog(GY_DEBUG_VERBOSE, "Flash Directory Entry - %d", gyVmuFlashDirEntryIndex(dev, entry));
    _gyPush();

    char fileNameBuff[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE+1] = { 0 };
    memcpy(fileNameBuff, entry->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "File Type",
           (entry->fileType == VMU_FLASH_FILE_TYPE_GAME)? "GAME" :
           (entry->fileType == VMU_FLASH_FILE_TYPE_DATA)? "DATA" : "NONE");
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Copy Protection",
           (entry->copyProtection == VMU_FLASH_COPY_PROTECTION_COPY_OK)? "NONE" : "PROTECTED");
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "First Block",              entry->firstBlock);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "File Name",                fileNameBuff);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Century", gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_CENTURY]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Year",    gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_YEAR]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Month",   gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_MONTH]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Day",     gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_DAY]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Hour",    gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_HOUR]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Minute",  gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_MINUTE]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Second",  gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_SECOND]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Creation Weekday", gyVmuFromBCD(entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_WEEKDAY]));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "File Size (Blocks)",       entry->fileSize);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Header Offset (Blocks)",   entry->headerOffset);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Unused",                   *(uint32_t*)entry->unused);

    _gyPop(1);

}





//==== HIGH-LEVEL FILE API =====

#if 0

int gyVmuFlashFileRead(struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, unsigned char* buffer, int includeHeader) {
    const unsigned char* chunk;
    unsigned char*       bufferPtr   = buffer;            //Points current block within buffer
    uint16_t    block       = entry->firstBlock; //Starting block is retrieved from Directory

    do {
        //Copy single block into buffer
        chunk = gyVmuFlashBlockReadOnly(dev, block);
        memcpy(bufferPtr, chunk, VMU_FLASH_BLOCK_SIZE);

        //Advance buffer pointer
        bufferPtr += VMU_FLASH_BLOCK_SIZE;

    } while((block = gyVmuFlashBlockNext(dev, block)) != VMU_FLASH_BLOCK_FAT_LAST_IN_FILE);

    //Return number of bytes copied
    return bufferPtr - buffer;
}
#endif

int gyVmuFlashCheckFormatted(const VMUDevice *dev) {
    const unsigned char* root = gyVmuFlashBlock(dev, VMU_FLASH_BLOCK_ROOT);

    for(unsigned i = 0; i < 0xf; ++i)
        if(root[i] != VMU_FLASH_BLOCK_ROOT_FORMATTED_BYTE)
            return 0;

    return 1;
}


uint16_t gyVmuFlashUserDataBlocks(const struct VMUDevice* dev) {
    return gyVmuFlashBlockRoot((VMUDevice*)dev)->userBlocks;
}

int gyVmuFlashUserDataUnlockUnusedBlocks(struct VMUDevice* dev) {
    (void)dev;
    return 0;
}

VMUFlashMemUsage gyVmuFlashMemUsage(const struct VMUDevice* dev) {
    VMUFlashMemUsage    mem     = { 0, 0 };

    for(uint16_t b = 0; b < gyVmuFlashUserDataBlocks(dev); ++b) {
        const uint16_t* fatEntry = gyVmuFlashBlockFATEntry(dev, b);

        if(*fatEntry != VMU_FLASH_BLOCK_FAT_UNALLOCATED)
            ++mem.blocksUsed;
        else
            ++mem.blocksFree;
    }

    //mem.blocksFree = gyVmuFlashUserDataBlocks(dev) - mem.blocksUsed;

    return mem;
}


int gyVmuFlashFileDelete(struct VMUDevice* dev, VMUFlashDirEntry* entry) {
    assert(entry);
    int blocksFreed = 0;
    char name[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE+1] = { '\0' };

    gyVmuFlashDirEntryName(entry, name);

    _gyLog(GY_DEBUG_VERBOSE, "Deleting file from flash: [%s]", name);
    _gyPush();
    uint16_t block = entry->firstBlock;

    while(block != VMU_FLASH_BLOCK_FAT_LAST_IN_FILE) {
        _gyLog(GY_DEBUG_VERBOSE, "[%d] Freeing block: [%d]", blocksFreed++, block);

        uint16_t nextBlock = gyVmuFlashBlockNext(dev, block);
        if(nextBlock == VMU_FLASH_BLOCK_FAT_UNALLOCATED) {
            _gyLog(GY_DEBUG_ERROR, "[%d] Unallocated block in the middle of file chain! [%d]", blocksFreed+1, block);
            return blocksFreed;
        }

        gyVmuFlashBlockFree(dev, block);

        block = nextBlock;
    }

    //Clear directory entry
    gyVmuFlashDirEntryFree(entry);
    _gyPop(1);
    return blocksFreed;
}

VMUFlashDirEntry* gyVmuFlashFileCreate(VMUDevice* dev, const VMUFlashNewFileProperties* properties, const unsigned char* data, VMU_LOAD_IMAGE_STATUS* status) {
    int blocks[gyVmuFlashUserDataBlocks(dev)];
    VMUFlashDirEntry* entry = NULL;

    //Can't assume filename is null terminated.
    char fileNameBuff[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE+1] = { 0 };
    memcpy(fileNameBuff, properties->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);

    _gyLog(GY_DEBUG_VERBOSE, "VMU Flash - Creating file [%s].", fileNameBuff);
    _gyPush();

    //=== 1 - Check if we're creating a GAME file while one already exists. ===
    if(properties->fileType == VMU_FLASH_FILE_TYPE_GAME && gyVmuFlashDirEntryGame(dev)) {
        _gyLog(GY_DEBUG_ERROR, "Only one GAME file can be present at a time!");
        _gyPop(1);
        *status = VMU_LOAD_IMAGE_GAME_DUPLICATE;
        return NULL;
    }

    //=== 2 - Make sure we don't already have a file with the same name. ===
    if(gyVmuFlashDirEntryFind(dev, properties->fileName)) {
        _gyLog(GY_DEBUG_ERROR, "File already present with the same name!");
        _gyPop(1);
        *status = VMU_LOAD_IMAGE_NAME_DUPLICATE;
        return NULL;
    }

    //=== 3 - Check whether there are enough free blocks available for the file. ===
    VMSFileInfo* vmsHeader = (VMSFileInfo*)(properties->fileType == VMU_FLASH_FILE_TYPE_GAME)?
                &data[VMU_FLASH_GAME_VMS_HEADER_OFFSET] : data;
    unsigned totalBytes      = properties->fileSizeBytes;
    unsigned blocksRequired  = gyVmuFlashBytesToBlocks(properties->fileSizeBytes);

    VMUFlashMemUsage memUsage = gyVmuFlashMemUsage(dev);
    if(memUsage.blocksFree < blocksRequired) {
        sprintf(_loadImageErrorMsg, "Not enough free blocks left on memory unit! [Free: %d, Required: %d]",
                memUsage.blocksFree, blocksRequired);

        _gyLog(GY_DEBUG_ERROR, _loadImageErrorMsg);
        _gyPop(1);
        *status = VMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS;
        return NULL;
    }

    /* Game data must all be stored contiguously starting at block 0,
     * so check whether memory card requires defrag.
     */
    if(properties->fileType == VMU_FLASH_FILE_TYPE_GAME) {
        unsigned contiguousBlocks = gyVmuFlashContiguousFreeBlocks(dev);

        //Defragment card if we couldn't find enough contiguous blocks.
        if(contiguousBlocks < blocksRequired) {
            _gyLog(GY_DEBUG_WARNING, "Not enough contiguous blocks available for GAME file [%d/%d]. Defrag required.",
                   contiguousBlocks, blocksRequired);

            if(!gyVmuFlashDefragment(dev)) {
                *status = VMU_LOAD_IMAGE_DEFRAG_FAILED;
                goto end;
            }

            contiguousBlocks = gyVmuFlashContiguousFreeBlocks(dev);
            if(contiguousBlocks < blocksRequired) {
                _gyLog(GY_DEBUG_ERROR, "Still not enough contiguous blocks available [%d/%d], Defrag must have failed!",
                       contiguousBlocks, blocksRequired);
                *status = VMU_LOAD_IMAGE_DEFRAG_FAILED;
                goto end;
            }
        }
    }

    //=== 4 - Create Flash Directory Entry for file. ===
    entry = gyVmuFlashDirEntryAlloc(dev);
    if(!entry) {
        _gyLog(GY_DEBUG_ERROR, "Could not allocate entry in Flash Directory (too many files present).");
        *status = VMU_LOAD_IMAGE_FILES_MAXED;
        _gyPop(1);
        return NULL;
    }

    _gyLog(GY_DEBUG_VERBOSE, "Creating Flash Directory Entry [index: %d]", gyVmuFlashDirEntryIndex(dev, entry));
    _gyPush();

    //Fill in Flash Directory Entry for file
    memset(entry, 0, sizeof(VMUFlashDirEntry));
    memcpy(entry->fileName, properties->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);
    entry->copyProtection   = properties->copyProtection;
    entry->fileType         = properties->fileType;
    entry->fileSize         = blocksRequired;
    entry->headerOffset     = (entry->fileType == VMU_FLASH_FILE_TYPE_DATA)? 0 : 1;

    //Add timestamp to directory
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_CENTURY]  = gyVmuToBCD(tm->tm_year/100+19);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_YEAR]     = gyVmuToBCD(tm->tm_year%100);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_MONTH]    = gyVmuToBCD(tm->tm_mon+1);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_DAY]      = gyVmuToBCD(tm->tm_mday);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_HOUR]     = gyVmuToBCD(tm->tm_hour);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_MINUTE]   = gyVmuToBCD(tm->tm_min);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_SECOND]   = gyVmuToBCD(tm->tm_sec);
    entry->timeStamp[VMU_FLASH_DIRECTORY_DATE_WEEKDAY]  = gyVmuToBCD(gyVmuWeekDay());

    gyVmuFlashDirEntryPrint(dev, entry);
    _gyPop(1);

    //=== 5 - Allocate FAT Blocks for File ===
    _gyLog(GY_DEBUG_VERBOSE, "Allocating FAT Blocks for file [Blocks: %d].", blocksRequired);
    _gyPush();

    //Allocate FAT blocks to hold the image, chain them togethers
    memset(blocks, -1, sizeof(int)*gyVmuFlashUserDataBlocks(dev));
    for(unsigned b = 0; b < blocksRequired; ++b) {
        blocks[b] = gyVmuFlashBlockAlloc(dev, (b > 0)? blocks[b-1] : VMU_FLASH_BLOCK_FAT_UNALLOCATED, properties->fileType);

        if(blocks[b] == VMU_FLASH_BLOCK_FAT_UNALLOCATED) {
            _gyLog(GY_DEBUG_ERROR, "Failed to allocate FAT block: [%d/%d]", b, blocksRequired);
            *status = VMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS;
            goto clean_fat_blocks;
        }
    }

    _gyPop(1);

    //=== 6 - Write VMS File  ===
    _gyLog(GY_DEBUG_VERBOSE, "Writing VMS File Data.");
    _gyPush();

    const unsigned  blocksToWrite   = gyVmuFlashBytesToBlocks((int)totalBytes);
    unsigned        bytesLeft       = totalBytes;

    for(unsigned b = 0; b < blocksToWrite; ++b) {
        const int bytesForBlock =
                (bytesLeft > VMU_FLASH_BLOCK_SIZE)? VMU_FLASH_BLOCK_SIZE : bytesLeft;

        unsigned char* block = gyVmuFlashBlock(dev, blocks[b]);
        if(!block) {
            _gyLog(GY_DEBUG_ERROR, "Failed to retrieve block [%d] while writing data: [%d/%d]", blocks[b], b, blocksToWrite);
            *status = VMU_LOAD_IMAGE_DEVICE_READ_ERROR;
            goto clean_fat_blocks;
        }

        memcpy(block,
               (data + b*VMU_FLASH_BLOCK_SIZE),
               bytesForBlock);

        const uint16_t* fatEntry = gyVmuFlashBlockFATEntry(dev, blocks[b]);
        assert(fatEntry);

        _gyLog(GY_DEBUG_VERBOSE, "[%d] Writing %d bytes. [Block: %d, Next: %d]",
               b, bytesForBlock, blocks[b], fatEntry? *fatEntry : -1);

        bytesLeft -= bytesForBlock;
    }
    entry->firstBlock = blocks[0];

    if(bytesLeft != 0) {
        _gyLog(GY_DEBUG_VERBOSE, "Failed to write entire file: [%d/%d bytes]", totalBytes - bytesLeft, totalBytes);
        *status = VMU_LOAD_IMAGE_DEVICE_WRITE_ERROR;
        goto clean_fat_blocks;
    }

    const VMSFileInfo* vms = gyVmuFlashDirEntryVmsHeader(dev, entry);
    if(!vms) {
        _gyLog(GY_DEBUG_ERROR, "Could not verify VMS header that was written to device!");
        *status = VMU_LOAD_IMAGE_DEVICE_READ_ERROR;
        goto clean_fat_blocks;
    }
    gyVmuPrintVMSFileInfo(vms);

    //Assuming shit all worked out successfully!
    *status = VMU_LOAD_IMAGE_SUCCESS;
    goto end;

clean_fat_blocks:
    for(unsigned b = 0; b < gyVmuFlashUserDataBlocks(dev); ++b) {
        if(blocks[b] == -1) break;
        gyVmuFlashBlockFree(dev, blocks[b]);
    }
clean_dir_entry:
    entry->fileType = VMU_FLASH_FILE_TYPE_NONE;
    entry = NULL;
end:
    _gyPop(2);

    return entry;
}


int gyVmuFlashContiguousFreeBlocks(const struct VMUDevice* dev) {
    int contiguousBlocks    = 0;
    unsigned userDataBlocks = gyVmuFlashUserDataBlocks(dev);

    for(unsigned i = 0; i < userDataBlocks; ++i) {
        if(*gyVmuFlashBlockFATEntry((VMUDevice*)dev, i) != VMU_FLASH_BLOCK_FAT_UNALLOCATED) {
            break;
        } else {
            ++contiguousBlocks;
        }
    }

    return contiguousBlocks;
}

int gyVmuFlashFileRead(struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, unsigned char* buffer, int includeHeader) {
    size_t bytesRead = 0;
    const size_t byteSize = entry->fileSize * VMU_FLASH_BLOCK_SIZE;
    bytesRead = gyVmuFlashFileReadBytes(dev, entry, buffer, byteSize, entry->headerOffset * VMU_FLASH_BLOCK_SIZE, includeHeader);
    return (bytesRead == byteSize)? 1 : 0;
}

int gyVmuFlashDefragment(struct VMUDevice* dev) {
    VMUDevice tempDevice;
    unsigned char tempFlash[FLASH_SIZE] = { '\0' };

    int success = 1;
    _gyLog(GY_DEBUG_VERBOSE, "Defragmenting VMU Flash Storage");
    _gyPush();

    const int fileCount                 = gyVmuFlashFileCount(dev);
    const VMUFlashMemUsage origMemUsage = gyVmuFlashMemUsage(dev);

    if(fileCount) {
        memcpy(&tempDevice, dev, sizeof(VMUDevice));

        _gyLog(GY_DEBUG_VERBOSE, "Uninstalling all files.");
        _gyPush();

        VMUFlashDirEntry* dirEntries[gyVmuFlashDirEntryCount(dev)];
        memset(dirEntries, 0, sizeof(VMUFlashDirEntry*)*gyVmuFlashDirEntryCount(dev));

        for(int f = 0; f < fileCount; ++f) {
            dirEntries[f] = gyVmuFlashFileAtIndex(dev, f);
        }

        for(int f = 0; f < fileCount; ++f) {
            VMUFlashDirEntry* entry = dirEntries[f];
            if(!entry) {
                _gyLog(GY_DEBUG_ERROR, "Failed to retrieve directory entry for file: [index %d]", f);
                goto failure;
            }

            int freeBlocks = gyVmuFlashFileDelete(dev, entry);
            if(!freeBlocks) {
                _gyLog(GY_DEBUG_ERROR, "Deleting file [%d] didn't free any blocks on device!", f);
                goto failure;
            }
        }

        //Ensure we actually got the device to a clean state
        int newFileCount = gyVmuFlashFileCount(dev);
        if(newFileCount) {
            _gyLog(GY_DEBUG_ERROR, "Not all files were deleted: [%d files]", newFileCount);
            goto failure;
        }

        VMUFlashMemUsage memUsage = gyVmuFlashMemUsage(dev);
        if(memUsage.blocksUsed) {
            _gyLog(GY_DEBUG_ERROR, "Not all blocks were freed: [%d remaining]", memUsage.blocksFree);
            goto failure;
        }

        _gyPop(1);

        _gyLog(GY_DEBUG_VERBOSE, "Reinstalling all files.");
        _gyPush();
        for(int f = 0; f < fileCount; ++f) {
            unsigned char tempImage[FLASH_SIZE];
            //Read file from temporary device to temporary buffer
            const VMUFlashDirEntry* entry = gyVmuFlashFileAtIndex(&tempDevice, f);
            if(!entry) {
                _gyLog(GY_DEBUG_ERROR, "Failed to retrieve directory entry on tempDevice for file: [file %d]", f);
                goto failure;
            }

            if(!gyVmuFlashFileRead(&tempDevice, entry, tempImage, 1)) {
                _gyLog(GY_DEBUG_ERROR, "Failed to extract file from temporary device: [file %d]", f);
                goto failure;
            }

            //Write file from buffer to device
            VMU_LOAD_IMAGE_STATUS status;
            VMUFlashNewFileProperties fileProperties;
            gyVmuFlashNewFilePropertiesFromDirEntry(&fileProperties, entry);
            entry = gyVmuFlashFileCreate(dev, &fileProperties, tempImage, &status);
            if(!entry || status != VMU_LOAD_IMAGE_SUCCESS) {
                _gyLog(GY_DEBUG_ERROR, "Failed to write file back to device: [file %d]", f);
                goto failure;
            }
        }

        //Verify that everything has been reinstalled.
        memUsage = gyVmuFlashMemUsage(dev);
        newFileCount = gyVmuFlashFileCount(dev);
        if(newFileCount != fileCount) {
            _gyLog(GY_DEBUG_ERROR, "Final file count does not match initial file count! [%d => %d]", fileCount, newFileCount);
            goto failure;
        }

        if(memUsage.blocksUsed != origMemUsage.blocksUsed) {
            _gyLog(GY_DEBUG_ERROR, "Final used block count does not match initial used block count! [%d => %d]", origMemUsage.blocksUsed, memUsage.blocksUsed);
            goto failure;
        }

        _gyPop(1);

    }

    goto end;


failure:
    success = 0;
    memcpy(dev, &tempDevice, sizeof(VMUDevice));
    _gyPop(1);
end:
    _gyPop(1);
    return success;
}

int gyVmuFlashFileCount(const struct VMUDevice* dev) {
    int count = 0;
    const VMUFlashDirEntry* entry = NULL;

    for(int i = gyVmuFlashDirEntryCount(dev)-1; i >= 0; --i) {
        entry = gyVmuFlashDirEntryByIndex((VMUDevice*)dev, (uint16_t)i);
        if(entry && entry->fileType == VMU_FLASH_FILE_TYPE_DATA || entry->fileType == VMU_FLASH_FILE_TYPE_GAME) {
            ++count;
        }
    }
    return count;
}

void gyVmuFlashDirEntryName(const VMUFlashDirEntry* entry, char* buffer) {
    memcpy(buffer, entry->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);
    buffer[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE] = '\0';
    for(int i = VMU_FLASH_DIRECTORY_FILE_NAME_SIZE-1; i >= 0; --i) {
        if(buffer[i] == ' ') buffer[i] = '\0';
        else return;
    }
}

const VMSFileInfo* gyVmuFlashDirEntryVmsHeader(const VMUDevice* dev, const struct VMUFlashDirEntry* dirEntry) {
    return (VMSFileInfo*)gyVmuFlashBlock(dev, dirEntry->firstBlock + dirEntry->headerOffset);
}

VMUFlashDirEntry* gyVmuFlashFileAtIndex(const VMUDevice* dev, int fileIdx) {
    int count = 0;
    VMUFlashDirEntry* entry = NULL;

    for(int i = gyVmuFlashDirEntryCount(dev)-1; i >= 0; --i) {
        entry = gyVmuFlashDirEntryByIndex((VMUDevice*)dev, (uint8_t)i);
        if(entry->fileType == VMU_FLASH_FILE_TYPE_DATA || entry->fileType == VMU_FLASH_FILE_TYPE_GAME) {
            if(fileIdx == count) {
                return entry;
            }
            if(++count > fileIdx) break;
        }
    }
    return NULL;
}

size_t gyVmuFlashFileReadBytes(struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, unsigned char* buffer, size_t bytes, size_t offset, int includeHeader) {
    assert(entry && buffer);
    size_t bytesRead = 0;
    
    const VMSFileInfo* vmsHeader = gyVmuFlashDirEntryVmsHeader(dev, entry);
    if(!vmsHeader) return bytesRead;
    //else if(!includeHeader) startOffset += gyVmuVmsFileInfoHeaderSize(vmsHeader);

    size_t startBlockNum = offset / VMU_FLASH_BLOCK_SIZE;
    size_t endBlockNum = (offset + bytes) / VMU_FLASH_BLOCK_SIZE;

    size_t numBlocks = endBlockNum - startBlockNum;
    uint16_t curBlock = entry->firstBlock;

    //Seek to startBlock
    for(unsigned b = 0; b < startBlockNum; ++b) {
        curBlock = gyVmuFlashBlockNext(dev, curBlock);
        if(curBlock == VMU_FLASH_BLOCK_FAT_UNALLOCATED) return bytesRead; //FUCKED
    }

    //Read start block (starting at offset)
    if(includeHeader || entry->headerOffset != curBlock) {
        size_t startBlockByteOffset = offset % VMU_FLASH_BLOCK_SIZE;
        size_t startBlockBytesLeft = VMU_FLASH_BLOCK_SIZE - startBlockByteOffset;
        size_t startBlockBytes = (bytes < startBlockBytesLeft)? bytes : startBlockBytesLeft;
        const unsigned char* firstBlockData = gyVmuFlashBlock(dev, curBlock);
        if(!firstBlockData) return bytesRead;
        memcpy(buffer, firstBlockData+startBlockByteOffset, startBlockBytes);
        bytesRead += startBlockBytes;
    }
    curBlock = gyVmuFlashBlockNext(dev, curBlock);

    //Read each block beyond the start block
    //for(unsigned b = 1; b < numBlocks; ++b) {
    while(bytesRead < bytes) {
        const unsigned char* data = gyVmuFlashBlock(dev, curBlock);
        if(!data) break; // Jesus fucking CHRIST

        if(includeHeader || curBlock != entry->headerOffset) {
            const size_t bytesLeft = bytes - bytesRead;
            const size_t byteCount = (VMU_FLASH_BLOCK_SIZE < bytesLeft)?
                        VMU_FLASH_BLOCK_SIZE : bytesLeft;

            memcpy(&buffer[bytesRead], data, byteCount);
            bytesRead += byteCount;
        }

        curBlock = gyVmuFlashBlockNext(dev, curBlock);
        if(curBlock == VMU_FLASH_BLOCK_FAT_UNALLOCATED) break;
    }

    return bytesRead;
}

void gyVmuFlashNewFilePropertiesFromVmi(VMUFlashNewFileProperties* fileProperties, const VMIFileInfo* vmi) {
    assert(fileProperties && vmi);

    memcpy(fileProperties->fileName, vmi->fileNameOnVms, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);
    fileProperties->fileSizeBytes = vmi->fileSize;

    switch((vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK) >> VMU_VMI_FILE_INFO_FILE_MODE_GAME_POS) {
    case VMI_FILE_MODE_GAME_DATA:
        fileProperties->fileType = VMU_FLASH_FILE_TYPE_DATA;
        break;
    case VMI_FILE_MODE_GAME_GAME:
        fileProperties->fileType = VMU_FLASH_FILE_TYPE_GAME;
        break;
    default:
        fileProperties->fileType = VMU_FLASH_FILE_TYPE_NONE;
    }

    switch((vmi->fileMode&VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_MASK) >> VMU_VMI_FILE_INFO_FILE_MODE_PROTECT_POS) {
    case VMI_FILE_MODE_PROTECTED_COPY_OK:
        fileProperties->copyProtection = VMU_FLASH_COPY_PROTECTION_COPY_OK;
        break;
    case VMI_FILE_MODE_PROTECTED_COPY_PROTECTED:
        fileProperties->copyProtection = VMU_FLASH_COPY_PROTECTION_COPY_PROTECTED;
        break;
    default:
        fileProperties->copyProtection = VMU_FLASH_COPY_PROTECTION_COPY_UNKNOWN;
    }
}

void gyVmuFlashNewFilePropertiesFromDirEntry(VMUFlashNewFileProperties* fileProperties, const VMUFlashDirEntry* entry) {
    assert(fileProperties && entry);

    memcpy(fileProperties->fileName, entry->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);
    fileProperties->fileSizeBytes = entry->fileSize * VMU_FLASH_BLOCK_SIZE;
    fileProperties->fileType = entry->fileType;
    fileProperties->copyProtection = entry->copyProtection;
}

const VMUFlashDirEntry* gyVmuFlashLoadImageVmsVmi(struct VMUDevice* dev, const char* vmiPath, const char* vmsPath, VMU_LOAD_IMAGE_STATUS* status) {
    const VMUFlashDirEntry* dirEntry = NULL;
    VMIFileInfo vmi;

    _gyLog(GY_DEBUG_VERBOSE, "Loading VMI/VMS images into flash: [%s]", vmsPath);
    _gyPush();

    if(!gyVmuFlashLoadVMI(&vmi, vmiPath)) {
        _gyLog(GY_DEBUG_ERROR, "Failed to load VMI file: [%s]", vmiPath);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;

    } else {
        gyVmuFlashPrintVMIFileInfo(&vmi);
        const VMI_FILE_MODE_GAME vmiFileType = (VMI_FILE_MODE_GAME)(vmi.fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK);

        //Check if we're trying to load a game and already have a game!
        if(vmiFileType == VMI_FILE_MODE_GAME_GAME && gyVmuFlashDirEntryGame(dev)) {
            _gyLog(GY_DEBUG_WARNING, "Game file already exists in current flash image!");
            *status = VMU_LOAD_IMAGE_GAME_DUPLICATE;
        } else {

            GYFile* fp = NULL;
            int retVal = gyFileOpen(vmsPath, "rb", &fp);

            if(!retVal || !fp) {
                _gyLog(GY_DEBUG_ERROR, "Failed to open VMS file: [%s]!", vmsPath);
                *status = VMU_LOAD_IMAGE_OPEN_FAILED;
            } else {
                size_t fileSize = 0;
                retVal = gyFileLength(fp, &fileSize);

                if(!retVal || !fileSize) {
                    _gyLog(GY_DEBUG_ERROR, "Failed to retrieve file size.");
                    *status = VMU_LOAD_IMAGE_READ_FAILED;

                } else {
                    size_t blockSize = fileSize / VMU_FLASH_BLOCK_SIZE;
                    VMUFlashMemUsage flashMem = gyVmuFlashMemUsage(dev);

                    if(flashMem.blocksFree < blockSize) {
                        _gyLog(GY_DEBUG_ERROR, "VMU does not have enough free blocks to load file: [%d/%d]", flashMem.blocksFree, blockSize);
                        *status = VMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS;
                    } else {

                        unsigned char* data = malloc(fileSize);
                        size_t bytesRead = 0;
                        retVal = gyFileRead(fp, data, fileSize, &bytesRead);

                        if(!retVal || bytesRead != fileSize) {
                            _gyLog(GY_DEBUG_ERROR, "Failed to read file contents! [Bytes read: %d/%d]", bytesRead, fileSize);
                            *status = VMU_LOAD_IMAGE_READ_FAILED;
                        } else {
                            VMSFileInfo* vms = (vmiFileType == VMI_FILE_MODE_GAME_GAME)?
                                        (VMSFileInfo*)&data[VMU_FLASH_GAME_VMS_HEADER_OFFSET] :
                                        (VMSFileInfo*)data;

                            _gyLog(GY_DEBUG_VERBOSE, "Loaded VMS Image into RAM. [%d bytes]", bytesRead);
                            gyVmuPrintVMSFileInfo(vms);

                            VMUFlashNewFileProperties fileProperties;
                            gyVmuFlashNewFilePropertiesFromVmi(&fileProperties, &vmi);

                            dirEntry = gyVmuFlashFileCreate(dev, &fileProperties, data, status);
                        }

                        free(data);
                    }
                }

                retVal = gyFileClose(&fp);
                if(!retVal) {
                    _gyLog(GY_DEBUG_ERROR, "Failed to close file handler!");
                }
            }
        }
    }
    _gyPop(1);
    return dirEntry;
}


int gyVmuVmiFindVmsPath(const char* vmiPath, char* vmsPath) {
    int success = 0;
    char basePath[GYRO_PATH_MAX_SIZE] = { '\0' };
    char tmpVmiPath[GYRO_PATH_MAX_SIZE] = { '\0' };

    _gyLog(GY_DEBUG_VERBOSE, "Trying to find VMS file corresponding VMI: [%s]", vmiPath);
    _gyPush();

    strcpy(tmpVmiPath, vmiPath);
    const char*curTok  = strtok(tmpVmiPath, ".");
    strcpy(basePath, curTok);

    GYFile* fp = NULL;
    strcpy(vmsPath, basePath);
    strcat(vmsPath, ".vms");
    _gyLog(GY_DEBUG_VERBOSE, "Trying the same filename with .VMS extension: [%s]", vmsPath);
    _gyPush();
    int retVal = gyFileOpen(vmsPath, "rb", &fp);
    int wasOpen = (int)fp;
    gyFileClose(&fp);

    if(retVal && wasOpen) {
        _gyLog(GY_DEBUG_VERBOSE, "Success! File exists.");
        _gyPop(1);
        success = 1;
        goto end;
    }
    _gyLog(GY_DEBUG_VERBOSE, "Nope.");
    _gyPop(1);
    VMIFileInfo vmiHeader;
    char vmsFileName[VMU_VMI_FILE_INFO_VMS_RESOURCE_NAME_SIZE] = { '\0' };

    if(gyVmuFlashLoadVMI(&vmiHeader, vmiPath)) {
        gyVmuVmiFileInfoResourceNameGet(&vmiHeader, vmsFileName);

        for(int i = strlen(basePath)-1; i >= 0; --i) {
            if(basePath[i] == '\\' || basePath[i] == '/') {
                basePath[i+1] = '\0';
                break;
            }
        }

        strcat(basePath, vmsFileName);
        strcat(basePath, ".vms");

        _gyLog(GY_DEBUG_VERBOSE, "Trying filename referenced by VM file: [%s]", basePath);
        _gyPush();

        fp = NULL;
        retVal = gyFileOpen(basePath, "rb", &fp);
        wasOpen = (int)fp;
        gyFileClose(&fp);

        if(retVal && wasOpen) {
            _gyLog(GY_DEBUG_VERBOSE, "Success. File exists.");
            strcpy(vmsPath, basePath);
            success = 1;

        } else {
            success = 0;
        }
        _gyPop(1);

    } else {
        success = 0;
    }

end:
    if(!success) _gyLog(GY_DEBUG_WARNING, "None found!");
    else _gyLog(GY_DEBUG_VERBOSE, "Found VMS: [%s]", vmsPath);
    _gyPop(1);
    return success;
}

int gyVmuVmsFindVmiPath(const char* vmsPath, char* vmiPath) {
    assert(vmsPath);
    int success = 0;
    char basePath[GYRO_PATH_MAX_SIZE] = { '\0' };
    char tmpVmsPath[GYRO_PATH_MAX_SIZE] = { '\0' };
    _gyLog(GY_DEBUG_VERBOSE, "Trying to find VMI file corresponding VMS: [%s]", vmsPath);
    _gyPush();

    strcpy(tmpVmsPath, vmsPath);
    const char*curTok  = strtok(tmpVmsPath, ".");
    strcpy(basePath, curTok);

    GYFile* fp = NULL;
    strcat(basePath, ".vmi");
    int retVal = gyFileOpen(basePath, "rb", &fp);
    int wasOpen = (int)fp;
    gyFileClose(&fp);

    if(!retVal || !wasOpen) {
        _gyLog(GY_DEBUG_WARNING, "None found!");
        success = 0;
    } else {
        strcpy(vmiPath, basePath);
        _gyLog(GY_DEBUG_VERBOSE, "Found VMI: [%s]", vmiPath);
        success = 1;
    }
    _gyPop(1);
    return success;
}

VMUFlashDirEntry* gyVmuFlashLoadImage(VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    char tempPath[GYRO_PATH_MAX_SIZE];
    char basePath[GYRO_PATH_MAX_SIZE];
    int success = 0;
    VMUFlashDirEntry* entry = NULL;

    strcpy(tempPath, path);
    const char*curTok  = strtok(tempPath, ".");
    strcpy(basePath, curTok);
    const char*prevTok = NULL;
    char ext[100] = { 0 };

    _gyLog(GY_DEBUG_VERBOSE, "Loading Generic Flash Image [%s]", path);
    _gyPush();

    if(dev->lcdFile) {
        gyVmuLcdFileUnload(dev->lcdFile);
        dev->lcdFile = NULL;
    }

    while(curTok) {
        prevTok = curTok;
        curTok = strtok(NULL, ".");
    }

    if(prevTok) {
        for(unsigned i = 0; i < strlen(prevTok); ++i)
            ext[i] = tolower(prevTok[i]);

        if(!strcmp(ext, "bin") || !strcmp(ext, "vmu")) {
            entry = gyVmuFlashLoadImageBin(dev, path, status);
        } else if(!strcmp(ext, "dcm")) {
            entry = gyVmuFlashLoadImageDcm(dev, path, status);
        } else if(!strcmp(ext, "dci")) {
            entry = gyVmuFlashLoadImageDci(dev, path, status);

        } else if(!strcmp(ext, "vms")) {
            //We have to find the corresponding VMI file
            char vmiPath[GYRO_PATH_MAX_SIZE];
            if(gyVmuVmsFindVmiPath(path, vmiPath)) {
                entry = gyVmuFlashLoadImageVmiVms(dev, vmiPath, path, status);
            } else {
                *status = VMU_LOAD_IMAGE_VMS_NO_VMI;
            }
        } else if(!strcmp(ext, "vmi")) {
            //We have to find the corresponding VMS file
            char vmsPath[GYRO_PATH_MAX_SIZE];
            if(gyVmuVmiFindVmsPath(path, vmsPath)) {
                entry = gyVmuFlashLoadImageVmiVms(dev, path, vmsPath, status);
            } else {
                *status = VMU_LOAD_IMAGE_VMI_NO_VMS;
            }

        } else {
            *status = VMU_LOAD_IMAGE_UNKNOWN_FORMAT;
        }

    } else {
        _gyLog(GY_DEBUG_ERROR, "No extension found on file!");

    }

    _gyPop(1);
    return entry;

}

VMUFlashDirEntry* gyVmuFlashLoadImageVmiVms(struct VMUDevice* dev, const char* vmipath, const char* vmspath, VMU_LOAD_IMAGE_STATUS* status) {
    VMUFlashDirEntry* dirEntry = NULL;
    _gyLog(GY_DEBUG_VERBOSE, "Load VMI+VMS file pair: [vmi: %s, vms: %s]", vmipath, vmspath);
    _gyPush();

    VMIFileInfo vmi;
    if(!gyVmuFlashLoadVMI(&vmi, vmipath)) {
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        goto end;
    }
    gyVmuFlashPrintVMIFileInfo(&vmi);

    GYFile* vmsFp = NULL;
    int retVal = gyFileOpen(vmspath, "rb", &vmsFp);
    if(!retVal || !vmsFp) {
        _gyLog(GY_DEBUG_ERROR, "Could not open VMS file: [%s]", vmspath);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        goto end;
    }

    size_t fileBytes = 0;
    retVal = gyFileLength(vmsFp, &fileBytes);
    if(!retVal || !fileBytes) {
        _gyLog(GY_DEBUG_ERROR, "Could not retrieve VMS file length");
        *status = VMU_LOAD_IMAGE_READ_FAILED;
        goto cleanup_file;
    }

    unsigned char* vmsData = malloc(fileBytes);
    size_t bytesRead = 0;
    retVal = gyFileRead(vmsFp, vmsData, fileBytes, &bytesRead);
    if(!retVal || bytesRead != fileBytes) {
        _gyLog(GY_DEBUG_ERROR, "Could not read entire file contents! [bytes read %d/%d]", bytesRead, fileBytes);
        *status = VMU_LOAD_IMAGE_READ_FAILED;
        goto cleanup_data;
    }

    _gyLog(GY_DEBUG_VERBOSE, "Loaded VMS file into RAM. [%d bytes]", bytesRead);
    _gyPush();
    VMSFileInfo* vms = (VMSFileInfo*)((vmi.fileMode&VMU_VMI_FILE_INFO_FILE_MODE_GAME_MASK)>>VMU_VMI_FILE_INFO_FILE_MODE_GAME_POS == VMI_FILE_MODE_GAME_GAME)?
                &vmsData[VMU_FLASH_GAME_VMS_HEADER_OFFSET] :
                vmsData;
    gyVmuPrintVMSFileInfo(vms);
    _gyPop(1);

    VMUFlashNewFileProperties fileProperties;
    gyVmuFlashNewFilePropertiesFromVmi(&fileProperties, &vmi);
    dirEntry = gyVmuFlashFileCreate(dev, &fileProperties, vmsData, status);


cleanup_data:
    free(vmsData);
cleanup_file:
    retVal = gyFileClose(&vmsFp);
    if(!retVal) {
        _gyLog(GY_DEBUG_WARNING, "VMS file did not close gracefully!");
    }
end:
    _gyPop(1);
    return dirEntry;
}


VMUFlashDirEntry* gyVmuFlashLoadImageDcm(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    GYFile* file;

    _gyLog(GY_DEBUG_VERBOSE, "Loading DCM Flash Image from file: [%s]", path);
    _gyPush();

    if (!gyFileOpen(path, "rb", &file)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open binary file for reading!");
        _gyPop(1);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        return NULL;
    }

    //Clear ROM
    memset(dev->flash, 0, sizeof(dev->flash));

    size_t bytesRead   = 0;
    size_t bytesTotal  = 0;

    while(bytesTotal < sizeof(dev->flash)) {
        if(gyFileRead(file, dev->flash+bytesTotal, sizeof(dev->flash)-bytesTotal, &bytesRead)) {
            bytesTotal += bytesRead;
        } else break;
    }

    gyFileClose(&file);

    gyVmuFlashFromNexusByteOrder(dev->flash, FLASH_SIZE);

    _gyLog(GY_DEBUG_VERBOSE, "Read %d bytes.", bytesTotal);
    assert(bytesTotal >= 0);
    assert(bytesTotal == sizeof(dev->flash));

    _gyPop(1);
    *status = VMU_LOAD_IMAGE_SUCCESS;
    gyVmuFlashRootBlockPrint(dev);
    gyVmuFlashPrintFilesystem(dev);

    return NULL;
}

VMUFlashDirEntry* gyVmuFlashLoadImageDci(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    VMUFlashDirEntry tempEntry;
    VMUFlashDirEntry* entry = NULL;
    uint8_t dataBuffer[FLASH_SIZE] = { 0 };

    _gyLog(GY_DEBUG_VERBOSE, "Loading DCI image from file: [%s]", path);
    _gyPush();

    GYFile* fp = NULL;
    if(!gyFileOpen(path, "rb", &fp) || !fp) {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "Failed to open file!");
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        goto end;
    }

    size_t bytesRead = 0;
    if(!gyFileRead(fp, &tempEntry, sizeof(VMUFlashDirEntry), &bytesRead) ||
            bytesRead != sizeof(VMUFlashDirEntry))
    {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "Failed to read directory entry! [Bytes read: %d/%d]",
                 bytesRead,
                 sizeof(VMUFlashDirEntry));
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_READ_FAILED;
        goto cleanup_file;
    }

    VMUFlashMemUsage memUsage = gyVmuFlashMemUsage(dev);
    if(memUsage.blocksFree < tempEntry.fileSize) {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "Not enough free blocks available on device. [Available: %d, Required: %d]",
                 memUsage.blocksFree,
                 entry->fileSize);
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_INADEQUATE_FREE_BLOCKS;
        goto cleanup_file;
    }

    if(tempEntry.fileType == VMU_FLASH_FILE_TYPE_GAME && gyVmuFlashDirEntryGame(dev)) {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "Only one GAME file may be present at a time, and the current image already has one!");
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_GAME_DUPLICATE;
        goto cleanup_file;
    }

    if(gyVmuFlashDirEntryFind(dev, tempEntry.fileName)) {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "A file already exists with the same name!");
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_NAME_DUPLICATE;
        goto cleanup_file;
    }

    if(!(entry = gyVmuFlashDirEntryAlloc(dev))) {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "Failed to allocate new Flash Directory Entry!");
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_FILES_MAXED;
        goto cleanup_file;
    }

    const size_t fileSize = tempEntry.fileSize * VMU_FLASH_BLOCK_SIZE;
    if(!gyFileRead(fp, dataBuffer, fileSize, &bytesRead) ||
            bytesRead != fileSize)
    {
        snprintf(_loadImageErrorMsg,
                 sizeof(_loadImageErrorMsg),
                 "Failed to read entire contents of file. [Bytes read: %d/%d]",
                 bytesRead,
                 fileSize);
        _gyLog(GY_DEBUG_ERROR, "%s", _loadImageErrorMsg);
        *status = VMU_LOAD_IMAGE_READ_FAILED;
        goto cleanup_file;
    }

    gyVmuFlashFromNexusByteOrder(dataBuffer, tempEntry.fileSize * VMU_FLASH_BLOCK_SIZE);

    VMUFlashNewFileProperties properties;
    gyVmuFlashNewFilePropertiesFromDirEntry(&properties, &tempEntry);

    entry = gyVmuFlashFileCreate(dev, &properties, dataBuffer, status);

cleanup_file:
    if(!gyFileClose(&fp)) {
        _gyLog(GY_DEBUG_WARNING, "File was not closed gracefully for some reason...");
    }
end:
    _gyPop(1);
    return entry;
}

/* Time to de-fuck the bytes for a DCI image!
 * Shit's all in reverse byte-order, in sets of 4
 * If this shit isn't divisible by four, PAD IT!!!
 */
void gyVmuFlashFromNexusByteOrder(uint8_t* data, size_t bytes) {
    size_t wordCount = bytes / 4;
    if(bytes % 4) ++wordCount;
    for(size_t w = 0; w < wordCount; ++w) {
        for(int b = 0; b < 2; ++b) {
            const uint8_t tempByte  = data[w*4 + b];
             data[w*4 + b]          = data[w*4 + 4-b-1];
             data[w*4 + 4-b-1]      = tempByte;
        }
    }
}


VMUFlashDirEntry* gyVmuFlashLoadImageBin(struct VMUDevice* dev, const char* path, VMU_LOAD_IMAGE_STATUS* status) {
    GYFile* file;

    _gyLog(GY_DEBUG_VERBOSE, "Loading .VMU/.BIN Flash image from file: [%s]", path);
    _gyPush();

    if (!gyFileOpen(path, "rb", &file)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open binary file for reading!");
        _gyPop(1);
        *status = VMU_LOAD_IMAGE_OPEN_FAILED;
        return NULL;
    }

    //Clear ROM
    memset(dev->flash, 0, sizeof(dev->flash));

    size_t bytesRead   = 0;
    size_t bytesTotal  = 0;

    while(bytesTotal < sizeof(dev->flash)) {
        if(gyFileRead(file, dev->flash+bytesTotal, sizeof(dev->flash)-bytesTotal, &bytesRead)) {
            bytesTotal += bytesRead;
        } else break;
    }

    gyFileClose(&file);

    _gyLog(GY_DEBUG_VERBOSE, "Read %d bytes.", bytesTotal);
    assert(bytesTotal >= 0);
    assert(bytesTotal == sizeof(dev->flash));

    _gyPop(1);
    *status = VMU_LOAD_IMAGE_SUCCESS;
    gyVmuFlashRootBlockPrint(dev);
    gyVmuFlashPrintFilesystem(dev);

    return NULL;
}

int gyVmuFlashFormatDefault(struct VMUDevice* dev) {
    VMUFlashRootBlock root;
    memset(&root, 0, sizeof(VMUFlashRootBlock));

    root.customColor   = 0;
    root.a             = 255;
    root.r             = 255;
    root.g             = 255;
    root.b             = 255;
    root.fatBlock      = VMU_FLASH_BLOCK_FAT_DEFAULT;
    root.fatSize       = VMU_FLASH_BLOCK_FAT_SIZE_DEFAULT;
    root.dirBlock      = VMU_FLASH_BLOCK_DIRECTORY_DEFAULT;
    root.dirSize       = VMU_FLASH_BLOCK_DIRECTORY_SIZE_DEFAULT;
    root.userBlocks    = VMU_FLASH_BLOCK_USERDATA_SIZE_DEFAULT;
    root.iconShape     = 0;
    return gyVmuFlashFormat(dev, &root);
}

int gyVmuFlashFormat(struct VMUDevice* dev, const VMUFlashRootBlock* rootVal) {
    int success = 1;
    //Clear all of Flash
    memset(dev->flash, 0, gyVmuFlashBytes(dev));
    VMUFlashRootBlock* root = gyVmuFlashBlockRoot(dev);

    //Copy in new root values, so we can start using accessors without shitting
    memcpy(root, rootVal, sizeof(VMUFlashRootBlock));

    for(int b = 0; b < gyVmuFlashBlockCount(dev); ++b) {
        uint16_t* fatEntry = gyVmuFlashBlockFATEntry(dev, b);
        *fatEntry = 0xfffc;
    }

    //Initialize all the stupid, default, static root-block shit
    memset(root->formatted, VMU_FLASH_ROOT_BLOCK_FORMATTED_BYTE, VMU_FLASH_ROOT_BLOCK_FORMATTED_SIZE);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_CENTURY]  = gyVmuToBCD(tm->tm_year/100+19);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_YEAR]     = gyVmuToBCD(tm->tm_year%100);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_MONTH]    = gyVmuToBCD(tm->tm_mon+1);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_DAY]      = gyVmuToBCD(tm->tm_mday);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_HOUR]     = gyVmuToBCD(tm->tm_hour);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_MINUTE]   = gyVmuToBCD(tm->tm_min);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_SECOND]   = gyVmuToBCD(tm->tm_sec);
    root->timeStamp[VMU_FLASH_DIRECTORY_DATE_WEEKDAY]  = gyVmuToBCD(gyVmuWeekDay());

    memset(root->unused1, 0, VMU_FLASH_ROOT_BLOCK_UNUSED1_SIZE);

    //No fucking idea what these are, but the Sega-formatted VMUs have them!!!
    uint8_t* rootBytes = (uint8_t*)root;
    rootBytes[0x40] = 255;
    rootBytes[0x44] = 255;

    //allocate FAT block in the FAT
    uint16_t* fatFat = gyVmuFlashBlockFATEntry(dev, gyVmuFlashBlockFat(dev));
    if(!fatFat) {
        _gyLog(GY_DEBUG_ERROR, "Could not retrieve FAT entry for FAT block: %d", gyVmuFlashBlockFat(dev));
        success = 0;
    } else *fatFat = VMU_FLASH_BLOCK_FAT_LAST_IN_FILE;

    //allocate Root block in the FAT
    uint16_t* rootFat = gyVmuFlashBlockFATEntry(dev, VMU_FLASH_BLOCK_ROOT);
    if(!fatFat) {
        _gyLog(GY_DEBUG_ERROR, "Could not retrieve FAT entry for Root block: %d", VMU_FLASH_BLOCK_ROOT);
        success = 0;
    } else *rootFat = VMU_FLASH_BLOCK_FAT_LAST_IN_FILE;

    //allocate Directory blocks in the FAT
    int dirStartBlock = root->dirBlock + root->dirSize - 1;
    int dirEndBlock = root->dirBlock;
    for(int b = dirStartBlock; b >= dirEndBlock; --b) {
        uint16_t* fatEntry = gyVmuFlashBlockFATEntry(dev, b);
        if(!fatEntry) {
            _gyLog(GY_DEBUG_ERROR, "Couldn't find fat entry [%d] when allocating directory!", b);
            success = 0;
        } else *fatEntry = (b == dirEndBlock)? VMU_FLASH_BLOCK_FAT_LAST_IN_FILE : b-1;
    }

    return success;

   // root->userBlocks = VMU_FLASH_BLOCK_USERDATA_SIZE;
}

void gyVmuFlashRootBlockPrint(const struct VMUDevice* dev) {
    _gyLog(GY_DEBUG_VERBOSE, "VMU Flash - Root [block: %d]", VMU_FLASH_BLOCK_ROOT);
    _gyPush();

    VMUFlashRootBlock* root = gyVmuFlashBlockRoot((VMUDevice*)dev);

    int formatOk = 1;
    for(int i = 0; i < VMU_FLASH_ROOT_BLOCK_FORMATTED_SIZE; ++i) {
        int val = root->formatted[i];
        if(val != VMU_FLASH_ROOT_BLOCK_FORMATTED_BYTE) {
            _gyLog(GY_DEBUG_WARNING, "Root.formattedSegment[%d] = %d (should be %d)", i, val, VMU_FLASH_ROOT_BLOCK_FORMATTED_BYTE);
            formatOk = 0;
        }
    }
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Formatted",        formatOk? "Ok" : "Unformatted");
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Use Custom Color", root->customColor? "Yes" : "No");
    char buffer[100];
    sprintf(buffer, "<%d, %d, %d, %d>", root->r, root->g, root->b, root->a);
    for(int t = 0; t < VMU_FLASH_ROOT_BLOCK_TIMESTAMP_SIZE; ++t) {
        _gyLog(GY_DEBUG_VERBOSE, "%d", root->timeStamp[t]);
    }



    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Color",            buffer);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Fat Block",        offsetof(VMUFlashRootBlock, fatBlock));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Fat Size",         offsetof(VMUFlashRootBlock, fatSize));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Directory Block",  offsetof(VMUFlashRootBlock, dirBlock));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Directory Size",   offsetof(VMUFlashRootBlock, dirSize));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Icon Shape",       offsetof(VMUFlashRootBlock, iconShape));
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Fat Block",        root->fatBlock);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Fat Size",         root->fatSize);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Directory Block",  root->dirBlock);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Directory Size",   root->dirSize);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Icon Shape",       root->iconShape);

    uint8_t* data = (uint8_t*)root;
    for(int t = 0; t < VMU_FLASH_BLOCK_SIZE; ++t) {
        _gyLog(GY_DEBUG_VERBOSE, "Root[%x] = %d", t, data[t]);
    }

    for(int b = 0; b < gyVmuFlashBlockCount(dev); ++b) {
        const uint16_t* block = gyVmuFlashBlockFATEntry((VMUDevice*)dev, b);
        _gyLog(GY_DEBUG_VERBOSE, "FAT[%x] = %d", b, *block);
    }

    gyVmuFlashPrintFilesystem(dev);

    _gyPop(1);
}

int gyVmuFlashPrintFilesystem(const struct VMUDevice* dev) {
    int success = 1;
    _gyLog(GY_DEBUG_VERBOSE, "Dumping VMU Filesystem");
    _gyPush();
    VMUFlashMemUsage memUsage = gyVmuFlashMemUsage(dev);
    int fileCount = gyVmuFlashFileCount(dev);

     _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40d", "Blocks Used",     memUsage.blocksUsed);
     _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40d", "Free",            memUsage.blocksFree);
     _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40d", "File Count",      fileCount);
    _gyPush();
    int num = 0;
    for(int d = 0; d < gyVmuFlashDirEntryCount(dev); ++d) {
        char name[VMU_VMS_FILE_INFO_DC_DESC_SIZE+1] = { '\0' };
        const VMUFlashDirEntry* dirEntry = gyVmuFlashDirEntryByIndex((VMUDevice*)dev, d);

        if(dirEntry->fileType == VMU_FLASH_FILE_TYPE_NONE) continue;
        const VMSFileInfo* vms = gyVmuFlashDirEntryVmsHeader(dev, dirEntry);
        if(!vms) {
            _gyLog(GY_DEBUG_ERROR, "Failed to retrieve VMS header of file? [dirEntry type: %d, dirEntry size: %d]", dirEntry->fileType, dirEntry->fileSize);
        } else {
            gyVmuVmsHeaderDcDescGet(vms, name);
            _gyLog(GY_DEBUG_VERBOSE, "[%d] - %s", num++, name);
            _gyPush();
            gyVmuFlashDirEntryPrint(dev, dirEntry);
            _gyPop(1);
        }
    }

    _gyPop(1);



    _gyPop(1);
    return success;
}


int gyVmuFlashExportImage(struct VMUDevice* dev, const char* path) {
    GYFile* fp = NULL;
    int success = 1;

    _gyLog(GY_DEBUG_VERBOSE, "Exporting VMU Flash image: [%s]", path);
    _gyPush();

    int retVal = gyFileOpen(path, "wb", &fp);

    if(!retVal || !fp) {
        _gyLog(GY_DEBUG_ERROR, "Failed to create the file!");
        success = 0;
        goto end;
    }

    size_t bytesWritten = gyFileWrite(fp, dev->flash, sizeof(char), FLASH_SIZE);

    if(bytesWritten < FLASH_SIZE) {
        _gyLog(GY_DEBUG_ERROR, "Couldn't write entire file! [bytes written %d/%d]", bytesWritten, FLASH_SIZE);
        success = 0;
    }

    success &= gyFileClose(&fp);
    if(!success) {
        _gyLog(GY_DEBUG_ERROR, "Could not gracefully close file.");
    }

end:
    _gyPop(1);
    return success;
}





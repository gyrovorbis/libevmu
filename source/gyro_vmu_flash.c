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
#include <gyro_file_api.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

static inline int tobcd(int n) {
  return ((n/10)<<4)|(n%10);
}

int gyVmuFlashBytesToBlocks(int bytes) {
    int blocks = bytes/VMU_FLASH_BLOCK_SIZE;
    if(bytes % VMU_FLASH_BLOCK_SIZE) ++blocks;
    return blocks;
}

int gyVmuFlashLoadImage(VMUDevice* dev, const char* path) {
    char tempPath[10000];
    int success = 0;
    strcpy(tempPath, path);
    const char*curTok  = strtok(tempPath, ".");
    const char*prevTok = NULL;
    char ext[100] = { 0 };

    _gyLog(GY_DEBUG_VERBOSE, "Loading Flash Image [%s]", path);
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

        if(!strcmp(ext, "vms")) {
            success = gyVmuFlashLoadVMS(dev, path);
        } else if(!strcmp(ext, "dci")) {
            success = gyVmuFlashLoadDCI(dev, path);
        } else if(!strcmp(ext, "vmi")) {
            VMIFileInfo info;
            if(gyVmuFlashLoadVMI(&info, path)) {
                gyVmuFlashPrintVMIFileInfo(&info);
                success = 1;
            } else success = 0;
        } else if(!strcmp(ext, "vmu")) {
            success = gyVmuFlashLoadVMUImage(dev, path);
        } else if(!strcmp(ext, VMU_LCD_FILE_EXTENSION)) {
            success = gyVmuLcdFileLoadAndStart(dev, path);
        } else {
            _gyLog(GY_DEBUG_ERROR, "Unknown extension! [%s]", prevTok);
        }


    } else {
        _gyLog(GY_DEBUG_ERROR, "No extension found on file!");

    }

    _gyPop(1);
    return success;

}

int gyVmuFlashLoadVMUImage(VMUDevice *dev, const char *path) {
    GYFile* file;

    _gyLog(GY_DEBUG_VERBOSE, "Loading Flash image from file [%s].", path);
    _gyPush();

    if (!gyFileOpen(path, "rb", &file)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open binary file for reading!");
        _gyPop(1);
        return 0;
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
    return 1;
}

int gyVmuFlashLoadDCI(VMUDevice *dev, const char *path) {
  gyVmuFlashLoadVMUImage(dev, path);

    for(unsigned i = 0; i < sizeof(dev->flash); i += 4) {
        for(unsigned j = i; j < i + 4; ++j) {
            unsigned temp       = dev->flash[j];
            dev->flash[j]       = dev->flash[i+3-j];
            dev->flash[i+3-j]   = temp;
        }
    }

    _gyPop(1);
    return 1;
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

unsigned char* gyVmuFlashBlock(struct VMUDevice* dev, uint16_t block) {
    return (block >= VMU_FLASH_BLOCK_COUNT)? NULL : &dev->flash[block*VMU_FLASH_BLOCK_SIZE];
}

const unsigned char* gyVmuFlashBlockReadOnly(const struct VMUDevice* dev, uint16_t block) {
    return (block >= VMU_FLASH_BLOCK_COUNT)? NULL : &dev->flash[block*VMU_FLASH_BLOCK_SIZE];
}

uint16_t* gyVmuFlashBlockFATEntry(VMUDevice* dev, uint16_t block) {
    return (block >= VMU_FLASH_BLOCK_COUNT)? NULL : ((uint16_t*)gyVmuFlashBlock(dev, VMU_FLASH_BLOCK_FAT)+block);
}

const uint16_t* gyVmuFlashBlockFATEntryReadOnly(const VMUDevice* dev, uint16_t block) {
    return (block >= VMU_FLASH_BLOCK_COUNT)? NULL : ((uint16_t*)gyVmuFlashBlockReadOnly(dev, VMU_FLASH_BLOCK_FAT)+block);
}

uint16_t gyVmuFlashBlockNext(const VMUDevice* dev, uint16_t block) {
    const uint16_t* fatEntry = gyVmuFlashBlockFATEntryReadOnly(dev, block);
    return fatEntry? *fatEntry : VMU_FLASH_BLOCK_FAT_UNALLOCATED;
}

/* DATA file blocks are allocated as the first available free block starting at the highest address.
 * GAME file blocks are allocated contiguously starting at address 0.
 */
uint16_t gyVmuFlashBlockAlloc(VMUDevice* dev, uint16_t previous, int fileType) {
    uint16_t    block = VMU_FLASH_BLOCK_FAT_UNALLOCATED;
    uint16_t*   fat   = (uint16_t*)gyVmuFlashBlock(dev, VMU_FLASH_BLOCK_FAT);

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
    assert(block < VMU_FLASH_BLOCK_COUNT);
    memset(gyVmuFlashBlock(dev, block), 0, VMU_FLASH_BLOCK_SIZE);
    *gyVmuFlashBlockFATEntry(dev, block) = VMU_FLASH_BLOCK_FAT_UNALLOCATED;
}






//=== FLASH DIRECTORY API ====


VMUFlashDirEntry* gyVmuFlashDirEntryByIndex(VMUDevice* dev, uint8_t index) {
    VMUFlashDirEntry* entry = ((VMUFlashDirEntry*)gyVmuFlashBlockReadOnly(dev, VMU_FLASH_BLOCK_DIRECTORY)+index);
    return (entry && entry->fileType == VMU_FLASH_FILE_TYPE_NONE)? NULL : entry;
}

const VMUFlashDirEntry* gyVmuFlashDirEntryByIndexReadOnly(const VMUDevice* dev, uint8_t index) {
    VMUFlashDirEntry* entry = ((VMUFlashDirEntry*)gyVmuFlashBlockReadOnly(dev, VMU_FLASH_BLOCK_DIRECTORY)+index);
    return (entry && entry->fileType == VMU_FLASH_FILE_TYPE_NONE)? NULL : entry;
}

const VMUFlashDirEntry* gyVmuFlashDirEntryByName(const struct VMUDevice* dev, const char* name) {
    const VMUFlashDirEntry* entry;
    uint8_t block           = 0;
    const size_t len        = strlen(name);

    while((entry = gyVmuFlashDirEntryByIndexReadOnly(dev, block++))) {
        if(entry->fileType == VMU_FLASH_FILE_TYPE_NONE) continue;

        int match = 1;

        for(unsigned i = 0; i < VMU_FLASH_DIRECTORY_FILE_NAME_SIZE && i < len; ++i) {
            if(entry->fileName[i] != name[i]) {
                match = 0;
                entry = NULL;
                break;
            }
        }

        if(match) break;
    }

    return entry;
}

const VMUFlashDirEntry* gyVmuFlashDirEntryGame(const struct VMUDevice* dev) {
    const VMUFlashDirEntry* entry = NULL;
    for(unsigned i = 0; i < VMU_FLASH_BLOCK_DIRECTORY_SIZE; ++i) {
        entry = gyVmuFlashDirEntryByIndexReadOnly(dev, i);
        if(entry && entry->fileType == VMU_FLASH_FILE_TYPE_GAME)
            return entry;
    }

    return NULL;
}


uint8_t gyVmuFlashDirEntryIndex(const struct VMUDevice* dev, const VMUFlashDirEntry* entry) {
    const unsigned char* dir = gyVmuFlashBlockReadOnly(dev, VMU_FLASH_BLOCK_DIRECTORY);
    const ptrdiff_t diff = (const unsigned char*)entry - dir;
    assert(diff % sizeof(VMUFlashDirEntry) == 0);
    return diff/sizeof(VMUFlashDirEntry);
}

const VMUFlashDirEntry* gyVmuFlashDirEntryDataNext(const struct VMUDevice* dev, const VMUFlashDirEntry* prev) {
    const VMUFlashDirEntry*     entry       = NULL;
    uint16_t                    dirIndex    = prev? gyVmuFlashDirEntryIndex(dev, prev) : 0;

    if(prev) {
        //Check if we're at the end
        if(dirIndex == VMU_FLASH_BLOCK_DIRECTORY_ENTRIES-1)
            return NULL; //There is no next entry
    } else { //No previous entry, beginning iteration
        //Grab first entry
        prev = gyVmuFlashDirEntryByIndexReadOnly(dev, 0);
        //Return first entry if it's valid
        if(prev->fileType == VMU_FLASH_FILE_TYPE_DATA) {
            return prev;
        }
    }

    //Start iterating from the next index, looking for the next data file
    for(dirIndex = dirIndex + 1; dirIndex < VMU_FLASH_BLOCK_DIRECTORY_ENTRIES; ++dirIndex) {
        prev = gyVmuFlashDirEntryByIndexReadOnly(dev, dirIndex);
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
    for(unsigned i = 0; i < VMU_FLASH_BLOCK_DIRECTORY_ENTRIES; ++i) {
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




int gyVmuFlashFileRead(const struct VMUDevice* dev, const struct VMUFlashDirEntry* entry, char* buffer) {
    const unsigned char* chunk;
    char*       bufferPtr   = buffer;            //Points current block within buffer
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

int gyVmuFlashCheckFormatted(const VMUDevice *dev) {
    const unsigned char* root = gyVmuFlashBlockReadOnly(dev, VMU_FLASH_BLOCK_ROOT);

    for(unsigned i = 0; i < 0xf; ++i)
        if(root[i] != VMU_FLASH_BLOCK_ROOT_FORMATTED_BYTE)
            return 0;

    return 1;
}



uint8_t gyVmuFlashUserDataBlocks(const struct VMUDevice* dev) {
    (void)dev;
    return VMU_FLASH_BLOCK_USERDATA_SIZE; //needs to take into account extra 40/41 blocks!!!
}

int gyVmuFlashUserDataUnlockUnusedBlocks(struct VMUDevice* dev) {
    (void)dev;
    return 0;
}

VMUFlashMemUsage gyVmuFlashMemUsage(const struct VMUDevice* dev) {
    VMUFlashMemUsage    mem     = { 0, 0 };
    uint16_t*           fat     = (uint16_t*)&dev->flash[VMU_FLASH_BLOCK_FAT];


    for(unsigned i = 0; i < VMU_FLASH_BLOCK_DIRECTORY_SIZE; ++i) {
        if(fat[i] != VMU_FLASH_BLOCK_FAT_UNALLOCATED)
            ++mem.blocksUsed;
    }

    mem.blocksFree = gyVmuFlashUserDataBlocks(dev) - mem.blocksUsed;

    return mem;
}


int gyVmuFlashFileDelete(struct VMUDevice* dev, VMUFlashDirEntry* entry) {
    int blocksFreed = 0;

    for(uint16_t block = entry->firstBlock;
        block != VMU_FLASH_BLOCK_FAT_LAST_IN_FILE;
        block = gyVmuFlashBlockNext(dev, block)) {

        gyVmuFlashBlockFree(dev, block);

        ++blocksFreed;
    }

    //Clear directory entry
    gyVmuFlashDirEntryFree(entry);

    return blocksFreed;
}

const VMUFlashDirEntry* gyVmuFlashFileCreate(VMUDevice* dev, const VMUFlashNewFileProperties* properties, const char* data) {
    (void)data; //wasn't this supposed to actaully populate the file with data too?

    //Can't assume filename is null terminated.
    char fileNameBuff[VMU_FLASH_DIRECTORY_FILE_NAME_SIZE+1] = { 0 };
    memcpy(fileNameBuff, properties->fileName, VMU_FLASH_DIRECTORY_FILE_NAME_SIZE);

    _gyLog(GY_DEBUG_VERBOSE, "VMU Flash - Creating file [%s].", fileNameBuff);
    _gyPush();

    //=== 1 - Check if we're creating a GAME file while one already exists. ===
    if(properties->fileType == VMU_FLASH_FILE_TYPE_GAME && gyVmuFlashDirEntryGame(dev)) {
        _gyLog(GY_DEBUG_ERROR, "Only one GAME file can be present at a time!");
        _gyPop(1);
        return NULL;
    }

    //=== 2 - Make sure we don't already have a file with the same name. ===
    if(gyVmuFlashDirEntryByName(dev, properties->fileName)) {
        _gyLog(GY_DEBUG_ERROR, "File already present with the same name!");
        _gyPop(1);
        return NULL;
    }

    //=== 3 - Check whether there are enough free blocks available for the file. ===
    unsigned headerBytes     = gyVmuVmsFileInfoHeaderSize(properties->vmsHeader);
    unsigned totalBytes      = properties->vmsHeader->dataBytes + headerBytes;
    unsigned blocksRequired  = gyVmuFlashBytesToBlocks(totalBytes);

    VMUFlashMemUsage memUsage = gyVmuFlashMemUsage(dev);
    if(memUsage.blocksFree < blocksRequired) {
        _gyLog(GY_DEBUG_ERROR, "Not enough free blocks left on memory unit! [Free: %d, Required: %d]",
               memUsage.blocksFree, blocksRequired);
        _gyPop(1);
        return NULL;
    }

    //=== 4 - Create Flash Directory Entry for file. ===
    VMUFlashDirEntry* entry = gyVmuFlashDirEntryAlloc(dev);
    if(!entry) {
        _gyLog(GY_DEBUG_ERROR, "Could not allocate entry in Flash Directory (too many files present).");
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
    entry->headerOffset     = (entry->fileType == VMU_FLASH_FILE_TYPE_DATA)? 0 :
                              gyVmuFlashBytesToBlocks(properties->vmsHeader->dataBytes);
    /*  NOT POSITIVE IF HEADER OFFSET IS RIGHT FOR GAME DATA.
     *  Don't know whether it's stored at the end of the game, or always at block 1,
     *  in the middle of the game data...
     */

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

    /* Game data must all be stored contiguously starting at block 0,
     * so check whether memory card requires defrag.
     */
    if(entry->fileType == VMU_FLASH_FILE_TYPE_GAME) {
        const unsigned contiguousBlocks = gyVmuFlashContiguousFreeBlocks(dev);

        //Defragment card if we couldn't find enough contiguous blocks.
        if(contiguousBlocks < blocksRequired) {
            _gyLog(GY_DEBUG_WARNING, "Not enough contiguous blocks available for GAME file [%d/%d].",
                   contiguousBlocks, blocksRequired);

            gyVmuFlashDefragment(dev);
        }
    }

    //NEED TO HOPEFULLY JUST CALL INTO FILE WRITE AND ONLY DO CREATION EXTRA LOGIC HERE!!1111
    int blocks[VMU_FLASH_BLOCK_USERDATA_SIZE+VMU_FLASH_BLOCK_UNUSED_SIZE] = { -1 };
    for(unsigned i = 0; i < blocksRequired; ++i) {
        blocks[i] = gyVmuFlashBlockAlloc(dev, (i > 0)? blocks[i-1] : VMU_FLASH_BLOCK_FAT_UNALLOCATED, properties->fileType);
        if(blocks[i] == VMU_FLASH_BLOCK_FAT_UNALLOCATED) {
            //Try to free up the other blocks we've claimed, so they aren't stranded.
            for(unsigned j = 0; j < i; ++j) gyVmuFlashBlockFree(dev, blocks[j]);
            _gyLog(GY_DEBUG_ERROR, "Block allocation failure! [Block: %u]", i);
            _gyPop(2);
            return NULL;
        }
    }

    _gyPop(1);

    //=== 6 - Write VMS File Header ===
    _gyLog(GY_DEBUG_VERBOSE, "Writing VMS File Header.");
    _gyPush();

    unsigned blocksToWrite   = gyVmuFlashBytesToBlocks(headerBytes);
    unsigned bytesLeft       = headerBytes;
    for(unsigned i = 0; i < blocksToWrite; ++i) {
        int bytesForBlock = (bytesLeft > VMU_FLASH_BLOCK_SIZE)? VMU_FLASH_BLOCK_SIZE : bytesLeft;
        memcpy(gyVmuFlashBlock(dev, blocks[entry->headerOffset + i]),
                ((char*)properties->vmsHeader+i*VMU_FLASH_BLOCK_SIZE),
                bytesForBlock);
        bytesLeft -= bytesForBlock;
    }
    assert(bytesLeft == 0);

    _gyPop(1);


    //=== 7 - Write VMS File Data ===
    _gyLog(GY_DEBUG_VERBOSE, "Writing VMS File Data.");
    _gyPush();

    int fileOffset   = (properties->fileType == VMU_FLASH_FILE_TYPE_GAME)?
                        0 : blocksToWrite; //Whether file is stored before or after VMS header
    blocksToWrite   = gyVmuFlashBytesToBlocks(properties->vmsHeader->dataBytes);
    bytesLeft       = properties->vmsHeader->dataBytes;
    for(unsigned i = 0; i < blocksToWrite; ++i) {
        int bytesForBlock = (bytesLeft > VMU_FLASH_BLOCK_SIZE)? VMU_FLASH_BLOCK_SIZE : bytesLeft;
        memcpy(gyVmuFlashBlock(dev, blocks[fileOffset + i]),
                ((char*)properties->vmsHeader+i*VMU_FLASH_BLOCK_SIZE),
                bytesForBlock);
        bytesLeft -= bytesForBlock;
    }
    assert(bytesLeft == 0);

    _gyPop(2);


    return entry;
}


int gyVmuFlashContiguousFreeBlocks(const struct VMUDevice* dev) {
    int contiguousBlocks    = 0;
    unsigned userDataBlocks = gyVmuFlashUserDataBlocks(dev);

    for(unsigned i = 0; i < userDataBlocks; ++i) {
        if(*gyVmuFlashBlockFATEntryReadOnly(dev, i) != VMU_FLASH_BLOCK_FAT_UNALLOCATED) {
            break;
        } else {
            ++contiguousBlocks;
        }
    }

    return contiguousBlocks;
}

int gyVmuFlashDefragment(struct VMUDevice* dev) {
    (void)dev;
    return 1;
}



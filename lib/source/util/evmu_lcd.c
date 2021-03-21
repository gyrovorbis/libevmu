#include "gyro_vmu_lcd.h"
#include "gyro_vmu_display.h"
#include "gyro_vmu_device.h"
#include <gyro_file_api.h>
#include <gyro_input_api.h>
#include <gyro_system_api.h>
#include <assert.h>
#include <stdlib.h>

static inline size_t _minFileSize(void) {
    return sizeof(LCDFileHeader)+sizeof(LCDCopyright); //assuming you don't have a single frame, which may not even be valid anyway...
}

LCDFile* gyVmuLcdFileLoad(const char* filePath) {
    assert(sizeof(LCDFileHeader) == VMU_LCD_FILE_HEADER_SIZE); //You have a SERIOUS problem here, nigga!
    assert(sizeof(LCDFrameInfo) == VMU_LCD_FRAME_INFO_SIZE);
    assert(sizeof(LCDCopyright) == VMU_LCD_COPYRIGHT_SIZE);

    GYFile* fp;
    LCDFile* file = NULL;

    _gyLog(GY_DEBUG_VERBOSE, "Loading .LCD animation file [%s]", filePath);
    _gyPush();

    gyFileOpen(filePath, "r", &fp);

    if(fp) {
        size_t bytes;
        gyFileLength(fp, &bytes);

        if(bytes < _minFileSize()) {
            _gyLog(GY_DEBUG_ERROR, "Sorry, but there's no fucking way %d bytes is large enough to be a valid .LCD file...", bytes);
            goto closefile;
        }

        file                = malloc(sizeof(LCDFile));
        memset(file, 0, sizeof(LCDFile));
        file->currentFrame  = -1;
        file->fileData      = malloc(bytes);
        file->fileLength    = bytes;
        gyFileRead(fp, file->fileData, bytes, &bytes);

        if(bytes != file->fileLength) {
            _gyLog(GY_DEBUG_ERROR, "Unable to read entire file! [%d of %d bytes]", bytes, file->fileLength);
            goto freedata;
        }

        //Validate header data
        file->header        = (LCDFileHeader*)file->fileData;

        char buff[VMU_LCD_SIGNATURE_SIZE+1] = { 0 };
        memcpy(buff, file->header->signature, VMU_LCD_SIGNATURE_SIZE);
        if(strcmp(buff, "LCDi") != 0) {
            _gyLog(GY_DEBUG_ERROR, "Unknown file signature: %s", buff);
            goto freedata;
        }

        if(file->header->bitDepth != 1) {
            _gyLog(GY_DEBUG_ERROR, "Unsupported bit depth: %d", file->header->bitDepth);
            goto freedata;
        }

        if(file->header->width != VMU_DISP_PIXEL_WIDTH || file->header->height != VMU_DISP_PIXEL_HEIGHT) {
            _gyLog(GY_DEBUG_ERROR, "Unsupported resolution: <%d, %d>", file->header->width, file->header->height);
            goto freedata;
        }

        const size_t expectedSize = sizeof(LCDFileHeader)
                + file->header->frameCount*sizeof(LCDFrameInfo)
                + file->header->frameCount*VMU_LCD_FRAME_DATA_SIZE
                + sizeof(LCDCopyright);
        if(bytes != expectedSize) {
            _gyLog(GY_DEBUG_ERROR, "File size [%d] does not match expected file size [%d]!", bytes, expectedSize);
            goto freedata;
        }

        size_t offset = VMU_LCD_FILE_HEADER_SIZE;

        //Populate frame info pointers
        file->frameInfo = malloc(file->header->frameCount*sizeof(LCDFrameInfo*));
        for(unsigned i = 0; i < file->header->frameCount; ++i) {
            file->frameInfo[i] = (LCDFrameInfo*)&file->fileData[offset];
            offset += VMU_LCD_FRAME_INFO_SIZE;
        }

        //Populate frame data pointers
        file->frameData = malloc(file->header->frameCount*sizeof(uint8_t*));
        for(unsigned i = 0; i < file->header->frameCount; ++i) {
            file->frameData[i] = &file->fileData[offset];
            offset += VMU_LCD_FRAME_DATA_SIZE;
        }

        file->copyright = (LCDCopyright*)&file->fileData[offset];
        offset += VMU_LCD_COPYRIGHT_SIZE;

        assert(offset == bytes); //Something is FUNDAMENTALLY reested otherwise!

        gyVmuLcdPrintFileInfo(file);
        goto closefile;

    } else {
        _gyLog(GY_DEBUG_ERROR, "Could not open file!");
        goto done;
    }

freedata:
    free(file->fileData);
    free(file);
    file = NULL;
closefile:
    gyFileClose(&fp);
done:
    _gyPop(1);
    return file;
}

void gyVmuLcdFileUnload(LCDFile* lcdFile) {
    if(lcdFile) {
        free(lcdFile->frameInfo);
        free(lcdFile->frameData);
        free(lcdFile->fileData);
        free(lcdFile);
    }
}

void gyVmuLcdPrintFileInfo(const LCDFile* lcdFile) {
    _gyLog(GY_DEBUG_VERBOSE, "LCD File Header Info");
    _gyPush();

    char buff[VMU_LCD_SIGNATURE_SIZE+1] = { 0 };
    memcpy(buff, lcdFile->header->signature, VMU_LCD_SIGNATURE_SIZE);

    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Signature",                buff);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Version Number",           lcdFile->header->version);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Frame Width",              lcdFile->header->width);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Frame Height",             lcdFile->header->height);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Bit Depth",                lcdFile->header->bitDepth);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40x", "Reserved",                 lcdFile->header->reserved);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Repeat Count",             lcdFile->header->repeatCount);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40u", "Frame Count",              lcdFile->header->frameCount);

    char buff2[VMU_LCD_COPYRIGHT_SIZE+1] = { 0 };
    memcpy(buff2, lcdFile->copyright->copyright, VMU_LCD_COPYRIGHT_SIZE);

    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %40s", "Copyright",                buff2);


    _gyPop(1);
}

void gyVmuLcdFileFrameStart(struct VMUDevice* dev, uint16_t frameIndex) {
    LCDFile* lcdFile = dev->lcdFile;
    assert(frameIndex < lcdFile->header->frameCount);
    lcdFile->currentFrame   = frameIndex;
    lcdFile->elapsedTime    = 0.0f;
    if(lcdFile->state == LCD_FILE_STATE_COMPLETE) lcdFile->state = LCD_FILE_STATE_STOPPED;

    for(unsigned i = 0; i < VMU_LCD_FRAME_DATA_SIZE; ++i) {
        //_gyLog(GY_DEBUG_VERBOSE, "FRAME[%d] = %x", i, lcdFile->frameData[frameIndex]);

        gyVmuDisplayPixelSet(dev, i%VMU_DISP_PIXEL_WIDTH, i/VMU_DISP_PIXEL_WIDTH,
                             (lcdFile->frameData[frameIndex][i] == VMU_LCD_FILE_PIXEL_ON)? 1 : 0);
    }
}

void gyVmuLcdFileUpdate(VMUDevice* dev, float deltaTime) {
    LCDFile* lcdFile = dev->lcdFile;

    if(lcdFile->state == LCD_FILE_STATE_PLAYING) {
        lcdFile->elapsedTime += deltaTime;

        if(lcdFile->elapsedTime >= lcdFile->frameInfo[lcdFile->currentFrame]->delay*VMU_LCD_DELAY_SECS) {
            if(++lcdFile->currentFrame >= lcdFile->header->frameCount) {
                if(lcdFile->header->repeatCount != VMU_LCD_REPEAT_INFINITE && ++lcdFile->loopCount >= lcdFile->header->repeatCount) {
                    --lcdFile->currentFrame;
                    lcdFile->state = LCD_FILE_STATE_COMPLETE;
                    return;
                } else {
                    lcdFile->currentFrame = 0;
                }
            }
            gyVmuLcdFileFrameStart(dev, lcdFile->currentFrame);
        }
    }
}


int gyVmuLcdFileLoadAndStart(VMUDevice *dev, const char *filePath) {
    dev->lcdFile = gyVmuLcdFileLoad(filePath);

    if(dev->lcdFile) {
        gyVmuDisplayEnabledSet(dev, 1);
        gyVmuDisplayUpdateEnabledSet(dev, 1);
        gyVmuLcdFileFrameStart(dev, 0);
        for(int i = 0; i < VMU_DISP_ICN_COUNT; ++i) gyVmuDisplayIconSet(dev, i, 0);
        dev->lcdFile->state = LCD_FILE_STATE_PLAYING;
        return 1;
    } else {
        return 0;
    }
}

void gyVmuLcdFileStopAndUnload(VMUDevice *dev) {
    gyVmuLcdFileUnload(dev->lcdFile);
    dev->lcdFile = NULL;
}


void gyVmuLcdFileProcessInput(VMUDevice *dev) {

    //Start/pause
    if(dev->gamepad.a == GY_BUT_STATE_TAPPED) {
        if(dev->lcdFile->state == LCD_FILE_STATE_PLAYING) {
            dev->lcdFile->state = LCD_FILE_STATE_STOPPED;
        } else if(dev->lcdFile->state == LCD_FILE_STATE_STOPPED) {
            dev->lcdFile->state = LCD_FILE_STATE_PLAYING;
        }
    }

    //Reset
    if(dev->gamepad.b == GY_BUT_STATE_TAPPED) {
        gyVmuLcdFileFrameStart(dev, 0);
        dev->lcdFile->state = LCD_FILE_STATE_PLAYING;
    }

    //Single frame advance/rewind
    if(dev->gamepad.l == GY_BUT_STATE_TAPPED) {
        if(dev->lcdFile->currentFrame > 0)
            gyVmuLcdFileFrameStart(dev, dev->lcdFile->currentFrame-1);
    }
    if(dev->gamepad.r == GY_BUT_STATE_TAPPED) {
        if(dev->lcdFile->currentFrame+1 < dev->lcdFile->header->frameCount)
            gyVmuLcdFileFrameStart(dev, dev->lcdFile->currentFrame+1);
    }

    //Continuous Advance/Rewind
    if(dev->gamepad.u & GY_BUT_STATE_DOWN) {
        if(dev->lcdFile->currentFrame > 0)
            gyVmuLcdFileFrameStart(dev, dev->lcdFile->currentFrame-1);
    }
    if(dev->gamepad.d & GY_BUT_STATE_DOWN) {
        if(dev->lcdFile->currentFrame+1 < dev->lcdFile->header->frameCount)
            gyVmuLcdFileFrameStart(dev, dev->lcdFile->currentFrame+1);
    }

}

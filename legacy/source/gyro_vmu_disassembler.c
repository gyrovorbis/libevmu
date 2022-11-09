#include "gyro_vmu_disassembler.h"
#include "gyro_vmu_device.h"
#include "gyro_vmu_flash.h"
#include <gyro_file_api.h>
#include <gyro_system_api.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


void gyVmuDisassembleInstruction(VMUInstr instr, VMUInstrOperands operands, char* buffer, uint16_t pc, int showPc) {
    char strBuff[100];
    char tempBuff[100];
    char mneumonic[100];

    strcpy(mneumonic, _instrMap[instr.instrBytes[INSTR_BYTE_OPCODE]].mnemonic);
    char* tok = strtok(mneumonic, " ,");

    sprintf(strBuff, "%s ", tok);

    const int argc = instrArgsArgc(_instrMap[instr.instrBytes[INSTR_BYTE_OPCODE]].args);
    for(int j = 0; j < argc; ++j) {
        tok = strtok(NULL, " ,");
        assert(tok); //operand not present within pneumonic!

        if(j != 0) strcat(strBuff, ", ");

        if(strcmp(tok, "r8") == 0) {
            sprintf(tempBuff, "0x%x", operands.addrRel);
        } else if(strcmp(tok, "r16") == 0) {
            sprintf(tempBuff, "0x%0x", operands.addrRel);
        } else if(strcmp(tok, "d9") == 0) {
            sprintf(tempBuff, "0x%x", operands.addrMode[ADDR_MODE_DIR]);
        } else if(strcmp(tok, "@Ri") == 0) {
            sprintf(tempBuff, "@R%u", operands.addrMode[ADDR_MODE_IND]);
        } else if(strcmp(tok, "@Rj") == 0) {
            sprintf(tempBuff, "@R%u", operands.addrMode[ADDR_MODE_IND]);
        } else if(strcmp(tok, "#i8") == 0) {
            sprintf(tempBuff, "#0x%x", operands.addrMode[ADDR_MODE_IMM]);
        } else if(strcmp(tok, "a12") == 0) {
            sprintf(tempBuff, "#0x%x", operands.addrMode[ADDR_MODE_ABS]);
        } else if(strcmp(tok, "a16") == 0) {
            sprintf(tempBuff, "#0x%x", operands.addrMode[ADDR_MODE_ABS]);
        } else if(strcmp(tok, "b3") == 0) {
            sprintf(tempBuff, "#%u", operands.addrMode[ADDR_MODE_BIT]);
        } else {
            assert(0); //unknown operand token!
        }

        strcat(strBuff, tempBuff);
    }

    char tmp[20];
    sprintf(tmp, "0x%x", pc);

    if(showPc) {
        sprintf(buffer, "%-5s: %s", tmp, strBuff);

    } else {
        sprintf(buffer, "%-5s: %s", tmp, strBuff);

    }


}

bool gyVmuDisassemble(const unsigned char* buffer, size_t size, const char* outputFile, bool writeAddr) {
    char strBuff[100];
    char tempBuff[100];
    char mneumonic[100];
    VMUInstr instr;
    VMUInstrOperands operands;
    GYFile *fp;

    if (!gyFileOpen(outputFile, "w", &fp)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open output file [%s]!", outputFile);
        _gyPop(1);
        return 0;
    } else {
        _gyLog(GY_DEBUG_VERBOSE, "Opened file for writing [%s]", outputFile);
    }

    size_t i = 0;
    while(i < size) {

        if(buffer[i] == 0xff && buffer[i+1] == 0xff && buffer[i+2] == 0xff && buffer[i+3] == 0xff) {
            do {
                if(writeAddr)
                    gyFilePrintf(fp, "%-5x: NOP\n", i);
                ++i;
            } while(buffer[i] == 0xff);
        }

        gyVmuInstrFetch(&buffer[i], &instr);
        gyVmuInstrDecodeOperands(&instr, &operands);

        strcpy(mneumonic, _instrMap[instr.instrBytes[INSTR_BYTE_OPCODE]].mnemonic);
        char* tok = strtok(mneumonic, " ,");

        sprintf(strBuff, "%s ", tok);

        const int argc = instrArgsArgc(_instrMap[instr.instrBytes[INSTR_BYTE_OPCODE]].args);
        for(int j = 0; j < argc; ++j) {
            tok = strtok(NULL, " ,");
            assert(tok); //operand not present within pneumonic!

            if(j != 0) strcat(strBuff, ", ");

            if(strcmp(tok, "r8") == 0) {
                sprintf(tempBuff, "%d", operands.addrRel);
            } else if(strcmp(tok, "r16") == 0) {
                sprintf(tempBuff, "%d", operands.addrRel);
            } else if(strcmp(tok, "d9") == 0) {
                sprintf(tempBuff, "%u", operands.addrMode[ADDR_MODE_DIR]);
            } else if(strcmp(tok, "@Ri") == 0) {
                sprintf(tempBuff, "@R%u", operands.addrMode[ADDR_MODE_IND]);
            } else if(strcmp(tok, "@Rj") == 0) {
                sprintf(tempBuff, "@R%u", operands.addrMode[ADDR_MODE_IND]);
            } else if(strcmp(tok, "#i8") == 0) {
                sprintf(tempBuff, "#%u", operands.addrMode[ADDR_MODE_IMM]);
            } else if(strcmp(tok, "a12") == 0) {
                sprintf(tempBuff, "#%u", operands.addrMode[ADDR_MODE_ABS]);
            } else if(strcmp(tok, "a16") == 0) {
                sprintf(tempBuff, "#%u", operands.addrMode[ADDR_MODE_ABS]);
            } else if(strcmp(tok, "b3") == 0) {
                sprintf(tempBuff, "#%u", operands.addrMode[ADDR_MODE_BIT]);
            } else {
                assert(0); //unknown operand token!
            }

            strcat(strBuff, tempBuff);
        }

        if(writeAddr) {
            if(!gyFilePrintf(fp, "%-5x: %s\n", i, strBuff)) {
                break;
            }
        } else {
            if(!gyFilePrintf(fp, "%s\n", strBuff)) {
                break;
            }
        }

        i += instr.bytes;

    }
    gyFileClose(&fp);

    _gyPop(1);
    return i == size;

}


bool gyVmuDisassembleFlashGame(const VMUDevice* dev, const char* outputFile, bool showAddr) {
    _gyLog(GY_DEBUG_VERBOSE, "Disassembling Game File");
    _gyPush();

#if 0
    VMUFlashDirEntry* dirEntry;
    VMUFlashDirEntry* entry;
    if(!(dirEntry = gyVmuFlashDirEntryGame(dev))) {
        _gyLog(GY_DEBUG_ERROR, "No game image loaded on current device!");
        _gyPop(1);
        return 0;
    }
#endif

    _gyPop(1);
    return gyVmuDisassemble(dev->flash, 8*VMU_FLASH_BLOCK_SIZE, outputFile, showAddr);

}

/* Check if we're disassembling a VMS or something and stop before we interpet
 * VMS header as code? Can come later...
 */
bool gyVmuDisassembleFile(const char* inFile, const char* outFile, bool showAddr) {
    GYFile* fp;
    unsigned char* buff;
    size_t fileSize, bytesRead;
    int success = 1;

    _gyLog(GY_DEBUG_VERBOSE, "Disassembling file [%s] to [%s].", inFile, outFile);

    if(!gyFileOpen(inFile, "r", &fp)) {
        _gyLog(GY_DEBUG_ERROR, "Could not open input file [%s]!", inFile);
        success = 0;

    } else {

        gyFileLength(fp, &fileSize);

        if(!fileSize) {
            _gyLog(GY_DEBUG_WARNING, "The input file is empty. There's not shit to disassemble here.");
            success = 0;
        } else {

            buff = malloc(sizeof(char)*fileSize);
            gyFileRead(fp, buff, fileSize, &bytesRead);

            if(!bytesRead) {
                _gyLog(GY_DEBUG_ERROR, "Could not read from the file!");
                success = 0;
            } else {

                if(bytesRead != fileSize) {
                    _gyLog(GY_DEBUG_WARNING, "Could not read the entire file! Attempting to continue anyway... (%d/%d bytes)",
                           bytesRead, fileSize);
                    success = 0;
                }
                success &= gyVmuDisassemble(buff, bytesRead, outFile, showAddr);
            }
            free(buff);
        }
        gyFileClose(&fp);
    }

    _gyPop(1);
    return success;

}

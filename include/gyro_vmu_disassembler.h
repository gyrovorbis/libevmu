#ifndef GYRO_VMU_DISASSEMBLER_H
#define GYRO_VMU_DISASSEMBLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include "gyro_vmu_instr.h"

struct VMUDevice;
void gyVmuDisassembleInstruction(VMUInstr instr, VMUInstrOperands operands, char* buffer, uint16_t pc, int showPc);
bool gyVmuDisassembleFlashGame(const struct VMUDevice* dev, const char* outputFile, bool showAddr);
bool gyVmuDisassemble(const unsigned char* buffer, size_t size, const char* outputFile, bool showAddr);
bool gyVmuDisassembleFile(const char* inFile, const char* outFile, bool showAddr);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_DISASSEMBLER_H


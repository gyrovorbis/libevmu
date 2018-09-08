#ifndef GYRO_VMU_CPU_H
#define GYRO_VMU_CPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "gyro_vmu_instr.h"

#define GYRO_VMU_CPU_WORD_SIZE  8

struct VMUDevice;

int gyVmuCpuTick(struct VMUDevice* dev, float deltaTime);
int gyVmuCpuReset(struct VMUDevice* dev);



void gyVmuCpuInstrExecuteNext(struct VMUDevice* dev);


void gyVmuCpuInstrExecute(struct VMUDevice* dev, const VMUInstr* instr, const VMUInstrOperands* operands);

void gyVmuBiosDisassemblyPrints(char* buff);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_CPU_H


#ifndef EVMU_CPU_H
#define EVMU_CPU_H

#include <stdint.h>
#include "evmu_peripheral.h"
#include "evmu_isa.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVMU_CPU_WORD_SIZE  8


GBL_DECLARE_HANDLE(EvmuCpu);

// log previous X instructions?
// profiling + instrumentation shit



/* EVMU_CPU particulars
 * 1) enable/disable warnings
 *    - stack overflow
 *    - flash write warning (maybe make flash handle it)
 *
 * 2) Events
 *    - instruction executed
 *    - stack changed
 *    - PSW changed
 *    - PC changed
 *    - change instruction
 *
 * 3) Debug commands:
 *    - execute asm directly?
 *    - log every instruction execution
 */

GBL_DECLARE_ENUM(EVMU_CPU_PROPERTY) {
    EVMU_CPU_PROPERTY_PROGRAM_COUNTER = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,
    EVMU_CPU_PROPERTY_TICKS,
    EVMU_CPU_PROPERTY_CLOCK_SOURCE, //SCLK, MLK or whatever
    EVMU_CPU_PROPERTY_INSTRUCTION_SOURCE, //Flash or ROM
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPCODE,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPERAND_1,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPERAND_2,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_OPERAND_3,
    EVMU_CPU_PROPERTY_CURRENT_INSTRUCTION_CYCLES,
    EVMU_CPU_PROPERTY_COUNT
};


EVMU_API evmuCpuInstructionCurrent(EvmuCpu hCpu, EvmuAddress* pPc, uint8_t* pOpcode, EvmuDecodedOperands* pOperands, EvmuWord* pIndirectAddress, uint8_t* pElapsedCycles);
EVMU_API evmuCpuProgramCounterSet(EvmuCpu hCpu, EvmuAddress pc);
EVMU_API evmuCpuInstructionExecute(EvmuCpu hCpu, const EvmuInstruction* pInstruction);

EVMU_API evmuCpuRegisterIndirectAddress(EvmuCpu hcpu, uint8_t reg, EvmuAddress* pAddress);
EVMU_API evmuCpuStackPush(EvmuCpu hCpu, const EvmuWord* pWords, EvmuSize* pCount);
EVMU_API evmuCpuStackPop(EvmuCpu hCpu, EvmuWord* pWords, EvmuSize* pCount);

// is halted
// get clock source




EVMU_API gyVmuCpuTick(EvmuCpu* pCpu, EvmuTicks ticks);

#if 0
int gyVmuCpuTick(struct VMUDevice* dev, float deltaTime);
int gyVmuCpuReset(struct VMUDevice* dev);
void gyVmuCpuInstrExecuteNext(struct VMUDevice* dev);
void gyVmuCpuInstrExecute(struct VMUDevice* dev, const VMUInstr* instr, const VMUInstrOperands* operands);
void gyVmuBiosDisassemblyPrints(char* buff);
#endif
#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_CPU_H


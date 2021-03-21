#ifndef EVMU_WRAM_H
#define EVMU_WRAM_H


#include "../hw/evmu_peripheral.h"

#define EVMU_WRAM_BANK_COUNT    2
#define EVMU_WRAM_BANK_SIZE     256

//4 - Working Memory (WRAM)
#define EVMU_WRAM_SIZE          (EVMU_WRAM_BANK_COUNT*EVMU_WRAM_BANK_SIZE)

#ifdef __cplusplus
extern "C" {
#endif

GBL_DEFINE_HANDLE(EvmuWram) // Programmable Interrupt Controller

/* SFRs owned:
 *  VSEL - Configuration, needed by Serial communications too?
 *  VRMAD1, VRMAD2 - Low and high byte of address to read/write from
 *  VTRBF - Register to read/write from to access WRAM[VRMAD1<<8|VRMAD2]
 *            Can autoincrement shitbased on register VSEL.INCE!
 */

GBL_DECLARE_ENUM(EVMU_WRAM_PROPERTY) {
    EVMU_WRAM_PROPERTY_VSEL_ADDRESS_COUNTER_AUTOINCREMENT = EVMU_PERIPHERAL_PROPERTY_BASE_COUNT,    //VSEL.INCE
    EVMU_WRAM_PROPERTY_VSEL_P1_SERIAL_MAPLE_SELECT_CONTROL,                                         //VSEL.SIOSEL (guess VMU is 0, DC is 1)
    EVMU_WRAM_PROPERTY_VSEL_DC_TRANSFER_IN_PROGRESS,                                                //VSEL.ASEL
    EVMU_WRAM_PROPERTY_ACCESS_ADDRESS,                                                              //VRMAD1 + VRMAD2
    EVMU_WRAM_PROPERTY_ADDRESS_VALUE,                                                               //current value at VRMAD address were it to be read
    EVMU_WRAM_PROPERTY_COUNT
} EVMU_WRAM_PROPERTY;


EVMU_API evmuWramRead(const EvmuWram* pWram, EvmuAddress baseAddress, EvmuWord* pData, GblSize* pBytes);
EVMU_API evmuWramWrite(const EvmuWram* pWram, EvmuAddress baseAddress, const EvmuWord* pData, GblSize* pBytes);



#ifdef __cplusplus
}
#endif

#endif // EVMU_WRAM_H

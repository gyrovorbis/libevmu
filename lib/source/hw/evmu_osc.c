#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_sfr.h>
#include "evmu_device_.h"
#if 0
unsigned gyVmuOscCyclesPerSec(EvmuDevice dev) {
    //unsigned char pcon = dev->sfr[SFR_OFFSET(SFR_ADDR_PCON)];
    unsigned char ocr = dev->sfr[SFR_OFFSET(SFR_ADDR_OCR)];
    float val;

    //INACCURATE, THERE IS A REMAINDER FROM THESE DIVISIONS!!!!
    if(ocr&SFR_OCR_OCR5_MASK) {
        val = VMU_OSC_QUARTZ_FREQ_HZ/((ocr&SFR_OCR_OCR7_MASK)? 6 : 12);
    } else {
        val =  VMU_OSC_RC_FREQ_HZ/((ocr&SFR_OCR_OCR7_MASK)? 6 : 12);
    }

    return val;
}


float gyVmuOscSecPerCycle(EvmuDevice dev) {
    float val = 1.0f/(float)(gyVmuOscCyclesPerSec(dev));
    return val;
}
#endif

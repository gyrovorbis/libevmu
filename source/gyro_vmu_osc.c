#include "gyro_vmu_osc.h"
#include "gyro_vmu_sfr.h"
#include "gyro_vmu_device.h"

uint64_t gyVmuOscCyclesPerSec(VMUDevice* dev) {
    //unsigned char pcon = dev->sfr[SFR_OFFSET(SFR_ADDR_PCON)];
    unsigned char ocr = dev->sfr[SFR_OFFSET(SFR_ADDR_OCR)];
    double val;

    //INACCURATE, THERE IS A REMAINDER FROM THESE DIVISIONS!!!!
    if(ocr&SFR_OCR_OCR5_MASK) {
        val = ((double)VMU_OSC_QUARTZ_FREQ_HZ)/((ocr&SFR_OCR_OCR7_MASK)? 6.0 : 12.0);
    } else {
        val =  ((double)VMU_OSC_RC_FREQ_HZ)/((ocr&SFR_OCR_OCR7_MASK)? 6.0 : 12.0);
    }

    return val;
}


double gyVmuOscSecPerCycle(VMUDevice *dev) {
    double val = 1.0/(double)(gyVmuOscCyclesPerSec(dev));
    return val;
}

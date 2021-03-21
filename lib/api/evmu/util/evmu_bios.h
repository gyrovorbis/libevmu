#ifndef GYRO_VMU_BIOS_H
#define GYRO_VMU_BIOS_H

#ifdef __cplusplus
extern "C" {
#endif

#define BIOS_ADDR_FM_WRT_EX     0x100
#define BIOS_ADDR_FM_WRTA_EX    0x108
#define BIOS_ADDR_FM_VRF_EX     0x110
#define BIOS_ADDR_FM_PRD_EX     0x120
#define BIOS_ADDR_TIMER_EX      0x130
#define BIOS_ADDR_SLEEP_EX      0x140
#define BIOS_ADDR_EXIT_EX       0x1f0

struct VMUDevice;

// API CALL TO DIRECTLY SET BIOS DATE/TIME

int gyVmuBiosSystemCodeActive(struct VMUDevice* device);

int gyVmuBiosLoadImage(struct VMUDevice* device, const unsigned char* buffer, int size);
int gyVmuBiosHandleCall(struct VMUDevice* dev);

#if 0
0x100 – Write flash ROM.
This call writes 128 bytes of data into the FLASH ROM at a specified location. You may only write inside a mini-game file.
Input variables: 0x07C-0x07F Start address (32 bits big endian), 0x080-0x0FF Data to write.
Returns with ACC: 0 = write ok, $FF = illegal write. Returns to address 0x105.

0x108 - Write flash alternative entry point. Seems to do the same thing, but returns to 0x10b.

0x110 – Verify flash ROM

0x120 – Read flash ROM. This call reads 128 bytes of data from the FLASH ROM. Input variables: 0x07C-0x07F Start address (32 bits big endian). Returns data into memory locations 0x080-0x0FF.

0x130 – This call updates the OS’s time-of-day clock and may check the battery state. Should be called whenever the timer/counter 1 interrupt occurs.

0x140 - This call puts the VMU to sleep until the user presses the Sleep button.

0x1f0 – Main exit vector. Using this vector exits the game and returns to clock mode or dreamcast-connect mode.
#endif


#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_BIOS_H


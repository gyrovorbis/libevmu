#ifndef GYRO_VMU_MEM_MAP_H
#define GYRO_VMU_MEM_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

//Memory Spaces

//1 - Read-Only Memory
#define ROM_SIZE                65536
#define ROM_PAGE_SIZE           4096
#define ROM_SYS_PROG_ADDR_BASE  0x0000
#define ROM_SYS_PROG_SIZE       16384
#define ROM_OS_PROG_ADDR_BASE   0xed00
#define ROM_OS_PROG_SIZE        4096

//2 - Flash Memory
#define FLASH_BANK_SIZE         65536
#define FLASH_BANKS             2
#define FLASH_SIZE              (FLASH_BANK_SIZE*FLASH_BANKS)

//3 - Video/LCD Memory (XRAM)
#define XRAM_BANK_SIZE          0x80
#define XRAM_BANK_COUNT         3

//4 - Working Memory (WRAM)
#define WRAM_SIZE               0x200

//5 - Random Access Memory
#define RAM_BANK_SIZE           256
#define RAM_BANK_COUNT          2

#define RAM_STACK_SIZE          128
#define RAM_STACK_ADDR_BASE     0x80
#define RAM_STACK_ADDR_END      0xff

#define RAM_GP_SIZE             256
#define RAM_GP_ADDR_BASE        0x00

#define RAM_SFR_SIZE            128
#define RAM_SFR_ADDR_BASE       0x100

#define RAM_REG_SIZE            16
#define RAM_REG_ADDR_BASE       0x00

//System Variables
#define RAM_ADDR_YEAR_MSB_BCD   0x10
#define RAM_ADDR_YEAR_LSB_BCD   0x11
#define RAM_ADDR_MONTH_BCD      0x12
#define RAM_ADDR_DAY_BCD        0x13
#define RAM_ADDR_HOUR_BCD       0x14
#define RAM_ADDR_MINUTE_BCD     0x15

#define RAM_ADDR_YEAR_MSB       0x17
#define RAM_ADDR_YEAR_LSB       0x18
#define RAM_ADDR_MONTH          0x19
#define RAM_ADDR_DAY            0x1a
#define RAM_ADDR_HOUR           0x1b
#define RAM_ADDR_MINUTE         0x1c
#define RAM_ADDR_SEC            0x1d
#define RAM_ADDR_HALF_SEC       0x1e //0 or 1
#define RAM_ADDR_LEAP_YEAR      0x1f //0 for no, 1 for yes

#define RAM_ADDR_CLK_INIT       0x31 //0xff - date set, 00 - date not set
#define RAM_ADDR_QUART_YEAR_MSB 0x50 //current year divided by 4 (high byte)
#define RAM_ADDR_QUART_YEAR_LSB 0x51 //current year divided by 4 (low byte)
#define RAM_ADDR_CURSOR_POS_COL 0x60 //cursor position, column (0-7)
#define RAM_ADDR_CURSOR_POS_ROW 0x61 //cursor position, row (0-3)
#define RAM_ADDR_GAME_LAST_BLK  0x6d //Last block used by mini-game
#define RAM_ADDR_BATTERY_CHECK  0x6e //Battery check flag (0xff - disable auto battery check, 0x00 - enable auto battery check)

#define SFR_OFFSET(a)           (a-RAM_SFR_ADDR_BASE)

#define SFR_XRAM_ROW_BYTES      6       //1 row = 6 bytes * 8 bits per byte = 48 pixels wide
#define SFR_XRAM_ROWS           16      //16 rows * 2 banks = 32 pixels tall

//Special Function Registers (0x100 - 0x1ff)
#define SFR_ADDR_ACC            0x100   //Accumulator
#define SFR_ADDR_PSW            0x101   //Processor Status Word
#define SFR_ADDR_B              0x102   //B Register (general purpose)
#define SFR_ADDR_C              0x103   //C Register (general purpose)
#define SFR_ADDR_TRL            0x104   //Table Reference (low byte)
#define SFR_ADDR_TRH            0x105   //Table Reference (high byte)
#define SFR_ADDR_SP             0x106   //Stack Pointer
#define SFR_ADDR_PCON           0x107   //Power Control register
#define SFR_ADDR_IE             0x108   //Interrupt Enable control
#define SFR_ADDR_IP             0x109   //Interrupt Priority Ranking control
//0x10A-0x10C Not Used
#define SFR_ADDR_EXT            0x10d   //External Memory control - Whether program is read from ROM (BIOS) or FLASH (GAME)
#define SFR_ADDR_OCR            0x10e   //Oscillation Control Register (32kHz/600kHz/6MHz)
//0x10f Not Used
#define SFR_ADDR_T0CNT          0x110   //Timer/Counter 0 control
#define SFR_ADDR_T0PRR          0x111   //Timer 0 Prescalar Data register
#define SFR_ADDR_T0L            0x112   //Timer 0 Low
#define SFR_ADDR_T0LR           0x113   //Timer 0 Low Reload register
#define SFR_ADDR_T0H            0x114   //Timer 0 High
#define SFR_ADDR_T0HR           0x115   //Timer 0 High Reload register
//0x116-0x117 Not Used
#define SFR_ADDR_T1CNT          0x118   //Timer 1 control
//0x119 Not Used
#define SFR_ADDR_T1LC           0x11a   //Timer 1 Low Compare Data register
#define SFR_ADDR_T1L            0x11b   //Timer 1 Low (Read-only)
#define SFR_ADDR_T1LR           0x11b   //Timer 1 Low Reload register (Write-only)
#define SFR_ADDR_T1HC           0x11c   //Timer 1 High Compare Data register
#define SFR_ADDR_T1H            0x11d   //Timer 1 High (Read-only)
#define SFR_ADDR_T1HR           0x11d   //Timer 1 High Reload Register (Write-only)
//0x11E-0x11F Not used
#define SFR_ADDR_MCR            0x120   //Mode Control register
//0x121 Not Used
#define SFR_ADDR_STAD           0x122   //Start Address register
#define SFR_ADDR_CNR            0x123   //Character Number register
#define SFR_ADDR_TDR            0x124   //Time Division register
#define SFR_ADDR_XBNK           0x125   //Bank Address register
//0x126 Not Used
#define SFR_ADDR_VCCR           0x127   //LCD Contrast Control register
//0x128-0x12f Not Used
#define SFR_ADDR_SCON0          0x130   //SIO0 Control register
#define SFR_ADDR_SBUF0          0x131   //SIO0 Buffer
#define SFR_ADDR_SBR            0x132   //SIO Baud Rate Generator register
//0x133 Not Used
#define SFR_ADDR_SCON1          0x134   //SI01 Control register
#define SFR_ADDR_SBUF1          0x135   //SI01 Buffer
//0x136-0x143 Not Used
#define SFR_ADDR_P1             0x144   //Port 1 Latch
#define SFR_ADDR_P1DDR          0x145   //Port 1 Data Direction register
#define SFR_ADDR_P1FCR          0x146   //Port 1 Function Control register
//0x147-0x14b Not (officially) Used
//0x148 Unknown BIOS Use
#define SFR_ADDR_P3             0x14c   //Port 3 Latch
#define SFR_ADDR_P3DDR          0x14d   //Port 3 Data Direction register
#define SFR_ADDR_P3INT          0x14e   //Port 3 Interrupt Control register
//0x14F-0x15B Not (officially) Used
//0x151 Unknown BIOS Use
#define SFR_ADDR_FLASH          0x154   //Used by LDF and STF flash instructions (in BIOS)
//0x155 Unknown BIOS Use
#define SFR_ADDR_P7             0x15c   //Port 7 Latch
#define SFR_ADDR_I01CR          0x15d   //External Interrupt 0, 1 Control
#define SFR_ADDR_I23CR          0x15e   //External Interrupt 2, 3 Control
#define SFR_ADDR_ISL            0x15f   //Input Signal Selection register
/*
 * 0x160-0x162 Not (officially) Used
The BIOS clears bits 2 and 4, and sets bits 0 and 1.

These registers seem to write data to the Maple bus from the Work RAM.
One little routine does this series of operations:

1. Write 3 to VLREG
2. Clears VSEL.0
3. sets VRMAD1 to 0 (zeros address)
4. writes 32 bytes to VTRBF
5. Sets VSEL.0
6. Sets SFR161.1
7.Waits for SFR161.0 to be set
8. loop lines 2-7
*/
#define SFR_ADDR_VSEL           0x163   //VMS Control register
#define SFR_ADDR_VRMAD1         0x164   //Work RAM Access Address 1
#define SFR_ADDR_VRMAD2         0x165   //Work RAM Access Address 2
#define SFR_ADDR_VTRBF          0x166   //Send/Receive Buffer
#define SFR_ADDR_VLREG          0x167   //Length registration
//0x168-0x17E Not Used
#define SFR_ADDR_BTCR           0x17f   //Base Time Control register
#define SFR_ADDR_XRAM_BASE      0x180   //LCD Frame Buffer base
#define SFR_ADDR_XRAM_END       0x1fb   //LCD Frame Buffer end
#define XRAM_OFFSET(a)          (a-SFR_ADDR_XRAM_BASE)
//0x1FB-0x1FF Not Used

#define SFR_ADDR_XRAM_ICN_FILE  0x181   //File Icon - XRAM Bank 2
#define SFR_ADDR_XRAM_ICN_GAME  0x182   //Game Icon - XRAM Bank 2
#define SFR_ADDR_XRAM_ICN_CLOCK 0x183   //Clock Icon - XRAM Bank 2
#define SFR_ADDR_XRAM_ICN_FLASH 0x184   //Flash Access Icon - XRAM Bank 2

//XRAM - LCD Framebuffer (0x180f- 0x1fb)
#define SFR_XRAM_BYTE(x, y)     (SFR_ADDR_XRAM_BASE+(y*SFR_XRAM_ROW_BYTES)+x)    //LCD Frame Buffer Grid

//General-Purpose Registers
#define REG_R0_OFFSET           0x0
#define REG_R1_OFFSET           0x1
#define REG_R2_OFFSET           0x2

struct  VMUDevice;

int     gyVmuMemRead(struct VMUDevice* dev, int addr);
int     gyVmuMemReadLatch(struct VMUDevice* dev, int addr);
void    gyVmuMemWrite(struct VMUDevice* dev, int addr, int val);

#ifdef __cplusplus
}
#endif

#endif // GYRO_VMU_H


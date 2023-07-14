/*! \file
 *  \brief   Defines for all known memory regions and addresses
 *  \ingroup address_space
 *  \sa      evmu_sfr.h
 *
 *  This single file serves as an amalgomation of years worth of
 *  research, scouring the internet, pouring over documents, and
 *  attepting to reverse engineer the VMU. It represents everything
 *  I know about every address compromising its main RAM address
 *  space.
 *
 *  \author    2023 Falco Girgis
 *  \copyright MIT License
 */

#ifndef EVMU_ADDRESS_SPACE_H
#define EVMU_ADDRESS_SPACE_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \defgroup address_space Address Space
 *  \brief Macro definitions for main data address space
 */

//============== Segments Within Address Space ==============
/*! \defgroup address_space_segments Segments
 *  \ingroup  address_space
 *  \brief    General info for internal/RAM addres space
 *  @{ */
#define EVMU_ADDRESS_SEGMENT_RAM_BASE       0x00                                 //!< Start address of RAM
#define EVMU_ADDRESS_SEGMENT_RAM_END        0xff                                 //!< Last address of RAM
#define EVMU_ADDRESS_SEGMENT_RAM_SIZE       256                                  //!< Size of RAM
#define EVMU_ADDRESS_SEGMENT_RAM_BANKS      2                                    //!< Number of RAM banks
#define EVMU_RAM_OFFSET(a)                  (a-EVMU_ADDRESS_SEGMENT_RAM_BASE)    //! Returns an offset relative to RAM
#define EVMU_RAM_ADDRESS(o)                 (EVMU_ADDRESS_SEGMENT_RAM_BASE+o)    //! Returns an address from RAM offset

#define EVMU_ADDRESS_SEGMENT_STACK_BASE     0x80                                 //!< Start address of stack
#define EVMU_ADDRESS_SEGMENT_STACK_END      0xff                                 //!< Last address of stack
#define EVMU_ADDRESS_SEGMENT_STACK_SIZE     128                                  //!< Stack size
#define EVMU_ADDRESS_SEGMENT_STACK_BANKS    1                                    //!< Number of stack banks
#define EVMU_STACK_OFFSET(a)                (a-EVMU_ADDRESS_SEGMENT_STACK_BASE)  //!< Returns an offset relative to stack
#define EVMU_STACK_ADDRESS(o)               (EVMU_ADDRESS_SEGMENT_STACK_BEGIN+o) //!< Returns an address from stack offset

#define EVMU_ADDRESS_SEGMENT_SFR_BASE       0x100                                //!< Start address of SFR segment
#define EVMU_ADDRESS_SEGMENT_SFR_END        0x17f                                //!< Last address of SFR segment
#define EVMU_ADDRESS_SEGMENT_SFR_SIZE       128                                  //!< Size of SFR segment
#define EVMU_ADDRESS_SEGMENT_SFR_BANKS      1                                    //!< Number of SFR banks
#define EVMU_SFR_OFFSET(a)                  (a-EVMU_ADDRESS_SEGMENT_SFR_BASE)    //!< Returns an SFR offset from address
#define EVMU_SFR_ADDRESS(o)                 (EVMU_ADDRESS_SEGMENT_SFR_BEGIN+o)   //!< Returns an address from SFR offsset

#define EVMU_XRAM_ROW_BYTES                 6                                    //!< Number of bytes per row in XRAM
#define EVMU_XRAM_ROW_COUNT                 16                                   //!< Number of rows in XRAM
#define EVMU_ADDRESS_SEGMENT_XRAM_BASE      0x180                                //!< Start address of XRAM
#define EVMU_ADDRESS_SEGMENT_XRAM_END       0x1fb                                //!< Last address of XRAM
#define EVMU_ADDRESS_SEGMENT_XRAM_SIZE      0x80                                 //!< Size of XRAM
#define EVMU_ADDRESS_SEGMENT_XRAM_BANKS     3                                    //!< Number of XRAM banks
#define EVMU_XRAM_OFFSET(a)                 (a-EVMU_ADDRESS_SEGMENT_XRAM_BASE)   //!< Returns an address from XRAM offset
#define EVMU_XRAM_ADDRESS(o)                (EVMU_ADDRESS_SEGMENT_XRAM_BASE+o)   //!< Returns an XRAM offset from address
//! @}

//================= System Variables (Ram Bank 0) ==================
/*! \defgroup system_variables System Variables
 *  \ingroup  address_space
 *  \brief    Reserved system variables (Ram Bank 0)
 *
 *  Addresses which are used by the BIOS routines for OS-related tasks
 */

/*! \defgroup system_date_time Date/Time Variables
 *  \ingroup  system_variables
 *  \brief    BIOS variables used to maintain date and time
 *  @{
 */
//  -------------------- System Date/Time Storage --------------------
#define EVMU_ADDRESS_SYSTEM_YEAR_MSB_BCD    0x10    //!< BCD Year High Byte
#define EVMU_ADDRESS_SYSTEM_YEAR_LSB_BCD    0x11    //!< BCD Year Low Byte
#define EVMU_ADDRESS_SYSTEM_MONTH_BCD       0x12    //!< BCD Month
#define EVMU_ADDRESS_SYSTEM_DAY_BCD         0x13    //!< BCD Day
#define EVMU_ADDRESS_SYSTEM_HOUR_BCD        0x14    //!< BCD Hour
#define EVMU_ADDRESS_SYSTEM_MINUTE_BCD      0x15    //!< BCD Minute
#define EVMU_ADDRESS_SYSTEM_SEC_BCD         0x16    //!< BCD Seconds(?) (MAYBE DAY OF WEEK)
#define EVMU_ADDRESS_SYSTEM_YEAR_MSB        0x17    //!< Non-BCD Year High Byte
#define EVMU_ADDRESS_SYSTEM_YEAR_LSB        0x18    //!< Non-BCD Year Low Byte
#define EVMU_ADDRESS_SYSTEM_MONTH           0x19    //!< Non-BCD Month
#define EVMU_ADDRESS_SYSTEM_DAY             0x1a    //!< Non-BCD Day
#define EVMU_ADDRESS_SYSTEM_HOUR            0x1b    //!< Non-BCD Hour
#define EVMU_ADDRESS_SYSTEM_MINUTE          0x1c    //!< Non-BCD Minute
#define EVMU_ADDRESS_SYSTEM_SEC             0x1d    //!< Non-BCD Second
#define EVMU_ADDRESS_SYSTEM_HALF_SEC        0x1e    //!< 0 or 1, driven by base-timer interrupt every 0.5s, WORK AREA, DON'T WRITE
#define EVMU_ADDRESS_SYSTEM_LEAP_YEAR       0x1f    //!< 0 for no, 1 for yes, WORK AREA, DON'T WRITE
//! @}

/*! \defgroup system-general General Variables
 *  \ingroup system_variables
 *  \brief   Known general system variables
 * @{
 */
//-------------------- General Bios Variables --------------------
#define EVMU_ADDRESS_SYSTEM_MODE            0x30    //!< probably BIOS logic... LOOKS LIKE ITS THE MODE INDEX!!!
#define EVMU_ADDRESS_SYSTEM_DATE_SET        0x31    //!< 0xff - date set, 00 - date not set
//0x33 - 0x34 seem to be INACTIVITY COUNTERS for putting the bitch to sleep!!!!
//      UNKKNOWN [32 bytes]            0x32-0x49    //!< other BIOS settings?
#define EVMU_ADDRESS_SYSTEM_QUART_YEAR_MSB  0x50    //!< current year divided by 4 (high byte)
#define EVMU_ADDRESS_SYSTEM_QUART_YEAR_LSB  0x51    //!< current year divided by 4 (low byte)
#define EVMU_ADDRESS_SYSTEM_CURSOR_POS_COL  0x60    //!< cursor position, column (0-7)
#define EVMU_ADDRESS_SYSTEM_CURSOR_POS_ROW  0x61    //!< cursor position, row (0-3)
#define EVMU_ADDRESS_SYSTEM_GAME_LAST_BLK   0x6d    //!< Last block used by mini-game
#define EVMU_ADDRESS_SYSTEM_BATTERY_CHECK   0x6e    //!< Battery check flag (0xff - disable auto battery check, 0x00 - enable auto battery check)
//      UNKNOWN [17 bytes]             0x6f-0x7f    //!< Other BIOS magical shit?
//-------------------- Stack Storage --------------------
#define EVMU_ADDRESS_SYSTEM_STACK_BASE      0x80    //!< First entry for stack storage
//      STACK STORAGE [126 BYTES]      0x81-0xfe    //!< Generic storage on the stack
#define EVMU_ADDRESS_SYSTEM_STACK_END       0xff    //!< Last entry for stack storage
/*! @} */
//============== Firmware Argument Addresses (Ram Bank 1) =======================
/*! \defgroup bios_input_variables Firmware Input Variables
 *  \ingroup  address_space
 *  \brief    BIOS input variables (RAM bank 1)
 *
 *  Ram Bank 1 variables which are used for passing values to firmware routines.
 *
 *  @{*/
#define EVMU_ADDRESS_USER_FMBANK            0x7d    //!< Flash memory read bank address
#define EVMU_ADDRESS_USER_FMADD_H           0x7e    //!< Flash memory read start upper address
#define EVMU_ADDRESS_USER_FMADD_L           0x7f    //!< Flash memory read start lower address
#define EVMU_ADDRESS_USER_FM_RET_BASE       0x80    //!< Flash read/write start address
#define EVMU_ADDRESS_USER_FM_RET_SIZE       128     //!< Flash read/write count (128 bytes)
#define EVMU_ADDRESS_USER_FM_RET_END        0xff    //!< Flash read/write end address
/*! @} */
//==================== SPECIAL FUNCTION REGISTERS ====================
/*! \defgroup sfrs Special Function Registers
 *  \ingroup  address_space
 *  \brief    Special function register locations and descriptions
 *
 *  These are registers which are reserved by the SoC and its peripherals,
 *  which typically have special usages and properties.
 *
 *  @{*/
//-------------------- CPU, Interrupts, Clocks Registers --------------------
#define EVMU_ADDRESS_SFR_ACC                0x100   //!< Accumulator
#define EVMU_ADDRESS_SFR_PSW                0x101   //!< Processor Status Word
#define EVMU_ADDRESS_SFR_B                  0x102   //!< B Register (general purpose)
#define EVMU_ADDRESS_SFR_C                  0x103   //!< C Register (general purpose)
#define EVMU_ADDRESS_SFR_TRL                0x104   //!< Table Reference (low byte)
#define EVMU_ADDRESS_SFR_TRH                0x105   //!< Table Reference (high byte)
#define EVMU_ADDRESS_SFR_SP                 0x106   //!< Stack Pointer
#define EVMU_ADDRESS_SFR_PCON               0x107   //!< Power Control register
#define EVMU_ADDRESS_SFR_IE                 0x108   //!< Interrupt Enable control
#define EVMU_ADDRESS_SFR_IP                 0x109   //!< Interrupt Priority Ranking control
//      ALL 1s                        0x10a-0x10c   //!< OPEN BUS
#define EVMU_ADDRESS_SFR_EXT                0x10d   //!< External Memory control - Whether program is read from ROM (BIOS) or FLASH (GAME)
#define EVMU_ADDRESS_SFR_OCR                0x10e   //!< Oscillation Control Register (32kHz/600kHz/6MHz)
//      ALL 1s                              0x10f   //!< OPEN BUS
//--------------------Timer 0 Config Regisers --------------------
#define EVMU_ADDRESS_SFR_T0CNT              0x110   //!< Timer 0 control
#define EVMU_ADDRESS_SFR_T0PRR              0x111   //!< Timer 0 Prescalar Data register
#define EVMU_ADDRESS_SFR_T0L                0x112   //!< Timer 0 Low Byte
#define EVMU_ADDRESS_SFR_T0LR               0x113   //!< Timer 0 Low Byte Reload register
#define EVMU_ADDRESS_SFR_T0H                0x114   //!< Timer 0 High Byte
#define EVMU_ADDRESS_SFR_T0HR               0x115   //!< Timer 0 High Byte Reload register
//      ALL 1s                              0x116   //!< OPEN BUS
//      ALL 1s                              0x117   //!< OPEN BUS
//-------------------- Timer 1 Config Registers --------------------
#define EVMU_ADDRESS_SFR_T1CNT              0x118   //!< Timer 1 control
//      ALL 1s                              0x119   //!< OPEN BUS
#define EVMU_ADDRESS_SFR_T1LC               0x11a   //!< Timer 1 Low Compare Data register
#define EVMU_ADDRESS_SFR_T1L                0x11b   //!< Timer 1 Low (Read-only)
#define EVMU_ADDRESS_SFR_T1LR               0x11b   //!< Timer 1 Low Reload register (Write-only)
#define EVMU_ADDRESS_SFR_T1HC               0x11c   //!< Timer 1 High Compare Data register
#define EVMU_ADDRESS_SFR_T1H                0x11d   //!< Timer 1 High (Read-only)
#define EVMU_ADDRESS_SFR_T1HR               0x11d   //!< Timer 1 High Reload Register (Write-only)
//      ALL 1s                        0x11e-0x11f   //!< OPEN BUS
//-------------------- LCD Controller Registers --------------------
#define EVMU_ADDRESS_SFR_MCR                0x120   //!< Mode Control register
//      ALL 1s                              0x121   //!< OPEN BUS
#define EVMU_ADDRESS_SFR_STAD               0x122   //!< Start Address register
#define EVMU_ADDRESS_SFR_CNR                0x123   //!< Character Number register
#define EVMU_ADDRESS_SFR_TDR                0x124   //!< Time Division register
#define EVMU_ADDRESS_SFR_XBNK               0x125   //!< Bank Address register
//      ALL 1s                              0x126   //!< OPEN BUS
#define EVMU_ADDRESS_SFR_VCCR               0x127   //!< LCD Contrast Control register
//      UNKNOWN [8 bytes]             0x128-0x12f   //!< LCD or Serial or Maple registers?
//-------------------- Serial Interface 0 Registers --------------------
#define EVMU_ADDRESS_SFR_SCON0              0x130   //!< SIO0 Control register
#define EVMU_ADDRESS_SFR_SBUF0              0x131   //!< SIO0 Buffer
#define EVMU_ADDRESS_SFR_SBR                0x132   //!< SIO Baud Rate Generator register
//      ALL 1s                              0x133   //!< OPEN BUS
//-------------------- Serial Interface 1 Registers --------------------
#define EVMU_ADDRESS_SFR_SCON1              0x134   //!< SI01 Control register
#define EVMU_ADDRESS_SFR_SBUF1              0x135   //!< SI01 Buffer
//      ALL 1s                        0x136-0x143   //!< Serial or Maple Xfer settings? DC Mode? Port1 config?
//-------------------- Port 1 Registers --------------------
#define EVMU_ADDRESS_SFR_P1                 0x144   //!< Port 1 Latch
#define EVMU_ADDRESS_SFR_P1DDR              0x145   //!< Port 1 Data Direction register
#define EVMU_ADDRESS_SFR_P1FCR              0x146   //!< Port 1 Function Control register
//      0xfe                                0x147   //!< Unknown Port1 configuration (All high EXCEPT bit 7 R/W)
//      ALL 1s                              0x148   //!< "unknown, rom writes this once with 0x00 only, related to SFR_x51, SFR_x55"
//      ALL 1s                        0x149-0x14b   //!< Extra Port 1 or Port3 config?
//-------------------- Port 3 Registers --------------------
#define EVMU_ADDRESS_SFR_P3                 0x14c   //!< Port 3 Latch
#define EVMU_ADDRESS_SFR_P3DDR              0x14d   //!< Port 3 Data Direction register
#define EVMU_ADDRESS_SFR_P3INT              0x14e   //!< Port 3 Interrupt Control register
//      UNKNOWN [2 bytes]             0x14f-0x150   //!< Port 3 Shit?
//      ALL 1s                              0x151   //!< "unknown, rom sets bit 5 only, related to SFR_x48, SFR_x55"
//      UNKNOWN [2 bytes]             0x152-0x153   //!< P3 or Flash shit?
//-------------------- Flash Controller Register --------------------
#define EVMU_ADDRESS_SFR_FPR                0x154   //!< Flash Program Register: Used by LDF and STF flash instructions (in BIOS)
//      ALL 1s                              0x155   //!< "unknown, rom writes this once with 0xFF only, related to SFR_x48, SFR_x51"
//      UNKNOWN [6 bytes]             0x156-0x15b   //!< Maybe more flash controller-y shit?
//-------------------- Port 7 Registers ----------
#define EVMU_ADDRESS_SFR_P7                 0x15c   //!< Port 7 Latch
#define EVMU_ADDRESS_SFR_I01CR              0x15d   //!< External Interrupt 0, 1 Control
#define EVMU_ADDRESS_SFR_I23CR              0x15e   //!< External Interrupt 2, 3 Control
#define EVMU_ADDRESS_SFR_ISL                0x15f   //!< Input Signal Selection register
//-------------------- Maple Communications Controller Registers ----------
#define EVMU_ADDRESS_SFR_MAPLETXRXCTL       0x160   //!< "UNDOC [ - | - | - | LASTPKT (Set if packet is last) | - | HW_ENA (is hw on?) | TX (TX normal packet) | TXS (tx starting packet) ]"
#define EVMU_ADDRESS_SFR_MAPLESTA           0x161   //!< "UNDOC [ - | UNK (if set causes bus to be reinited) | ERR3 | ERR2 | - | IRQREQ (cleared by handler) | ERR1 | TXDONE (Set when tx done) ] (errs s\et if RXed packet was bad)"}
#define EVMU_ADDRESS_SFR_MAPLERST           0x162   //!< Set and clear to reset Maple BUS
//      UNDISCOVERED BIOS? [3 bytes]  0x160-0x162   //!< Control WRAM<=>Maple BUS transition
/*
            0x160-0x162 Not (officially) Used
            The BIOS clears bits 2 and 4, and sets bits 0 and 1.

            These registers seem to write data to the Maple bus from the Work RAM.
            One little routine does this series of operations:

            1. Write 3 to VLREG
            2. Clears VSEL.0
            3. sets VRMAD1 to 0 (zeros address)
            4. writes 32 bytes to VTRBF
            5. Sets VSEL.0
            6. Sets SFR161.1
            7. Waits for SFR161.0 to be set
            8. loop lines 2-7

ACCORDING TO SANYO VMU SIMULATOR SCREENSHOTS IN DOCS
0x162 - Control Flag2 (VCFLG2)
    - bit 7: SRES (only one shown)
*/
//-------------------- WRAM Registers --------------------
#define EVMU_ADDRESS_SFR_VSEL           0x163   //!< VMS Control register
#define EVMU_ADDRESS_SFR_VRMAD1         0x164   //!< Work RAM Access Address 1
#define EVMU_ADDRESS_SFR_VRMAD2         0x165   //!< Work RAM Access Address 2
#define EVMU_ADDRESS_SFR_VTRBF          0x166   //!< Send/Receive Buffer
#define EVMU_ADDRESS_SFR_VLREG          0x167   //!< Length registration, # of maple words to send on BUS
//      UNKNOWN [23 bytes]        0x168-0x17e   //!< WRAM/maple control or base timer shit?
//-------------------- Base Timer Registers --------------------
#define EVMU_ADDRESS_SFR_BTCR           0x17f   //!< Base Time Control register
/*! @} */

//==================== XRAM Registers ====================
/*! \defgroup xram_sfrs XRAM Registers
 *  \ingroup  address_space
 *  \brief    Special registers within XRAM region
 *
 *  These are addresses within the XRAM memory region,
 *  which is basically used as VRAM and a framebuffer.
 *
 *  @{*/
// -------------------- LCD Framebuffer (0x180f - 0x1fb) [Banks 0 + 1 ] --------------------
//! Retrurns the XRAM byte containing the pixel value at the given location
#define EVMU_ADDRESS_XRAM_BYTE(x, y)   \
    (EVMU_ADDRESS_SEGMENT_XRAM_BASE+(y*EVMU_XRAM_ROW_BYTES)+x)    //LCD Frame Buffer Grid

// -------------------- BIOS Icons [Bank 2] --------------------
//      FREE RAM                        0x180
#define EVMU_ADDRESS_XRAM_ICN_FILE      0x181   //!< File Icon - XRAM Bank 2
#define EVMU_ADDRESS_XRAM_ICN_GAME      0x182   //!< Game Icon - XRAM Bank 2
#define EVMU_ADDRESS_XRAM_ICN_CLOCK     0x183   //!< Clock Icon - XRAM Bank 2
#define EVMU_ADDRESS_XRAM_ICN_FLASH     0x184   //!< Flash Access Icon - XRAM Bank 2
//      FREE RAM                        0x185-18b
//      ALL 1s                          0x18c-0x1ff
//==================== EXTRA? ====================
//      UNKNOWN [4 bytes]         0x1fc-0x1ff   //!< End of address-space... anything else going on? In Xram?
/*! @} */

#ifdef __cplusplus
}
#endif

#endif // EVMU_ADDRESS_SPACE_H

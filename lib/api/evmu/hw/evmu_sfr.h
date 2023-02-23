#ifndef EVMU_SFR_H
#define EVMU_SFR_H

#ifdef __cplusplus
extern "C" {
#endif

//PSW - Processor Status Word (0x101)
#define EVMU_SFR_PSW_CY_POS          7       //Carry Flag - Set when carry from MSB occurs in addition
#define EVMU_SFR_PSW_CY_MASK         0x80    //  or borrow to MSB occurs in subtraction. Cleared if neither happens.
#define EVMU_SFR_PSW_AC_POS          6       //Auxiliary Carry Flag - Works like regular carry flab, but
#define EVMU_SFR_PSW_AC_MASK         0x40    //  registers carry from or borrow to third bit (nyble carry)
#define EVMU_SFR_PSW_IRBK1_POS       4       //Indirect Address Register Bank - contains base address for
#define EVMU_SFR_PSW_IRBK1_MASK      0x10    //  indirect addressing
#define EVMU_SFR_PSW_IRBK0_POS       3
#define EVMU_SFR_PSW_IRBK0_MASK      0x8
#define EVMU_SFR_PSW_OV_POS          2       //Overflow Flag - set when overflow occurs in signed addition or subtraction or
#define EVMU_SFR_PSW_OV_MASK         0x4     //  result of a multiplication exceeds 256. Set to 0 when divide by 0 is performed
#define EVMU_SFR_PSW_RAMBK0_POS      1       //RAM Bank - Selects 1 of two banks for 256 general-purpose RAM bytes at (0x0-0xff)
#define EVMU_SFR_PSW_RAMBK0_MASK     0x2     //  Bank 0 for system and CPU stack. Bank 1 for game software (firmware automatically changes before entering game mode)
#define EVMU_SFR_PSW_P_POS           0       //ACC Parity - set when number of bits in accumulator is odd
#define EVMU_SFR_PSW_P_MASK          0x1     //  Read-Only

//PCON - Power Control Register (0x107)
#define EVMU_SFR_PCON_HOLD_POS       1       //Stop CPU and Timers - When 1, puts the VMS into deep suspension
#define EVMU_SFR_PCON_HOLD_MASK      0x2     //  CPU, all timers and clock stop, only external interrupt or reset resumes
#define EVMU_SFR_PCON_HALT_POS       0       //Suspend Program Execution - When 1, causes CPU to stop executing instructions
#define EVMU_SFR_PCON_HALT_MASK      0x1     //  all timers and peripherals continue normally, and any interrupt wakes CPU up

//IE - Interrupt Enable Control (0x108)
#define EVMU_SFR_IE_IE7_POS          7       //Master Interrupt Enable - when 0, all interrupts blocked except nonmaskable ones
#define EVMU_SFR_IE_IE7_MASK         0x80    //  only external interrupts INT0 and INT1 can be set to nonmaskable
#define EVMU_SFR_IE_IE1_POS          2       //INT1 Priority Level - when 1, priority of external interrupt INT1 is lowered from
#define EVMU_SFR_IE_IE1_MASK         0x2     //  nonmaskable to low
#define EVMU_SFR_IE_IE0_POS          1       //INT0/INT1 Priority Level - When 1, priority of both external interrupts INT0 and INT1 are
#define EVMU_SFR_IE_IE0_MASK         0x1     //lowered from nonmaskable to low

//IP - Interrupt Priority Control (0x109)
#define EVMU_SFR_IP_P3_POS           7
#define EVMU_SFR_IP_P3_MASK          0x80
#define EVMU_SFR_IP_RBF_POS          6       //I'm assuming, undocumented
#define EVMU_SFR_IP_RBF_MASK         0x40    //Undocumented, assuming!
#define EVMU_SFR_IP_SIO1_POS         5
#define EVMU_SFR_IP_SIO1_MASK        0x20
#define EVMU_SFR_IP_SIO0_POS         4
#define EVMU_SFR_IP_SIO0_MASK        0x10
#define EVMU_SFR_IP_T1_POS           3
#define EVMU_SFR_IP_T1_MASK          0x8
#define EVMU_SFR_IP_T0H_POS          2
#define EVMU_SFR_IP_T0H_MASK         0x4
#define EVMU_SFR_IP_INT3_POS         1
#define EVMU_SFR_IP_INT3_MASK        0x2
#define EVMU_SFR_IP_INT2_POS         0
#define EVMU_SFR_IP_INT2_MASK        0x1

//T0CNT - Timer 0 Control (0x110)
#define EVMU_SFR_T0CNT_T0LIE_POS     0       //T0L interrupt request enabled - 1: enabled 0: disabled
#define EVMU_SFR_T0CNT_T0LIE_MASK    0x1
#define EVMU_SFR_T0CNT_T0LOVF_POS    1       //T0L overflow flag - 1: overflow 0: no overflow
#define EVMU_SFR_T0CNT_T0LOVF_MASK   0x2
#define EVMU_SFR_T0CNT_T0HIE_POS     2       //T0H interrupt request enabled - 1: enabled 0: disabled
#define EVMU_SFR_T0CNT_T0HIE_MASK    0x4
#define EVMU_SFR_T0CNT_P0HOVF_POS    3       //T0H overflow flag - 1: overflow 0: no overflow
#define EVMU_SFR_T0CNT_P0HOVF_MASK   0x8
#define EVMU_SFR_T0CNT_P0LEXT_POS    4       //T0L input clock select - 0: Prescaler output
#define EVMU_SFR_T0CNT_P0LEXT_MASK   0x10    // 1: External pin input signal Pin for external input can be specified by input select register (ISL)
#define EVMU_SFR_T0CNT_P0LONG_POS    5       //Timer/counter 0 bit length selector - 0: 8-bit 1: 16-bit
#define EVMU_SFR_T0CNT_P0LONG_MASK   0x20
#define EVMU_SFR_T0CNT_P0LRUN_POS    6       //T0L count control - 0: count stop/data reload 1: count start
#define EVMU_SFR_T0CNT_P0LRUN_MASK   0x40
#define EVMU_SFR_T0CNT_P0HRUN_POS    7       //T0H count control - 0: count stop/data reload 1: count start
#define EVMU_SFR_T0CNT_P0HRUN_MASK   0x80

//EXT - External Memory Control (0x10d)
//#define EVMU_SFR_EXT_MASK            0x01
//#define EVMU_SFR_EXT_POS             0
#define EVMU_SFR_EXT_ROM             0x08    //value of EXT register for when external memory uses internal ROM
#define EVMU_SFR_EXT_FLASH_BANK_0    0x01    //value of EXT register for when external memory uses flash bank 0
#define EVMU_SFR_EXT_FLASH_BANK_1    0x00    //value of EXT register for when external memory uses flash bank 1
/*
Bios initializes bit 3 to 1 and never EVER changes it.
  set1  ext, $03
  Almost positive it controls how the LDC instruction is sourced when in SYSTEM mode.
  0: LDC always fetches from Flash
  1: LDC fetches from flash in user mode and ROM in system mode
  */

//OCR - Oscillation Control Register (0x10e)
#define EVMU_SFR_OCR_OCR7_POS        7       //Clock Divisor - when 0, divides clock frequency by 12
#define EVMU_SFR_OCR_OCR7_MASK       0x80    //  when 1, divides clock frequency by 6
#define EVMU_SFR_OCR_OCR5_POS        5       //Subclock Mode Enabled - when 1, enables sublock mode (32kHz) (slow and power conserving)
#define EVMU_SFR_OCR_OCR5_MASK       0x20    //  when 0, enables RC clock mode (600kHz), uses more power
#define EVMU_SFR_OCR_OCR4_POS        4       //Main Clock Mode Enable - when 1, enables main clock (6Mhz)
#define EVMU_SFR_OCR_OCR4_MASK       0x10    //  should only be set when subclock mode is disabled and plugged into controller
#define EVMU_SFR_OCR_OCR1_POS        1       //RC Clock Control - when 1, stops RC oscillator
#define EVMU_SFR_OCR_OCR1_MASK       0x2     //  preserves power, can only be done in subclock mode
#define EVMU_SFR_OCR_OCR0_POS        0       //Main Clock Control - when 1, stops main clock
#define EVMU_SFR_OCR_OCR0_MASK       0x1     //  should always be set when VMS isn't docked

//T1CNT - Timer 1 Control (0x118)
#define EVMU_SFR_T1CNT_T1HRUN_POS    7       //Timer 1 High Running - 1 starts T1H timer, 0 stops it
#define EVMU_SFR_T1CNT_T1HRUN_MASK   0x80
#define EVMU_SFR_T1CNT_T1LRUN_POS    6       //Timer 1 Low Running - 1 starts T1L timer, 0 stops it
#define EVMU_SFR_T1CNT_T1LRUN_MASK   0x40
#define EVMU_SFR_T1CNT_T1LONG_POS    5       //16-bit Timer Enable - 1 combines T1H and T1L into one 16-bit timer
#define EVMU_SFR_T1CNT_T1LONG_MASK   0x20    //  0 keeps them two separate 8-bit timers
#define EVMU_SFR_T1CNT_ELDT1C_POS    4       //Timer 1 Compare Data Load Enable - When 0, writes to T1LC and T1HC will not take effect until set to 1 again.
#define EVMU_SFR_T1CNT_ELDT1C_MASK   0x10    //  Can be used to set 16-bit compare value automatically (ignored when no timers are running)
#define EVMU_SFR_T1CNT_T1HOVF_POS    3       //Timer 1 High Overflow - set to 1 when T1H overflows
#define EVMU_SFR_T1CNT_T1HOVF_MASK   0x8
#define EVMU_SFR_T1CNT_T1HIE_POS     2       //Timer 1 High Interrupt Enable
#define EVMU_SFR_T1CNT_T1HIE_MASK    0x4     //  When 1, enableds interrupts (vector $2B) when T1H overflows
#define EVMU_SFR_T1CNT_T1LOVF_POS    1       //Timer 1 Low Overflow
#define EVMU_SFR_T1CNT_T1LOVF_MASK   0x2     // Set to 1 whenever T1L overflows
#define EVMU_SFR_T1CNT_T1LIE_POS     0       //Timer 1 Low Interrupt Enable
#define EVMU_SFR_T1CNT_T1LIE_MASK    0x1     //  When 1, enables interrupts (vector $2B) when T1L overflows

//MCR - Mode Control Register (0x120)
#define EVMU_SFR_MCR_MCR4_POS        4       //Refresh Rate - 1 gives display refresh rate of 166Hz
#define EVMU_SFR_MCR_MCR4_MASK       0x10    //  0 gives refresh rate of 83Hz
#define EVMU_SFR_MCR_MCR3_POS        3       //Refresh Control - 0 stops refreshing LCD
#define EVMU_SFR_MCR_MCR3_MASK       0x8
#define EVMU_SFR_MCR_MCR0_POS        0       //Graphics Mode - 1 selects graphics mode
#define EVMU_SFR_MCR_MCR0_MASK       0x1     //  0 no fucking clue

//VCCR - LCD Contrast Control Register (0x127)
#define EVMU_SFR_VCCR_VCCR7_POS      7       //Enable (1) LCD Display
#define EVMU_SFR_VCCR_VCCR7_MASK     0x80

//SCON0 - Serial Control Register 0 (0x130)
#define EVMU_SFR_SCON0_POL_POS       7       //Polarity - 0: maintain, 1: output = SBUF.0
#define EVMU_SFR_SCON0_POL_MASK      0x80
#define EVMU_SFR_SCON0_OV_POS        6       //Overrun flag - 1: true, 0: false
#define EVMU_SFR_SCON0_OV_MASK       0x40
#define EVMU_SFR_SCON0_LEN_POS       4       //Transfer bit length - 0: 8-bit xfer, 1: continuous xfer
#define EVMU_SFR_SCON0_LEN_MASK      0x10
#define EVMU_SFR_SCON0_CTRL_POS      3       //Transfer control - 1: start, 0: stop
#define EVMU_SFR_SCON0_CTRL_MASK     0x8
#define EVMU_SFR_SCON0_MSB_POS       2       //MSB/LSB select - 1: msb first, 0: lsb first
#define EVMU_SFR_SCON0_MSB_MASK      0x4
#define EVMU_SFR_SCON0_END_POS       1       //Transfer end flag - 1: end 0: in progress
#define EVMU_SFR_SCON0_END_MASK      0x2
#define EVMU_SFR_SCON0_IE_POS        0       //Interrupt enabled - 1: enabled, 0: disabled
#define EVMU_SFR_SCON0_IE_MASK       0x1

//#define EVMU_SFR_SCON1          0x134      //SI01 Control register
#define EVMU_SFR_SCON1_OV_POS        6       //Overrun flag - 1: true, 0: false
#define EVMU_SFR_SCON1_OV_MASK       0x40
#define EVMU_SFR_SCON1_LEN_POS       4       //Transfer bit length - 0: 8-bit xfer, 1: continuous xfer
#define EVMU_SFR_SCON1_LEN_MASK      0x10
#define EVMU_SFR_SCON1_CTRL_POS      3       //Transfer control - 1: start, 0: stop
#define EVMU_SFR_SCON1_CTRL_MASK     0x8
#define EVMU_SFR_SCON1_MSB_POS       2       //MSB/LSB select - 1: msb first, 0: lsb first
#define EVMU_SFR_SCON1_MSB_MASK      0x4
#define EVMU_SFR_SCON1_END_POS       1       //Transer end flag - 1: end 0: in progress
#define EVMU_SFR_SCON1_END_MASK      0x2
#define EVMU_SFR_SCON1_IE_POS        0       //Interrupt enabled - 1: enabled, 0: disabled
#define EVMU_SFR_SCON1_IE_MASK       0x1

//P1 - Port 1 Latch (0x144)
#define EVMU_SFR_P1_P17_POS          7
#define EVMU_SFR_P1_P17_MASK         0x80
#define EVMU_SFR_P1_PIN12_POS        5       //Connected to external connector on VMS unit
#define EVMU_SFR_P1_PIN12_MASK       0x20
#define EVMU_SFR_P1_PIN10_POS        4
#define EVMU_SFR_P1_PIN10_MASK       0x10
#define EVMU_SFR_P1_PIN11_POS        3
#define EVMU_SFR_P1_PIN11_MASK       0x8
#define EVMU_SFR_P1_PIN3_POS         2
#define EVMU_SFR_P1_PIN3_MASK        0x4
#define EVMU_SFR_P1_PIN4_POS         1
#define EVMU_SFR_P1_PIN4_MASK        0x2
#define EVMU_SFR_P1_PIN5_POS         0
#define EVMU_SFR_P1_PIN5_MASK        0x1

//P1DDR - Port 1 Data Direction Register (0x145)
#define EVMU_SFR_P1DDR_P17DDR_POS    7
#define EVMU_SFR_P1DDR_P17DDR_MASK   0x80
#define EVMU_SFR_P1DDR_P16DDR_POS    6
#define EVMU_SFR_P1DDR_P16DDR_MASK   0x40
#define EVMU_SFR_P1DDR_P15DDR_POS    5
#define EVMU_SFR_P1DDR_P15DDR_MASK   0x20
#define EVMU_SFR_P1DDR_P14DDR_POS    4
#define EVMU_SFR_P1DDR_P14DDR_MASK   0x10
#define EVMU_SFR_P1DDR_P13DDR_POS    3
#define EVMU_SFR_P1DDR_P13DDR_MASK   0x8
#define EVMU_SFR_P1DDR_P12DDR_POS    2
#define EVMU_SFR_P1DDR_P12DDR_MASK   0x4
#define EVMU_SFR_P1DDR_P11DDR_POS    1
#define EVMU_SFR_P1DDR_P11DDR_MASK   0x2
#define EVMU_SFR_P1DDR_P10DDR_POS    0
#define EVMU_SFR_P1DDR_P10DDR_MASK   0x1

//P1FCR - Port 1 Function Control Register (0x146)
#define EVMU_SFR_P1FCR_P17FCR_POS    7
#define EVMU_SFR_P1FCR_P17FCR_MASK   0x80
#define EVMU_SFR_P1FCR_P16FCR_POS    6
#define EVMU_SFR_P1FCR_P16FCR_MASK   0x40
#define EVMU_SFR_P1FCR_P15FCR_POS    5
#define EVMU_SFR_P1FCR_P15FCR_MASK   0x20
#define EVMU_SFR_P1FCR_P14FCR_POS    4
#define EVMU_SFR_P1FCR_P14FCR_MASK   0x10
#define EVMU_SFR_P1FCR_P13FCR_POS    3
#define EVMU_SFR_P1FCR_P13FCR_MASK   0x8
#define EVMU_SFR_P1FCR_P12FCR_POS    2
#define EVMU_SFR_P1FCR_P12FCR_MASK   0x4
#define EVMU_SFR_P1FCR_P11FCR_POS    1
#define EVMU_SFR_P1FCR_P11FCR_MASK   0x2
#define EVMU_SFR_P1FCR_P10FCR_POS    0
#define EVMU_SFR_P1FCR_P10FCR_MASK   0x1

//P3 - Port 3 Latch (0x14c)
#define EVMU_SFR_P3_SLEEP_POS        7       //Button Press States
#define EVMU_SFR_P3_SLEEP_MASK       0x80    //0 - Pressed
#define EVMU_SFR_P3_MODE_POS         6       //1 - Released
#define EVMU_SFR_P3_MODE_MASK        0x40
#define EVMU_SFR_P3_B_POS            5
#define EVMU_SFR_P3_B_MASK           0x20
#define EVMU_SFR_P3_A_POS            4
#define EVMU_SFR_P3_A_MASK           0x10
#define EVMU_SFR_P3_RIGHT_POS        3
#define EVMU_SFR_P3_RIGHT_MASK       0x8
#define EVMU_SFR_P3_LEFT_POS         2
#define EVMU_SFR_P3_LEFT_MASK        0x4
#define EVMU_SFR_P3_DOWN_POS         1
#define EVMU_SFR_P3_DOWN_MASK        0x2
#define EVMU_SFR_P3_UP_POS           0
#define EVMU_SFR_P3_UP_MASK          0x1

//P3INT - Port 3 Interrupt Control Register (0x14e)
#define EVMU_SFR_P3INT_P32INT_POS    2       //Set to 1 to allow P3 input to break HOLD mode
#define EVMU_SFR_P3INT_P32INT_MASK   0x4
#define EVMU_SFR_P3INT_P31INT_POS    1       //Set to 1 when there is input on P3, must be cleared manually by ISR
#define EVMU_SFR_P3INT_P31INT_MASK   0x2
#define EVMU_SFR_P3INT_P30INT_POS    0       //Set to 1 to enable interrupts when P1INT is set
#define EVMU_SFR_P3INT_P30INT_MASK   0x1

//FLASH - Flash Program Register (0x154)
#define EVMU_SFR_FPR_UNLOCK_MASK    0x2
#define EVMU_SFR_FPR_UNLOCK_POS     1        //Set to true when entering flash write unlock sequence for STF
#define EVMU_SFR_FPR_ADDR_MASK      0x1
#define EVMU_SFR_FPR_ADDR_POS       0        //Used as 9th (17th!?!?!?) bit for flash address calculations

//P7 - Port 7 Latch (0x15c)
#define EVMU_SFR_P7_P73_POS         3       //External connector pin 6 state (VMU connection)
#define EVMU_SFR_P7_P73_MASK        0x8
#define EVMU_SFR_P7_P72_POS         2       //External connector pin 13 state (DC connection)
#define EVMU_SFR_P7_P72_MASK        0x4
#define EVMU_SFR_P7_P71_POS         1       //Battery voltage - 0 when battery is low, disabled with system var 0x6e
#define EVMU_SFR_P7_P71_MASK        0x2
#define EVMU_SFR_P7_P70_POS         0       //External voltage - 1 when plugged into conroller (5v)
#define EVMU_SFR_P7_P70_MASK        0x1

//VSEL - VMS Control Register (0x163)
#define EVMU_SFR_VSEL_INCE_POS      4       //Auto-Increment - If set, VRMAD1/VRMAD2 automatically incremented
#define EVMU_SFR_VSEL_INCE_MASK     0x10    //  after a load or store to VTRBF

//BTCR - Base Timer Control Register (0x17f)
#define EVMU_SFR_BTCR_INT0_CYCLE_CTRL_POS   7
#define EVMU_SFR_BTCR_INT0_CYCLE_CTRL_MASK  0x80
#define EVMU_SFR_BTCR_OP_CTRL_POS           6
#define EVMU_SFR_BTCR_OP_CTRL_MASK          0x40
#define EVMU_SFR_BTCR_INT1_CYCLE_CTRL_POS   4
#define EVMU_SFR_BTCR_INT1_CYCLE_CTRL_MASK  0x30
#define EVMU_SFR_BTCR_INT1_SRC_POS          3
#define EVMU_SFR_BTCR_INT1_SRC_MASK         0x8
#define EVMU_SFR_BTCR_INT1_REQ_EN_POS       2
#define EVMU_SFR_BTCR_INT1_REQ_EN_MASK      0x4
#define EVMU_SFR_BTCR_INT0_SRC_POS          1
#define EVMU_SFR_BTCR_INT0_SRC_MASK         0x2
#define EVMU_SFR_BTCR_INT0_REQ_EN_POS       0
#define EVMU_SFR_BTCR_INT0_REQ_EN_MASK      0x1

//XRAM - Icons (0x181-0x184)
#define EVMU_SFR_XRAM_ICN_FILE_POS          6       //Icon data stored in XRAM bank 2
#define EVMU_SFR_XRAM_ICN_FILE_MASK         0x40
#define EVMU_SFR_XRAM_ICN_GAME_POS          4
#define EVMU_SFR_XRAM_ICN_GAME_MASK         0x10
#define EVMU_SFR_XRAM_ICN_CLOCK_POS         2
#define EVMU_SFR_XRAM_ICN_CLOCK_MASK        0x4
#define EVMU_SFR_XRAM_ICN_FLASH_POS         0
#define EVMU_SFR_XRAM_ICN_FLASH_MASK        0x1

#ifdef __cplusplus
}
#endif

#endif // EVMU_SFR_H


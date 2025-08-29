/*
 *   OpenBIOS Wii "Flipper" GX driver
 *
 *   (C) 2025 John Davis
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   version 2
 *
 */

#ifndef FLIPPER_GX_H
#define FLIPPER_GX_H

//
// Video Interface registers.
//
#define WII_VI_BASE       0x0C002000
#define WII_VI_LENGTH     0x100

#define WII_VI_FBWIDTH    640
#define WII_VI_FBHEIGHT   480
#define WII_VI_FBDEPTH    2
#define WII_VI_FBSTRIDE   (WII_VI_FBWIDTH * WII_VI_FBDEPTH)

#define BIT0  (1<<0)
#define BIT1  (1<<1)
#define BIT2  (1<<2)
#define BIT3  (1<<3)
#define BIT4  (1<<4)
#define BIT5  (1<<5)
#define BIT6  (1<<6)
#define BIT7  (1<<7)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)
#define BIT16 (1<<16)
#define BIT17 (1<<17)
#define BIT18 (1<<18)
#define BIT19 (1<<19)
#define BIT20 (1<<20)
#define BIT21 (1<<21)
#define BIT22 (1<<22)
#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)
#define BIT27 (1<<27)
#define BIT28 (1<<28)
#define BIT29 (1<<29)
#define BIT30 (1<<30)
#define BIT31 (1<<31)

// Vertical Timing.
#define WII_VI_REG_VTR                0x00

// Display configuration.
#define WII_VI_REG_DCR                0x02
#define WII_VI_REG_DCR_ENABLE         BIT0
#define WII_VI_REG_DCR_RESET          BIT1
#define WII_VI_REG_DCR_NOINTERLACE    BIT2
#define WII_VI_REG_DCR_DLR            BIT3
#define WII_VI_REG_DCR_LE0_OFF        0
#define WII_VI_REG_DCR_LE0_1F         BIT4
#define WII_VI_REG_DCR_LE0_2F         BIT5
#define WII_VI_REG_DCR_LE0_AF         (BIT4 | BIT5)
#define WII_VI_REG_DCR_LE1_OFF        0
#define WII_VI_REG_DCR_LE1_1F         BIT6
#define WII_VI_REG_DCR_LE1_2F         BIT7
#define WII_VI_REG_DCR_LE1_AF         (BIT6 | BIT7)
#define WII_VI_REG_DCR_FMT_NTSC       0
#define WII_VI_REG_DCR_FMT_PAL        BIT8
#define WII_VI_REG_DCR_FMT_MPAL       BIT9

// Horizontal Timing 0.
#define WII_VI_REG_HTR0               0x04
// Horizontal Timing 1.
#define WII_VI_REG_HTR1               0x08
// Odd Field Vertical Timing.
#define WII_VI_REG_VTO                0x0C
// Even Field Vertical Timing.
#define WII_VI_REG_VTE                0x10
// Odd Field Burst Blanking Interval.
#define WII_VI_REG_BBEI               0x14
// Even Field Burst Blanking Interval.
#define WII_VI_REG_BBOI               0x18
// Top Field Base Register Left.
#define WII_VI_REG_TFBL               0x1C
#define WII_VI_REG_TFBL_PAGE_OFFSET   BIT28
// Top Field Base Register Right.
#define WII_VI_REG_TFBR               0x20
// Bottom Field Base Register Left.
#define WII_VI_REG_BFBL               0x24
#define WII_VI_REG_BFBL_PAGE_OFFSET   BIT28
// Bottom Field Base Register Right.
#define WII_VI_REG_BFBR               0x28
// Current Vertical Position.
#define WII_VI_REG_DPV                0x2C
// Current Horizontal Position.
#define WII_VI_REG_DPH                0x2E
// Display Interrupts.
#define WII_VI_REG_DI0                0x30
#define WII_VI_REG_DI1                0x34
#define WII_VI_REG_DI2                0x38
#define WII_VI_REG_DI3                0x3C
// Display Latch Registers.
#define WII_VI_REG_DL0                0x40
#define WII_VI_REG_DL1                0x44
// Scaling Width.
#define WII_VI_REG_HSW                0x48
// Horizontal Scaling.
#define WII_VI_REG_HSR                0x4A
// Filter Coefficient Table 0.
#define WII_VI_REG_FCT0               0x4C
// Filter Coefficient Table 1.
#define WII_VI_REG_FCT1               0x50
// Filter Coefficient Table 2.
#define WII_VI_REG_FCT2               0x54
// Filter Coefficient Table 3.
#define WII_VI_REG_FCT3               0x58
// Filter Coefficient Table 4.
#define WII_VI_REG_FCT4               0x5C
// Filter Coefficient Table 5.
#define WII_VI_REG_FCT5               0x60
// Filter Coefficient Table 6.
#define WII_VI_REG_FCT6               0x64
// AA.
#define WII_VI_REG_AA                 0x68
// VI Clock Select.
#define WII_VI_REG_VICLK              0x6C
// VI DTV Status.
#define WII_VI_REG_VISEL              0x6E

#endif

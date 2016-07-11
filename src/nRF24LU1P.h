/*
  Copyright (C) 2016 Bastille Networks

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef NRF24LU1P_H
#define NRF24LU1P_H

#include <stdbool.h>
#include <stdint.h>

// NOP for 1us
#define nop_us() \
      __asm      \
        nop      \
        nop      \
        nop      \
        nop      \
      __endasm   \

// Microsecond delay
inline void delay_us(uint16_t us) { do nop_us(); while(--us); }

// Shift feedback registers
__sfr __at (0xE6) rfctl;          // ref: nRF24LU1+ Product Spec, Section 6.5.1, Table 20
__sfr __at (0x90) rfcon;          // ref: nRF24LU1+ Product Spec, Section 6.5.1, Table 21
__sfr __at (0xA0) usbcon;         // ref: nRF24LU1+ Product Spec, Section 7.3, Table 24
__sfr __at (0xF2) AESIV;          // ref: nRF24LU1+ Product Spec, Section 8.2, Table 70
__sfr __at (0xF5) AESIA1;         // ref: nRF24LU1+ Product Spec, Section 8.2, Table 71
__sfr __at (0x80) P0;             // ref: nRF24LU1+ Product Spec, Section 13.1, Table 94
__sfr __at (0x94) P0DIR;          // ref: nRF24LU1+ Product Spec, Section 13.1, Table 95
__sfr __at (0xE5) RFDAT;          // ref: nRF24LU1+ Product Spec, Section 15.1.2, Table 108
__sfr __at (0xAB) TICKDV;         // ref: nRF24LU1+ Product Spec, Section 19.3.2, Table 128
__sfr __at (0xAB) REGXH;          // ref: nRF24LU1+ Product Spec, Section 19.3.6, Table 129
__sfr __at (0xAC) REGXL;          // ref: nRF24LU1+ Product Spec, Section 19.3.6, Table 129
__sfr __at (0xAD) REGXC;          // ref: nRF24LU1+ Product Spec, Section 19.3.6, Table 129
__sfr __at (0xA8) ien0;           // ref: nRF24LU1+ Product Spec, Section 22.4.1, Table 139
__sfr __at (0xB8) ien1;           // ref: nRF24LU1+ Product Spec, Section 22.4.2, Table 140

// SFR bits
__sbit __at (0x90) rfce;          // ref: nRF24LU1+ Product Spec, Section 6.5.1, Table 21
__sbit __at (0x91) rfcsn;         // ref: nRF24LU1+ Product Spec, Section 6.5.1, Table 21
__sbit __at (0xC0) RFRDY;         // ref: nRF24LU1+ Product Spec, Section 22.4.4, Table 146

// Memory mapped register
#define __xreg(A) (*((__xdata uint8_t *)A))

// Memory mapped registers
#define bout1addr  __xreg(0xC781) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define bout2addr  __xreg(0xC782) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define binstaddr  __xreg(0xC788) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define bin1addr   __xreg(0xC789) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define bin2addr   __xreg(0xC78A) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define out1bc     __xreg(0xC7C7) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define in0bc      __xreg(0xC7B5) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define in1bc      __xreg(0xC7B7) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define ivec       __xreg(0xC7A8) // ref: nRF24LU1+ Product Spec, Section 7.3, Table 26
#define in_irq     __xreg(0xC7A9) // ref: nRF24LU1+ Product Spec, Section 7.9.8, Table 42
#define out_irq    __xreg(0xC7AA) // ref: nRF24LU1+ Product Spec, Section 7.9.9, Table 43
#define usbirq     __xreg(0xC7AB) // ref: nRF24LU1+ Product Spec, Section 7.9.10, Table 44
#define in_ien     __xreg(0xC7AC) // ref: nRF24LU1+ Product Spec, Section 7.9.11, Table 45
#define out_ien    __xreg(0xC7AD) // ref: nRF24LU1+ Product Spec, Section 7.9.12, Table 46
#define usbien     __xreg(0xC7AE) // ref: nRF24LU1+ Product Spec, Section 7.9.13, Table 47
#define ep0cs      __xreg(0xC7B4) // ref: nRF24LU1+ Product Spec, Section 7.9.14, Table 48
#define in1cs      __xreg(0xC7B6) // ref: nRF24LU1+ Product Spec, Section 7.9.16, Table 50
#define out1cs     __xreg(0xC7C6) // ref: nRF24LU1+ Product Spec, Section 7.9.17, Table 53
#define usbcs      __xreg(0xC7D6) // ref: nRF24LU1+ Product Spec, Section 7.9.19, Table 55
#define inbulkval  __xreg(0xC7DE) // ref: nRF24LU1+ Product Spec, Section 7.9.24, Table 60
#define outbulkval __xreg(0xC7DF) // ref: nRF24LU1+ Product Spec, Section 7.9.25, Table 61
#define inisoval   __xreg(0xC7E0) // ref: nRF24LU1+ Product Spec, Section 7.9.26, Table 62
#define outisoval  __xreg(0xC7E1) // ref: nRF24LU1+ Product Spec, Section 7.9.27, Table 63

// XDATA buffers
uint8_t __at (0xC700) in0buf[64];
uint8_t __at (0xC680) in1buf[64];
uint8_t __at (0xC640) out1buf[64];
uint8_t __at (0xC7E8) setupbuf[8];

/*************************************
 * Radio SPI registers and constants *
 *************************************/

// Configuration
enum CONFIG
{
  PRIM_RX     = 0x01,
  PWR_UP      = 0x02,
  CRC0        = 0x04,
  EN_CRC      = 0x08,
  MASK_MAX_RT = 0x10,
  MASK_TX_DS  = 0x20,
  MASK_RX_DR  = 0x40,
};

// Auto Acknowledgement
enum EN_AA
{
  ENAA_NONE = 0x00,
  ENAA_P0   = 0x01,
  ENAA_P1   = 0x02,
  ENAA_P2   = 0x04,
  ENAA_P3   = 0x08,
  ENAA_P4   = 0x10,
  ENAA_P5   = 0x20,
};

// Enabled RX Addresses
enum EN_RXADDR
{
  ENRX_P0 = 0x01,
  ENRX_P1 = 0x02,
  ENRX_P2 = 0x04,
  ENRX_P3 = 0x08,
  ENRX_P4 = 0x10,
  ENRX_P5 = 0x20,
};

// Address Widths
enum SETUP_AW
{
  AW_2 = 0x00,
  AW_3 = 0x01,
  AW_4 = 0x02,
  AW_5 = 0x03,
};

// RF Setup
enum RF_SETUP
{
  CONT_WAVE = 0x80,
  PLL_LOCK  = 0x10,
  RATE_2M   = 0x08,
  RATE_1M   = 0x00,
  RATE_250K = 0x20,
  RF_PWR_4  = 0x06,
  RF_PWR_3  = 0x04,
  RF_PWR_2  = 0x02,
  RF_PWR_1  = 0x00,
};

// Dynamic payloads
enum DYNPD
{
  DPL_P5 = 0x20,
  DPL_P4 = 0x10,
  DPL_P3 = 0x08,
  DPL_P2 = 0x04,
  DPL_P1 = 0x02,
  DPL_P0 = 0x01,
};

// Features
enum FEATURE
{
  EN_DPL     = 0x04,
  EN_ACK_PAY = 0x02,
  EN_DYN_ACK = 0x01
};

// Status flags
enum STATUS
{
  RX_DR   = 0x40,
  TX_DS   = 0x20,
  MAX_RT  = 0x10,
  TX_FULL = 0x01,
};

// nRF24 SPI commands
enum nrf24_command
{
  R_REGISTER       = 0x00,
  W_REGISTER       = 0x20,
  R_RX_PL_WID      = 0x60,
  R_RX_PAYLOAD     = 0x61,
  W_TX_PAYLOAD     = 0xA0,
  W_ACK_PAYLOAD    = 0xA8,
  FLUSH_TX         = 0xE1,
  FLUSH_RX         = 0xE2,
  _NOP             = 0xFF,
};

// nRF24 registers
enum nrf24_register
{
  CONFIG      = 0x00,
  EN_AA       = 0x01,
  EN_RXADDR   = 0x02,
  SETUP_AW    = 0x03,
  SETUP_RETR  = 0x04,
  RF_CH       = 0x05,
  RF_SETUP    = 0x06,
  STATUS      = 0x07,
  OBSERVE_TX  = 0x08,
  RPD         = 0x09,
  RX_ADDR_P0  = 0x0A,
  RX_ADDR_P1  = 0x0B,
  RX_ADDR_P2  = 0x0C,
  RX_ADDR_P3  = 0x0D,
  RX_ADDR_P4  = 0x0E,
  RX_ADDR_P5  = 0x0F,
  TX_ADDR     = 0x10,
  RX_PW_P0    = 0x11,
  RX_PW_P1    = 0x12,
  RX_PW_P2    = 0x13,
  RX_PW_P3    = 0x14,
  RX_PW_P4    = 0x15,
  RX_PW_P5    = 0x16,
  FIFO_STATUS = 0x17,
  DYNPD       = 0x1C,
  FEATURE     = 0x1D,
};

#endif

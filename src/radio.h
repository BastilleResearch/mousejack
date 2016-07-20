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


#include <stdint.h>
#include "nRF24LU1P.h"

// Enter ESB promiscuous mode
//   prefix:        address prefix; used for vendors with fixed start of address bytes
//   prefix_length: prefix length, in bytes
void enter_promiscuous_mode(uint8_t * prefix, uint8_t prefix_length);

// Enter generic promiscuous mode
//   prefix:        address prefix; used for vendors with fixed start of address bytes
//   prefix_length: prefix length, in bytes
//   rate:          data rate (0=250K, 1=1M, 2=2M)
void enter_promiscuous_mode_generic(uint8_t * prefix, uint8_t prefix_length, uint8_t rate, uint8_t payload_length);

// Configure addressing on pipe 0
//   address: address bytes
//   length:  address length
void configure_address(uint8_t * address, uint8_t length);

// Configure MAC layer on pipe 0
//   feature: FEATURE register
//   dynpd:   DYNPD register
//   en_aa:   EN_AA register
void configure_mac(uint8_t feature, uint8_t dynpd, uint8_t en_aa);

// Configure PHY layer on pipe 0
//   config:   CONFIG register
//   rf_setup: RF_SETUP register
//   rx_pw:    RX_PW_P0 register
void configure_phy(uint8_t config, uint8_t rf_setup, uint8_t rx_pw);

// SPI wrte
//   command:  SPI command
//   buffer:   buffer to write over SPI
//   length:   number of bytes to write
void spi_write(uint8_t command, uint8_t * buffer, uint8_t length);

// SPI read
//   command:  SPI command
//   buffer:   buffer to fill with data read over SPI
//   length:   number of bytes to read
void spi_read(uint8_t command, uint8_t * buffer, uint8_t length);

// Write a single byte register over SPI
//   reg:  register to write to
//   byte: value to write
void write_register_byte(uint8_t reg, uint8_t byte);

// Read a single byte register over SPI
//   reg:    register to write to
//   return: value read
uint8_t read_register_byte(uint8_t reg);

// Read a register over SPI
//   reg:    register to read from
//   buffer: buffer to fill
//   length: number of bytes to read
#define read_register(reg,buffer,length) spi_read(R_REGISTER|reg,buffer,length)

// Write a register over SPI
//   reg:    register to write to
//   buffer: buffer to write
//   length: number of bytes to write
#define write_register(REG,BUFFER,LENGTH) spi_write(W_REGISTER|REG,BUFFER,LENGTH)

// Flush the RX FIFO
#define flush_rx() spi_write(FLUSH_RX,NULL,0)

// Flush the TX FIFO
#define flush_tx() spi_write(FLUSH_TX,NULL,0)

// Update a CRC16-CCITT with 1-8 bits from a given byte
//   crc:    current CRC
//   byte:   new byte
//   bits:   number of bits to process from byte
//   return: updated CRC
uint16_t crc_update(uint16_t crc, uint8_t byte, uint8_t bits);

// Default promiscuous mode address
__xdata static const uint8_t promiscuous_address[2] = { 0xAA, 0x00 };

// Radio mode
enum radio_mode_t
{
  // ESB sniffer mode
  sniffer = 0,

  // ESB promiscuous mode
  promiscuous = 1,

  // Generic promiscuous mode
  promiscuous_generic = 2,
};

// Radio mode
__xdata static uint8_t radio_mode;

// Promiscuous mode state
__xdata static int pm_prefix_length;      // Promixcuous mode address prefix length
__xdata static uint8_t pm_prefix[5];      // Promixcuous mode address prefix
__xdata static uint8_t pm_payload_length; // Promiscuous mode payload length
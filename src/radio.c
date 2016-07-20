#include <string.h>
#include <stdbool.h>

#include "usb.h"
#include "radio.h"
#include "nRF24LU1P.h"

// Enter ESB promiscuous mode
void enter_promiscuous_mode(uint8_t * prefix, uint8_t prefix_length)
{
  // Update the promiscuous mode state
  int x;
  for(x = 0; x < prefix_length; x++) pm_prefix[prefix_length - 1 - x] = prefix[x];
  pm_prefix_length = prefix_length > 5 ? 5 : prefix_length;
  radio_mode = promiscuous;
  pm_payload_length = 32;

  // CE low
  rfce = 0;

  // Enable RX pipe 0
  write_register_byte(EN_RXADDR, ENRX_P0);

  // Set the default promiscuous mode RX address
  if(pm_prefix_length == 0) configure_address(promiscuous_address, 2);

  // Set the RX address to a single prefix byte and a premable byte
  else if(pm_prefix_length == 1)
  {
    uint8_t address[2] = { pm_prefix[0], (pm_prefix[0] & 0x80) == 0x80 ? 0xAA : 0x55 };
    configure_address(address, 2);
  }

  // If the prefix is two or more bytes, set it as the address
  else configure_address(pm_prefix, pm_prefix_length);

  // Disable dynamic payload length and automatic ACK handling
  configure_mac(0, 0, ENAA_NONE);

  // Disable CRC, enable RX, 2Mbps data rate, pm_payload_length byte payload width
  configure_phy(PRIM_RX | PWR_UP, RATE_2M, pm_payload_length);

  // CE high
  rfce = 1;
  in1bc = 0;
}

// Enter generic promiscuous mode
void enter_promiscuous_mode_generic(uint8_t * prefix, uint8_t prefix_length, uint8_t rate, uint8_t payload_length)
{
  // Update the promiscuous mode state
  int x;
  for(x = 0; x < prefix_length; x++) pm_prefix[prefix_length - 1 - x] = prefix[x];
  pm_prefix_length = prefix_length > 5 ? 5 : prefix_length;
  radio_mode = promiscuous_generic;
  pm_payload_length = payload_length;

  // CE low
  rfce = 0;

  // Enable RX pipe 0
  write_register_byte(EN_RXADDR, ENRX_P0);

  // Set the default promiscuous mode RX address
  if(pm_prefix_length == 0) configure_address(promiscuous_address, 2);

  // Set the RX address to a single prefix byte and a premable byte
  else if(pm_prefix_length == 1)
  {
    uint8_t address[2] = { pm_prefix[0], (pm_prefix[0] & 0x80) == 0x80 ? 0xAA : 0x55 };
    configure_address(address, 2);
  }

  // If the prefix is two or more bytes, set it as the address
  else configure_address(pm_prefix, pm_prefix_length);

  // Disable dynamic payload length and automatic ACK handling
  configure_mac(0, 0, ENAA_NONE);

  // Disable CRC, enable RX, specified data rate, and pm_payload_length byte payload width
  switch(rate)
  {
    case 0:  configure_phy(PRIM_RX | PWR_UP, RF_PWR_4 | RATE_250K, pm_payload_length); break;
    case 1:  configure_phy(PRIM_RX | PWR_UP, RF_PWR_4 | RATE_1M, pm_payload_length); break;
    default: configure_phy(PRIM_RX | PWR_UP, RF_PWR_4 | RATE_2M, pm_payload_length); break;
  }

  // CE high
  rfce = 1;
  in1bc = 0;
}

// Configure addressing on pipe 0
void configure_address(uint8_t * address, uint8_t length)
{
  write_register_byte(EN_RXADDR, ENRX_P0);
  write_register_byte(SETUP_AW, length - 2);
  write_register(TX_ADDR, address, length);
  write_register(RX_ADDR_P0, address, length);
}

// Configure MAC layer functionality on pipe 0
void configure_mac(uint8_t feature, uint8_t dynpd, uint8_t en_aa)
{
  write_register_byte(FEATURE, feature);
  write_register_byte(DYNPD, dynpd);
  write_register_byte(EN_AA, en_aa);
}

// Configure PHY layer on pipe 0
void configure_phy(uint8_t config, uint8_t rf_setup, uint8_t rx_pw)
{
  write_register_byte(CONFIG, config);
  write_register_byte(RF_SETUP, rf_setup);
  write_register_byte(RX_PW_P0, rx_pw);
}

// Transfer a byte over SPI
uint8_t spi_transfer(uint8_t byte)
{
  RFDAT = byte;
  RFRDY = 0;
  while(!RFRDY);
  return RFDAT;
}

// Write a register over SPI
void spi_write(uint8_t command, uint8_t * buffer, uint8_t length)
{
  int x;
  rfcsn = 0;
  spi_transfer(command);
  for(x = 0; x < length; x++) spi_transfer(buffer[x]);
  rfcsn = 1;
}

// Read a register over SPI
void spi_read(uint8_t command, uint8_t * buffer, uint8_t length)
{
  int x;
  rfcsn = 0;
  spi_transfer(command);
  for(x = 0; x < length; x++) buffer[x] = spi_transfer(0xFF);
  rfcsn = 1;
}

// Write a single byte register over SPI
void write_register_byte(uint8_t reg, uint8_t byte)
{
  write_register(reg, &byte, 1);
}

// Read a single byte register over SPI
uint8_t read_register_byte(uint8_t reg)
{
  uint8_t value;
  read_register(reg, &value, 1);
  return value;
}

// Update a CRC16-CCITT with 1-8 bits from a given byte
uint16_t crc_update(uint16_t crc, uint8_t byte, uint8_t bits)
{
  crc = crc ^ (byte << 8);
  while(bits--)
    if((crc & 0x8000) == 0x8000) crc = (crc << 1) ^ 0x1021;
    else crc = crc << 1;
  crc = crc & 0xFFFF;
  return crc;
}

// Handle a USB radio request
void handle_radio_request(uint8_t request, uint8_t * data)
{
  // Enter the Nordic bootloader
  if(request == LAUNCH_NORDIC_BOOTLOADER)
  {
    nordic_bootloader();
    return;
  }

  // Enter the bootloader
  if(request == LAUNCH_LOGITECH_BOOTLOADER)
  {
    const uint8_t command[9] = {'E', 'n', 't', 'e', 'r', ' ', 'I', 'C', 'P'};
    uint8_t command_length = 9;
    int x;
    for(x = 0; x < command_length; x++)
    {
      AESIA1 = x;
      AESIV = command[x];
    }
    logitech_bootloader();
    return;
  }

  // Enable the LNA (CrazyRadio PA)
  else if(request == ENABLE_LNA)
  {
    P0DIR &= ~0x10;
    P0 |= 0x10;
    in1bc = 0;
    return;
  }

  // Set the current channel
  else if(request == SET_CHANNEL)
  {
    rfce = 0;
    write_register_byte(RF_CH, data[0]);
    in1bc = 1;
    in1buf[0] = data[0];
    flush_rx();
    flush_tx();
    rfce = 1;
    return;
  }

  // Get the current channel
  else if(request == GET_CHANNEL)
  {
    spi_read(RF_CH, in1buf, 1);
    in1bc = 1;
    return;
  }

  // Enter ESB promiscuous mode
  else if(request == ENTER_PROMISCUOUS_MODE)
  {
    enter_promiscuous_mode(&data[1] /* address prefix */, data[0] /* prefix length */);
  }

  // Enter generic promiscuous mode
  else if(request == ENTER_PROMISCUOUS_MODE_GENERIC)
  {
    enter_promiscuous_mode_generic(&data[3] /* address prefix */, data[0] /* prefix length */, data[1] /* rate */, data[2] /* payload length */);
  }

  // Enter continuous tone test mode
  else if(request == ENTER_TONE_TEST_MODE)
  {
    configure_phy(PWR_UP, CONT_WAVE | PLL_LOCK, 0);
    in1bc = 0;
    return;
  }

  // Receive a packet
  else if(request == RECEIVE_PACKET)
  {
    uint8_t value;

    // Check if a payload is available
    read_register(FIFO_STATUS, &value, 1);
    if((value & 1) == 0)
    {
      // ESB sniffer mode
      if(radio_mode == sniffer)
      {
        // Get the payload width
        read_register(R_RX_PL_WID, &value, 1);
        if(value <= 32)
        {
          // Read the payload and write it to EP1
          read_register(R_RX_PAYLOAD, &in1buf[1], value);
          in1buf[0] = 0;
          in1bc = value + 1;
          flush_rx();
          return;
        }
        else
        {
          // Invalid payload width
          in1bc = 1;
          in1buf[0] = 0xFF;
          flush_rx();
          return;
        }
      }

      // ESB promiscuous mode
      else if(radio_mode == promiscuous)
      {
        int x, offset;
        uint8_t payload_length;
        uint16_t crc, crc_given;
        uint8_t payload[37];

        // Read in the "promiscuous" mode payload, concatenated to the prefix
        for(x = 0; x < pm_prefix_length; x++) payload[pm_prefix_length - x - 1] = pm_prefix[x];
        read_register(R_RX_PAYLOAD, &payload[pm_prefix_length], pm_payload_length);

        // In promiscuous mode without a defined address prefix, we attempt to
        // decode the payload as-is, and then shift it by one bit and try again
        // if the first attempt did not pass the CRC check. The purpose of this
        // is to minimize missed detections that happen if we were to use both
        // 0xAA and 0x55 as the nonzero promiscuous mode address bytes.
        for(offset = 0; offset < 2; offset++)
        {
          // Shift the payload right by one bit if this is the second pass
          if(offset == 1)
          {
            for(x = 31; x >= 0; x--)
            {
              if(x > 0) payload[x] = payload[x - 1] << 7 | payload[x] >> 1;
              else payload[x] = payload[x] >> 1;
            }
          }

          // Read the payload length
          payload_length = payload[5] >> 2;

          // Check for a valid payload length, which is less than the usual 32 bytes
          // because we need to account for the packet header, CRC, and part or all
          // of the address bytes.
          if(payload_length <= (pm_payload_length-9) + pm_prefix_length)
          {
            // Read the given CRC
            crc_given = (payload[6 + payload_length] << 9) | ((payload[7 + payload_length]) << 1);
            crc_given = (crc_given << 8) | (crc_given >> 8);
            if(payload[8 + payload_length] & 0x80) crc_given |= 0x100;

            // Calculate the CRC
            crc = 0xFFFF;
            for(x = 0; x < 6 + payload_length; x++) crc = crc_update(crc, payload[x], 8);
            crc = crc_update(crc, payload[6 + payload_length] & 0x80, 1);
            crc = (crc << 8) | (crc >> 8);

            // Verify the CRC
            if(crc == crc_given)
            {
              // Write the address to the output buffer
              memcpy(in1buf, payload, 5);

              // Write the ESB payload to the output buffer
              for(x = 0; x < payload_length + 3; x++)
                in1buf[5+x] = ((payload[6 + x] << 1) & 0xFF) | (payload[7 + x] >> 7);
              in1bc = 5 + payload_length;
              flush_rx();
              return;
            }
          }
        }
      }

      // Generic promiscuous mode
      else if(radio_mode == promiscuous_generic)
      {
        int x;
        uint8_t payload[37];

        // Read in the "promiscuous" mode payload, concatenated to the prefix
        for(x = 0; x < pm_prefix_length; x++) payload[pm_prefix_length - x - 1] = pm_prefix[x];
        read_register(R_RX_PAYLOAD, &payload[pm_prefix_length], pm_payload_length);

        // Write the payload to the output buffer
        memcpy(in1buf, payload, pm_prefix_length + pm_payload_length);
        in1bc = pm_prefix_length + pm_payload_length;
        // flush_rx();
        return;
      }
    }

    // No payload
    in1bc = 1;
    in1buf[0] = 0xFF;
    return;
  }

  // Enter sniffer mode
  else if(request == ENTER_SNIFFER_MODE)
  {
    radio_mode = sniffer;

    // Clamp to 2-5 byte addresses
    if(data[0] > 5) data[0] = 5;
    if(data[0] < 2) data[0] = 2;

    // CE low
    rfce = 0;

    // Configure the address
    configure_address(&data[1], data[0]);

    // Enable dynamic payload length, disable automatic ACK handling
    configure_mac(EN_DPL | EN_ACK_PAY, DPL_P0, ENAA_NONE);

    // 2Mbps data rate, enable RX, 16-bit CRC
    configure_phy(EN_CRC | CRC0 | PRIM_RX | PWR_UP, RATE_2M, 0);

    // CE high
    rfce = 1;

    // Flush the FIFOs
    flush_rx();
    flush_tx();
    in1bc = 0;
  }

  // Transmit an ACK payload
  else if(request == TRANSMIT_ACK_PAYLOAD)
  {
    uint16_t elapsed;
    uint8_t status;

    // Clamp to 1-32 byte payload
    if(data[0] > 32) data[0] = 32;
    if(data[0] < 1) data[0] = 1;

    // CE low
    rfce = 0;

    // Flush the TX/RX buffers
    flush_tx();
    flush_rx();

    // Clear the max retries and data sent flags
    write_register_byte(STATUS, MAX_RT | TX_DS | RX_DR);

    // Enable auto ACK handling and ACK payloads
    write_register_byte(EN_AA, ENAA_P0);
    write_register_byte(FEATURE, EN_DPL | EN_ACK_PAY);

    // Write the AC Kpayload
    spi_write(W_ACK_PAYLOAD, &data[1], data[0]);

    // CE high
    rfce = 1;

    // Wait up to 500ms for the ACK payload to be transmitted
    elapsed = 0;
    in1buf[0] = 0;
    while(elapsed < 500)
    {
      status = read_register_byte(STATUS);
      if((status & RX_DR) == RX_DR)
      {
        in1buf[0] = 1;
        break;
      }

      delay_us(1000);
      elapsed++;
    }

    // Disable auto ACK
    write_register_byte(EN_AA, ENAA_NONE);

    in1bc = 1;
  }

  // Transmit an ESB payload
  else if(request == TRANSMIT_PAYLOAD)
  {
    // Clamp to 1-32 byte payload
    if(data[0] > 32) data[0] = 32;
    if(data[0] < 1) data[0] = 1;

    // CE low
    rfce = 0;

    // Setup auto-retransmit
    // - timeout is in multiples of 250us
    write_register_byte(SETUP_RETR, (1 << data[1]) | data[2]);

    // Flush the TX/RX buffers
    flush_tx();
    flush_rx();

    // Clear the max retries and data sent flags
    write_register_byte(STATUS, MAX_RT | TX_DS | RX_DR);

    // Enable TX
    write_register_byte(CONFIG, read_register_byte(CONFIG) & ~PRIM_RX);

    // Enable auto ACK handling
    write_register_byte(EN_AA, ENAA_P0);

    // Write the payload
    spi_write(W_TX_PAYLOAD, &data[3], data[0]);

    // Bring CE high to initiate the transfer
    rfce = 1;
    delay_us(10);
    rfce = 0;

    // Wait for success, failure, or timeout
    while(true)
    {
      // Read the STATUS register
      rfcsn = 0;
      RFDAT = _NOP;
      RFRDY = 0;
      while(!RFRDY);
      rfcsn = 1;

      // Max retransmits reached
      if((RFDAT & 0x10) == 0x10)
      {
        in1buf[0] = 0;
        break;
      }

      // Successful transmit
      if((RFDAT & 0x20) == 0x20)
      {
        in1buf[0] = 1;
        break;
      }
    }

    // Disable auto ack
    write_register_byte(EN_AA, ENAA_NONE);

    // Enable RX
    write_register_byte(CONFIG, read_register_byte(CONFIG) | PRIM_RX);

    // CE high
    rfce = 1;
    in1bc = 1;
  }

  // Transmit a generic payload
  else if(request == TRANSMIT_PAYLOAD_GENERIC)
  {
    uint8_t address_start = data[0] + data[1] + 2;

    // Clamp to 1-32 byte payload
    if(data[0] > 32) data[0] = 32;
    if(data[0] < 1) data[0] = 1;

    // Clamp to 1-5 byte address
    if(data[1] > 5) data[1] = 5;
    if(data[1] < 1) data[1] = 1;

    // CE low
    rfce = 0;

    // Flush the TX buffer
    flush_tx();
    flush_rx();

    // Clear the max retries and data sent flags
    write_register_byte(STATUS, MAX_RT | TX_DS | RX_DR);

    // Enable TX
    write_register_byte(CONFIG, read_register_byte(CONFIG) & ~PRIM_RX);

    // Set the address
    configure_address(&data[address_start], data[1]);

    // Write the payload
    spi_write(W_TX_PAYLOAD, &data[2], data[0]);

    // Bring CE high to initiate the transfer
    rfce = 1;
    delay_us(10);
    rfce = 0;

    // Wait for transmit
    while(true)
    {
      // Read the STATUS register
      rfcsn = 0;
      RFDAT = _NOP;
      RFRDY = 0;
      while(!RFRDY);
      rfcsn = 1;

      // Successful transmit
      if((RFDAT & TX_DS) == TX_DS)
      {
        in1buf[0] = 1;
        break;
      }
    }

    // Enable RX and set the promiscuous mode address
    write_register_byte(CONFIG, read_register_byte(CONFIG) | PRIM_RX);
    configure_address(pm_prefix, pm_prefix_length);

    // CE high
    rfce = 1;
    in1bc = 1;
  }
}


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


#include "nRF24LU1P.h"
#include "usb_desc.h"

// Nordic nootloader entry point
static void (*nordic_bootloader)() = (void (*)())0x7800;

// Logitech nootloader entry point
static void (*logitech_bootloader)() = (void (*)())0x7400;

// USB configured state
static bool configured;

// Initialize the USB configuraiton
bool init_usb();

// Handle a USB setup request
void handle_setup_request();

// Reset the USB configuration
void usb_reset_config();

// Handle a USB radio request
void handle_radio_request(uint8_t request, uint8_t * data);

// USB IRQ handler
void usb_irq() __interrupt(12)  __using(1);

// USB request
struct usb_request_t
{
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
};

// USB request types
enum usb_request_type_t
{
  GET_STATUS = 0,
  SET_ADDRESS = 5,
  GET_DESCRIPTOR = 6,
  GET_CONFIGURATION = 8,
  SET_CONFIGURATION = 9,
};

//Vendor control messages and commands
#define TRANSMIT_PAYLOAD               0x04
#define ENTER_SNIFFER_MODE             0x05
#define ENTER_PROMISCUOUS_MODE         0x06
#define ENTER_TONE_TEST_MODE           0x07
#define TRANSMIT_ACK_PAYLOAD           0x08
#define SET_CHANNEL                    0x09
#define GET_CHANNEL                    0x0A
#define ENABLE_LNA                     0x0B
#define TRANSMIT_PAYLOAD_GENERIC       0x0C
#define ENTER_PROMISCUOUS_MODE_GENERIC 0x0D
#define RECEIVE_PACKET                 0x12
#define LAUNCH_LOGITECH_BOOTLOADER     0xFE
#define LAUNCH_NORDIC_BOOTLOADER       0xFF


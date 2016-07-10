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

#include "usb.h"
#include "radio.h"
#include "platform.h"

// Program entry point
void main()
{
  uint8_t heartbeat = 1;
  uint16_t heartbeat_count = 0;

  rfcon = 0x06; // enable RF clock
  rfctl = 0x10; // enable SPI
  ien0 = 0x80;  // enable interrupts
  TICKDV = 0xFF; // set the tick divider

  // Perform platform-specific initializations
  platform_init();
  platform_led_off(PLATFORM_LED_HEARTBEAT | PLATFORM_LED_DEBUG);

  // Initialise and connect the USB controller
  init_usb();

  // Flush the radio FIFOs
  flush_rx();
  flush_tx();

  // Everything is triggered via interrupts, so now we wait
  while(1)
  {
    REGXH = 0xFF;
    REGXL = 0xFF;
    REGXC = 0x08;

    // 1s, 50% duty cycle heatbeat.
    // This will appear slow down when processor is being kept busy in ISRs
    if (++heartbeat_count == 500) {
        if (heartbeat) {
            platform_led_on(PLATFORM_LED_HEARTBEAT);
        } else {
            platform_led_off(PLATFORM_LED_HEARTBEAT);
        }

        heartbeat ^= 1;
        heartbeat_count = 0;
    }

    delay_us(1000);
  }
}

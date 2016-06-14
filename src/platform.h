/*
  Copyright (C) 2016 Jon Szymaniak <jon.szymaniak@gmail.com>

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

#ifndef MOUSEJACK_PLATFORM_H_
#define MOUSEJACK_PLATFORM_H_

#include <stdbool.h>

#define PLATFORM_LED_HEARTBEAT  (1 << 0)
#define PLATFORM_LED_DEBUG      (1 << 1)

// Initialize platform-specific settings and I/O
void platform_init();

// Enable (true) or disable (false) external LN:s
void platform_enable_lna(bool enable);

// Set the state of LEDs on the platform. A `1` bit is ON, a `0` bit is OFF.
void platform_led(uint8_t led_state);

// Turn on the LEDs associated with a `1` bit the specified mask.
void platform_led_on(uint8_t led_mask);

// Turn off the LEDs associated with a `1` bit the specified mask.
void platform_led_off(uint8_t led_mask);

// Assert SPI Master chip select
void platform_assert_spi_master_cs(bool assert_cs);

#endif

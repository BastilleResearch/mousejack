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

#include "nRF24LU1P.h"
#include "platform.h"
#include "common.h"

// Generic platform support - no IO support
void platform_init()
{
    // Default to all pins as inpus
    P0DIR = 0xff;
}

void platform_enable_lna(bool enable)   { UNUSED_PARAMETER(enable);     }
void platform_led(uint8_t leds)         { UNUSED_PARAMETER(leds);       }
void platform_led_on(uint8_t led_mask)  { UNUSED_PARAMETER(led_mask);   }
void platform_led_off(uint8_t led_mask) { UNUSED_PARAMETER(led_mask);   }
void platform_assert_spi_master_cs(bool assert) { UNUSED_PARAMETER(assert); }

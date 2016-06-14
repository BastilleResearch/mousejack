/*
 * Modified Bitcraze CrazyRadio PA with SPI Master support
 *
 * IMPORTANT: This implementation assumes that U2 has been depopulated, and
 *            U2 pins 2 & 4 have been connected. Running this firmware on an
 *            unmodified CRPA may damage U2 and U3 (due to contention over
 *            the CSN line).
 *
 *
 * Copyright (C) 2016 Jon Szymaniak <jon.szymaniak@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nRF24LU1P.h"
#include "platform.h"
#include "common.h"
#include "crpa.h"

void platform_init()
{
    // Configure pins for SPI Master functionality via P0EXP[0:1] = 01
    uint8_t p0exp_val = P0EXP & ~0x3;
    P0EXP = p0exp_val | 0x01;

    // Enable SPI master
    SMCTRL |= (1 << 4);

    // Mark MISO as input, just for safe measure. This may be implicit
    // in configuring the pin for use as an SPI master pin.
    //
    // P0_5 is unused - set this as an input for lack of any other use.
    P0DIR = (CRPA_P0_MISO | CRPA_P0_5);

    platform_assert_spi_master_cs(false);
}

void platform_enable_lna(bool enable)
{
    crpa_enable_lna(enable);
}

void platform_assert_spi_master_cs(bool assert_cs)
{
    if (assert_cs) {
        P0 &= ~CRPA_P0_CSN;
    } else {
        P0 |= CRPA_P0_CSN;
    }
}

// Dummies - the LED pins are not available
void platform_led(uint8_t leds)         { UNUSED_PARAMETER(leds); }
void platform_led_on(uint8_t led_mask)  { UNUSED_PARAMETER(led_mask); }
void platform_led_off(uint8_t led_mask) { UNUSED_PARAMETER(led_mask); }

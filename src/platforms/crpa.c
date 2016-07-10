/*
 * Bitcraze CrazyRadio PA - Unmodified platform with LED support
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
#include <stdbool.h>

#include "nRF24LU1P.h"
#include "platform.h"
#include "common.h"
#include "crpa.h"

void platform_init()
{
    /* These pins must always be inputs because they're connected to the output
     * of U2 (noninverting buffer).
     */
    P0DIR = (CRPA_P0_CSN | CRPA_P0_5);
    P0 = 0;
}

void platform_enable_lna(bool enable)
{
    crpa_enable_lna(enable);
}

void platform_led(uint8_t leds)
{
    uint8_t p0_state = P0;

    if (leds & PLATFORM_LED_DEBUG) {
        p0_state |=  CRPA_P0_RED_LED2;
    } else {
        p0_state &= ~CRPA_P0_RED_LED2;
    }

    if (leds & PLATFORM_LED_HEARTBEAT) {
        p0_state |=  CRPA_P0_GRN_LED1;
    } else {
        p0_state &= ~CRPA_P0_GRN_LED1;
    }

    P0 = p0_state;
}

void platform_led_on(uint8_t led_mask)
{
    uint8_t p0_state = P0;

    if (led_mask & PLATFORM_LED_DEBUG) {
        p0_state |= CRPA_P0_RED_LED2;
    }

    if (led_mask & PLATFORM_LED_HEARTBEAT) {
        p0_state |= CRPA_P0_GRN_LED1;
    }

    P0 = p0_state;
}

void platform_led_off(uint8_t led_mask)
{
    uint8_t p0_state = P0;

    if (led_mask & PLATFORM_LED_DEBUG) {
        p0_state &= ~CRPA_P0_RED_LED2;
    }

    if (led_mask & PLATFORM_LED_HEARTBEAT) {
        p0_state &= ~CRPA_P0_GRN_LED1;
    }

    P0 = p0_state;
}

// No SPI master support available in this build
void platform_assert_spi_master_cs(bool assert_cs)
{
    UNUSED_PARAMETER(assert_cs);
}

/*
 * Bitcraze CrazyRadio PA common definitions
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
#ifndef CRPA_H_
#define CRPA_H_

// P0 mappning
#define CRPA_P0_SCK      (1 << 0)
#define CRPA_P0_MOSI     (1 << 1)
#define CRPA_P0_MISO     (1 << 2)
#define CRPA_P0_CSN      (1 << 3)
#define CRPA_P0_RXEN     (1 << 4)
#define CRPA_P0_5        (1 << 5)

// LEDs are on SPI pins
#define CRPA_P0_GRN_LED1    CRPA_P0_SCK
#define CRPA_P0_RED_LED2    CRPA_P0_MISO

#define crpa_enable_lna(enable_) do { \
    if (enable_) { \
        P0 |= CRPA_P0_RXEN; \
    } else { \
        P0 &= ~CRPA_P0_RXEN; \
    } \
} while (0)

#endif

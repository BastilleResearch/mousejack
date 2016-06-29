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


#include "usb.h"
#include "radio.h"

// Program entry point
void main()
{
  rfcon = 0x06; // enable RF clock
  rfctl = 0x10; // enable SPI
  ien0 = 0x80;  // enable interrupts
  TICKDV = 0xFF; // set the tick divider

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
    delay_us(1000);
  }
}

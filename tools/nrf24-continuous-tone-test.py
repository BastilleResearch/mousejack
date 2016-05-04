#!/usr/bin/env python2
'''
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
'''


import time, logging
from lib import common

# Parse command line arguments and initialize the radio
common.init_args('./nrf24-continuous-tone-test.py')
common.parse_and_init()

# Set the initial channel
common.radio.set_channel(common.channels[0])

# Put the radio in continuous tone test mode
common.radio.enter_tone_test_mode()

# Run indefinitely
while True:
  pass


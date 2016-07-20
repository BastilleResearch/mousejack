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

from unifying import *
import subprocess

# Make sure a firmware image path was passed in
if len(sys.argv) < 3:
  print "Usage: sudo ./logitech-usb-flash.py [firmware-image.hex]"

# Read in the firmware image
with open(sys.argv[1]) as f:
  lines = f.readlines()
  lines = [line.strip()[1:] for line in lines]
  lines = [line[2:6] + line[0:2] + line[8:-2] for line in lines]
  lines = ["20" + line + "0"*(62-len(line)) for line in lines]
  payloads = [line.decode('hex') for line in lines]
  payloads[0] = payloads[0][0:2] + chr((ord(payloads[0][2]) + 1)) + chr((ord(payloads[0][3]) - 1)) + payloads[0][5:]

# Instantiate the dongle
dongle = unifying_dongle()

# Init command (?)
logging.info("Initializing firmware update")
response = dongle.send_command(0x21, 0x09, 0x0200, 0x0000, "\x80" + "\x00"*31)

# # Clear the existing flash memory up to the size of the new firmware image
logging.info("Clearing existing flash memory up to boootloader")
for x in range(0, 0x70, 2):
  response = dongle.send_command(0x21, 0x09, 0x0200, 0x0000, "\x30" + chr(x) + "\x00\x01" + "\x00"*28)

# Send the data
logging.info("Transferring the new firmware")
for payload in payloads:
  response = dongle.send_command(0x21, 0x09, 0x0200, 0x0000, payload)
response = dongle.send_command(0x21, 0x09, 0x0200, 0x0000, payloads[0])

# Completed command (?)
logging.info("Mark firmware update as completed")
response = dongle.send_command(0x21, 0x09, 0x0200, 0x0000, "\x20\x00\x00\x01\x02" + "\x00"*27)

# Restart the dongle
logging.info("Restarting dongle into research firmware mode")
response = dongle.send_command(0x21, 0x09, 0x0200, 0x0000, "\x70" + "\x00"*31)

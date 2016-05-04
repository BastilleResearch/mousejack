#!/usr/bin/env python
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


import serial, binascii, time, sys, logging
from serial.tools import list_ports

# Setup logging
logging.basicConfig(level=logging.INFO, format='[%(asctime)s.%(msecs)03d]  %(message)s', datefmt="%Y-%m-%d %H:%M:%S")

# Serial commands
READ_PAGE       = 0x00
WRITE_PAGE      = 0x02

# Teensy serial client
class client(serial.Serial):

  # Constructor
  def __init__self(self, *args, **kwargs):
    Serial.__init__(self, *args, **kwargs)

  # Read until a newline
  def readline(self):
    string = ''
    while True:
      char = self.read()
      if char != '\n':
        string += char
      else: break
    return string

  # Read a page
  def read_page(self, page):
    command = map(chr, [READ_PAGE, page & 0xFF])
    self.write(command)
    return self.readline()

# Find the Teensy serial port
logging.info("Finding for Teensy COM port")
comport = None
search = 'USB VID:PID=16c0:04'.lower()
for port in list_ports.comports():
  if search in port[2].lower():
    comport = port[0]
    break
if not comport:
  raise Exception('Failed to find Teensy COM port.')

# Connect to the Teensy
logging.info('Connecting to Teensy over serial at {0}'.format(comport))
ser = client(port=comport, baudrate=115200)

# Read the flash memory
logging.info("Reading flash memory")
address = 0
for x in range(32768/512):
  page_hex = ser.read_page(x)
  page_bytes = page_hex.decode('hex')
  for y in range(16):
    line_bytes = '20{0:04X}00{1}'.format(address, ''.join("{:02X}".format(ord(c)) for c in page_bytes[y*32:(y+1)*32]))
    checksum = sum(map(ord, line_bytes.decode('hex')))
    checksum = (~checksum + 1 & 0xFF)
    print ':{0}{1:02X}'.format(line_bytes, checksum)
    address += 32
print ':00000001FF'

# Close the serial connection
ser.close()


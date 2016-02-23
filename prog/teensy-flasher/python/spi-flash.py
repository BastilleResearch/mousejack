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

# Verify that we received a command line argument
if len(sys.argv) < 2:
  print 'Usage: ./spi-flash.py path-to-firmware.bin'
  quit()

# Read in the firmware
with open(sys.argv[1], 'rb') as f:
  data = f.read()

# Zero pad the data to a multiple of 512 bytes 
data += '\000' * (512 - len(data) % 512)

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

  # Write a page
  def write_page(self, page, data):

    if len(data) != 512: 
      raise Exception("Expected 512 bytes of data, got {0}".format(len(data)))

    command = map(chr, [WRITE_PAGE, page & 0xFF])
    self.write(command)
    self.write(data)  
    self.readline()

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

# Write the data, one page at a time
logging.info('Writing image to flash')
for x in range(len(data)/512):
  page = data[x*512:x*512+512]
  ser.write_page(x, page)

# Verify that the image was written correctly, reading one page at a time
logging.info("Verifying write")
for x in range(len(data)/512):
  page_hex = ser.read_page(x)
  page_bytes = page_hex.decode('hex')
  if page_bytes != data[x*512:x*512+512]:
    raise Exception('Verification failed on page {0}, block {1}'.format(page, block))

# Close the serial connection
ser.close()

logging.info("Firmware programming completed successfully")
logging.info("\033[92m\033[1mPlease unplug your dongle or breakout board and plug it back in.\033[0m")


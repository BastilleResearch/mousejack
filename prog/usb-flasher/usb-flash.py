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


import usb, time, sys, array, logging

# Setup logging
logging.basicConfig(level=logging.INFO, format='[%(asctime)s.%(msecs)03d]  %(message)s', datefmt="%Y-%m-%d %H:%M:%S")

# Check pyusb dependency
try:
  from usb import core as _usb_core
except ImportError, ex:
  print '''
------------------------------------------
| PyUSB was not found or is out of date. |
------------------------------------------

Please update PyUSB using pip:

sudo pip install -U -I pip && sudo pip install -U -I pyusb
'''
  sys.exit(1)

# USB timeout sufficiently long for operating in a VM
usb_timeout = 2500

# Verify that we received a command line argument
if len(sys.argv) < 2:
  print 'Usage: ./usb-flash.py path-to-firmware.bin'
  quit()

# Read in the firmware
with open(sys.argv[1], 'rb') as f:
  data = f.read()

# Zero pad the data to a multiple of 512 bytes
data += '\000' * (512 - len(data) % 512)

# Find an attached device running CrazyRadio or RFStorm firmware
logging.info("Looking for a compatible device that can jump to the Nordic bootloader")
product_ids = [0x0102, 0x7777]
for product_id in product_ids:

  # Find a compatible device
  try:
    dongle = usb.core.find(idVendor=0x1915, idProduct=product_id)
    dongle.set_configuration()
  except AttributeError:
    continue

  # Device found, instruct it to jump to the Nordic bootloader
  logging.info("Device found, jumping to the Nordic bootloader")
  if product_id == 0x0102: dongle.write(0x01, [0xFF], timeout=usb_timeout)
  else: dongle.ctrl_transfer(0x40, 0xFF, 0, 0, (), timeout=usb_timeout)
  try: dongle.reset()
  except: pass

# Find an attached device running the Nordic bootloader, waiting up
# to 1000ms to allow time for USB to reinitialize after the the
# CrazyRadio or RFStorm firmware jumps to the bootloader
logging.info("Looking for a device running the Nordic bootloader")
start = time.time()
while time.time() - start < 1:

  # Find a devices running the Nordic bootloader
  try:
    dongle = usb.core.find(idVendor=0x1915, idProduct=0x0101)
    dongle.set_configuration()
    break
  except AttributeError:
    continue

# Verify that we found a compatible device
if not dongle:
  logging.info("No compatbile device found")
  raise Exception('No compatible device found.')

# Write the data, one page at a time
logging.info("Writing image to flash")
page_count = len(data) / 512
for page in range(page_count):

  # Tell the bootloader that we are going to write a page
  dongle.write(0x01, [0x02, page])
  dongle.read(0x81, 64, usb_timeout)

  # Write the page as 8 pages of 64 bytes
  for block in range(8):

    # Write the block
    block_write = data[page*512+block*64:page*512+block*64+64]
    dongle.write(0x01, block_write, usb_timeout)
    dongle.read(0x81, 64, usb_timeout)

# Verify that the image was written correctly, reading one page at a time
logging.info("Verifying write")
block_number = 0
for page in range(page_count):

  # Tell the bootloader that we are reading from the lower 16KB of flash
  dongle.write(0x01, [0x06, 0], usb_timeout)
  dongle.read(0x81, 64, usb_timeout)

  # Read the page as 8 pages of 64 bytes
  for block in range(8):

    # Read the block
    dongle.write(0x01, [0x03, block_number], usb_timeout)
    block_read = array.array('B', dongle.read(0x81, 64, usb_timeout)).tostring()
    if block_read != data[block_number*64:block_number*64+64]:
      raise Exception('Verification failed on page {0}, block {1}'.format(page, block))
    block_number += 1

logging.info("Firmware programming completed successfully")
logging.info("\033[92m\033[1mPlease unplug your dongle or breakout board and plug it back in.\033[0m")

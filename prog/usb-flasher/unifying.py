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

import usb, logging, time, sys, struct, os

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

# Sufficiently long timeout for use in a VM
usb_timeout = 2500

# Logitech Unifying dongle
class unifying_dongle:

  # Constructor
  def __init__(self):

    # Get the dongle instance
    self.dongle = usb.core.find(idVendor=0x046d, idProduct=0xc52b)
    if self.dongle:
      logging.info("Found Logitech Unifying dongle - HID mode")

      # Detach the kernel driver
      logging.info("Detaching kernel driver from Logitech dongle - HID mode")
      for ep in range(3):
        if self.dongle.is_kernel_driver_active(ep):
          self.dongle.detach_kernel_driver(ep)

      # Set the default configuration
      self.dongle.set_configuration()

      # Enter firmware update mode
      self.enter_firmware_update_mode()
      return

    # Get the dongle instance - Logitech dongle flashed with research firmware
    self.dongle = usb.core.find(idVendor=0x1915, idProduct=0x0102)
    if self.dongle:
      logging.info("Found dongle with research firmware, attempting to load Logitech bootloader")

      # Set the default configuration
      self.dongle.set_configuration()

      # Enter firmware update mode
      self.dongle.write(0x01, [0xFE], timeout=usb_timeout)
      try: self.dongle.reset()
      except: pass

      # Wait up to 5 seconds for the Logitech bootloader to show up
      start = time.time()
      while time.time() - start < 5:
        try:

          # Get the dongle instance
          self.dongle = usb.core.find(idVendor=0x046d, idProduct=0xaaaa)
          if self.dongle:
            logging.info("Found Logitech Unifying dongle - firmware update mode")

            # Detach the kernel driver
            logging.info("Putting dongle into firmware update mode - firmware update mode")
            for ep in range(3):
              if self.dongle.is_kernel_driver_active(ep):
                self.dongle.detach_kernel_driver(ep)

            # Set the configuration
            self.dongle.set_configuration(1)
            break

        except AttributeError:
          continue

      # Verify that the Logitech bootloader showed up
      if not self.dongle:
        raise Exception("Dongle failed to reset into firmware update mode")

    else:

      # Get the dongle instance (already in firmware update mode)
      self.dongle = usb.core.find(idVendor=0x046d, idProduct=0xaaaa)
      if not self.dongle:
        raise Exception("Unable to find Logitech Unifying USB dongle.")

      # Detach the kernel driver
      for ep in range(3):
        if self.dongle.is_kernel_driver_active(ep):
          self.dongle.detach_kernel_driver(ep)

      # Set the default configuration
      self.dongle.set_configuration()

  # Reset the dongle into firmware update mode
  def enter_firmware_update_mode(self):

    logging.info("Putting dongle into firmware update mode")

    # It's not 100% clear why this is necessary, but there is some state problem
    # when a Logitech dongle is first plugged in (and not used as an HID/HID++ device).
    # The following code makes everything work, but it's magic for the moment.
    try:
      self.send_command(0x21, 0x09, 0x0210, 0x0002, "\x10\xFF\x81\xF1\x00\x00\x00", ep=0x83)
    except Exception:
      pass

    # Request the firmware version
    response = self.send_command(0x21, 0x09, 0x0210, 0x0002, "\x10\xFF\x81\xF1\x01\x00\x00", ep=0x83)
    if response[5] != 0x12:
      logging.info('Incompatible Logitech Unifying dongle (type {:02X}). Only Nordic Semiconductor based dongles are supported.'.format(response[5]))
      sys.exit(1)

    # Tell the dongle to reset into firmware update mode
    try:
      self.send_command(0x21, 0x09, 0x0210, 0x0002, "\x10\xFF\x80\xF0\x49\x43\x50", ep=0x83)
    except usb.core.USBError:

      # An I/O error is possible here when the device resets before we can read the USB response
      pass

    # Wait up to 5 seconds for the Logitech bootloader to show up
    start = time.time()
    while time.time() - start < 5:
      try:

        # Get the dongle instance
        self.dongle = usb.core.find(idVendor=0x046d, idProduct=0xaaaa)
        if self.dongle:
          logging.info("Found Logitech Unifying dongle - firmware update mode")

          # Detach the kernel driver
          logging.info("Putting dongle into firmware update mode - firmware update mode")
          for ep in range(3):
            if self.dongle.is_kernel_driver_active(ep):
              self.dongle.detach_kernel_driver(ep)

          # Set the configuration
          self.dongle.set_configuration(1)
          break

      except AttributeError:
        continue

    # Verify that the Logitech bootloader showed up
    if not self.dongle:
      raise exception("Dongle failed to reset into firmware update mode")

  # Send a command to the Logitech bootloader
  def send_command(self, request_type, request, value, index, data, ep=0x81, timeout=usb_timeout):

    # Send the command
    ret = self.dongle.ctrl_transfer(request_type, request, value, index, data, timeout=timeout)
    response = self.dongle.read(ep, 32, timeout=timeout)
    logging.info(':'.join("{:02X}".format(c) for c in response))
    return response

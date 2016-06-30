#!/usr/bin/env python
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

    # Request the firmware version
    self.dongle.ctrl_transfer(0x21, 0x09, 0x0210, 0x0002, "\x10\xFF\x81\x00\x00\x00\x00", timeout=usb_timeout)
    self.dongle.ctrl_transfer(0x21, 0x09, 0x0210, 0x0002, "\x10\xFF\x81\xF1\x01\x00\x00", timeout=usb_timeout)
    response = self.dongle.read(0x83, 32, timeout=usb_timeout)
    if response[5] != 0x12:
      logging.info('Incompatible Logitech Unifying dongle (type {:02X}). Only Nordic Semiconductor based dongles are supported.'.format(response[5]))
      sys.exit(1)

    # Tell the dongle to reset into firmware update mode
    self.dongle.ctrl_transfer(0x21, 0x09, 0x0210, 0x0002, "\x10\xFF\x80\xF0\x49\x43\x50", timeout=usb_timeout)

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
  def send_command(self, request_type, request, value, index, data):

    # Send the command
    self.dongle.ctrl_transfer(request_type, request, value, index, data, timeout=usb_timeout)
    response = self.dongle.read(0x81, 32, timeout=usb_timeout)
    logging.info(':'.join("{:02X}".format(c) for c in response))
    return response

# Compute CRC-CCITT over 1 byte
def crc_update(crc, data):
  crc ^= (data << 8)
  for x in range(8):
    if (crc & 0x8000) == 0x8000: crc = ((crc << 1) ^ 0x1021) & 0xFFFF
    else: crc <<= 1
  crc &= 0xFFFF
  return crc

# Make sure a firmware image path was passed in
if len(sys.argv) < 3:
  print "Usage: sudo ./logitech-usb-flash.py [firmware-image.bin] [firmware-image.ihx]"

# Compute the CRC of the firmware image
logging.info("Computing the CRC of the firmware image")
path = sys.argv[1]
with open(path, 'rb') as f:
  data = f.read()
crc = 0xFFFF
for x in range(len(data)):
  crc = crc_update(crc, ord(data[x]))

# Read in the firmware hex file
logging.info("Preparing USB payloads")
path = sys.argv[2]
with open(path) as f:
  lines = f.readlines()
  lines = [line.strip()[1:] for line in lines]
  lines = [line[2:6] + line[0:2] + line[8:-2] for line in lines]
  lines = ["20" + line + "0"*(62-len(line)) for line in lines]
  payloads = [line.decode('hex') for line in lines]
  payloads[0] = payloads[0][0:2] + chr((ord(payloads[0][2]) + 1)) + chr((ord(payloads[0][3]) - 1)) + payloads[0][5:]

# Add the firmware CRC
payloads.append('\x20\x67\xFE\x02' + struct.pack('!H', crc) + '\x00'*26)

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

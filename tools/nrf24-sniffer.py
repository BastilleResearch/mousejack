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
common.init_args('./nrf24-sniffer.py')
common.parser.add_argument('-a', '--address', type=str, help='Address to sniff, following as it changes channels', required=True)
common.parser.add_argument('-t', '--timeout', type=float, help='Channel timeout, in milliseconds', default=100)
common.parser.add_argument('-k', '--ack_timeout', type=int, help='ACK timeout in microseconds, accepts [250,4000], step 250', default=250)
common.parser.add_argument('-r', '--retries', type=int, help='Auto retry limit, accepts [0,15]', default=1, choices=xrange(0, 16), metavar='RETRIES')
common.parser.add_argument('-p', '--ping_payload', type=str, help='Ping payload, ex 0F:0F:0F:0F', default='0F:0F:0F:0F', metavar='PING_PAYLOAD')
common.parse_and_init()

# Parse the address
address = common.args.address.replace(':', '').decode('hex')[::-1][:5]
address_string = ':'.join('{:02X}'.format(ord(b)) for b in address[::-1])
if len(address) < 2:
  raise Exception('Invalid address: {0}'.format(common.args.address))

# Put the radio in sniffer mode (ESB w/o auto ACKs)
common.radio.enter_sniffer_mode(address)

# Convert channel timeout from milliseconds to seconds
timeout = float(common.args.timeout) / float(1000)

# Parse the ping payload
ping_payload = common.args.ping_payload.replace(':', '').decode('hex')

# Format the ACK timeout and auto retry values
ack_timeout = int(common.args.ack_timeout / 250) - 1
ack_timeout = max(0, min(ack_timeout, 15))
retries = max(0, min(common.args.retries, 15))

# Sweep through the channels and decode ESB packets in pseudo-promiscuous mode
last_ping = time.time()
channel_index = 0
while True:

  # Follow the target device if it changes channels
  if time.time() - last_ping > timeout:

    # First try pinging on the active channel
    if not common.radio.transmit_payload(ping_payload, ack_timeout, retries):

      # Ping failed on the active channel, so sweep through all available channels
      success = False
      for channel_index in range(len(common.channels)):
        common.radio.set_channel(common.channels[channel_index])
        if common.radio.transmit_payload(ping_payload, ack_timeout, retries):

          # Ping successful, exit out of the ping sweep
          last_ping = time.time()
          logging.debug('Ping success on channel {0}'.format(common.channels[channel_index]))
          success = True
          break

      # Ping sweep failed
      if not success: logging.debug('Unable to ping {0}'.format(address_string))

    # Ping succeeded on the active channel
    else:
      logging.debug('Ping success on channel {0}'.format(common.channels[channel_index]))
      last_ping = time.time()

  # Receive payloads
  value = common.radio.receive_payload()
  if value[0] == 0:

    # Reset the channel timer
    last_ping = time.time()

    # Split the payload from the status byte
    payload = value[1:]

    # Log the packet
    logging.info('{0: >2}  {1: >2}  {2}  {3}'.format(
              common.channels[channel_index],
              len(payload),
              address_string,
              ':'.join('{:02X}'.format(b) for b in payload)))


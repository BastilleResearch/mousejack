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
common.init_args('./nrf24-network-mapper.py')
common.parser.add_argument('-a', '--address', type=str, help='Known address', required=True)
common.parser.add_argument('-k', '--ack_timeout', type=int, help='ACK timeout in microseconds, accepts [250,4000], step 250', default=500)
common.parser.add_argument('-r', '--retries', type=int, help='Auto retry limit, accepts [0,15]', default='5', choices=xrange(0, 16), metavar='RETRIES')
common.parser.add_argument('-p', '--ping_payload', type=str, help='Ping payload, ex 0F:0F:0F:0F', default='0F:0F:0F:0F', metavar='PING_PAYLOAD')
common.parse_and_init()

# Parse the address
address = common.args.address.replace(':', '').decode('hex')[::-1][:5]
address_string = ':'.join('{:02X}'.format(ord(b)) for b in address[::-1])
if len(address) < 2:
  raise Exception('Invalid address: {0}'.format(common.args.address))

# Put the radio in sniffer mode (ESB w/o auto ACKs)
common.radio.enter_sniffer_mode(address)

# Parse the ping payload
ping_payload = common.args.ping_payload.replace(':', '').decode('hex')

# Format the ACK timeout and auto retry values
ack_timeout = int(common.args.ack_timeout / 250) - 1
ack_timeout = max(0, min(ack_timeout, 15))
retries = max(0, min(common.args.retries, 15))

# Ping each address on each channel args.passes number of times
valid_addresses = []
for p in range(2):

  # Step through each potential address
  for b in range(256):

    try_address = chr(b) + address[1:]
    logging.info('Trying address {0}'.format(':'.join('{:02X}'.format(ord(b)) for b in try_address[::-1])))
    common.radio.enter_sniffer_mode(try_address)

    # Step through each channel
    for c in range(len(common.args.channels)):
      common.radio.set_channel(common.channels[c])

      # Attempt to ping the address
      if common.radio.transmit_payload(ping_payload, ack_timeout, retries):
        valid_addresses.append(try_address)
        logging.info('Successful ping of {0} on channel {1}'.format(
          ':'.join('{:02X}'.format(ord(b)) for b in try_address[::-1]),
          common.channels[c]))

# Print the results
valid_addresses = list(set(valid_addresses))
for addr in valid_addresses:
  logging.info('Found address {0}'.format(':'.join('{:02X}'.format(ord(b)) for b in addr[::-1])))

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


import logging, argparse
from nrf24 import *

channels = []
args = None
parser = None
radio = None

# Initialize the argument parser
def init_args(description):

  global parser
  parser = argparse.ArgumentParser(description,
    formatter_class=lambda prog: argparse.HelpFormatter(prog,max_help_position=50,width=120))
  parser.add_argument('-c', '--channels', type=int, nargs='+', help='RF channels', default=range(2, 84), metavar='N')
  parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose output', default=False)
  parser.add_argument('-l', '--lna', action='store_true', help='Enable the LNA (for CrazyRadio PA dongles)', default=False)
  parser.add_argument('-i', '--index', type=int, help='Dongle index', default=0)

# Parse and process common comand line arguments
def parse_and_init():

  global parser
  global args
  global channels
  global radio

  # Parse the command line arguments
  args = parser.parse_args()

  # Setup logging
  level = logging.DEBUG if args.verbose else logging.INFO
  logging.basicConfig(level=level, format='[%(asctime)s.%(msecs)03d]  %(message)s', datefmt="%Y-%m-%d %H:%M:%S")

  # Set the channels
  channels = args.channels
  logging.debug('Using channels {0}'.format(', '.join(str(c) for c in channels)))

  # Initialize the radio
  radio = nrf24(args.index)
  if args.lna: radio.enable_lna()


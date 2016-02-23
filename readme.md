# RFStorm nRF24LU1+ Research Firmware

Firmware and research tools for Nordic Semiconductor nRF24LU1+ based USB dongles and breakout boards. 

For information on the MouseJack vulnerabilities, please visit [mousejack.com](https://www.mousejack.com). 

## Requirements

- SDCC
- GNU Binutils 
- Python 
- PyUSB
- platformio

Install dependencies on Ubuntu: 

```
sudo apt-get install sdcc binutils python python-pip
sudo pip install -U pip
sudo pip install -U -I pyusb
sudo pip install -U platformio
```

## Supported Hardware 

The following hardware has been tested and is known to work. 

- CrazyRadio PA USB dongle 
- SparkFun nRF24LU1+ breakout board 

## Build the firmware 

```
make
```

## Flash over USB

nRF24LU1+ chips come with a factory programmed bootloader occupying the topmost 2KB of flash memory. The CrazyRadio firmware and RFStorm research firmware support USB commands to enter the Nordic bootloader. 

Dongles and breakout boards can be programmed over USB if they are running one of the following firmwares: 

- Nordic Semiconductor Bootloader 
- CrazyRadio Firmware 
- RFStorm Research Firmware 

To flash the firmware over USB:

```
make install
```

## Flash over SPI using a Teensy

If your dongle or breakout board is bricked, you can alternatively program it over SPI using a Teensy. 

This has only been tested with a Teensy 3.1/3.2, but is likely to work with other Arduino variants as well. 

### Build and Upload the Teensy Flasher

```
platformio run --project-dir teensy-flasher --target upload
```

### Connect the Teensy to the nRF24LU1+ 

| Teensy | CrazyRadio PA | Sparkfun nRF24LU1+ Breakout |
| ------ | ---------- | -------- |
| GND | 9 | GND |
| 8 | 3 | RESET |
| 9 | 2 | PROG |
| 10 | 10 | P0.3 |
| 11 | 6 | P0.1 |
| 12 | 8 | P0.2 |
| 13 | 4 | P0.0 |
| 3.3V | 5 | VIN |

### Flash the nRF24LU1+

```
sudo make spi_install
```

# Python Scripts 

## scanner

Pseudo-promiscuous mode device discovery tool, which sweeps a list of channels and prints out decoded Enhanced Shockburst packets. 

```
usage: ./nrf24-scanner.py [-h] [-c N [N ...]] [-v] [-l] [-p PREFIX] [-d DWELL]

optional arguments:
  -h, --help                          show this help message and exit
  -c N [N ...], --channels N [N ...]  RF channels
  -v, --verbose                       Enable verbose output
  -l, --lna                           Enable the LNA (for CrazyRadio PA dongles)
  -p PREFIX, --prefix PREFIX          Promiscuous mode address prefix
  -d DWELL, --dwell DWELL             Dwell time per channel, in milliseconds
```

Scan for devices on channels 1-5

```
./nrf24-scanner.py -c {1..5}
```

Scan for devices with an address starting in 0xA9 on all channels

```
./nrf24-scanner.py -p A9
```


## sniffer

Device following sniffer, which follows a specific nRF24 device as it hops, and prints out decoded Enhanced Shockburst packets from the device. 

```
usage: ./nrf24-sniffer.py [-h] [-c N [N ...]] [-v] [-l] -a ADDRESS [-t TIMEOUT] [-k ACK_TIMEOUT] [-r RETRIES]

optional arguments:
  -h, --help                                 show this help message and exit
  -c N [N ...], --channels N [N ...]         RF channels
  -v, --verbose                              Enable verbose output
  -l, --lna                                  Enable the LNA (for CrazyRadio PA dongles)
  -a ADDRESS, --address ADDRESS              Address to sniff, following as it changes channels
  -t TIMEOUT, --timeout TIMEOUT              Channel timeout, in milliseconds
  -k ACK_TIMEOUT, --ack_timeout ACK_TIMEOUT  ACK timeout in microseconds, accepts [250,4000], step 250
  -r RETRIES, --retries RETRIES              Auto retry limit, accepts [0,15]
```

Sniff packets from address 61:49:66:82:03 on all channels

```
./nrf24-sniffer.py -a 61:49:66:82:03
```

## network mapper

Star network mapper, which attempts to discover the active addresses in a star network by changing the last byte in the given address, and pinging each of 256 possible addresses on each channel in the channel list. 

```
usage: ./nrf24-network-mapper.py [-h] [-c N [N ...]] [-v] [-l] -a ADDRESS [-p PASSES] [-k ACK_TIMEOUT] [-r RETRIES]

optional arguments:
  -h, --help                                 show this help message and exit
  -c N [N ...], --channels N [N ...]         RF channels
  -v, --verbose                              Enable verbose output
  -l, --lna                                  Enable the LNA (for CrazyRadio PA dongles)
  -a ADDRESS, --address ADDRESS              Known address
  -p PASSES, --passes PASSES                 Number of passes (default 2)
  -k ACK_TIMEOUT, --ack_timeout ACK_TIMEOUT  ACK timeout in microseconds, accepts [250,4000], step 250
  -r RETRIES, --retries RETRIES              Auto retry limit, accepts [0,15]
```

Map the star network that address 61:49:66:82:03 belongs to

```
./nrf24-network-mapper.py -a 61:49:66:82:03
```

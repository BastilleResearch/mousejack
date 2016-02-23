SDCC    = sdcc
CFLAGS  = --model-large --std-c99
LDFLAGS = --xram-loc 0x8000 --xram-size 2048 --model-large
VPATH   = src/

OBJS = main.rel usb.rel usb_desc.rel radio.rel

all: dongle.bin

dongle.bin: $(OBJS)
	$(SDCC) $(LDFLAGS) $(OBJS:%=bin/%) -o bin/dongle.ihx
	objcopy -I ihex bin/dongle.ihx -O binary bin/dongle.bin

%.rel: %.c
	$(SDCC) $(CFLAGS) -c $< -o bin/$@

clean:
	rm -f bin/*

install:
	./prog/usb-flasher/usb-flash.py bin/dongle.bin

spi_install:
	./prog/teensy-flasher/python/spi-flash.py bin/dongle.bin

bin/:
	mkdir -p bin

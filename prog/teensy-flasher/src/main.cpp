#include "common.h"
#include "nRF24LU1P.h"

// Serial commands
#define READ_PAGE       0x00
#define WRITE_PAGE      0x02

// Setup serial and nRF24LU1+ SPI 
void setup()
{
  // Init Serial 
  Serial.begin(115200);
  Serial.setTimeout(250);

  // Configure the pins and enter programming mode
  init_nrf();
}

// Main loop; process commands over serial to read and write
// from the MainBlock and InfoPage flash memory 
void loop()
{
  static uint8_t read_buffer[2];
  static uint8_t data_buffer[512 + 2 /* address */];
  static int len;

  // Process incoming commands
  if(Serial.readBytes((char *)read_buffer, 2) == 2)
  {
    switch(read_buffer[0])
    {
      // Read a MainBlock page
      case READ_PAGE:
        read_page(read_buffer[1]);
        break;

      // Write a MainBlock page or InfoPage
      case WRITE_PAGE:

        // Read in 512 bytes
        len = Serial.readBytes((char *)&data_buffer[2], 512);
        if(len != 512)
        {
          printf("Expected 512 bytes, got %i\n", len);
          return;
        }

        // Write the page
        write_page(read_buffer[1], data_buffer);
        break;        
    }
  } 
}


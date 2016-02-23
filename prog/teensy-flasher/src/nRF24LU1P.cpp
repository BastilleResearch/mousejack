#include "common.h"
#include "nRF24LU1P.h"

// SPI settings
SPISettings spi_settings(SPI_SPEED, MSBFIRST, SPI_MODE0);

// Configure the pins and enter programming mode
void init_nrf()
{
  // Init SPI
  SPI.begin();

  // Configure the pins 
  pinMode(CS_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(PROG_PIN, OUTPUT);

  // Enter programming mode
  digitalWrite(CS_PIN, HIGH);
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(PROG_PIN, LOW);
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(PROG_PIN, HIGH);
  delayMicroseconds(5000);  
}

// Wait for a flash operation to complete
void flash_wait()
{
  uint8_t fsr;
  spi_read(RDSR, &fsr, 1);
  while((fsr & FSR_RDYN) == FSR_RDYN) spi_read(RDSR, &fsr, 1);
}

// Write a page of memory (512 bytes)
void write_page(uint16_t page, uint8_t * data)
{
  // Erase the page
  uint8_t bytes[1] = {page};
  spi_write(WREN, NULL, 0);
  spi_write(ERASE_PAGE, bytes, 1);
  flash_wait();

  // Set the start address
  uint16_t address = page * 512; 
  data[0] = address >> 8;
  data[1] = address & 0xFF;

  // Write the first half of the page
  spi_write(WREN, NULL, 0);
  spi_write(PROGRAM, data, 258);
  flash_wait();

  // Set the start address of the second half of the page
  address += 256; 
  data[256] = address >> 8;
  data[257] = address & 0xFF;

  // Write the second half of the page
  spi_write(WREN, NULL, 0);
  spi_write(PROGRAM, &data[256], 258);
  flash_wait();

  printf(" \n");
}

// Read a page of memory (512 bytes) and send
// the bytes over serial in Intel Hex format
void read_page(uint16_t page)
{
  // Initiate the SPI transaction
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);        

  // Send the READ command over SPI
  uint16_t address = page * 512; 
  SPI.transfer(READ);
  SPI.transfer(address >> 8);
  SPI.transfer(address & 0xFF);

  // Read the page 32 bytes at a time
  uint8_t buffer[32];
  for(int x = 0; x < 16; x++)
  {
    // Read 32 bytes
    memset(buffer, 0x00, 32);
    SPI.transfer(buffer, 32);

    // Print the bytes
    for(int b = 0; b < 32; b++) 
    {
      printf("%02X", buffer[b]);
    }
    delayMicroseconds(10);
  }

  // Complete the SPI transaction 
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();  

  printf("\n");
}

// Write some bytes over SPI
void spi_write(uint8_t command, uint8_t * buffer, uint16_t length)
{
  SPI.beginTransaction(spi_settings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command);
  for(int x = 0; x < length; x++) SPI.transfer(buffer[x]);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

// Read some bytes over SPI
void spi_read(uint8_t command, uint8_t * buffer, uint16_t length)
{
  SPI.beginTransaction(spi_settings);
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command);
  for(int x = 0; x < length; x++) buffer[x] = SPI.transfer(0xFF);
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();
}

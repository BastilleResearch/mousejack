// nRF24LU1+ SPI commands
#define WREN       0x06
#define WRDIS      0x04
#define RDSR       0x05
#define WRSR       0x01
#define READ       0x03
#define PROGRAM    0x02
#define ERASE_PAGE 0x52
#define ERASE_ALL  0x62
#define RDFPCR     0x89
#define RDISIP     0x84
#define RDISMP     0x85
#define ENDEBUG    0x86

// nRF24LU1+ Flash Status Register bitmask
#define FSR_DBG    0x80
#define FSR_STP    0x40
#define FSR_WEN    0x20
#define FSR_RDYN   0x10
#define FSR_INFEN  0x08
#define FSR_RDISMB 0x04
#define FSR_RDISIP 0x02

// Output pins (assuming MISO/MOSI/CLK are using the HW defaults)
#define RESET_PIN  8
#define PROG_PIN   9
#define CS_PIN     10

// SPI clock speed of 10MHz
#define SPI_SPEED  10000000

// SPI settings
extern SPISettings spi_settings;

// Configure the pins and enter programming mode
void init_nrf();

// Read a page of memory (512 bytes) and send
// the bytes over serial as hex characters 
void read_page(uint16_t page);

// Write a page of memory (512 bytes)
void write_page(uint16_t page, uint8_t * data);

// Wait for a flash operation to complete
void flash_wait();

// Write some bytes over SPI
void spi_write(uint8_t command, uint8_t * buffer, uint16_t length);

// Read some bytes over SPI
void spi_read(uint8_t command, uint8_t * buffer, uint16_t length);

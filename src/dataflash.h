#ifndef __DATAFLASH_H__
#define __DATAFLASH_H__


/////// Global Definitions ////////////////////////////////////////////////////////////////////////

//Global status register flags

#define T1_OVF  0x01
#define CLEARED 0x02


// DataFlash SPI port
#define DF_SPI_PORT PORTB
#define DF_SPI_DDR  DDRB
#define DF_SPI_PIN  PINB

// DataFlash SPI pins
#define DF_SPI_SCK  PB5
#define DF_SPI_MISO PB4
#define DF_SPI_MOSI PB3
#define DF_SPI_SS   PB2


// DataFlash reset port pin
//#define DF_RESET
         
// DataFlash ready/busy status port pin
//#define DF_READY

// DataFlash boot sector write protection
//#define DF_WRITEPROTECT

// DataFlash chip select port pin
//#define DF_CHIPSELECT


// DataFlash sizes
#define DF_PAGE_SIZE    528
#define DF_BLOCK_SIZE   (8*(DF_PAGE_SIZE))
#define DF_SECTOR_SIZE  (32*(DF_BLOCK_SIZE))

#define DF_MAX_BLOCKS   512
#define DF_MAX_PAGES    4096


// buffer 1 
#define DF_B1 0x00

// buffer 2
#define DF_B2 0x01


// defines for all opcodes

// buffer 1 write 
#define DF_CMD_WRITE_B1 0x84

// buffer 2 write 
#define DF_CMD_WRITE_B2 0x87

// buffer 1 read
#define DF_CMD_READ_B1 0x54

// buffer 2 read
#define DF_CMD_READ_B2 0x56

// buffer 1 to main memory page program with built-in erase
#define DF_CMD_PROG_B1_TO_MM_PAGE_WITH_ERASE 0x83

// buffer 2 to main memory page program with built-in erase
#define DF_CMD_PROG_B2_TO_MM_PAGE_WITH_ERASE 0x86

// buffer 1 to main memory page program without built-in erase
#define DF_CMD_PROG_B1_TO_MM_PAGE 0x88

// buffer 2 to main memory page program without built-in erase
#define DF_CMD_PROG_B2_TO_MM_PAGE 0x89

// main memory page program through buffer 1
#define DF_CMD_PROG_MM_PAGE_THROUGH_B1 0x82
 
// main memory page program through buffer 2
#define DF_CMD_PROG_MM_PAGE_THROUGH_B2 0x85
 
// auto page rewrite through buffer 1
#define DF_CMD_REWRITE_PAGE_THROUGH_B1 0x58
 
// auto page rewrite through buffer 2
#define DF_CMD_REWRITE_PAGE_THROUGH_B2 0x59
 
// main memory page compare to buffer 1
#define DF_CMD_COMP_MM_PAGE_TO_B1 0x60

// main memory page compare to buffer 2
#define DF_CMD_COMP_MM_PAGE_TO_B2 0x61
 
// main memory page to buffer 1 transfer
#define DF_CMD_MM_PAGE_TO_B1_XFER 0x53

// main memory page to buffer 2 transfer
#define DF_CMD_XFER_MM_PAGE_TO_B2 0x55

// read status register (reading density, compare and ready/busy status)
#define DF_CMD_READ_STATUS 0x57

// read manufacturer and device id
#define DF_CMD_READ_ID 0x9F

// main memory page read
#define DF_CMD_READ_MM_PAGE 0x52

// erase a 528 byte page
#define DF_CMD_ERASE_PAGE 0x81

// erase 512 pages
#define DF_CMD_ERASE_BLOCK 0x50

// erase whole chip (4 command bytes)
#define DF_CMD_ERASE_CHIP1 0xC7
#define DF_CMD_ERASE_CHIP2 0x94
#define DF_CMD_ERASE_CHIP3 0x80
#define DF_CMD_ERASE_CHIP4 0x9A


/////// Function Prototypes ///////////////////////////////////////////////////////////////////////

void  df_init(void);                            //inititalize dataflash (makes use of stdio)

void  df_read_open();                           //prepare reading the flash
void  df_read_close();                          //finished reading the flash
uint8 df_read_getc();                           //read next byte and increase offset

void  df_write_open();                          //formats device and returns true if enough space
void  df_write_close();                         //finalize written data, return success
void  df_write(uint8 *buf, uint16 size);        //sequentially write to the dataflash, return success



#endif //__DATAFLASH_H__

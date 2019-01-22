/**************************************************************************************************
*
* Access atmel dataflash over SPI.
*
* file              : dataflash.c
* compiler          : avr-gcc (WinAvr)
* revision          : 1.0
* date              : 12/2008
* author            : wiRe <http://w1r3.de>
*
***************************************************************************************************
*
* DESCRIPTION
*
* @see Atmel Application Note AVR335
*
**************************************************************************************************/
#include "common.h"
#include "dataflash.h"
#include "playback.h"
#include "uart.h"


/////// Local Macros //////////////////////////////////////////////////////////////////////////////

#define DF_SELECT()   SBIT(DF_SPI_PORT, DF_SPI_SS) = 0
#define DF_DESELECT() SBIT(DF_SPI_PORT, DF_SPI_SS) = 1


/////// Local Variables ///////////////////////////////////////////////////////////////////////////

//uint32 df_read_ofs = 0;

uint16 df_write_ofs = 0;
uint16 df_write_page = DF_MAX_PAGES;


/////// EEPROM Variables //////////////////////////////////////////////////////////////////////////

//uint8 ee_lba_lut[((DF_MAX_BLOCKS)+7)/8] EEMEM; //contains 1bit for all dataflash blocks (1=ok, 0=broken)

//uint8 ee_fname[13] EEMEM    //filename of upload
//  = {'$','$','$','$','$','$','$','$','$','.','$','$','$'};

//uint32 ee_fsize EEMEM;  //filesize of upload


/////// Functions /////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  exchange one byte with the spi device
//-------------------------------------------------------------------------------------------------
uint8 df_spi_tx(uint8 data)
{
  SPDR = data;                            // send data
  loop_until_bit_is_set(SPSR, SPIF);      // wait for data transfer to be completed
  return SPDR;                            // return answer
}


//-------------------------------------------------------------------------------------------------
//  init dataflash lib
//-------------------------------------------------------------------------------------------------
void df_init(void)
{
  //uint8 sz;


  DF_SPI_DDR |= (1<<DF_SPI_SCK)           // port initialisation
    |(1<<DF_SPI_MOSI)|(1<<DF_SPI_SS);     // SCK:OUT, MISO:IN, MOSI:OUT, SS:OUT

  DF_SPI_PORT |= (1<<DF_SPI_SCK)          // all outputs high, inputs have pullups
    |(1<<DF_SPI_MISO)|(1<<DF_SPI_MOSI)
    |(1<<DF_SPI_SS);

  SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA);   // enable SPI as master, fck/4

  DF_SELECT();                            // read manufacturer and device id
  df_spi_tx(DF_CMD_READ_ID);
  printf_P(PSTR("dataflash: %x, %x, %x\n"),
    df_spi_tx(0xFF), df_spi_tx(0xFF), df_spi_tx(0xFF));
  //for(sz = df_spi_tx(0xFF); sz > 0; sz--) putchar(df_spi_tx(0xFF)); //read extended information if available
  //putchar('\n');
  DF_DESELECT();


  //
  // run debug terminal, allowing direct access to the spi bus over rs232 uart
  // control code 0xEE deselects the device
  //
  #ifdef DATAFLASH_DEBUG_TERMINAL
    puts_P(PSTR("enter dataflash debug terminal..."));
    putchar('\0');

    while(1)
    {
      sz = uart_rx();
      if(sz == 0xEE)
      {
        DF_DESELECT();
      }
      else
      {
        DF_SELECT();
        uart_tx(df_spi_tx(sz));
      }
    }
  #endif
}


//-------------------------------------------------------------------------------------------------
void df_read_open()
{
  DF_SELECT();                            // start continuous array read
  df_spi_tx(0xE8);
  df_spi_tx(0x00);
  df_spi_tx(0x00);
  df_spi_tx(0x00);
  df_spi_tx(0x00);  //dmy
  df_spi_tx(0x00);
  df_spi_tx(0x00);
  df_spi_tx(0x00);
}


//-------------------------------------------------------------------------------------------------
void df_read_close()
{
  DF_DESELECT();
}


//-------------------------------------------------------------------------------------------------
//  read one more sample/byte (during playback)
//-------------------------------------------------------------------------------------------------
uint8 df_read_getc()
{
  return df_spi_tx(0xFF);
}


//-------------------------------------------------------------------------------------------------
//  prepare flash for writing (format, error checking, etc.)
//-------------------------------------------------------------------------------------------------
void df_write_open()
{
  playback_stop();                    //stop playing when we write

  //
  // NOTE: chip-erase doesnt work, only block- or page-wise
  //       we don't erase the chip before transfer (even if it's faster)
  //       instead we use program-width-erase command and clear the remaining pages at the end
  //
  //for(df_write_ofs = 0; df_write_ofs < DF_MAX_BLOCKS; df_write_ofs++)
  //{  
  //  DF_SELECT();                        //execute block erase command
  //  df_spi_tx(DF_CMD_ERASE_BLOCK);
  //  df_spi_tx((uint8)(df_write_ofs >> 3));
  //  df_spi_tx((uint8)(df_write_ofs << 5));
  //  df_spi_tx(0x00);
  //  DF_DESELECT();
  //
  //  DF_SELECT();                        //wait until block was erased
  //  df_spi_tx(DF_CMD_READ_STATUS);
  //  while((df_spi_tx(0xFF) & 0x80) == 0);
  //  DF_DESELECT();
  //}

  df_write_page = df_write_ofs = 0;   //reset offsets for writing
}


//-------------------------------------------------------------------------------------------------
//  finish writing to flash (flush buffers)
//-------------------------------------------------------------------------------------------------
void df_write_close()
{
  if(df_write_ofs > 0 && df_write_page < DF_MAX_PAGES)  //finish writing block
  {
    // fill remaining buffer
    DF_SELECT();
    df_spi_tx(DF_CMD_WRITE_B1);
    df_spi_tx(0x00);
    df_spi_tx((uint8)(df_write_ofs >> 8));
    df_spi_tx((uint8)(df_write_ofs     ));
    for(; df_write_ofs < DF_PAGE_SIZE; df_write_ofs++) df_spi_tx(0xFF);
    DF_DESELECT();
    
    DF_SELECT();
    df_spi_tx(DF_CMD_PROG_B1_TO_MM_PAGE_WITH_ERASE);
    df_spi_tx((uint8)(df_write_page >> 6));
    df_spi_tx((uint8)(df_write_page << 2));
    df_spi_tx(0x00);
    DF_DESELECT();

    DF_SELECT();                        //wait until operation finished
    df_spi_tx(DF_CMD_READ_STATUS);
    while((df_spi_tx(0xFF) & 0x80) == 0);   //TODO: check for timeout
    DF_DESELECT();

    df_write_page++;
  }

  for(; df_write_page < DF_MAX_PAGES; df_write_page++)  //erase remaining pages
  {
    DF_SELECT();                        //execute page erase command
    df_spi_tx(DF_CMD_ERASE_PAGE);
    df_spi_tx((uint8)(df_write_page >> 6));
    df_spi_tx((uint8)(df_write_page << 2));
    df_spi_tx(0x00);
    DF_DESELECT();

    DF_SELECT();                        //wait until block was erased
    df_spi_tx(DF_CMD_READ_STATUS);
    while((df_spi_tx(0xFF) & 0x80) == 0);
    DF_DESELECT();
  }

  playback_start();     //restart playing
}


//-------------------------------------------------------------------------------------------------
//  write data into flash sequentilly
//-------------------------------------------------------------------------------------------------
void df_write(uint8 *buf, uint16 size)
{
  uint16 remainder;

  while(size > 0)
  {
    // fillup the buffer
    DF_SELECT();
    df_spi_tx(DF_CMD_WRITE_B1);
    df_spi_tx(0x00);
    df_spi_tx((uint8)(df_write_ofs >> 8));
    df_spi_tx((uint8)(df_write_ofs     ));

    remainder = DF_PAGE_SIZE - df_write_ofs;
    if(remainder > size)  //not enough data for a full page, only add the bytes and exit
    {
      df_write_ofs += size;
      while(size--) df_spi_tx(*(buf++));
      DF_DESELECT();
      return;
    }

    size -= remainder;
    df_write_ofs = 0;
    while(remainder--) df_spi_tx(*(buf++));
    DF_DESELECT();
    
    // write page
    if(df_write_page < DF_MAX_PAGES)
    {
      DF_SELECT();
      df_spi_tx(DF_CMD_PROG_B1_TO_MM_PAGE_WITH_ERASE);
      df_spi_tx((uint8)(df_write_page >> 6));
      df_spi_tx((uint8)(df_write_page << 2));
      df_spi_tx(0x00);
      DF_DESELECT();

      DF_SELECT();                        //wait until operation finished
      df_spi_tx(DF_CMD_READ_STATUS);
      while((df_spi_tx(0xFF) & 0x80) == 0);   //TODO: check for timeout
      DF_DESELECT();

      df_write_page++;
    }
  }
}

/**************************************************************************************************
*
* Implementation of a xmodem file receiver with use of crc for error detection.
*
* file              : xmodem.c
* compiler          : avr-gcc (WinAvr)
* revision          : 1.0
* date              : 12/2008
* author            : jtyssoe
*                     wiRe <http://w1r3.de>
*
***************************************************************************************************
*
* DESCRIPTION
*
* @see Atmel Application Note AVR350
*
**************************************************************************************************/
#include "common.h"
#include "xmodem.h"
#include "uart.h"
#include "dataflash.h"


/////// Local Macros And Constants ////////////////////////////////////////////////////////////////


/////// Local Variables ///////////////////////////////////////////////////////////////////////////

uint8  xmodem_timeout;              //timeout counter (decreases with frequence: F_CPU>>18)
uint16 xmodem_seq;                  //sequence number of last successfully received packet

#ifndef PLAYBACK_XMODEM_READ
  uint8  xmodem_buf[XMODEM_BUF_SIZE]; //buffer for packet reception
#else
  uint8 xmodem_buf1[XMODEM_BUF_SIZE]; //use double-buffering on direct xmodem playback
  uint8 xmodem_buf2[XMODEM_BUF_SIZE];
  uint8 *xmodem_buf = xmodem_buf1;

  uint8 xmodem_getc_ofs = 0;
#endif


/////// Functions /////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  calculate crc16
//-------------------------------------------------------------------------------------------------
uint16 xmodem_crc(uint8 *ptr, uint16 size)
{
  register uint8 i;
  register uint16 crc = XMODEM_CRC_INITIALIZE;

  while(size--)
  {
    crc = crc ^ (((uint16)*(ptr++)) << 8);
    for(i = 8; i; i--)
    {
      if(crc & 0x8000) crc = (crc << 1) ^ 0x1021;
        else crc <<= 1;
    }
  }
  return crc ^ XMODEM_CRC_FINALIZE;
} 


//-------------------------------------------------------------------------------------------------
//  receive xmodem packet, validate and return state
//-------------------------------------------------------------------------------------------------
uint8 xmodem_rx_packet()
{
  uint16 crc;
  uint8 buf_index = 0;


  // transmission started?
  if(!xmodem_seq) uart_tx(XMODEM_CRCCHR);           //try to establish connection

  // receive bytes... (until packet buffer full, com-error or timeout)
  VUINT8(xmodem_timeout) = xmodem_seq? 42:126;      //~1/3sec
  while(buf_index < XMODEM_BUF_SIZE)
  {
    if(uart_rx_rdy())
    {
      while(uart_rx_rdy() && (buf_index < XMODEM_BUF_SIZE))
        xmodem_buf[buf_index++] = uart_rx();
      if(uart_get_error())
        return XMODEM_PACKET_ERR;                   //uart framing or overrun error
      if(xmodem_buf[0] == XMODEM_EOT)               //check for the end
        return XMODEM_PACKET_END;
      if(xmodem_buf[0] != XMODEM_SOH)               //valid start?
        return XMODEM_PACKET_BAD;                   //  otherwise we have a bad packet

      VUINT8(xmodem_timeout) = xmodem_seq? 42:126;  //~1/3sec
    }

    if(VUINT8(xmodem_timeout) == 0)
      return xmodem_seq? XMODEM_PACKET_OUT          //timeout error
                        :XMODEM_PACKET_NC;          //or no connection established on first paket
  }

  // validate received packet...
  if(xmodem_buf[1] == ((xmodem_seq+1) & 0xFF))      //sequential block number?
  {
    xmodem_buf[2] += xmodem_buf[1];                 //block number checksum valid?
    if(xmodem_buf[2] == 0xFF)
    {
      crc = xmodem_crc(                             //compute CRC and validate it (big-endian)
        &xmodem_buf[XMODEM_HEADER_SIZE],
        XMODEM_PACKET_SIZE);
      if((xmodem_buf[XMODEM_HEADER_SIZE+XMODEM_PACKET_SIZE+0] == (uint8)(crc >> 8))
        && (xmodem_buf[XMODEM_HEADER_SIZE+XMODEM_PACKET_SIZE+1] == (uint8)(crc)))
      {
        xmodem_seq++;                               //good packet, increment packet number
        return (xmodem_seq < 0xFFFF)?
          XMODEM_PACKET_GOOD : XMODEM_PACKET_END;   //end on packet number overflow
      } //bad CRC
    } //bad block number checksum
  } //bad block number or same block number
  else if(xmodem_buf[1] == (xmodem_seq & 0xFF))
  {                                                 //same block number ... ack got glitched
    return XMODEM_PACKET_DUP;                       //packet is duplicate, don't inc packet number
  }

  return XMODEM_PACKET_BAD;                         //otherwise we have a bad packet
}


//-------------------------------------------------------------------------------------------------
//  xmodem init
//-------------------------------------------------------------------------------------------------
void xmodem_init()
{
  TCCR0A = 0;                                       //setup timer 0 interrupt to measure timeouts
  TCCR0B = (1<<CS02)|(1<<CS00);                     //  fclk/1024
  TIMSK0 = (1<<TOIE0);  
}


//-------------------------------------------------------------------------------------------------
//  xmodem reception
//-------------------------------------------------------------------------------------------------
void xmodem_rx()
{
  uint8 retry_cntr = 0;


  xmodem_seq = 0;               //initialise to first xmodem packet number - 1
  uart_rx_flush();              //clear input buffer

  while(1)                      //start receiving...
  {
    switch(xmodem_rx_packet())  //validate the packet
    {
      case XMODEM_PACKET_GOOD:  //call external data handler
        
        #ifndef PLAYBACK_XMODEM_READ
          //xmodem_rx_write(
          //  &xmodem_buf[XMODEM_HEADER_SIZE],
          //  XMODEM_PACKET_SIZE);

          if(xmodem_seq == 1)   //first packet?
            df_write_open();    //  prepare flash first

          df_write(&xmodem_buf[XMODEM_HEADER_SIZE], XMODEM_PACKET_SIZE);

        #else
          while(((xmodem_buf == xmodem_buf1) && VUINT8(xmodem_buf2[0]))
            || ((xmodem_buf == xmodem_buf2) && VUINT8(xmodem_buf1[0])));
          xmodem_buf =
            (xmodem_buf == xmodem_buf1)? xmodem_buf2 : xmodem_buf1;
        #endif

        uart_tx(XMODEM_ACK);
        retry_cntr = 0;         //reset retry counter
        break;

      case XMODEM_PACKET_DUP:
        // a counter for duplicate packets could be added here, to enable a
        // for example, exit gracefully if too many consecutive duplicates,
        // otherwise do nothing, we will just ack this
        
        uart_tx(XMODEM_ACK);
        break;

      case XMODEM_PACKET_END:
        
        #ifndef PLAYBACK_XMODEM_READ
          if(xmodem_seq)
            df_write_close();   //finish writing to dataflash (only if it was started)
        #endif
        
        uart_tx(XMODEM_ACK);    //end of file transmission, just acknowledge
        return;                 //  and exit

      case XMODEM_PACKET_NC:    //just wait until connection can get established
        break;

      default:
        // paket bad, timeout or error -
        // if required, insert an error handler of some description,
        // for example, exit gracefully if too many errors
        
        _delay_ms(1000);        //purge com port        
        uart_rx_flush();        //
        uart_get_error();       //
        uart_tx(XMODEM_NAK);    //  and signal error
        break;
    }

    if(retry_cntr++ >= XMODEM_MAX_RETRIES)    //too many retries?
    {
      #ifndef PLAYBACK_XMODEM_READ
        if(xmodem_seq)
          df_write_close();     //finish writing to dataflash (only if it was started)
      #endif
      
      return;
    }
  }
}


#ifdef PLAYBACK_XMODEM_READ
  uint8 xmodem_getc()
  {
    uint8 result = 0x80;


    if(xmodem_seq)  //samples available?
    {
      result = (xmodem_buf == xmodem_buf1)?  //get sample from background buffer
        xmodem_buf2[3 + xmodem_getc_ofs] : xmodem_buf1[3 + xmodem_getc_ofs];

      if(xmodem_getc_ofs == 127) //free background buffer
      {
        ((xmodem_buf == xmodem_buf1)?xmodem_buf2:xmodem_buf1)[0] = 0;
        xmodem_getc_ofs = 0;
      }
      else
      {
        xmodem_getc_ofs++;
      }
    }

    return result;
  }

  void xmodem_read_rewind() {}  //dummies, not supported/needed for xmodem streaming
  void xmodem_read_open()   {}
  void xmodem_read_close()  {}
#endif


/////// Interrupt Handlers ////////////////////////////////////////////////////////////////////////
ISR( TIMER0_OVF_vect )
{
  if(xmodem_timeout) xmodem_timeout--;
}

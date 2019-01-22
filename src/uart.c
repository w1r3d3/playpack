/**************************************************************************************************
*
* Interrupt driven buffered UART.
*
* file              : uart.c
* compiler          : avr-gcc (WinAvr)
* revision          : 1.0
* date              : 12/2008
* author            : Peter Dannegger
*                     wiRe <http://w1r3.de>
*
**************************************************************************************************/
#include "common.h"
#include "uart.h"


/////// Local Macros //////////////////////////////////////////////////////////////////////////////

#define	UTX0_IEN SBIT(UCSR0B, UDRIE0)
#define	URX0_IEN SBIT(UCSR0B, RXCIE0)

#define FIFO_INC_OFS(x, max)	x = ++x >= max ? 0 : x	// count up and wrap around


/////// Local Variables ///////////////////////////////////////////////////////////////////////////

static uint8 uart_error;

static uint8 uart_rx_fifo[UART_RX_FIFO_SIZE];
static uint8 uart_rx_in;
static uint8 uart_rx_out;

static uint8 uart_tx_fifo[UART_TX_FIFO_SIZE];
static uint8 uart_tx_in;
static uint8 uart_tx_out;


/////// Functions /////////////////////////////////////////////////////////////////////////////////

void uart_init(uint16 baudrate_div)
{
  UBRR0H = baudrate_div >> 8;
  UBRR0L = baudrate_div;	            // set baud rate

  UCSR0A = 0;				                  // no U2X, MPCM
  UCSR0C = 1<<UCSZ01 ^ 1<<UCSZ00	    // 8 Bit
    #ifdef URSEL0
	    ^ 1<<URSEL0			                // if UCSR0C shared with UBRR0H
    #endif
	  ;
  UCSR0B = 1<<RXEN0 ^ 1<<TXEN0 ^	    // enable RX, TX
    1<<RXCIE0;			                  // enable RX interrupt

  uart_rx_in = uart_rx_out = 0;       // set buffer empty
  uart_tx_in = uart_tx_out = 0;

  uart_error = 0;                     // clear error flags
}


uint8 uart_get_error()
{
  register uint8 result = VUINT8(uart_error);

  VUINT8(uart_error) ^= result;
  return result;
}


void uart_rx_flush()
{
  uart_rx_out = VUINT8(uart_rx_in);
}


uint8 uart_rx_rdy()
{
  return uart_rx_out ^ VUINT8(uart_rx_in);		// uart_rx_in modified by interrupt !
}


uint8 uart_rx()
{
  uint8 data;


  while( !uart_rx_rdy() );            // until at least one byte in
  data = uart_rx_fifo[uart_rx_out];		// get byte
  FIFO_INC_OFS(uart_rx_out, UART_RX_FIFO_SIZE);
  URX0_IEN = 1;				                // enable RX interrupt
  return data;
}


void uart_tx_flush()
{
  uart_tx_in = VUINT8(uart_tx_out);
}


uint8 uart_tx_rdy()
{
  uint8 i = uart_tx_in;


  FIFO_INC_OFS(i, UART_TX_FIFO_SIZE);
  return VUINT8(uart_tx_out) ^ i;		  // 0 = busy
}


void uart_tx(uint8 c)
{
  uint8 i = uart_tx_in;


  FIFO_INC_OFS( i, UART_TX_FIFO_SIZE );
  uart_tx_fifo[uart_tx_in] = c;
  while( i == VUINT8(uart_tx_out));	  // until at least one byte free (DANGER: locks if interrupts are disabled!!!)
					                            // uart_tx_out modified by interrupt !
  uart_tx_in = i;
  UTX0_IEN = 1;                       // enable TX interrupt
}


/////// Interrupt Handlers ////////////////////////////////////////////////////////////////////////

ISR( UART_RX_vect )                   // another byte was received
{
  uint8 i = uart_rx_in;


  if (UCSR0A & FE0)                   // framing error?
  {
    uart_error |= UART_ERROR_RX_FRAME;
  }

  if (UCSR0A & DOR0)                  // overflow error?
  {
    uart_error |= UART_ERROR_RX_OVRFLW;
  }

  FIFO_INC_OFS(i, UART_RX_FIFO_SIZE);
  if( i == uart_rx_out ) {			      // buffer overflow
    URX0_IEN = 0;			                // disable RX interrupt
    uart_error |= UART_ERROR_RX_OVRFLW;
    return;
  }
  uart_rx_fifo[uart_rx_in] = UDR0;
  uart_rx_in = i;
}


ISR( UART_UDRE_vect )                 // another byte can be transmitted
{
  if( uart_tx_in == uart_tx_out ){		// nothing to sent
    UTX0_IEN = 0;			                // disable TX interrupt
    return;
  }
  UDR0 = uart_tx_fifo[uart_tx_out];
  FIFO_INC_OFS( uart_tx_out, UART_TX_FIFO_SIZE );
}

/**************************************************************************************************
*
* playPack main routine.
*
* file              : main.c
* compiler          : avr-gcc (WinAvr)
* revision          : 1.0
* date              : 12/2008
* author            : wiRe <http://w1r3.de>
*
**************************************************************************************************/
#include "common.h"
#include "uart.h"
#include "dataflash.h"
#include "playback.h"
#include "xmodem.h"


/////// ATmega48 Fuse And Lock Bits ///////////////////////////////////////////////////////////////

FUSES = {.low = 0xDF, .high = 0xDD, .extended = 0xFF};
LOCKBITS = 0xFC;


/////// Constants /////////////////////////////////////////////////////////////////////////////////

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define STARTUP_DELAY 1000  //startup delay in ms

#ifndef F_BAUD
  #define F_BAUD 115200
#endif


/////// Functions /////////////////////////////////////////////////////////////////////////////////

static int _putc(char c, FILE *f)
{
  //if (c == '\n') _putc('\r', f);
  uart_tx(c);
  //loop_until_bit_is_set(UCSR0A, UDRIE0);
  //UDR0 = c;
  return 0;
}

static FILE _stdout = FDEV_SETUP_STREAM(_putc, NULL, _FDEV_SETUP_WRITE);


/////// Main Routine //////////////////////////////////////////////////////////////////////////////

int main()
{
  _delay_ms(STARTUP_DELAY);

  uart_init( UART_BAUDRATE(F_BAUD) );
  stdout = &_stdout;
  sei();              //stdio available now

  printf_P(PSTR("\nplayPack v%u.%u\n"\
    "(c)2008 wiRe <http://w1r3.de>\n"),
    VERSION_MAJOR, VERSION_MINOR);

  df_init();        //init dataflash
  xmodem_init();    //init xmodem receiver
  playback_init();  //init playback
  playback_start(); //  and start playing sound immediately

  printf_P(PSTR("use xmodem to upload raw audio data (%uHz, u8, mono)...\n"), PLAYBACK_FREQUENCY);
  while(1)
  {
    xmodem_rx();  //allow reception of files over uart/zmodem
    //putchar('\n');
    //while(1);
  }

  return 0;
}

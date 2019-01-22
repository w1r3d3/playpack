/**************************************************************************************************
*
* Playback of audio informations.
*
* file              : playback.c
* compiler          : avr-gcc (WinAvr)
* revision          : 1.0
* date              : 12/2008
* author            : wiRe <http://w1r3.de>
*
***************************************************************************************************
*
* TODO
*
* - support more audio file formats (actually only raw samples at fixed frequencies are allowed)
*   like eg. AIFF, WAV, VOC, etc.
* - support simple audio compression formats
*
**************************************************************************************************/
#include "common.h"
#include "dataflash.h"
#include "playback.h"
#include "xmodem.h"


/////// Local Constants ///////////////////////////////////////////////////////////////////////////
#define PLAYBACK_FAST_PWM


/////// Local Variables ///////////////////////////////////////////////////////////////////////////

uint8 playback_sample = 0;


/////// Functions /////////////////////////////////////////////////////////////////////////////////

void playback_init()
{
  #ifndef PLAYBACK_FAST_PWM                     //setup timer1
    TCCR1A = (1<<COM1A1)|(0<<WGM11)|(0<<WGM10); //  phase and fequency correct pwm (mode 8)
    TCCR1B =             (1<<WGM13)|(0<<WGM12); //
  #else
    TCCR1A = (1<<COM1A1)|(0<<WGM11)|(1<<WGM10); //  8bit fast pwm (mode 5)
    TCCR1B =             (0<<WGM13)|(1<<WGM12); //
  #endif

  ICR1 = 0x00FF;                                //  8bit => TOP = 0xFF
  OCR1A = playback_sample;                      //  init pwm output

  DDRB |= (1<<1);               //declare PB1/OC1A as output pin
  TCCR1B |= (1<<CS10);          //start timer1: clock source = fclk/1

  TCCR2A = (1<<WGM21);          //setup timer2: ctc mode, no clock source
  TCCR2B = 0;
  TCNT2 = 0;
  SBIT(TIFR2, OCF2A) = 1;       //enable output compare interrupt OC2A
  SBIT(TIMSK2, OCIE2A) = 1;
  OCR2A = ((F_CPU/8) / PLAYBACK_FREQUENCY);
}

void playback_start()
{
  #ifndef PLAYBACK_XMODEM_READ
    df_read_open();
  #else
    xmodem_read_open();
  #endif

  TCCR2B = (1<<CS21);           //start timer2: set clock source fclk/8
}

void playback_stop()
{
  if(TCCR2B != 0)
  {
    TCCR2B = 0;                 //pause timer2
    SBIT(TIFR2, OCF2A) = 1;

    #ifndef PLAYBACK_XMODEM_READ
      df_read_close();
    #else
      xmodem_read_close();
    #endif
  }
}


/////// Interrupt Handlers ////////////////////////////////////////////////////////////////////////

ISR(TIMER2_COMPA_vect)
{
  // play next sample
  #ifndef PLAYBACK_DEBUG
  
    OCR1AL = playback_sample;
  
  #else //PLAYBACK_DEBUG

    //
    // send sample (byte from flash) to uart instead playing
    //
    if((playback_sample >= 32 && playback_sample < 128) || playback_sample == '\n')
    {
      loop_until_bit_is_set(UCSR0A, UDRIE0);
      UDR0 = playback_sample;  
    }

  #endif

  #ifndef PLAYBACK_XMODEM_READ
    playback_sample = df_read_getc();       //read next sample and incread offset
  #else
    playback_sample = xmodem_read_getc();   //...directly from xmodem
  #endif
}

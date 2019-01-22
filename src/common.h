#ifndef __COMMON_H__
#define __COMMON_H__


/////// Include Files /////////////////////////////////////////////////////////////////////////////

#include <inttypes.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


/////// Global Constants and Types ////////////////////////////////////////////////////////////////

#ifndef F_CPU
  #error cpu frequency not specified
#endif

#undef PLAYBACK_XMODEM_READ   //play samples directly from xmodem stream instead from flash

// delay values based on the actual cpu clock frequency
//#define _DELAY_VALUE(_d8_) (((_d8_)*(F_CPU))+7999999ul)/8000000ul;
//
//#define DELAY_250NS _DELAY_VALUE(2)
//#define DELAY_500NS _DELAY_VALUE(4)
//#define DELAY_1US   _DELAY_VALUE(8)
//#define DELAY_2US   _DELAY_VALUE(16)
//#define DELAY_3US   _DELAY_VALUE(24)
//#define DELAY_5US   _DELAY_VALUE(40)
//#define DELAY_10US  _DELAY_VALUE(80)
//#define DELAY_20US  _DELAY_VALUE(160)

// easier type writing
typedef unsigned char  uint8;
typedef   signed char   int8;
typedef unsigned short uint16;
typedef   signed short  int16;
typedef unsigned long  uint32;
typedef   signed long   int32;

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

// access bits like variables
struct bits {
  uint8 b0:1;
  uint8 b1:1;
  uint8 b2:1;
  uint8 b3:1;
  uint8 b4:1;
  uint8 b5:1;
  uint8 b6:1;
  uint8 b7:1;
} __attribute__((__packed__));

// single bit port access
#define SBIT_(port,pin) ((*(volatile struct bits*)&port).b##pin)
#define	SBIT(x,y)	SBIT_(x,y)

// always inline function declaration
#define AIL(x)   static x __attribute__ ((always_inline)); static x

// never inline function declaration
#define NIL(x)   x __attribute__ ((noinline)); x

// volatile access (reject unwanted removing access):
#define VUINT8(x)   (*(volatile uint8 *)&(x))
#define  VINT8(x)   (*(volatile  int8 *)&(x))
#define VUINT16(x)  (*(volatile uint16*)&(x))
#define  VINT16(x)  (*(volatile  int16*)&(x))
#define VUINT32(x)  (*(volatile uint32*)&(x))
#define  VINT32(x)  (*(volatile  int32*)&(x))

// boolean constants
#define TRUE  -1
#define FALSE 0  


#endif //__COMMON_H__

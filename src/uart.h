#ifndef __UART_H__
#define __UART_H__


///////////////////////////////	Global Constants And Macros //////////////

// size of fifo buffer must be between 2 .. 256
#define UART_RX_FIFO_SIZE	64
#define UART_TX_FIFO_SIZE	64

#define UART_BAUDRATE(_baudrate_) ((((F_CPU)/16)/(_baudrate_)) - 1)

#define UART_ERROR_RX_OVRFLW (1<<0)
#define UART_ERROR_RX_FRAME  (1<<1)
#define UART_ERROR_TX_OVRFLW (1<<2)


#define CR  0x0D  //'\n'
#define LF	0x0A  //'\r'
#define XON	0x11


///////////////////////////////	Function Prototypes //////////////////////

void  uart_init(uint16 baudrate_div); // use macro UART_BAUDRATE

uint8 uart_get_error();   // retrieve last occured error flags and clear

void  uart_rx_flush();    // clear inbound fifo
uint8 uart_rx_rdy();	    // 0 = rx fifo empty
uint8 uart_rx();			    // get next received byte from fifo

void  uart_tx_flush();    // clear outbound fifo
uint8 uart_tx_rdy();	    // 0 = tx fifo empty
void  uart_tx(uint8 c);		// insert byte into fifo


///////////////////////////////	Resolve AVR naming chaos /////////////////

//-----------------------------	Interrupt vectors -----------------------/
#ifndef	UART_RX_vect
#if   defined USART_RX_vect
#define		UART_RX_vect	USART_RX_vect
#elif defined	USART_RXC_vect
#define		UART_RX_vect	USART_RXC_vect
#elif defined	USART0_RXC_vect
#define		UART_RX_vect	USART0_RXC_vect
#endif
#endif

#if !defined UART_UDRE_vect && defined USART_UDRE_vect
#define	UART_UDRE_vect USART_UDRE_vect
#endif

//-----------------------------	Register names --------------------------/
#ifndef	UCSR0A
#define	UCSR0A	UCSRA
#endif
#ifndef	UCSR0B
#define	UCSR0B	UCSRB
#endif
#ifndef	UCSR0C
#define	UCSR0C	UCSRC
#endif
#ifndef	UDR0
#define	UDR0	  UDR
#endif
#ifndef	UBRR0L
#define	UBRR0L	UBRRL
#endif
#ifndef	UBRR0H
#define	UBRR0H	UBRRH
#endif

//-----------------------------	Bit names -------------------------------/
#ifndef	UCSZ00
#define	UCSZ00	UCSZ0
#endif
#ifndef	UCSZ01
#define	UCSZ01	UCSZ1
#endif
#if !defined URSEL0 && defined URSEL
#define	URSEL0	URSEL
#endif
#ifndef	RXEN0
#define	RXEN0	  RXEN
#endif
#ifndef	TXEN0
#define	TXEN0	  TXEN
#endif
#ifndef	FE0
#define	FE0	    FE
#endif
#ifndef	DOR0
#define	DOR0	  DOR
#endif
#ifndef	UDRIE0
#define	UDRIE0	UDRIE
#endif
#ifndef	RXCIE0
#define	RXCIE0	RXCIE
#endif


#endif //__UART_H__

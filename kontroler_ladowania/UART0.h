//UART0.h : header file for the UART0
//

#ifndef _UART0_H_
#define _UART0_H_

/////////////////////////////////////////////////////////////////////////////
//UART0
#define TDR0 PORTB.6

#define RX_BUFFER_SIZE0 128


#if RX_BUFFER_SIZE0<256
extern unsigned char rx_counter0;
#else
extern unsigned int rx_counter0;
#endif

#pragma used+
void uart0_init(void);
char getchar0(void);
void putchar0(char c);
#pragma used-

#define TX_BUFFER_SIZE0 64


#endif


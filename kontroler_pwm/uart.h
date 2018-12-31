//UARTUART.h : header file for the UART UART0
//

#ifndef _UART_H_
#define _UART_H_

/////////////////////////////////////////////////////////////////////////////
//UARTUART

#define UART_BAUD 9600
#define RX_BUFFER_SIZE 250

#if RX_BUFFER_SIZE<256
extern unsigned char rx_counter;
#else
extern unsigned int rx_counter;
#endif

void uart_init(void);
char getchar(void);
void putchar(char c);

#endif

//UART1.h : header file for the UART1
//

#ifndef _UART1_H_
#define _UART1_H_

/////////////////////////////////////////////////////////////////////////////
//UART1

#define RX_BUFFER_SIZE1 255

#if RX_BUFFER_SIZE1<256
extern unsigned char rx_counter1;
#else
extern unsigned int rx_counter1;
#endif

#pragma used+
void uart1_init(void);
char getchar1(void);
void putchar1(char c);
#pragma used-

#endif

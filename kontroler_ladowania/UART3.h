//UARTUART3.h : header file for the UART UART3
//

#ifndef _UART3_H_
#define _UART3_H_

/////////////////////////////////////////////////////////////////////////////
//UARTUART3

#define RX_BUFFER_SIZE3 64

#if RX_BUFFER_SIZE3<256
extern unsigned char rx_counter3;
#else
extern unsigned int rx_counter3;
#endif


#pragma used+
void uart3_init(void);
char getchar3(void);
void putchar3(char c);
#pragma used-


#endif

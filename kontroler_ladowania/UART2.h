//UARTUART2.h : header file for the UART UART2
//

#ifndef _UART2_H_
#define _UART2_H_

/////////////////////////////////////////////////////////////////////////////
//UARTUART2

#define RX_BUFFER_SIZE2 64
#define TX_BUFFER_SIZE2 64

#if RX_BUFFER_SIZE2<256
extern unsigned char rx_counter2;
#else
extern unsigned int rx_counter2;
#endif

#if RX_BUFFER_SIZE2 < 256
extern unsigned char rx_counter2;
#else
extern unsigned int rx_counter2;
#endif

#pragma used+
void uart2_init(void);
char getchar2(void);
void putchar2(char c);
#pragma used-


#endif

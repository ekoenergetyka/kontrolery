//UARTUART.c : source file for the UART UART
//

#include <io.h>
#include "tools.h"
#include "uart.h"

/////////////////////////////////////////////////////////////////////////////
//UARTUART

#define RSDIR PORTC.0

#ifndef _IO_BITS_DEFINITIONS_
#define TXB8 0
#define RXB8 1
#define UPE 2
#define OVR 3
#define FE 4
#define UDRE 5
#define TXC  6
#define RXC 7
#endif

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define SHIFT_REGISTER_EMPTY (1 << TXC)
#define RX_COMPLETE (1<<RXC)

void uart_init(void)
{
 // USART initialization
 // Communication Parameters: 8 Data, 1 Stop, No Parity
 // Communication Parameters: 8 Data, 2 Stop, No Parity
 // USART Receiver: On
 // USART Transmitter: On
 // USART Mode: Asynchronous
 // USART Baud Rate: 9600
 UCSRA=0x00; UCSRB=0x98; UCSRC=0x86; // 1 stop
 UCSRA=0x00; UCSRB=0x98; UCSRC=0x8E; // 2 stop

 UBRRH = ((unsigned int)((( _MCU_CLOCK_FREQUENCY_ /(16.0 * UART_BAUD))-0.5))) >> 8;
 UBRRL =  (unsigned int)((( _MCU_CLOCK_FREQUENCY_ /(16.0 * UART_BAUD))-0.5));

 CLEAR(C, 0); OUTPUT(C, 0);
 RSDIR = 0;
}

// USART0 Receiver buffer
char rx_buffer[RX_BUFFER_SIZE];

#if RX_BUFFER_SIZE<256
unsigned char rx_wr_index,rx_rd_index,rx_counter;
#else
unsigned int rx_wr_index,rx_rd_index,rx_counter;
#endif

// This flag is set on USART Receiver buffer overflow
bit rx_buffer_overflow;

// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
char status,data;
status=UCSRA;
data=UDR;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   rx_buffer[rx_wr_index]=data;
   if (++rx_wr_index == RX_BUFFER_SIZE) rx_wr_index=0;
   if (++rx_counter == RX_BUFFER_SIZE)
      {
      rx_counter=0;
      rx_buffer_overflow=1;
      };
   };
}

#ifndef _DEBUG_TERMINAL_IO_
// Get a character from the USART0 Receiver buffer
#define _ALTERNATE_GETCHAR_
#pragma used+
char getchar(void)
{
char data;
while (rx_counter==0);
data=rx_buffer[rx_rd_index];
if (++rx_rd_index == RX_BUFFER_SIZE) rx_rd_index=0;
#asm("cli")
--rx_counter;
#asm("sei")
return data;
}
#pragma used-

#endif

#ifndef _DEBUG_TERMINAL_IO_
// Write a character to the USART0 Transmitter buffer
#define _ALTERNATE_PUTCHAR_
#pragma used+
void putchar(char c)
{
 while ((UCSRA & DATA_REGISTER_EMPTY) == 0);
 RSDIR = 1;
 UDR=c;

 while ((UCSRA & SHIFT_REGISTER_EMPTY) == 0);
 UCSRA |= SHIFT_REGISTER_EMPTY;
 RSDIR = 0;
}
#pragma used-
#endif


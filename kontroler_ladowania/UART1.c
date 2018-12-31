//UART1.c : source file for the UART1
//

#include <io.h>
#include "Tools.h"
#include "UART.h"
#include "UART1.h"

/////////////////////////////////////////////////////////////////////////////
//UART1

void uart1_init(void)
{
 UBRR1L = (unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART1_BAUD))-0.5));
 UBRR1H = ((unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART1_BAUD))-0.5))) >> 8;
 UCSR1A = 0x00;
 UCSR1B = 0x98;
 UCSR1C = 0x06;
}

// USART1 Receiver buffer
char rx_buffer1[RX_BUFFER_SIZE1];

#if RX_BUFFER_SIZE1<256
unsigned char rx_wr_index1,rx_rd_index1,rx_counter1;
#else
unsigned int rx_wr_index1,rx_rd_index1,rx_counter1;
#endif

// This flag is set on USART1 Receiver buffer overflow
bit rx_buffer_overflow1;

// USART1 Receiver interrupt service routine
interrupt [USART1_RXC] void usart1_rx_isr(void)
{
char status,data;
status=UCSR1A;
data=UDR1;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   rx_buffer1[rx_wr_index1]=data;
   if (++rx_wr_index1 == RX_BUFFER_SIZE1) rx_wr_index1=0;
   if (++rx_counter1 == RX_BUFFER_SIZE1)
      {
      rx_counter1=0;
      rx_buffer_overflow1=1;
      };
   };
}

// Get a character from the USART1 Receiver buffer
#pragma used+
char getchar1(void)
{
char data;
while (rx_counter1==0);
data=rx_buffer1[rx_rd_index1];
if (++rx_rd_index1 == RX_BUFFER_SIZE1) rx_rd_index1=0;
#asm("cli")
--rx_counter1;
#asm("sei")
return data;
}
#pragma used-
// Write a character to the USART1 Transmitter
#pragma used+
void putchar1(char c)
{
 while ((UCSR1A & DATA_REGISTER_EMPTY)==0);
 UDR1=c;
}
#pragma used-

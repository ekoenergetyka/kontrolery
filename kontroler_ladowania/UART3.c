//UARTUART1.c : source file for the UART UART1
//

#include <io.h>
#include "UART.h"
#include "UART3.h"

/////////////////////////////////////////////////////////////////////////////
//UARTUART1

void uart3_init(void)
{
 // Character Size: 8-bit
 // Mode: Asynchronous
 // Parity: Disabled
 // Stop Bit: 1-bit
 UBRR3L = (unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART3_BAUD))-0.5));
 UBRR3H = ((unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART3_BAUD))-0.5))) >> 8;
 UCSR3A = 0x00;
 UCSR3C = 0x06;
 UCSR3B = 0x98;
}

// USART3 Receiver buffer
char rx_buffer3[RX_BUFFER_SIZE3];

#if RX_BUFFER_SIZE3<256
unsigned char rx_wr_index3,rx_rd_index3,rx_counter3;
#else
unsigned int rx_wr_index3,rx_rd_index3,rx_counter3;
#endif

// This flag is set on USART2 Receiver buffer overflow
bit rx_buffer_overflow3;


// USART3 Receiver buffer
#if RX_BUFFER_SIZE3<256
unsigned char rx_wr_index3,rx_rd_index3,rx_counter3;
#else
unsigned int rx_wr_index3,rx_rd_index3,rx_counter3;
#endif

// This flag is set on USART3 Receiver buffer overflow
bit rx_buffer_overflow3;

// USART3 Receiver interrupt service routine
interrupt [USART3_RXC] void usart3_rx_isr(void)
{
char status,data;
status=UCSR3A;
data=UDR3;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   rx_buffer3[rx_wr_index3]=data;
   if (++rx_wr_index3 == RX_BUFFER_SIZE3) rx_wr_index3=0;
   if (++rx_counter3 == RX_BUFFER_SIZE3)
      {
      rx_counter3=0;
      rx_buffer_overflow3=1;
      };
   };
}

// Get a character from the USART3 Receiver buffer
#pragma used+
char getchar3(void)
{
char data;
while (rx_counter3==0);
data=rx_buffer3[rx_rd_index3];
if (++rx_rd_index3 == RX_BUFFER_SIZE3) rx_rd_index3=0;
#asm("cli")
--rx_counter3;
#asm("sei")
return data;
}
#pragma used-

// Write a character to the USART3 Transmitter
#pragma used+
void putchar3(char c)
{
 while ((UCSR3A & DATA_REGISTER_EMPTY)==0);
 UDR3=c;
}
#pragma used-


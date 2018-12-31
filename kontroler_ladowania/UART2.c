//UARTUART2.c : source file for the UART UART2
//

#include <io.h>
#include "UART.h"
#include "UART2.h"

/////////////////////////////////////////////////////////////////////////////
//UARTUART2

void uart2_init(void)
{
 // USART2 initialization
 // Communication Parameters: 8 Data, 1 Stop, No Parity
 // USART2 Receiver: On
 // USART2 Transmitter: On
 // USART2 Mode: Asynchronous
 // USART2 Baud Rate: 115200
 UCSR2A=(0<<RXC2) | (0<<TXC2) | (0<<UDRE2) | (0<<FE2) | (0<<DOR2) | (0<<UPE2) | (0<<U2X2) | (0<<MPCM2);
 UCSR2B=(1<<RXCIE2) | (0<<TXCIE2) | (0<<UDRIE2) | (1<<RXEN2) | (1<<TXEN2) | (0<<UCSZ22) | (0<<RXB82) | (0<<TXB82);
 UCSR2C=(0<<UMSEL21) | (0<<UMSEL20) | (0<<UPM21) | (0<<UPM20) | (0<<USBS2) | (1<<UCSZ21) | (1<<UCSZ20) | (0<<UCPOL2);
 UBRR2H = ((unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART2_BAUD))-0.5))) >> 8;
 UBRR2L = (unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART2_BAUD))-0.5));
}

// USART2 Receiver buffer
char rx_buffer2[RX_BUFFER_SIZE2];

#if RX_BUFFER_SIZE2<256
unsigned char rx_wr_index2,rx_rd_index2,rx_counter2;
#else
unsigned int rx_wr_index2,rx_rd_index2,rx_counter2;
#endif

// This flag is set on USART2 Receiver buffer overflow
bit rx_buffer_overflow2;

// USART2 Receiver interrupt service routine
interrupt [USART2_RXC] void usart2_rx_isr(void)
{
char status,data;
status=UCSR2A;
data=UDR2;
if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
   {
   rx_buffer2[rx_wr_index2]=data;
   if (++rx_wr_index2 == RX_BUFFER_SIZE2) rx_wr_index2=0;
   if (++rx_counter2 == RX_BUFFER_SIZE2)
      {
      rx_counter2=0;
      rx_buffer_overflow2=1;
      };
   };
}

// Get a character from the USART2 Receiver buffer
#pragma used+
char getchar2(void)
{
char data;
while (rx_counter2==0);
data=rx_buffer2[rx_rd_index2];
if (++rx_rd_index2 == RX_BUFFER_SIZE2) rx_rd_index2=0;
#asm("cli")
--rx_counter2;
#asm("sei")
return data;
}
#pragma used-

// Write a character to the USART2 Transmitter
#pragma used+
void putchar2(char c)
{
 while ((UCSR2A & DATA_REGISTER_EMPTY)==0);
 UDR2=c;
}
#pragma used-

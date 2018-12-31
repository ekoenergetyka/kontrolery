//UART0.c : source file for the UART0
//

#include <io.h>
#include "Tools.h"
#include "UART.h"
#include "UART0.h"

/////////////////////////////////////////////////////////////////////////////
//UART0

void uart0_init(void)
{
 // USART0 initialization
 // Communication Parameters: 8 Data, 1 Stop, No Parity
 // USART0 Receiver: On
 // USART0 Transmitter: On
 // USART0 Mode: Asynchronous
 // USART0 Baud Rate: 9600
 UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
 UCSR0B=(1<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (1<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
 UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
 UBRR0H = ((unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART0_BAUD))-0.5))) >> 8;
 UBRR0L = (unsigned int)(((_MCU_CLOCK_FREQUENCY_/(16.0 * UART0_BAUD))-0.5));
 
 OUTPUT(B, 6);
 TDR0 = 0;
}

// USART0 Receiver buffer
char rx_buffer0[RX_BUFFER_SIZE0];

#if RX_BUFFER_SIZE0<256
unsigned char rx_wr_index0,rx_rd_index0,rx_counter0;
#else
unsigned int rx_wr_index0,rx_rd_index0,rx_counter0;
#endif

// This flag is set on USART0 Receiver buffer overflow
bit rx_buffer_overflow0;

// USART0 Receiver interrupt service routine
interrupt [USART0_RXC] void usart0_rx_isr(void)
{
 char status,data;
 
 status=UCSR0A;
 data=UDR0;
 if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
 {
  rx_buffer0[rx_wr_index0]=data;
  if (++rx_wr_index0 == RX_BUFFER_SIZE0) rx_wr_index0=0;
  if (++rx_counter0 == RX_BUFFER_SIZE0)
  {
   rx_counter0=0;
   rx_buffer_overflow0=1;
  }
 } 
}

// Get a character from the USART0 Receiver buffer
#pragma used+
char getchar0(void)
{
char data;
while (rx_counter0==0);
data=rx_buffer0[rx_rd_index0];
if (++rx_rd_index0 == RX_BUFFER_SIZE0) rx_rd_index0=0;
#asm("cli")
--rx_counter0;
#asm("sei")
return data;
}
#pragma used-

#pragma used+
void putchar0(char c)
{
 while ((UCSR0A & DATA_REGISTER_EMPTY) == 0);
 TDR0 = 1;  
 
 UDR0 = c;

 while ((UCSR0A & SHIFT_REGISTER_EMPTY) == 0);
 UCSR0A |= SHIFT_REGISTER_EMPTY;
 TDR0 = 0;
}
#pragma used-


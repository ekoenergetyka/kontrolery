#include <io.h>
#include <delay.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

// I2C Bus functions
#asm
   .equ __i2c_port=0x0B ;PORTD
   .equ __sda_bit=1
   .equ __scl_bit=0
#endasm
#include <i2c.h>

#include "IOsys.h"
#include "Tools.h"
#include "Uart.h"
#include "Uart0.h"
#include "Uart1.h"
#include "Uart2.h"
#include "Uart3.h"

// Bit Write Protect pamiêci EEPROM
#define EEP_ADDR  0xAE
#define EEPROM_BUS_ADDRESS EEP_ADDR

unsigned int relays;
unsigned char le0, le1, le2, le3, le4, le5;

void sys_init()
{
// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=0x80;
CLKPR=0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port A initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTA=0x00;
DDRA=0x00;

// Port B initialization
// Func7=Out Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=0 State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTB=0x00;
DDRB=0x80;

 // Port B initialization
 // Func7=Out Func6=In Func5=In Func4=In Func3=In Func2=Out Func1=Out Func0=Out
 // State7=0 State6=T State5=T State4=T State3=T State2=0 State1=0 State0=0
 PORTB=0x00;
 DDRB=0x87;

// Port C initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTC=0x00;
DDRC=0x00;

// Port D initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTD=0x00;
DDRD=0x00;

// Port E initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTE=0x00;
DDRE=0x00;

// Port F initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTF=0x00;
DDRF=0x00;

// Port G initialization
// Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State5=T State4=T State3=T State2=T State1=T State0=T
PORTG=0x00;
DDRG=0x00;

// Port H initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTH=0x00;
DDRH=0x00;

// Port J initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTJ=0x00;
DDRJ=0x00;

// Port K initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTK=0x00;
DDRK=0x00;

// Port L initialization
// Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
// State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
PORTL=0x00;
DDRL=0x00;


// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer 1 Stopped
// Mode: Normal top=FFFFh
// OC1A output: Discon.
// OC1B output: Discon.
// OC1C output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR1A=0x00;
TCCR1B=0x00;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;
OCR1CH=0x00;
OCR1CL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer 2 Stopped
// Mode: Normal top=FFh
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=0x00;
TCCR2A=0x00;
TCCR2B=0x00;
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// Timer/Counter 3 initialization
// Clock source: System Clock
// Clock value: Timer 3 Stopped
// Mode: Normal top=FFFFh
// OC3A output: Discon.
// OC3B output: Discon.
// OC3C output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 3 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR3A=0x00;
TCCR3B=0x00;
TCNT3H=0x00;
TCNT3L=0x00;
ICR3H=0x00;
ICR3L=0x00;
OCR3AH=0x00;
OCR3AL=0x00;
OCR3BH=0x00;
OCR3BL=0x00;
OCR3CH=0x00;
OCR3CL=0x00;

// Timer/Counter 4 initialization
// Clock source: System Clock
// Clock value: Timer 4 Stopped
// Mode: Normal top=FFFFh
// OC4A output: Discon.
// OC4B output: Discon.
// OC4C output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 4 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR4A=0x00;
TCCR4B=0x00;
TCNT4H=0x00;
TCNT4L=0x00;
ICR4H=0x00;
ICR4L=0x00;
OCR4AH=0x00;
OCR4AL=0x00;
OCR4BH=0x00;
OCR4BL=0x00;
OCR4CH=0x00;
OCR4CL=0x00;

// Timer/Counter 5 initialization
// Clock source: System Clock
// Clock value: Timer 5 Stopped
// Mode: Normal top=FFFFh
// OC5A output: Discon.
// OC5B output: Discon.
// OC5C output: Discon.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer 5 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR5A=0x00;
TCCR5B=0x00;
TCNT5H=0x00;
TCNT5L=0x00;
ICR5H=0x00;
ICR5L=0x00;
OCR5AH=0x00;
OCR5AL=0x00;
OCR5BH=0x00;
OCR5BL=0x00;
OCR5CH=0x00;
OCR5CL=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
// INT3: Off
// INT4: Off
// INT5: Off
// INT6: Off
// INT7: Off
EICRA=0x00;
EICRB=0x00;
EIMSK=0x00;
// PCINT0 interrupt: Off
// PCINT1 interrupt: Off
// PCINT2 interrupt: Off
// PCINT3 interrupt: Off
// PCINT4 interrupt: Off
// PCINT5 interrupt: Off
// PCINT6 interrupt: Off
// PCINT7 interrupt: Off
// PCINT8 interrupt: Off
// PCINT9 interrupt: Off
// PCINT10 interrupt: Off
// PCINT11 interrupt: Off
// PCINT12 interrupt: Off
// PCINT13 interrupt: Off
// PCINT14 interrupt: Off
// PCINT15 interrupt: Off
// PCINT16 interrupt: Off
// PCINT17 interrupt: Off
// PCINT18 interrupt: Off
// PCINT19 interrupt: Off
// PCINT20 interrupt: Off
// PCINT21 interrupt: Off
// PCINT22 interrupt: Off
// PCINT23 interrupt: Off
PCMSK0=0x00;
PCMSK1=0x00;
PCMSK2=0x00;
PCICR=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=0x00;
// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=0x00;
// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=0x00;
// Timer/Counter 3 Interrupt(s) initialization
TIMSK3=0x00;
// Timer/Counter 4 Interrupt(s) initialization
TIMSK4=0x00;
// Timer/Counter 5 Interrupt(s) initialization
TIMSK5=0x00;

 // Analog Comparator initialization
 // Analog Comparator: Off
 // Analog Comparator Input Capture by Timer/Counter 1: Off
 ACSR=0x80;
 ADCSRB=0x00;

 // SPI initialization
 // SPI Type: Master
 // SPI Clock Rate: 691.200 kHz
 // SPI Clock Phase: Cycle Start
 // SPI Clock Polarity: Low
 // SPI Data Order: MSB First
 SPCR=(0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (1<<SPR0);
 SPSR=(0<<SPI2X);

 // SPI initialization
 // SPI Type: Master
 // SPI Clock Rate: 2764,800 kHz
 // SPI Clock Phase: Cycle Start
 // SPI Clock Polarity: Low
 // SPI Data Order: MSB First
 SPCR=0x50;
 SPSR=0x00;


 // Watchdog Timer initialization
 // Watchdog Timer Prescaler: OSC/512k
 // Watchdog Timer interrupt: Off
#pragma optsize-
#asm("wdr")
 WDTCSR=0x38;
 WDTCSR=0x28;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif


 // I2C Bus initialization
 OUTPUT(F,6); SET(F,6);
 WR_DISSABLE;
 i2c_init();

 // Init CAN PORTS
// OUTPUT(B,6); CLEAR(B,6); // TDR0 RS485
// OUTPUT(B,4); SET(B,4);   // EXT1 as CAN CS
// OUTPUT(F,5); CLEAR(F,5); // EXT3 as LED LE

 // Init CAN PORTS
 OUTPUT(B,6); CLEAR(B,6); // TDR0 RS485
 OUTPUT(B,4); SET(B,4);   // EXT1 as CAN CS
 OUTPUT(G,5); CLEAR(G,5); // EXT3 as LED LE
}

#define PUD1PORT F
#define PUD1PIN  7

#define PUD2PORT G
#define PUD2PIN  5

#define PUD3PORT C
#define PUD3PIN  3

#define PUD4PORT L
#define PUD4PIN  0

void buzzer(bool on)
{
 OUTPUT(D,7);
 if (on) SET(D,7); else CLEAR(D,7);
}

void input_init()
{
// OUTPUT(F,PUD1PIN); SET(F, PUD1PIN);
// OUTPUT(G,PUD2PIN); SET(G, PUD2PIN);
// OUTPUT(C,PUD3PIN); SET(C, PUD3PIN);
// OUTPUT(L,PUD4PIN); SET(L, PUD4PIN);
 // Port C initialization

 // Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
 DDRC=(0<<DDC7) | (0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
 // State: Bit7=P Bit6=P Bit5=P Bit4=P Bit3=P Bit2=P Bit1=P Bit0=P
 PORTC=(1<<PORTC7) | (1<<PORTC6) | (1<<PORTC5) | (1<<PORTC4) | (1<<PORTC3) | (1<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

 // Port K initialization
 // Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
 DDRK=(0<<DDK7) | (0<<DDK6) | (0<<DDK5) | (0<<DDK4) | (0<<DDK3) | (0<<DDK2) | (0<<DDK1) | (0<<DDK0);
 // State: Bit7=P Bit6=P Bit5=P Bit4=P Bit3=P Bit2=P Bit1=P Bit0=P
 PORTK=(1<<PORTK7) | (1<<PORTK6) | (1<<PORTK5) | (1<<PORTK4) | (1<<PORTK3) | (1<<PORTK2) | (1<<PORTK1) | (1<<PORTK0);

 // Port L initialization
 // Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
 DDRL=(0<<DDL7) | (0<<DDL6) | (0<<DDL5) | (0<<DDL4) | (0<<DDL3) | (0<<DDL2) | (0<<DDL1) | (0<<DDL0);
 // State: Bit7=P Bit6=P Bit5=P Bit4=P Bit3=P Bit2=P Bit1=P Bit0=P
 PORTL=(1<<PORTL7) | (1<<PORTL6) | (1<<PORTL5) | (1<<PORTL4) | (1<<PORTL3) | (1<<PORTL2) | (1<<PORTL1) | (1<<PORTL0);
}

void output_init()
{
 OUTPUT(J,3); CLEAR(J,3);  // R1
 OUTPUT(J,4); CLEAR(J,4);  // R2
 OUTPUT(J,5); CLEAR(J,5);  // R3
 OUTPUT(J,6); CLEAR(J,6);  // R4
 OUTPUT(G,2); CLEAR(G,2);  // R5
 OUTPUT(A,7); CLEAR(A,7);  // R6
 OUTPUT(A,6); CLEAR(A,6);  // R7
 OUTPUT(A,5); CLEAR(A,5);  // R8
 OUTPUT(A,4); CLEAR(A,4);  // R9
 OUTPUT(A,3); CLEAR(A,3);  // R10
 OUTPUT(A,2); CLEAR(A,2);  // R11
 OUTPUT(A,1); CLEAR(A,1);  // R12
 OUTPUT(A,0); CLEAR(A,0);  // R13
 OUTPUT(J,7); CLEAR(J,7);  // R14
}

void set_output(char output, char state)
{
 switch (output)
 {
  case  R1: if (state) { SET(J,3); LR1_ON;  relays |= 0x0001; } else {CLEAR(J,3); LR1_OF;  relays &= ~0x0001; } break;
  case  R2: if (state) { SET(J,4); LR2_ON;  relays |= 0x0002; } else {CLEAR(J,4); LR2_OF;  relays &= ~0x0002; } break;
  case  R3: if (state) { SET(J,5); LR3_ON;  relays |= 0x0004; } else {CLEAR(J,5); LR3_OF;  relays &= ~0x0004; } break;
  case  R4: if (state) { SET(J,6); LR4_ON;  relays |= 0x0008; } else {CLEAR(J,6); LR4_OF;  relays &= ~0x0008; } break;
  case  R5: if (state) { SET(G,2); LR5_ON;  relays |= 0x0010; } else {CLEAR(G,2); LR5_OF;  relays &= ~0x0010; } break;
  case  R6: if (state) { SET(A,7); LR6_ON;  relays |= 0x0020; } else {CLEAR(A,7); LR6_OF;  relays &= ~0x0020; } break;
  case  R7: if (state) { SET(A,6); LR7_ON;  relays |= 0x0040; } else {CLEAR(A,6); LR7_OF;  relays &= ~0x0040; } break;
  case  R8: if (state) { SET(A,5); LR8_ON;  relays |= 0x0080; } else {CLEAR(A,5); LR8_OF;  relays &= ~0x0080; } break;
  case  R9: if (state) { SET(A,4); LR9_ON;  relays |= 0x0100; } else {CLEAR(A,4); LR9_OF;  relays &= ~0x0100; } break;
  case R10: if (state) { SET(A,3); LR10_ON; relays |= 0x0200; } else {CLEAR(A,3); LR10_OF; relays &= ~0x0200; } break;
  case R11: if (state) { SET(A,2); LR11_ON; relays |= 0x0400; } else {CLEAR(A,2); LR11_OF; relays &= ~0x0400; } break;
  case R12: if (state) { SET(A,1); LR12_ON; relays |= 0x0800; } else {CLEAR(A,1); LR12_OF; relays &= ~0x0800; } break;
  case R13: if (state) { SET(A,0); LR13_ON; relays |= 0x1000; } else {CLEAR(A,0); LR13_OF; relays &= ~0x1000; } break;
  case R14: if (state) { SET(J,7); LR14_ON; relays |= 0x2000; } else {CLEAR(J,7); LR14_OF; relays &= ~0x2000; } break;
 }
}

bool get_output(char output)
{
 unsigned int mask = 1;

 mask <<= output;

 return relays & mask;
}


void output(char on_off, ...)
{
 va_list argptr;
 int i;

 /* initialize argptr */
 va_start(argptr, on_off);

 /* add all the function arguments after nsum */
 while ((i = va_arg(argptr,int)) != EOF)
   set_output(i, on_off);

 /* terminate the use of argptr */
 va_end(argptr);
}

void output_on(char n_on, ...)
{
 va_list argptr;
 int i;

 /* initialize argptr */
 va_start(argptr, n_on);

 /* add all the function arguments after nsum */
 for (i=1; i <= n_on; i++) set_output(va_arg(argptr,int), 1);

 /* terminate the use of argptr */
 va_end(argptr);
}

void output_off(char n_off, ...)
{
 va_list argptr;
 int i;

 /* initialize argptr */
 va_start(argptr, n_off);

 /* add all the function arguments after nsum */
 for (i=1; i <= n_off; i++) set_output(va_arg(argptr,int), 0);

 /* terminate the use of argptr */
 va_end(argptr);
}

/* read a data from the EEPROM */
void eeprom_read_stream(unsigned int address, char *src, unsigned char n)
{
 i2c_start();
 i2c_write(EEPROM_BUS_ADDRESS);
 i2c_write(address >> 8);
 i2c_write(address);
 i2c_start();
 i2c_write(EEPROM_BUS_ADDRESS | 1);

 if (n > 0) n--;

 while(n--)
 {
  *src = i2c_read(1);
  src++;
 }
 *src = i2c_read(0);

 i2c_stop();
}


/* write a data to the EEPROM */
void eeprom_write_stream(unsigned int address, char *src, unsigned char n)
{
 i2c_start();
 i2c_write(EEPROM_BUS_ADDRESS);
 i2c_write(address >> 8);
 i2c_write(address);

 while (n--)
 {
  i2c_write(*src); // printf("%02x ", *src);
  src++;
 }
 i2c_stop();

 /* 5ms delay to complete the write operation */
 delay_ms(7);
}

#pragma used+


char poutput = COM2;
void putchar(char c)
{
 switch (poutput)
 {
  case COM0: putchar0(c); break;  // RS485
  case COM1: putchar1(c); break;
  case COM2: putchar2(c); break;
  case COM3: putchar3(c); break;  // internal socket IDC10

  default: break;
 }
}

void cprintf(char stream, char flash *fmt, ...)
{
 char tmp;
 va_list argptr;

 if (stream != NULL)
 {
  tmp = poutput;
  poutput = stream;

  va_start(argptr, fmt);
  vprintf(fmt, argptr);
  va_end(argptr);

  poutput = tmp;
 }
}

#pragma used-


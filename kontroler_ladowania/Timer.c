#include <io.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include "timer.h"
#include "iosys.h"
#include "Tools.h"

unsigned char short_ticks = 0;
unsigned long timer_ticks = 0;

unsigned char inputs1 = 0;
unsigned char inputs2 = 0;
unsigned char inputs3 = 0;
unsigned int  bmask   = 0;

unsigned char TXC_shot = 0;
unsigned char RXC_shot = 0;
unsigned char TX2_shot = 0;
unsigned char RX2_shot = 0;
unsigned char TX1_shot = 0;
unsigned char RX1_shot = 0;
unsigned char TX0_shot = 0;
unsigned char RX0_shot = 0;
unsigned char E1_shot  = 0;
unsigned char E2_shot  = 0;

//unsigned char cntR1  = 0;
//unsigned char cntR2  = 0;
//unsigned char cntR3  = 0;
//unsigned char cntR4  = 0;
//unsigned char cntR5  = 0;
//unsigned char cntR6  = 0;
//unsigned char cntR7  = 0;
//unsigned char cntR8  = 0;
//unsigned char cntR9  = 0;
//unsigned char cntR10 = 0;
//unsigned char cntR11 = 0;
//unsigned char cntR12 = 0;
//unsigned char cntR13 = 0;
//unsigned char cntR14 = 0;

#define VC_DEC_OR_SET(high, low, mask) low = ~(low & mask);  high = low ^ (high & mask)
#define V_COUNTER(high, low, delta) high = (high ^ low) & delta; low = ~low & delta

// Timer2 overflow interrupt service routine
interrupt [TIM2_OVF] void timer2_ovf_isr(void)
{
 static unsigned char counter;

 static unsigned char i_state1, i_state2, i_state3;
 static unsigned char cntl1, cnth1, cntl2, cnth2, cntl3, cnth3;
 unsigned char delta;

 // Reinitialize Timer2 value
// TCNT2=0x94;  // 11.0592
 TCNT2=0x70;  // 14.7456

 // Place your code here
 short_ticks++;
 timer_ticks++;

 if (TXC_shot) { TXC_shot--;  if (TXC_shot == 0)  LTXC_OF; }
 if (RXC_shot) { RXC_shot--;  if (RXC_shot == 0)  LRXC_OF; }
 if (TX2_shot) { TX2_shot--;  if (TX2_shot == 0)  LTX2_OF; }
 if (RX2_shot) { RX2_shot--;  if (RX2_shot == 0)  LRX2_OF; }
 if (TX1_shot) { TX1_shot--;  if (TX1_shot == 0)  LTX1_OF; }
 if (RX1_shot) { RX1_shot--;  if (RX1_shot == 0)  LRX1_OF; }
 if (TX0_shot) { TX0_shot--;  if (TX0_shot == 0)  LTX0_OF; }
 if (RX0_shot) { RX0_shot--;  if (RX0_shot == 0)  LRX0_OF; }

 if (E1_shot)  { E1_shot--;   if (E1_shot == 0)   LE1_OF; }
 if (E2_shot)  { E2_shot--;   if (E2_shot == 0)   LE2_OF; }

 if (++counter > TPS / 3)
 {
  counter = 0;

  if (bmask & (1<<R1))  if (get_output(R1))  set_output(R1, 0);  else set_output(R1, 1);
  if (bmask & (1<<R2))  if (get_output(R2))  set_output(R2, 0);  else set_output(R2, 1);
  if (bmask & (1<<R3))  if (get_output(R3))  set_output(R3, 0);  else set_output(R3, 1);
  if (bmask & (1<<R4))  if (get_output(R4))  set_output(R4, 0);  else set_output(R4, 1);
  if (bmask & (1<<R5))  if (get_output(R5))  set_output(R5, 0);  else set_output(R5, 1);
  if (bmask & (1<<R6))  if (get_output(R6))  set_output(R6, 0);  else set_output(R6, 1);
  if (bmask & (1<<R7))  if (get_output(R7))  set_output(R7, 0);  else set_output(R7, 1);
  if (bmask & (1<<R8))  if (get_output(R8))  set_output(R8, 0);  else set_output(R8, 1);
  if (bmask & (1<<R9))  if (get_output(R9))  set_output(R9, 0);  else set_output(R9, 1);
  if (bmask & (1<<R10)) if (get_output(R10)) set_output(R10, 0); else set_output(R10, 1);
  if (bmask & (1<<R11)) if (get_output(R11)) set_output(R11, 0); else set_output(R11, 1);
  if (bmask & (1<<R12)) if (get_output(R12)) set_output(R12, 0); else set_output(R12, 1);
  if (bmask & (1<<R13)) if (get_output(R13)) set_output(R13, 0); else set_output(R13, 1);
  if (bmask & (1<<R14)) if (get_output(R14)) set_output(R14, 0); else set_output(R14, 1);
 }

// if (cntR1)  { cntR1--;   if (cntR1 == 0)   set_output(R1, 0);  }
// if (cntR2)  { cntR2--;   if (cntR2 == 0)   set_output(R2, 0);  }
// if (cntR3)  { cntR3--;   if (cntR3 == 0)   set_output(R3, 0);  }
// if (cntR4)  { cntR4--;   if (cntR4 == 0)   set_output(R4, 0);  }
// if (cntR5)  { cntR5--;   if (cntR5 == 0)   set_output(R5, 0);  }
// if (cntR6)  { cntR6--;   if (cntR6 == 0)   set_output(R6, 0);  }
// if (cntR7)  { cntR7--;   if (cntR7 == 0)   set_output(R7, 0);  }
// if (cntR8)  { cntR8--;   if (cntR8 == 0)   set_output(R8, 0);  }
// if (cntR9)  { cntR9--;   if (cntR9 == 0)   set_output(R9, 0);  }
// if (cntR10) { cntR10--;  if (cntR10 == 0)  set_output(R10, 0); }
// if (cntR11) { cntR11--;  if (cntR11 == 0)  set_output(R11, 0); }
// if (cntR12) { cntR12--;  if (cntR12 == 0)  set_output(R12, 0); }
// if (cntR13) { cntR13--;  if (cntR13 == 0)  set_output(R13, 0); }
// if (cntR14) { cntR14--;  if (cntR14 == 0)  set_output(R14, 0); }

 delta = ~PINK; delta ^= i_state1;  //VC_DEC_OR_SET(cnth1, cntl1, delta);
// cnth1 = (cnth1 ^ cntl1) & delta1; cntl1 = ~cntl1 & delta1; i_state1 ^= (cntl1 & cnth1); inputs1 = i_state1;
 V_COUNTER(cnth1, cntl1, delta); i_state1 ^= (cntl1 & cnth1); inputs1 = i_state1;

 delta = ~PINC; delta ^= i_state2;  //VC_DEC_OR_SET(cnth2, cntl2, delta);
// cnth2 = (cnth2 ^ cntl2) & delta2; cntl2 = ~cntl2 & delta2; i_state2 ^= (cntl2 & cnth2); inputs2 = i_state2;
 V_COUNTER(cnth2, cntl2, delta); i_state2 ^= (cntl2 & cnth2); inputs2 = i_state2;

 delta = ~PINL; delta ^= i_state3;  //VC_DEC_OR_SET(cnth3, cntl3, delta);
// cnth3 = (cnth3 ^ cntl3) & delta3; cntl3 = ~cntl3 & delta3; i_state3 ^= (cntl3 & cnth3); inputs3 = i_state3;
 V_COUNTER(cnth3, cntl3, delta); i_state3 ^= (cntl3 & cnth3); inputs3 = i_state3;

 if (inputs1 & BI1) LW1_ON; else LW1_OF;
 if (inputs1 & BI2) LW2_ON; else LW2_OF;
 if (inputs1 & BI3) LW3_ON; else LW3_OF;
 if (inputs1 & BI4) LW4_ON; else LW4_OF;
 if (inputs1 & BI5) LW5_ON; else LW5_OF;
 if (inputs1 & BI6) LW6_ON; else LW6_OF;
 if (inputs1 & BI7) LW7_ON; else LW7_OF;
 if (inputs1 & BI8) LW8_ON; else LW8_OF;

 if (inputs2 & BI1) LW9_ON;  else LW9_OF;
 if (inputs2 & BI2) LW10_ON; else LW10_OF;
 if (inputs2 & BI3) LW11_ON; else LW11_OF;
 if (inputs2 & BI4) LW12_ON; else LW12_OF;
 if (inputs2 & BI5) LW13_ON; else LW13_OF;
 if (inputs2 & BI6) LW14_ON; else LW14_OF;
 if (inputs2 & BI7) LW15_ON; else LW15_OF;
 if (inputs2 & BI8) LW16_ON; else LW16_OF;

 if (inputs3 & BI1) LW17_ON; else LW17_OF;
 if (inputs3 & BI2) LW18_ON; else LW18_OF;
 if (inputs3 & BI3) LW19_ON; else LW19_OF;
 if (inputs3 & BI4) LW20_ON; else LW20_OF;
 if (inputs3 & BI5) LW21_ON; else LW21_OF;
 if (inputs3 & BI6) LW22_ON; else LW22_OF;
 if (inputs3 & BI7) LW23_ON; else LW23_OF;
 if (inputs3 & BI8) LW24_ON; else LW24_OF;
}

unsigned long sys_timer_count (void)
{
 register unsigned long t;

 #asm("cli")
 t = timer_ticks;
 #asm("sei")

 return (t);
}

unsigned long sys_timer_elapsed_count (unsigned long count)
{
 return (sys_timer_count() - count);
}

void sys_timer_wait(unsigned long count)
{
 unsigned long timer_period = sys_timer_count();

 while (sys_timer_elapsed_count(timer_period) < count)
 {
  #asm("wdr");
 }
}

void sys_timer_init()
{
// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 14.400 kHz
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// OC1C output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer Period: 4.5511 s
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
// Compare C Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<COM1C1) | (0<<COM1C0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
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
// Clock value: 14.400 kHz
// Mode: Normal top=0xFF
// OC2A output: Disconnected
// OC2B output: Disconnected
// Timer Period: 10 ms
ASSR=(0<<EXCLK) | (0<<AS2);
TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
TCCR2B=(0<<WGM22) | (1<<CS22) | (1<<CS21) | (1<<CS20);
TCNT2=0x70;
OCR2A=0x00;
OCR2B=0x00;

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=(0<<ICIE1) | (0<<OCIE1C) | (0<<OCIE1B) | (0<<OCIE1A) | (0<<TOIE1);

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (1<<TOIE2);
}

void set_blinks(char output, char state)
{
 unsigned int mask = 1;

 mask <<= output;

 if (state) bmask |= mask; else bmask &= ~mask;
}


void blinks(char on_off, ...)
{
 va_list argptr;
 int i;

 /* initialize argptr */
 va_start(argptr, on_off);

 /* add all the function arguments after nsum */
 while ((i = va_arg(argptr,int)) != EOF)
   set_blinks(i, on_off);

 /* terminate the use of argptr */
 va_end(argptr);
}


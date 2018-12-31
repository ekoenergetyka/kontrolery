#ifndef _IOSYS_H_
#define _IOSYS_H_

#define ON  1
#define OFF 0

#define R1  0
#define R2  1
#define R3  2
#define R4  3
#define R5  4
#define R6  5
#define R7  6
#define R8  7
#define R9  8
#define R10 9
#define R11 10
#define R12 11
#define R13 12
#define R14 13

#define BI1  (1 << 7)
#define BI2  (1 << 6)
#define BI3  (1 << 5)
#define BI4  (1 << 4)
#define BI5  (1 << 3)
#define BI6  (1 << 2)
#define BI7  (1 << 1)
#define BI8  (1 << 0)

#define I1  (inputs1 & BI1)
#define I2  (inputs1 & BI2)
#define I3  (inputs1 & BI3)
#define I4  (inputs1 & BI4)
#define I5  (inputs1 & BI5)
#define I6  (inputs1 & BI6)
#define I7  (inputs1 & BI7)
#define I8  (inputs1 & BI8)

#define I9  (inputs2 & BI1)
#define I10 (inputs2 & BI2)
#define I11 (inputs2 & BI3)
#define I12 (inputs2 & BI4)
#define I13 (inputs2 & BI5)
#define I14 (inputs2 & BI6)
#define I15 (inputs2 & BI7)
#define I16 (inputs2 & BI8)

#define I17 (inputs3 & BI1)
#define I18 (inputs3 & BI2)
#define I19 (inputs3 & BI3)
#define I20 (inputs3 & BI4)
#define I21 (inputs3 & BI5)
#define I22 (inputs3 & BI6)
#define I23 (inputs3 & BI7)
#define I24 (inputs3 & BI8)

#define LE1_ON le4 |=  (1<<0)
#define LE1_OF le4 &= ~(1<<0)
#define LE2_ON le4 |=  (1<<1)
#define LE2_OF le4 &= ~(1<<1)

#define LTXC_ON le1 |=  (1<<7)
#define LTXC_OF le1 &= ~(1<<7)
#define LRXC_ON le1 |=  (1<<6)
#define LRXC_OF le1 &= ~(1<<6)

#define LTX2_ON le1 |=  (1<<5)
#define LTX2_OF le1 &= ~(1<<5)
#define LRX2_ON le1 |=  (1<<4)
#define LRX2_OF le1 &= ~(1<<4)

#define LTX1_ON le1 |=  (1<<3)
#define LTX1_OF le1 &= ~(1<<3)
#define LRX1_ON le1 |=  (1<<2)
#define LRX1_OF le1 &= ~(1<<2)

#define LTX0_ON le1 |=  (1<<1)
#define LTX0_OF le1 &= ~(1<<1)
#define LRX0_ON le1 |=  (1<<0)
#define LRX0_OF le1 &= ~(1<<0)

#define LR1_ON  le4 |=  (1<<2)
#define LR1_OF  le4 &= ~(1<<2)
#define LR2_ON  le4 |=  (1<<3)
#define LR2_OF  le4 &= ~(1<<3)
#define LR3_ON  le4 |=  (1<<4)
#define LR3_OF  le4 &= ~(1<<4)
#define LR4_ON  le4 |=  (1<<5)
#define LR4_OF  le4 &= ~(1<<5)
#define LR5_ON  le4 |=  (1<<6)
#define LR5_OF  le4 &= ~(1<<6)
#define LR6_ON  le4 |=  (1<<7)
#define LR6_OF  le4 &= ~(1<<7)
#define LR7_ON  le2 |=  (1<<0)
#define LR7_OF  le2 &= ~(1<<0)
#define LR8_ON  le2 |=  (1<<1)
#define LR8_OF  le2 &= ~(1<<1)
#define LR9_ON  le2 |=  (1<<2)
#define LR9_OF  le2 &= ~(1<<2)
#define LR10_ON le2 |=  (1<<3)
#define LR10_OF le2 &= ~(1<<3)
#define LR11_ON le2 |=  (1<<4)
#define LR11_OF le2 &= ~(1<<4)
#define LR12_ON le2 |=  (1<<5)
#define LR12_OF le2 &= ~(1<<5)
#define LR13_ON le2 |=  (1<<6)
#define LR13_OF le2 &= ~(1<<6)
#define LR14_ON le2 |=  (1<<7)
#define LR14_OF le2 &= ~(1<<7)

#define LW1_ON  le0 |=  (1<<0)
#define LW1_OF  le0 &= ~(1<<0)
#define LW2_ON  le0 |=  (1<<1)
#define LW2_OF  le0 &= ~(1<<1)
#define LW3_ON  le0 |=  (1<<2)
#define LW3_OF  le0 &= ~(1<<2)
#define LW4_ON  le0 |=  (1<<3)
#define LW4_OF  le0 &= ~(1<<3)
#define LW5_ON  le0 |=  (1<<4)
#define LW5_OF  le0 &= ~(1<<4)
#define LW6_ON  le0 |=  (1<<5)
#define LW6_OF  le0 &= ~(1<<5)
#define LW7_ON  le0 |=  (1<<6)
#define LW7_OF  le0 &= ~(1<<6)
#define LW8_ON  le0 |=  (1<<7)
#define LW8_OF  le0 &= ~(1<<7)
#define LW9_ON  le5 |=  (1<<7)
#define LW9_OF  le5 &= ~(1<<7)
#define LW10_ON le5 |=  (1<<6)
#define LW10_OF le5 &= ~(1<<6)
#define LW11_ON le5 |=  (1<<5)
#define LW11_OF le5 &= ~(1<<5)
#define LW12_ON le5 |=  (1<<4)
#define LW12_OF le5 &= ~(1<<4)
#define LW13_ON le5 |=  (1<<3)
#define LW13_OF le5 &= ~(1<<3)
#define LW14_ON le5 |=  (1<<2)
#define LW14_OF le5 &= ~(1<<2)
#define LW15_ON le5 |=  (1<<1)
#define LW15_OF le5 &= ~(1<<1)
#define LW16_ON le5 |=  (1<<0)
#define LW16_OF le5 &= ~(1<<0)
#define LW17_ON le3 |=  (1<<7)
#define LW17_OF le3 &= ~(1<<7)
#define LW18_ON le3 |=  (1<<6)
#define LW18_OF le3 &= ~(1<<6)
#define LW19_ON le3 |=  (1<<5)
#define LW19_OF le3 &= ~(1<<5)
#define LW20_ON le3 |=  (1<<4)
#define LW20_OF le3 &= ~(1<<4)
#define LW21_ON le3 |=  (1<<3)
#define LW21_OF le3 &= ~(1<<3)
#define LW22_ON le3 |=  (1<<2)
#define LW22_OF le3 &= ~(1<<2)
#define LW23_ON le3 |=  (1<<1)
#define LW23_OF le3 &= ~(1<<1)
#define LW24_ON le3 |=  (1<<0)
#define LW24_OF le3 &= ~(1<<0)


#define WR_ENABLE   CLEAR(F,6)
#define WR_DISSABLE SET(F,6)

#pragma used+
extern unsigned int relays;
extern unsigned char le0, le1, le2, le3, le4, le5;


void sys_init();
void input_init();
void output_init();
void set_output(char output, char state);
bool get_output(char output);
void output(char on_off, ...);
void output_on(char n_on, ...);
void output_off(char n_off, ...);

void buzzer(bool on);

void eeprom_read_stream(unsigned int address, char *src, unsigned char n);
void eeprom_write_stream(unsigned int address, char *src, unsigned char n);

void cprintf(char stream, char flash *fmt, ...);

#pragma used-

#endif


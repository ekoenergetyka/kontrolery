//PCExtINT.h : header file for the PC External Interrupts
//

#ifndef _PCEXTINT_H_
#define _PCEXTINT_H_

/////////////////////////////////////////////////////////////////////////////
//PCExtINT

void extint_init();

extern bool change;
extern unsigned char in_rising;
extern unsigned char in_falling;
extern unsigned int  in0_count[8];
extern unsigned long in0_abscount[8];


#endif // _PCEXTINT_H_

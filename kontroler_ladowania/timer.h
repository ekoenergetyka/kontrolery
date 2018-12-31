#define TPS (100)
#define T10ms 1
#define TPM (TPS * 60)
#define IDLE_TIME 30
#define TCNT1 (*(unsigned int *) 0x84)
#define xtal _MCU_CLOCK_FREQUENCY_
#define TCNCT1_TPS (xtal / 1024)

#define R_DELAY_OFF(relay, time) cnt## (#relay) = time

extern unsigned char inputs1;
extern unsigned char inputs2;
extern unsigned char inputs3;

extern unsigned char short_ticks;
extern unsigned int  bmask;

extern unsigned char TXC_shot;
extern unsigned char RXC_shot;
extern unsigned char TX2_shot;
extern unsigned char RX2_shot;
extern unsigned char TX1_shot;
extern unsigned char RX1_shot;
extern unsigned char TX0_shot;
extern unsigned char RX0_shot;
extern unsigned char E1_shot;
extern unsigned char E2_shot;

//extern unsigned char cntR1;
//extern unsigned char cntR2;
//extern unsigned char cntR3;
//extern unsigned char cntR4;
//extern unsigned char cntR5;
//extern unsigned char cntR6;
//extern unsigned char cntR7;
//extern unsigned char cntR8;
//extern unsigned char cntR9;
//extern unsigned char cntR10;
//extern unsigned char cntR11;
//extern unsigned char cntR12;
//extern unsigned char cntR13;
//extern unsigned char cntR14;

#pragma used+

void sys_timer_init();
unsigned long sys_timer_count (void);
unsigned long sys_timer_elapsed_count (unsigned long count);
void sys_timer_wait(unsigned long count);

void blinks(char on_off, ...);

#pragma used-

/*********************************************
Chip type           : ATMega16
Clock frequency     : 3,686400 MHz
Memory model        : Tiny
External SRAM size  : 0
Data Stack size     : 32
*********************************************/

#include <mega16.h>

// Standard Input/Output functions
#include <stdio.h>
#include <delay.h>
#include <string.h>
#include <stdbool.h>
#include <spi.h>

// #define USE_CAN

#include "tools.h"
#include "uart.h"

#ifdef USE_CAN
#include ".\CAN\can.h"
#endif

#define FALSE (0)
#define TRUE (! FALSE)

#define WE1 PINC.4
#define WE2 PINC.5
#define WE3 PINC.6
#define WE4 PINC.7
#define WE5 PINA.6

#define LCK PINC.3
#define LCK_FLAG 0x20

#define SHUKO1_PLUG 0x01
#define SHUKO2_PLUG 0x02

#define PK1 PORTD.6
#define PK2 PORTD.4
#define PK3 PORTD.3
#define PK4 PORTD.2
#define PK5 PORTB.4
#define PK6 PORTB.3

#define PWMOUT PORTD.5

#define LED_CS PORTA.3
#define CFG_IN PINB.6
#define SCK    PORTB.7

#define LED_IENN PORTA.2
#define LED_OEVN PORTA.0
#define LED_RXTX PORTA.1
#define LED_ON  0
#define LED_OFF 1

#define SW1 0x04
#define SW2 0x08
#define SW3 0x01
#define SW4 0x02

#define TPS 500 

#define INPUT_ON_TIME  (TPS / 10)
#define INPUT_OFF_TIME (TPS / 10)

unsigned int o_input_state = 0, input_state = 0;
unsigned int o_relay_state = 0, relay_state = 0;
unsigned int config = 0;
unsigned int start_count, timer_ticks, can_tick;

unsigned int led_ie = 0;
unsigned int led_oe = 0;
unsigned int led_tx = 0;

unsigned int update_config()
{
 unsigned int temp = 0;
 unsigned char i;

 SPCR=0x00;

 SCK = 0; delay_us(10);
 LED_CS = 0; delay_us(10); LED_CS = 1; delay_us(10);

 for (i = 0; i < 8; i++)
 {
  temp <<= 1;
  if (! CFG_IN) temp |= 0x0001;

  SCK = 0;  delay_us(10);  SCK = 1;  delay_us(10);
 }

 LED_CS = 0;

 temp >>= 4;

 SPCR=0x51;

 return temp;
}

// #define TCNT1 (*(unsigned int *) 0x84)

unsigned int timer_count (void)
{
 // return (TCNT1);
 register unsigned int tmp;
 
 #asm("cli")
 tmp = timer_ticks;
 #asm("sei") 
 return tmp; 
}

unsigned timer_elapsed_count (unsigned int count)
{
 return (timer_count () - count);
}

void timer_wait (unsigned int count)
{
 register unsigned int start_count;

 start_count = timer_count ();

 while (timer_elapsed_count (start_count) <= count)
 {
  #asm("wdr");
 }
}


unsigned char inputs = 0x00;
unsigned char ivalue = 0x00;
unsigned char istate = 0x00;

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
 unsigned char value;
 static unsigned char cnt0, cnt1;
 unsigned char delta, sample;

 // Reinitialize Timer 0 value
 TCNT0 = 0x83;

 value = 0;
 if (WE1 == 0) value |= 0x01;
 if (WE2 == 0) value |= 0x02;
 if (WE3 == 0) value |= 0x04;
 if (WE4 == 0) value |= 0x08;
 if (WE5 == 0) value |= 0x10;
 if (LCK == 0) value |= 0x20;
 sample = ivalue = value;
                     
 delta = sample ^ istate;
 cnt1 = (cnt1 ^ cnt0) & delta;
 cnt0 = ~cnt0 & delta;
 istate ^= (cnt0 & cnt1); 
          
 input_state = inputs = istate;

 if(LED_IENN == LED_ON)
 {
  if (led_ie != 0)
  {
   led_ie--;
   if (led_ie == 0) LED_IENN = LED_OFF;
  }
 }

 if(LED_OEVN == LED_ON)
 {
  if (led_oe != 0)
  {
   led_oe--;
   if (led_oe == 0) LED_OEVN = LED_OFF;
  }
 }

 if(LED_RXTX == LED_ON)
 {
  if (led_tx != 0)
  {
   led_tx--;
   if (led_tx == 0) LED_RXTX = LED_OFF;
  }
 }

 timer_ticks++; 
}


#define ADC_VREF_TYPE 0x40

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
 ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
 // Delay needed for the stabilization of the ADC input voltage
 delay_us(10);
 // Start the AD conversion
 ADCSRA|=0x40;
 // Wait for the AD conversion to complete
 while ((ADCSRA & 0x10)==0);
 ADCSRA|=0x10;
 
 return ADCW;
}

void io_init()
{
 // Input/Output Ports initialization
 // Port A initialization
 // Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
 // State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
 PORTA=0x00; DDRA=0x00;

 // Port B initialization
 // Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
 // State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
 PORTB=0x00; DDRB=0x00;

 // Port C initialization
 // Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
 // State6=T State5=T State4=T State3=T State2=T State1=T State0=T
 PORTC=0x00; DDRC=0x00;

 // Port D initialization
 // Func7=In Func6=In Func5=In Func4=In Func3=In Func2=In Func1=In Func0=In
 // State7=T State6=T State5=T State4=T State3=T State2=T State1=T State0=T
 PORTD=0x00; DDRD=0x00;

 INPUT(C,3); CLEAR(C, 3);   // LCK
 INPUT(C,4); CLEAR(C, 4);   // WE1
 INPUT(C,5); CLEAR(C, 5);   // WE2
 INPUT(C,6); CLEAR(C, 6);   // WE3
 INPUT(C,7); CLEAR(C, 7);   // WE4
 INPUT(A,6); CLEAR(C, 7);   // TEMP

 CLEAR(D, 6); OUTPUT(D, 6); // PK1
 CLEAR(D, 4); OUTPUT(D, 4); // PK2
 CLEAR(D, 3); OUTPUT(D, 3); // PK3
 CLEAR(D, 2); OUTPUT(D, 2); // PK4
 CLEAR(B, 4); OUTPUT(B, 4); // PK5
 CLEAR(B, 3); OUTPUT(B, 3); // PK6

 CLEAR(A, 2); OUTPUT(A, 2); // LED
 CLEAR(A, 0); OUTPUT(A, 0); // LED
 CLEAR(A, 1); OUTPUT(A, 1); // LED

 CLEAR(A, 4); INPUT (A, 4); // POM  CP+
 CLEAR(A, 5); INPUT (A, 5); // POM  CP-
 CLEAR(A, 7); INPUT (A, 7); // POM  PP

 SET(D, 5); OUTPUT(D, 5);   // PWM CP T1
 SET(D, 7); INPUT (D, 7);   // PWM CP T2 OC2    
 
 SET(B, 6); INPUT (B, 6);   // CFG READ
 CLEAR(B, 7); OUTPUT(B, 7); // SCK
 CLEAR(A, 3); OUTPUT(A, 3); // LED CS 
 
 CLEAR(C,0); OUTPUT(C,0); // RX TX 
 
 // ADC initialization
 // ADC Clock frequency: 115,200 kHz
 // ADC Voltage Reference: VCC
 // ADC Auto Trigger Source: ADC Stopped
 ADMUX=ADC_VREF_TYPE & 0xff;
 ADCSRA=0x85;

 // Timer/Counter 0 initialization
 // Clock source: System Clock
 // Clock value: 57,600 kHz
 // Mode: Normal top=FFh
 // OC0 output: Disconnected
 TCCR0=0x03; TCNT0=0x83; OCR0=0x00;

 // Timer/Counter 1 initialization
 // Clock source: System Clock
 // Clock value: 3686,400 kHz
 // Mode: Fast PWM top=ICR1
 // OC1A output: Toggle
 // OC1B output: Discon.
 // Noise Canceler: Off
 // Input Capture on Falling Edge
 // Timer1 Overflow Interrupt: Off
 // Input Capture Interrupt: Off
 // Compare A Match Interrupt: Off
 // Compare B Match Interrupt: Off
  TCCR1A=0x82; TCCR1B=0x19; TCNT1H=0x00; TCNT1L=0x00; ICR1H=0x0E; ICR1L=0x66;
  OCR1AH=0x07; OCR1AL=0x33; OCR1BH=0x07; OCR1BL=0x33;

 // Timer/Counter 2 initialization
 // Clock source: System Clock
 // Clock value: 3,600 kHz
 // Mode: Normal top=FFh
 // OC2 output: Disconnected
 ASSR=0x00; TCCR2=0x07; TCNT2=0x00; OCR2=0x00;
 
 // External Interrupt(s) initialization
 // INT0: Off
 // INT1: Off
 MCUCR=0x00;

 // Timer(s)/Counter(s) Interrupt(s) initialization
 TIMSK=0x01;        
 
 // SPI CAN PORT SETUP
 CLEAR(B, 7); OUTPUT(B, 7);  // SCK
 CLEAR(B, 5); OUTPUT(B, 5);  // MOSI
 SET(B, 6);   INPUT (B, 6);  // MISO

 // CAN PORT SETUP
 SET(B, 0); OUTPUT(B, 0);   // CAN_RST
 SET(B, 1); OUTPUT(B, 1);   // CAN_CS

 // SPI initialization
 // SPI Type: Master
 // SPI Clock Rate: 230.400 kHz
 // SPI Clock Phase: Cycle Half
 // SPI Clock Polarity: Low
 // SPI Data Order: MSB First
 SPCR=0x51;
 SPSR=0x00;
}


#define BEEPER 0x0008
#define BEEPER_ON   relay_state |=  BEEPER
#define BEEPER_OFF  relay_state &= ~BEEPER

#define SHUKO1 0x0002
#define SHUKO1_ON   relay_state |=  SHUKO1
#define SHUKO1_OFF  relay_state &= ~SHUKO1

#define SHUKO2 0x0004
#define SHUKO2_ON   relay_state |=  SHUKO2
#define SHUKO2_OFF  relay_state &= ~SHUKO2

//#define VENTIL_ON   ; // relay_state |=  0x0002
//#define VENTIL_OFF  ; // relay_state &= ~0x0002
#define VENTIL_ON   relay_state |=  0x0002
#define VENTIL_OFF  relay_state &= ~0x0002
#define POWER_ON    relay_state |=  0x0001
#define POWER_OFF   relay_state &= ~0x0001
#define LOCK_PLUG   relay_state |=  0x0010
#define UNLOCK_PLUG relay_state |=  0x0020

void update_relay(unsigned int new_relay_state)
{
 static unsigned int old_relay_state = 0, relay_count5 = 0, relay_count6 = 0;;

 if (old_relay_state != new_relay_state)
 {
  old_relay_state = new_relay_state;

  if (new_relay_state & 0x0001) PK1 = 1; else PK1 = 0;
  if (new_relay_state & 0x0002) PK2 = 1; else PK2 = 0;
  if (new_relay_state & 0x0004) PK3 = 1; else PK3 = 0;
  if (new_relay_state & 0x0008) PK4 = 1; else PK4 = 0;

  if (new_relay_state & 0x0010)
  {              
   relay_count5 = timer_count ();
   relay_state &= (~(0x0010));
   old_relay_state &= (~(0x0010));
   PK5 = 1; PK6 = 0;
  }
  else PK5 = 0;

  if (new_relay_state & 0x0020)
  {
   relay_count6 = timer_count ();
   relay_state &= (~(0x0020));
   old_relay_state &= (~(0x0020));
   PK6 = 1; PK5 = 0;
  }
  else PK6 = 0;

  led_oe = (TPS / 5);
  LED_OEVN = LED_ON;
 }

 if ((PK5) && (timer_elapsed_count (relay_count5) > TPS / 2)) PK5 = 0; 
 if ((PK6) && (timer_elapsed_count (relay_count6) > TPS / 2)) PK6 = 0; 
}

#define CP_12DCP 100
#define CP_12DCM 0
#define CP_16A 26 
#define CP_32A 50
#define CP_63A 89

void set_pwm(unsigned int pwm)
{
 unsigned long ltmp;
    
 if (pwm < 10)
 {           
  TCCR1A=0x00;        
  PWMOUT = 0;
 }
 else if (pwm > 90)
 {
  TCCR1A=0x00;
  PWMOUT = 1;
 } 
 else
 {
  ltmp = pwm;
  ltmp *= 0x0e66;
  pwm = ltmp / 100;
  OCR1AH = pwm / 0xff; OCR1AL = pwm % 0xff;;
  TCCR1A=0x82;
 }                                       
}

#define V_CP_PLUS 4
#define V_CP_MINUS 5
#define V_PP 7

unsigned int adc_conversion(unsigned char channel)
{
 unsigned int ADCresult = 0;
 unsigned char i;

 // do the ADC conversion 8 times for better accuracy
 for(i = 0; i < 32; i++) ADCresult += read_adc(channel);

 return ADCresult >> 5;     // average the samples
}
 

#define H1_CLOSE PK5
#define H2_OPEN  PK6

#define NO_PLUG 0
#define PLUG16A 1
#define PLUG32A 2
#define PLUG63A 3

#define NO_CAR       0
#define CAR_PRESENT  1
#define CAR_READY    2
#define CAR_VENTIL   3
#define WAIT_LOCK    4
#define UNKNOW_STATE (-1)   
#define INACTIVE     (-2)

#define NO_LOCK 0
#define LOCKED  1

#define B_LEVEL 9180
#define B_LEVEL_L (B_LEVEL - (B_LEVEL / 10))  
#define B_LEVEL_H (B_LEVEL + (B_LEVEL / 10))

#define C_LEVEL 6183
#define C_LEVEL_L (C_LEVEL - (C_LEVEL / 10))  
#define C_LEVEL_H (C_LEVEL + (C_LEVEL / 10))

#define D_LEVEL 4926
#define D_LEVEL_L (D_LEVEL - (D_LEVEL / 10))  
#define D_LEVEL_H (D_LEVEL + (D_LEVEL / 10))

#define P16A 680
#define P16A_LEVEL_L (P16A - (P16A / 10)) 
#define P16A_LEVEL_H (P16A + (P16A / 10)) 

#define P32A 220
#define P32A_LEVEL_L (P32A - (P32A / 10)) 
#define P32A_LEVEL_H (P32A + (P32A / 10)) 

#define P63A 100
#define P63A_LEVEL_L (P63A - (P63A / 10)) 
#define P63A_LEVEL_H (P63A + (P63A / 10)) 

unsigned int plug_state = NO_PLUG;
unsigned int car_state  = NO_CAR;
unsigned int lock_state = NO_LOCK;
unsigned int car_fsm    = INACTIVE;

unsigned int o_plug_state = NO_PLUG;
unsigned int o_car_state  = NO_CAR;
unsigned int o_lock_state = NO_LOCK;
unsigned int o_car_fsm    = INACTIVE;

unsigned int sock_cur   = 0;

unsigned int adcV_PP;
unsigned int adcV_CPM;
unsigned int adcV_CPP;

unsigned int Volt_PP;
unsigned int Volt_CPM;
unsigned int Volt_CPP;

unsigned int Res_PP;
bool enable_mk = false;
bool enable_s1 = false;
bool enable_s2 = false;
bool o_enable_mk = false;
bool dissable = false;

unsigned int Volt_calculation(int adc, float R1, float R2)
{
 float Volt;

 Volt  = adc; 
 Volt *= 5.0;
 Volt /= 1024;   
 
 Volt *= (R1 + R2);
 Volt /= R2;     

 return Volt * 1000.0;
}

unsigned int R2_calculation(int v1, int v2, float R1)
{
 float res, factor;
                  
 factor = v2; factor /= v1;
 
 res = factor * R1;
 factor = 1.0 - factor;
 res /= factor; 

 return res;  
}

void set_CP(unsigned char sock, unsigned char plug)
{
 if (sock == PLUG63A) 
 {                  
  if (plug == PLUG16A)      set_pwm(CP_16A);
  else if (plug == PLUG32A) set_pwm(CP_32A);  
  else if (plug == PLUG63A) set_pwm(CP_63A);
  else if (plug == NO_PLUG) set_pwm(CP_12DCP);
 }
 else if (sock == PLUG32A)
 {
  if (plug == PLUG16A)      set_pwm(CP_16A);
  else if (plug == PLUG32A) set_pwm(CP_32A);  
  else if (plug == PLUG63A) set_pwm(CP_32A);
  else if (plug == NO_PLUG) set_pwm(CP_12DCP);
 }      
 else if (sock == PLUG16A)
 {
  if (plug == PLUG16A)      set_pwm(CP_16A);
  else if (plug == PLUG32A) set_pwm(CP_16A);  
  else if (plug == PLUG63A) set_pwm(CP_16A);
  else if (plug == NO_PLUG) set_pwm(CP_12DCP);
 }                                
}

bool b_level()
{
 return (Volt_CPP > B_LEVEL_L) && (Volt_CPP < B_LEVEL_H);
}

bool c_level()
{
 return (Volt_CPP > C_LEVEL_L) && (Volt_CPP < C_LEVEL_H);
}

bool d_level()
{
 return (Volt_CPP > D_LEVEL_L) && (Volt_CPP < D_LEVEL_H);
}

 
void main(void)
{
 unsigned int lock_count, scan_count, beep_count;
#ifdef USE_CAN
 CanMessage r_msg, s_msg;
 bool can_send_flag = false;
#endif
 
 io_init();

 uart_init();

 // Analog Comparator initialization
 // Analog Comparator: Off
 // Analog Comparator Input Capture by Timer/Counter 1: Off
 ACSR=0x80; SFIOR=0x00;

 // Watchdog Timer initialization
 // Watchdog Timer Prescaler: OSC/2048k
#pragma optsize-
  WDTCR=0x1F;
  WDTCR=0x0F;
#ifdef _OPTIMIZE_SIZE_
  #pragma optsize+
#endif

 // Global enable interrupts
 #asm("sei")

            
 timer_wait(TPS);
 set_pwm(CP_12DCP);

 LED_IENN = LED_OFF;
 LED_OEVN = LED_OFF;
 LED_RXTX = LED_OFF;

 config = update_config();

 if ((config & SW1) && (config & SW2)) MY_MODBUS_ADDRESS = 32;
 else if (config & SW1) MY_MODBUS_ADDRESS = 12;
 else if (config & SW2) MY_MODBUS_ADDRESS = 22;
 else MY_MODBUS_ADDRESS = 1;

 if ((config & SW3) && (config & SW4)) sock_cur = PLUG63A;
 else if (config & SW3) sock_cur = PLUG16A;
 else if (config & SW4) sock_cur = PLUG32A;
 else sock_cur = PLUG32A; 
                                              
#ifdef USE_CAN
 if (can_init(CAN_20KBPS) != CAN_OK)
 {
 } 
 else if (can_setNormalMode() != CAN_OK)
 {
 }
#endif
 
 relay_state |= 0x20;  // Otwarcie zamka po resecie
 scan_count = start_count = can_tick = beep_count = timer_count ();
               
 o_enable_mk = enable_mk = false;
 
 while (1)
 {
  update_relay(relay_state);                     
  
  adcV_PP  = adc_conversion(V_PP);
  adcV_CPP = adc_conversion(V_CP_PLUS);
  adcV_CPM = adc_conversion(V_CP_MINUS);
  
  Volt_PP = Volt_calculation(adcV_PP, 100000, 27000);
//  Volt_CPM;
  Volt_CPP = Volt_calculation(adcV_CPP, 120000, 100000) + 1500;
  Res_PP = R2_calculation(12000, Volt_PP, 330);

  if (Res_PP >= P16A_LEVEL_H) plug_state = NO_PLUG; 
  else if ((Res_PP > P16A_LEVEL_L) && (Res_PP < P16A_LEVEL_H)) plug_state = PLUG16A; // 680 ohm dla 16  
  else if ((Res_PP > P32A_LEVEL_L) && (Res_PP < P32A_LEVEL_H)) plug_state = PLUG32A; // 220 ohm dla 32  
  else if ((Res_PP > P63A_LEVEL_L) && (Res_PP < P63A_LEVEL_H)) plug_state = PLUG63A; // 100 ohm dla 63  
  else plug_state = UNKNOW_STATE;
  
//  if (o_enable_mk == false && enable_mk == true)
//  {
//   o_enable_mk = enable_mk; 
//   dissable = false;
//
//   car_fsm = NO_CAR;
//  }
//  else if (o_enable_mk == true && enable_mk == true)
  {
   if (timer_elapsed_count (scan_count) > TPS / 5) 
   {
    scan_count = timer_count ();

    if (input_state & LCK_FLAG) lock_state = NO_LOCK; else lock_state =  LOCKED;

      if ((plug_state == PLUG16A) || (plug_state == PLUG32A) || (plug_state == PLUG63A))
      {                             
       if (Volt_CPP > B_LEVEL_H)  
       {                    
        if (car_state != NO_CAR)
        {
         car_state = NO_CAR;
         set_pwm(CP_12DCP);                      
         VENTIL_OFF;
         POWER_OFF;
         if (lock_state == LOCKED) UNLOCK_PLUG;
        }        
       }
       else if (b_level())  
       {                       
        if (car_state != CAR_PRESENT)
        {
         car_state = CAR_PRESENT;
         set_CP(sock_cur, plug_state);                 
         POWER_OFF;
         VENTIL_OFF;
         if (lock_state == LOCKED) UNLOCK_PLUG;
        }
       }
       else if (c_level()) 
       {
        if (car_state != CAR_READY) 
        {
         car_state = CAR_READY;
         set_CP(sock_cur, plug_state);                 
         POWER_ON;
         VENTIL_OFF;           
         if (lock_state == NO_LOCK) LOCK_PLUG;
        }                      
       }                     
       else if (d_level())
       {                     
        if (car_state != CAR_VENTIL)
        {
         car_state = CAR_VENTIL;
         set_CP(sock_cur, plug_state);                 
         POWER_ON;
         VENTIL_ON;
         if (lock_state == NO_LOCK) LOCK_PLUG;                      
        } 
       }
       else
       {
        if (car_state != UNKNOW_STATE)
        { 
         car_state = UNKNOW_STATE;                      
         set_pwm(CP_12DCP);
         VENTIL_OFF;
         POWER_OFF;
         if (lock_state == LOCKED) UNLOCK_PLUG;        
        }
       }
      } 
      else
      {
       if (car_state != NO_CAR)
       {
        car_state = NO_CAR;
        set_pwm(CP_12DCP);                      
        VENTIL_OFF;
        POWER_OFF;
        if (lock_state == LOCKED) UNLOCK_PLUG;
       }        
      }
   }
  }
//  else if (o_enable_mk == true && enable_mk == false)
//  {
//   o_enable_mk = enable_mk;
//   dissable = false;
//
//   set_pwm(CP_12DCP);
//   VENTIL_OFF;
//   POWER_OFF;
//   UNLOCK_PLUG;          
//   
//   car_state = NO_CAR;
//   plug_state = NO_PLUG;                    
//   if (input_state & LCK_FLAG) lock_state = NO_LOCK; else lock_state =  LOCKED;  
//  }
//  else if (o_enable_mk == false && enable_mk == false)
//  {
//   if (dissable == true) 
//   {
//    dissable = false;
//    UNLOCK_PLUG;        
//   }
//   if (input_state & LCK_FLAG) lock_state = NO_LOCK; else lock_state =  LOCKED;  
//  } 

#ifdef USE_CAN
  
  if ((o_plug_state != plug_state) || (o_car_state != car_state) || (o_lock_state != lock_state) || (o_car_fsm != car_fsm) ||
      (o_input_state != input_state) || (o_relay_state != relay_state))
  {
   o_plug_state = plug_state;
   o_car_state = car_state;
   o_lock_state = lock_state;
   o_car_fsm = car_fsm;
   o_input_state = input_state;
   o_relay_state = relay_state;
   
   can_send_flag = true;
  }
  

  // Odbiór pakietów z CAN
  if ((can_checkReceive() == CAN_MSGAVAIL))
  {             
   if (can_readMessage(&r_msg) == CAN_OK)
   {
    led_tx = (TPS / 20);
    LED_RXTX = LED_ON;
                                        
    if (r_msg.extended_identifier == CAN_STDID)
    {
     if (r_msg.identifier == 0x0123)  
     { 
      if (r_msg.dlc == 1) 
      {             
       enable_mk = (r_msg.dta[0] & 0x01) == 0x01;
       enable_s1 = (r_msg.dta[0] & 0x02) == 0x02;
       enable_s2 = (r_msg.dta[0] & 0x04) == 0x04;
      } 

      can_send_flag = true;
     }                              
     else if (r_msg.identifier == 0x0124)
     {
      beep_count = timer_count ();
      BEEPER_ON;
     }
    }
   }            
  }

  if (can_send_flag)
  {                
   can_send_flag = false;
   
   can_initMessageStruct(&s_msg);
   s_msg.identifier = 0x0123;
   s_msg.extended_identifier = CAN_STDID;
   s_msg.dlc = 8;   
                               
   // 0 - wy³¹czone wszystkie; b0 - en menekes; b1 - en shuko 1; b2 - en shuko 2
   s_msg.dta[0] |= (enable_mk) ? 0x01 : 0;
   s_msg.dta[0] |= (enable_s1) ? 0x02 : 0;
   s_msg.dta[0] |= (enable_s2) ? 0x04 : 0;
   s_msg.dta[1] = plug_state;  // NO_PLUG 0; PLUG16A 1; PLUG32A 2; PLUG63A 3
   s_msg.dta[2] = car_state;   // NO_CAR 0; CAR_PRESENT 1; CAR_READY 2; CAR_VENTIL 3; WAIT_LOCK 4; UNKNOW_STATE (-1); INACTIVE (-2) 
   s_msg.dta[3] = lock_state;  // NO_LOCK 0; LOCKED 1
   s_msg.dta[4] = config;      // stan automatu
   s_msg.dta[5] = input_state; // b0 wej1; b1 wej2; b2 wej2; b3 wej3;
   s_msg.dta[6] = relay_state; // b0 pk1 - stycznik zasilania; b1 pk2 - wentylacja ... 
   s_msg.dta[7] = sock_cur;    // 0 - brak wtyku; 1 - 16A; 2 - 32A; 3 - 63A  

   if ( can_sendMessage(&s_msg) == CAN_OK )
   {
    led_tx = (TPS / 20);
    LED_RXTX = LED_ON;
   }
   else
   {
    led_tx = (TPS / 5);
    LED_RXTX = LED_ON;
   }

   can_checkError();

   can_tick = timer_count ();
  }
                                       
  if (timer_elapsed_count (can_tick) > TPS)
  {
   can_tick = timer_count ();
   
   can_send_flag = true; 
  }
#endif
                         
  if (timer_elapsed_count (start_count) > TPS * 2)
  {
   start_count = timer_count ();

   config = update_config();
   if ((config & SW1) && (config & SW2)) MY_MODBUS_ADDRESS = 32;
   else if (config & SW1) MY_MODBUS_ADDRESS = 12;
   else if (config & SW2) MY_MODBUS_ADDRESS = 22;
   else MY_MODBUS_ADDRESS = 1;
   
   if ((config & SW3) && (config & SW4)) sock_cur = PLUG63A;
   else if (config & SW3) sock_cur = PLUG16A;
   else if (config & SW4) sock_cur = PLUG32A;
   else sock_cur = PLUG32A;
  }

  if (relay_state & BEEPER)
  {
   if (timer_elapsed_count (beep_count) > TPS / 10) BEEPER_OFF;
  }
            
  if (enable_s1)
  {
   // Sprawdzamy krancówkê jeœli za³¹czona i przekaŸnik wy³¹czony to za³¹czamy przekaŸnik
   if (input_state & SHUKO1_PLUG)
   { 
    if (!(relay_state & SHUKO1)) SHUKO1_ON;
   }
   else if (relay_state & SHUKO1) SHUKO1_OFF;
  }         
  else if (relay_state & SHUKO1) SHUKO1_OFF;

  if (enable_s2)
  {
   // Sprawdzamy krancówkê jeœli za³¹czona i przekaŸnik wy³¹czony to za³¹czamy przekaŸnik 
   if (input_state & SHUKO2_PLUG)
   { 
    if (!(relay_state & SHUKO2)) SHUKO2_ON;
   }
   else if (relay_state & SHUKO2) SHUKO2_OFF;
  }         
  else if (relay_state & SHUKO2) SHUKO2_OFF;

  #asm("wdr");
 }
}


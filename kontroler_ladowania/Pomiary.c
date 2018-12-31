#include <IO.h>
#include <delay.h>
#include <math.h>
#include "Pomiary.h"


#define ADC_VREF_TYPE 0x40

void init_adc(void)
{
 // ADC initialization
 // ADC Clock frequency: 28,800 kHz
 // ADC Voltage Reference: AVCC pin
 // ADC Auto Trigger Source: None
 // Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
 // ADC4: On, ADC5: On, ADC6: On, ADC7: On
 DIDR0=0x00;
 // Digital input buffers on ADC8: On, ADC9: On, ADC10: On, ADC11: On
 // ADC12: On, ADC13: On, ADC14: On, ADC15: On
 DIDR2=0x00;
 ADMUX=ADC_VREF_TYPE & 0xff;
 ADCSRA=0x87;

 // Analog Comparator initialization
 // Analog Comparator: Off
 // Analog Comparator Input Capture by Timer/Counter 1: Off
 ACSR=0x80; ADCSRB=0x00;
}

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
 ADMUX=(adc_input & 0x07) | (ADC_VREF_TYPE & 0xff);
 if (adc_input & 0x08) ADCSRB |= 0x08; else ADCSRB &= 0xf7;
 // Delay needed for the stabilization of the ADC input voltage
 delay_us(10);
 // Start the AD conversion
 ADCSRA|=0x40;
 // Wait for the AD conversion to complete
 while ((ADCSRA & 0x10)==0);
 ADCSRA|=0x10;
 return ADCW;
}


// array to calculate the temperature
float Temperature;
unsigned int Light;

float Temp_calculation(int iADC_value)
{
 float V_ADC;

 V_ADC = (iADC_value * V_ref) / 1024;         // calculate the voltage over the NTC

 // Rt = V_ADC / ((VR_ref  - V_ADC) / R_s);
 // A = 1 / 298.0 - ( (1 / Beta) * log(R_th) );
 // Temperature = 1 / (A + ((1/Beta)*(log(Rt)))) - T_zero;

 // calculate the temperature
 V_ADC = (Beta /( (log(V_ADC /(VR_ref-V_ADC)) / log(__E) ) + (Beta / T_amb))) - T_zero;

 return V_ADC;
}


void TEMP_conversion(void)
{
 unsigned int ADCresult = 0;
 unsigned char i;

 // do the ADC conversion 8 times for better accuracy
 for(i = 0; i < 32; i++) ADCresult += read_adc(TEMPERATURE_CHANNEL);
 ADCresult = ADCresult >> 5;     // average the samples

 // call the temperature calculation function
 Temperature = Temp_calculation(ADCresult);
}
 
void LIGHT_conversion(void)
{
 unsigned int ADCresult = 0;
 unsigned char i;

 // do the ADC conversion 8 times for better accuracy
 for(i = 0; i < 32; i++) ADCresult += read_adc(LIGHT_CHANNEL);
 ADCresult = ADCresult >> 5;     // average the samples

 Light = ADCresult;
}

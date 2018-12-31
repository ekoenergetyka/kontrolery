#ifndef _POMIARY_INCLUDED_
#define _POMIARY_INCLUDED_

// Declare your global variables here
/************************************************************************
*
*                 Vref = 5.0V
*                 _____
*                   |
*                   |
*                 -----
*                 |   |
*                 |   |  R_1 = 10 Kohm
*                 |   |
*                 -----
*                   |
*                   |---------------------X    ADC channel #0
*                   |
*                 -----
*                 |   |
*                 |   |  R_NTC = ? ohm   (10Kohm @ 25 degree celcius)
*                 |   |
*                 -----
*                   |
*                   |---------------------X    ADC channel #1
*                   |
*                   |
*                  ---
*                   -
*                  GND
*
************************************************************************/
// #define V_ref           3.369       // voltage-reference
#define V_ref           5.00        // voltage-reference
#define VR_ref          5.00        // Thermistor supply voltage

#define R1VAL 110000.0
#define R2VAL  10000.

// NTC-thermistor defines
#define R_s             10000.00    // the series resistor resistans
#define Beta            3380.000    // the Beta-value for the NTC transitor
#define R_th            10000.00    // the thermistor resistans @ 25 degree Celsius
#define T_amb           298.0000    // the temperature in Kelvin for 25 degree Celsius
#define T_zero          273.0000    // the temperature in Kelvin for 0 degree Celsius
#define __E             2.718281828459045235360287


#define TEMPERATURE_CHANNEL 3
#define LIGHT_CHANNEL 8

// Function declarations
#pragma used+
extern float Temperature;
extern unsigned int Light;


void init_adc(void);
void TEMP_conversion (void);
void LIGHT_conversion(void);

#pragma used-


#endif

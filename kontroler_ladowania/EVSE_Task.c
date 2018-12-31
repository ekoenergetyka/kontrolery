#include <io.h>
#include <stdbool.h>
#include <stdio.h>
#include <delay.h>
#include "IOSys.h"
#include "Timer.h"
#include "EVSE_Task.h"
#include "Tools.h"
#include "Uart.h"

int inverter_state = 0;

unsigned char EVSE_exit_code   = 0;
unsigned char EVSE_exit_state  = 0;

unsigned char exit(unsigned char next_state, unsigned char exit_state, unsigned char error_code)
{
 EVSE_exit_state = exit_state;
 EVSE_exit_code = error_code;

 return next_state;
}

void EVSE_setup_task(bool on)
{
 static int old_state = EVSE_SHUT_OFF, state = EVSE_SHUT_OFF;
 static unsigned long timeout_period;

 if (old_state != state)
 {
  old_state = state;
  timeout_period = sys_timer_count();

//  cprintf(COM1, "\r\nS %d  ES %d  EC %d\r\n", state, EVSE_exit_state, EVSE_exit_code);
 }

 switch (state)
 {
  case EVSE_SHUT_OFF:
  {
   output_off(1, DC_RELAY);
   output_off(1, INV1_RELAY);
   output_off(1, INV2_RELAY);
   output_off(1, AC1_RELAY);
   output_off(1, AC2_RELAY);

   output_off(1, L_READY);
   output_off(1, L_CHARGE);
   output_off(1, L_ERROR);

   state = CHECK_FIRE_ERROR;
  }
  break;

  case CHECK_FIRE_ERROR:
  {
   if (NO_FIRE_ERROR) state = CHECK_FIRE_ALARM;
  }
  break;

  case CHECK_FIRE_ALARM:
  {
   if (NO_FIRE_ALARM) state = CHECK_AC_SUPPLY;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
  }
  break;

  case CHECK_AC_SUPPLY:
  {
   if (AC_SUPPLY) state = CHECK_AC_RELAY;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
  }
  break;

  case CHECK_AC_RELAY:
  {
   if (AC_RELAY_ON) state = CHECK_OVERHEAT;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
  }
  break;

  case CHECK_OVERHEAT:
  {
   if ((BIM_SET_1_OK) && (BIM_SET_2_OK)) state = EVSE_DEFAULT;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
  }
  break;

  case EVSE_DEFAULT:
  {
   if (on)                 state = CHECK_AC1_ON;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
   else if (BIM_SET_1_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (BIM_SET_2_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
  }
  break;

  case CHECK_AC1_ON:
  {
   output_on(1, AC1_RELAY);
   if (AC1_RELAY_ON)       state = CHECK_AC2_ON;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
   else if (BIM_SET_1_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (BIM_SET_2_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);

   // Dlugi czas bo za³¹czeniem potwierdzenia steruje uk³ad SOFTSTART falownika
   else if (TIMEOUT(timeout_period, TPS * 25)) state = exit(EVSE_ERROR, state, CHECK_AC1_ON);
   else if (!on)
   {
    output_off(1, INV1_RELAY);
    output_off(1, INV2_RELAY);

    delay_ms(10);

    output_off(1, AC1_RELAY);
    output_off(1, AC2_RELAY);

    state = EVSE_DEFAULT;
   }
  }
  break;

  case CHECK_AC2_ON:
  {
   output_on(1, AC2_RELAY);
   if (AC2_RELAY_ON)       state = CHECK_INV1_RDY;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
   else if (BIM_SET_1_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (BIM_SET_2_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);

   // Dlugi czas bo za³¹czeniem potwierdzenia steruje uk³ad SOFTSTART falownika
   else if (TIMEOUT(timeout_period, TPS * 25)) state = exit(EVSE_ERROR, state, CHECK_AC2_ON);
   else if (!on)
   {
    output_off(1, INV1_RELAY);
    output_off(1, INV2_RELAY);

    delay_ms(10);

    output_off(1, AC1_RELAY);
    output_off(1, AC2_RELAY);

    state = EVSE_DEFAULT;
   }
  }
  break;

  case CHECK_INV1_RDY:
  {
   if ((INVERTER1_RDY) && (TIMEOUT(timeout_period, TPS * 2)))  state = CHECK_INV2_RDY;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
   else if (BIM_SET_1_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (BIM_SET_2_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);

   else if (AC1_RELAY_OFF) state = exit(EVSE_ERROR, state, CHECK_AC1_ON);
   else if (AC2_RELAY_OFF) state = exit(EVSE_ERROR, state, CHECK_AC2_ON);

//   else if (TIMEOUT(timeout_period, TPS * 20)) state = exit(EVSE_ERROR, state, CHECK_INV1_RDY);
   else if (TIMEOUT(timeout_period, TPS * 25)) state = CHECK_INV2_RDY;

   else if (!on)
   {
    output_off(1, INV1_RELAY);
    output_off(1, INV2_RELAY);

    delay_ms(10);

    output_off(1, AC1_RELAY);
    output_off(1, AC2_RELAY);

    state = EVSE_DEFAULT;
   }
  }
  break;


  case CHECK_INV2_RDY:
  {
   if ((INVERTER2_RDY) && (TIMEOUT(timeout_period, TPS * 2)))  state = EVSE_STUNDBY;
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
   else if (BIM_SET_1_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (BIM_SET_2_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);


   else if (AC1_RELAY_OFF) state = exit(EVSE_ERROR, state, CHECK_AC1_ON);
   else if (AC2_RELAY_OFF) state = exit(EVSE_ERROR, state, CHECK_AC2_ON);

//   else if (TIMEOUT(timeout_period, TPS * 20)) state = exit(EVSE_ERROR, state, CHECK_INV2_RDY);
   else if (TIMEOUT(timeout_period, TPS * 25)) state = EVSE_STUNDBY;

   else if (!on)
   {
    output_off(1, INV1_RELAY);
    output_off(1, INV2_RELAY);

    delay_ms(10);

    output_off(1, AC1_RELAY);
    output_off(1, AC2_RELAY);

    state = EVSE_DEFAULT;
   }
  }
  break;

  case EVSE_STUNDBY:
  {
   if (!on)
   {
    output_off(1, INV1_RELAY);
    output_off(1, INV2_RELAY);

    delay_ms(10);

    output_off(1, AC1_RELAY);
    output_off(1, AC2_RELAY);

    state = EVSE_DEFAULT;
   }
   else if (FIRE_ERROR)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ERROR);
   else if (FIRE_ALARM)    state = exit(EVSE_ERROR, state, CHECK_FIRE_ALARM);
   else if (NO_AC_SUPPLY)  state = exit(EVSE_ERROR, state, CHECK_AC_SUPPLY);
   else if (AC_RELAY_OFF)  state = exit(EVSE_ERROR, state, CHECK_AC_RELAY);
   else if (BIM_SET_1_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (BIM_SET_2_ALR) state = exit(EVSE_ERROR, state, CHECK_OVERHEAT);
   else if (AC1_RELAY_OFF) state = exit(EVSE_ERROR, state, CHECK_AC1_ON);
   else if (AC2_RELAY_OFF) state = exit(EVSE_ERROR, state, CHECK_AC2_ON);

//   else if (INVERTER1_OFF) state = exit(EVSE_ERROR, state, CHECK_INV1_RDY);
//   else if (INVERTER2_OFF) state = exit(EVSE_ERROR, state, CHECK_INV2_RDY);
   else if ((INVERTER1_OFF) && (INVERTER2_OFF)) state = exit(EVSE_ERROR, state, (INVERTER1_OFF) ? CHECK_INV1_RDY : CHECK_INV2_RDY);
  }
  break;

  case EVSE_ERROR:
  {
   output_off(1, INV1_RELAY);
   output_off(1, INV2_RELAY);

   delay_ms(10);

   output_off(1, AC1_RELAY);
   output_off(1, AC2_RELAY);

   output_on(1, L_ERROR);

   if (!on) state = EVSE_SHUT_OFF;
  }
  break;
 }

 inverter_state = state;
}


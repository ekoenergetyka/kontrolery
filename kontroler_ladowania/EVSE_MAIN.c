/*****************************************************
Chip type               : ATmega640
Program type            : Application
AVR Core Clock frequency: 11.059200 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 2048
*****************************************************/

#include <io.h>
#include <stdbool.h>
#include <delay.h>
#include <ctype.h>
#include <math.h>
#include <spi.h>
#include <float.h>

#include "IOsys.h"
#include "Tools.h"
#include "Uart.h"
#include "Uart0.h"
#include "Uart1.h"
#include "Uart2.h"
#include "Uart3.h"
#include "Timer.h"
#include "Protocol.h"


#include ".\CAN\can.h"

#include "INSYS.h"
#include "EVSE_Task.h"

#define _ALTERNATE_PUTCHAR_
#include <stdio.h>
#include <string.h>

#define CAN_INIT_FAIL  0x01  // B³ad inicjacji kontrolra CAN
#define CAN_MODE_FAIL  0x02  // B³¹d ustawiania trybu transmisji kontrolera CAN
#define CAN_TRANS_FAIL 0x04  // B³¹d transmisji pakietu
#define CAN_READ_FAIL  0x80  // Brak pakietów od autobusu
unsigned int CAN_STATUS = 0x00;


unsigned int inv_U = 0;
unsigned int inv_I = 0;
unsigned int real_U1 = 0,  real_I1 = 0, health_INV1 = 0;
unsigned int real_U2 = 0,  real_I2 = 0, health_INV2 = 0;
unsigned char o_master_state = 0;  // Nadrzêdny stan wymuszany np. na skutek b³êdu
unsigned char master_state = 0;    // Nadrzêdny stan wymuszany np. na skutek b³êdu
unsigned char int_err_code = 0;    // Kod b³êdu
unsigned char old_err_code = 0;    // Poprzedni kod b³êdu do wykrywania zmian
unsigned char int_wrr_code = 0;    // Kod ostrze¿enia
unsigned char exit_state = 0;      // Stan wyjœcia na skutek b³êdu

unsigned int READ_RESULT[6];

#define STOP_LOGIC_MASK 0x0001
#define STOP_LOGIC (WORKING_FLAGS & STOP_LOGIC_MASK)
unsigned int WORKING_FLAGS = 0;

void update_leds()
{
 static unsigned char ole0, ole1, ole2, ole3, ole4, ole5;

 if (ole0 != le0 || ole1 != le1 || ole2 != le2 || ole3 != le3 || ole4 != le4 || ole5 != le5)
 {
  ole0 = le0; ole1 = le1; ole2 = le2; ole3 = le3; ole4 = le4; ole5 = le5;

  CLEAR(G,5);
  spi(le5);
  spi(le4);
  spi(le3);
  spi(le2);
  spi(le1);
  spi(le0);
  SET(G,5);
  CLEAR(G,5);
 }
}

#define IN_RELAY_ON    (inputs & IN_RELAY_STATE)
#define IN_RELAY_OFF   (!(inputs & IN_RELAY_STATE))
#define CAN_NODATA     (CAN_STATUS & CAN_READ_FAIL)
#define CAN_DATA_OK    (!(CAN_STATUS & CAN_READ_FAIL))

#define GET_CHIP_ID   0x99


#define EVSE_ISO_VOLTAGE 400
#define EVSE_MAX_VOLTAGE 800
#define EVSE_MAX_CURRENT 660
#define RX_TX_OK_LTIME 1
#define RX_TX_ERR_LTIME 1

struct S_CCU620  S620;
struct S_CCU621  S621;
struct S_CLC520  S520;
struct S_CLC521  S521;

#define CAN_RX_OK      0
#define CAN_RX_ERROR   1
#define CAN_RX_TIMEOUT 2

#define CAN_TX_OK      0
#define CAN_TX_ERROR   1

#define EVSE_GO_OFF false
#define EVSE_GO_ON  true

bool rem_CPenable = false;
bool rem_Enable   = false;
bool rem_STOP     = false;
bool StopRequest  = false;
bool CAN_TOUT     = false;

int can_rx_process()
{
 static unsigned long can_rx_period;
 static bool setup = false;
 static unsigned char rqs_state = 0;
 int result;
 CanMessage r_msg;

 if (!setup)
 {
  setup = true;
  can_rx_period = sys_timer_count();
 }

 while ((can_checkReceive() == CAN_MSGAVAIL))
 {
  RXC_shot = RX_TX_OK_LTIME; LRXC_ON; update_leds();

  can_initMessageStruct(&r_msg);
  if (can_readMessage(&r_msg) == CAN_OK)
  {
   if (r_msg.extended_identifier == CAN_STDID)
   {
    switch (r_msg.identifier)
    {
     case CCU620:
       can_rx_period = sys_timer_count();
       CAN_TOUT = false;

       unp_CCU620(r_msg.dta, &S620);

//       cprintf(DEBUG_PORT, "\r\n");
//       cprintf(DEBUG_PORT, "ReadySate %d  ", b->EVReadySate);
       if (rqs_state != S620.EVRequestProgState)
       {
        rqs_state = S620.EVRequestProgState;
//        cprintf(DEBUG_PORT, "\r\nCCU REQ ProgState %p  ", S620.EVRequestProgState);
       }
//       cprintf(DEBUG_PORT, "SOC %2d  ", b->EVSOC);
//       cprintf(DEBUG_PORT, "ErrorCode %d  ", b->EVErrorCodeType);
     break;

     case CCU621:
       can_rx_period = sys_timer_count();
       CAN_TOUT = false;

       unp_CCU621(r_msg.dta, &S621);

//       cprintf(DEBUG_PORT, "\r\n");
//       cprintf(DEBUG_PORT, "TC %4d  ", b->EVTargetCurrent);
//       cprintf(DEBUG_PORT, "TV %5d  ", b->EVTargetVoltage);
//       cprintf(DEBUG_PORT, "MaxC %4d  ", b->EVMaximumCurrentLimit);
//       cprintf(DEBUG_PORT, "MaxV %5d  ", b->EVMaximumVoltageLimit);
     break;

     case CCU630:
       can_rx_period = sys_timer_count();
       CAN_TOUT = false;

       rem_CPenable = ((r_msg.dta[0] & (1 << 0)) == (1 << 0));
       rem_Enable   = ((r_msg.dta[0] & (1 << 1)) == (1 << 1));
       rem_STOP     = ((r_msg.dta[0] & (1 << 2)) == (1 << 2));

       if (rem_STOP) StopRequest = true; else StopRequest = false;

       result = CAN_RX_OK;
     break;
    }
   }
   result = CAN_RX_OK;
  }
  else
  {
   E1_shot = RX_TX_ERR_LTIME; LE1_ON; update_leds();
   result = CAN_RX_ERROR;
  }
 }
// else
 {
  if (sys_timer_elapsed_count(can_rx_period) >= TPS * 2)
  {
   can_rx_period = sys_timer_count();
   E1_shot = RX_TX_ERR_LTIME; LE1_ON; update_leds();

   result = CAN_RX_TIMEOUT;

   CAN_TOUT = true;
  }
 }
 can_checkError();

 return result;
}

#define TX_TIMEOUT(A) (sys_timer_elapsed_count(tx_timer) >= (A))

int can_tx_process()
{
 static unsigned char sel = 0;
 static unsigned long tx_timer = 0;
 CanMessage s_msg;
 int result = CAN_TX_OK;

 if (tx_timer == 0) tx_timer = sys_timer_count();

 if (TX_TIMEOUT(TPS / 5))
 {
  tx_timer = sys_timer_count();

  can_initMessageStruct(&s_msg);
  s_msg.dlc = 8; s_msg.extended_identifier = CAN_STDID;

  switch(++sel % 3)
  {
   case 0: s_msg.identifier = CLC520; pac_CLC520(s_msg.dta, &S520); break;
   case 1: s_msg.identifier = CLC521; pac_CLC521(s_msg.dta, &S521); break;
   case 2: s_msg.identifier = CLC404;
           s_msg.dta[0] = S620.EVRequestProgState;
           s_msg.dta[1] = exit_state;
           s_msg.dta[2] = int_err_code;
           s_msg.dta[3] = int_wrr_code;
           break;
  }

  if (can_sendMessage(&s_msg) == CAN_OK)
  {
   result = CAN_TX_OK;
   TXC_shot = RX_TX_OK_LTIME; LTXC_ON; update_leds();
  }
  else
  {
   result = CAN_TX_ERROR;
   E1_shot = RX_TX_ERR_LTIME; LE1_ON; update_leds();
  }
  can_checkError();
 }

 return result;
}

#define INV_SWITCH_ON  true
#define INV_SWITCH_OFF false

unsigned int mb_rd_inv_ecnt = 0;
unsigned int mb_wr_inv_ecnt = 0;

int old_EVRequestProgState = -1;
int old_EVSEStatusCode = 0;
int old_EVSEProcessing = 0;

int can_rx_state = CAN_RX_OK;

unsigned long sending_period; // timer dla wysy³ki stanów

unsigned long pstate_timer;   // timer dla prog_state_task

// Zmienne dla restartów
unsigned long test_period, restar1_time, restart2_time;
bool restarts_enable = false;
bool restart1_flag = false;
bool restart2_flag = false;
bool restart1_soft = false;
bool restart2_soft = false;
bool restart1_hard = false;
bool restart2_hard = false;

bool pstate_second_elapsed = false;
bool pstate_2second_elapsed = false;
bool pstate_timer_elapsed = false;

#define PSTATE_TIMER_INIT  { pstate_timer = sys_timer_count(); pstate_timer_elapsed = false; }
#define PSTATE_TIMER(A)     sys_timer_elapsed_count(pstate_timer) >= (A)
//#define PSTATE_ENTER        old_EVRequestProgState != S620.EVRequestProgState
#define PSTATE_ENTER       enter_state != master_state

unsigned char enter_state = (-1);

void prog_state_task()
{
  if (rem_Enable == false) S620.EVRequestProgState = PSTATE_DEFAULT;

  if (master_state != PSTATE_ERROR) master_state = S620.EVRequestProgState;

  switch (master_state)
  {
   case PSTATE_DEFAULT:
   {
    // Sekwencja wykonywana jednorazowo na wiejœciu do stanu
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Od³¹czamy inwerter
     output(OFF, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy stycznik DC
     output(OFF, DC_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy styczniki AC1, AC2
     output_off(2, AC1_RELAY, AC2_RELAY); delay_ms(50);

     S520.EVSEProgState = PSTATE_DEFAULT;
     S520.EVSEProcessing = FINISHED;
     S520.EVSEStatusCode = EVSE_READY;
     S520.EVSEIsolationStatus = ISO_INVALID;

     S521.EVSEPresentCurrent = 0;
     S521.EVSEPresentVoltage = 0;
     S521.EVSEMaximumCurrentLimit = EVSE_MAX_CURRENT * 10;
     S521.EVSEMaximumVoltageLimit = EVSE_MAX_VOLTAGE * 10;

     restarts_enable = false;
     restart1_flag = restart1_soft = restart1_hard = false;
     restart2_flag = restart2_soft = restart2_hard = false;

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    if (rem_CPenable == false) output(OFF, CPENABLE, EOF); else output(ON, CPENABLE, EOF);

    inv_U = S621.EVTargetVoltage / 10;
    if (inv_U > S621.EVMaximumVoltageLimit / 10) inv_U = S621.EVMaximumVoltageLimit / 10;
    inv_I = S621.EVTargetCurrent;
    if (inv_I > S621.EVMaximumCurrentLimit) inv_I = S621.EVMaximumCurrentLimit;

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if (((AC1_RELAY_ON) || (AC2_RELAY_ON)) && (pstate_second_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (AC1_RELAY_ON && AC2_RELAY_ON) ? CHECK_AC12_ON : (AC1_RELAY_ON) ? CHECK_AC1_ON : CHECK_AC2_ON;
    }
    else if (((INVERTER1_ON) || (INVERTER2_ON)) && (pstate_second_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (INVERTER1_ON && INVERTER2_ON) ? CHECK_INV12_RDY : (INVERTER1_ON) ? CHECK_INV1_RDY : CHECK_INV2_RDY;
    }
    else if ((DC_RELAY_ON) && (pstate_second_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_OFF;
    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_STUNDBY:
   {
    // Sekwencja wykonywana jednorazowo na wiejœciu do stanu
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Wy³¹czamy inwerter
     output(OFF, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy stycznik DC
     output(OFF, DC_RELAY, EOF); delay_ms(50);
     // Za³¹czamy styczniki AC1, AC2
     output_on(2, AC1_RELAY, AC2_RELAY); delay_ms(50);

     S520.EVSEProgState = PSTATE_STUNDBY;
     S520.EVSEProcessing = ONGOING;
     S520.EVSEStatusCode = EVSE_READY;

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    if (pstate_timer_elapsed == false)
    {
     if ((AC1_RELAY_ON) && (AC2_RELAY_ON) && (INVERTER1_ON) && (INVERTER2_ON) || (PSTATE_TIMER(TPS * 30)))
     {
      S520.EVSEProcessing = FINISHED;
      pstate_timer_elapsed = true;
     }
    }

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if ((AC1_RELAY_OFF) && (AC2_RELAY_OFF) && (pstate_timer_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC12_NON;
    }
    else if ((INVERTER1_OFF) && (INVERTER2_OFF) && (pstate_timer_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_INV12_NRDY;
    }
    else if (DC_RELAY_ON)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_OFF;
    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_CABLE_CHECK:
   {
    // Sekwencja wykonywana jednorazowo na wiejœciu do stanu
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Za³¹czamy stycznik DC
     output(ON, DC_RELAY, EOF);

     // Za³¹czamy inwerter
//     output(ON, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);

     inv_U = EVSE_ISO_VOLTAGE;
     inv_I = 10;

     S520.EVSEStatusCode = EVSE_ISO_MONIT_ACTIVE;   // Status podczas testu musi byæ STS_ISO_MONIT_ACTIVE - aktywny pomiar izolacji
     S520.EVSEIsolationStatus = ISO_INVALID;        // ISO_INVALID - stan podczas testu
     S520.EVSEProgState = PSTATE_CABLE_CHECK;       // Podtwierdzamy stan testu PSTATE_CABLE_CHECK
     S520.EVSEProcessing = ONGOING;
     S520.EVSEStatusCode = EVSE_READY;

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
     pstate_2second_elapsed = false;
     pstate_timer_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;

     // Za³¹czamy inwerter
     output(ON, INV1_RELAY, EOF);
    }

    if (pstate_2second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS * 2)) pstate_2second_elapsed = true;

     // Za³¹czamy inwerter
     output(ON, INV2_RELAY, EOF);
    }

    if (pstate_timer_elapsed == false)
    {
     if (PSTATE_TIMER(TPS * 10)) pstate_timer_elapsed = true;
    }

    S521.EVSEPresentVoltage = MAX(real_U1, real_U2) * 10;
    S521.EVSEPresentCurrent = 0;
    S521.EVSEMaximumCurrentLimit = EVSE_MAX_CURRENT * 10;
    S521.EVSEMaximumVoltageLimit = EVSE_MAX_VOLTAGE * 10;

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if ((AC1_RELAY_OFF) && (AC2_RELAY_OFF))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC12_NON;
    }
    else if ((INVERTER1_OFF) && (INVERTER2_OFF))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_INV12_NRDY;
    }
    else if ((DC_RELAY_OFF) && (pstate_second_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_ON;
    }
    else if ((DC_OUT_FAULT) && (pstate_timer_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_OUT_SHORT;
    }
    else if ((DC_ISO_FAULT) && (pstate_timer_elapsed))
    {
     S520.EVSEProcessing = FINISHED;
     S520.EVSEStatusCode = EVSE_NOTREADY;
     S520.EVSEIsolationStatus = ISO_FAULT;   // Stan FAULT
     int_err_code = DC_ISO_FAILER;
    }
    else if ((DC_ISO_WARN) && (pstate_timer_elapsed))
    {
     S520.EVSEProcessing = FINISHED;
     S520.EVSEStatusCode = EVSE_NOTREADY;
     S520.EVSEIsolationStatus = ISO_WARNING; // Stan WARNING

     int_err_code = DC_ISO_WARNING;
    }
    else if (pstate_timer_elapsed)
    {
     S520.EVSEProcessing = FINISHED;
     S520.EVSEStatusCode = EVSE_READY;       // Status na EVSE_READY
     S520.EVSEIsolationStatus = ISO_VALID;   // Stan VALID
     int_err_code = EVSE_NO_ERROR;
    }
//    else if (PSTATE_TIMER(TPS * 40))
//    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
//     int_err_code = DC_RELAY_NOT_ON;
//    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_PRECHARGE:
   {
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Za³¹czamy stycznik DC
     output(ON, DC_RELAY, EOF); delay_ms(50);
     // Za³¹czamy inwertery
     output(ON, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);

     S520.EVSEProcessing = FINISHED;            // Operacja  zakoñczona
     S520.EVSEProgState = PSTATE_PRECHARGE;     // Podtwierdzamy stan PSTATE_PRECHARGE
     S520.EVSEStatusCode = EVSE_READY;          // Status na EVSE_READY

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    // Ustawiamy wstêpnie napiêcie i pr¹d wyjœciowy falownika
    inv_U = S621.EVTargetVoltage / 10;
    if (inv_U > S621.EVMaximumVoltageLimit / 10) inv_U = S621.EVMaximumVoltageLimit / 10;
    inv_I = 10;

    S521.EVSEPresentVoltage = MAX(real_U1, real_U2) * 10;
    S521.EVSEPresentCurrent = 0;
    S521.EVSEMaximumCurrentLimit = EVSE_MAX_CURRENT * 10;
    S521.EVSEMaximumVoltageLimit = EVSE_MAX_VOLTAGE * 10;

    if (pstate_timer_elapsed == false)
    {
     if (PSTATE_TIMER(TPS * 5)) pstate_timer_elapsed = true;
    }

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if (((AC1_RELAY_OFF) && (AC2_RELAY_OFF)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC12_NON;
    }
    else if (((INVERTER1_OFF) && (INVERTER2_OFF)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_INV12_NRDY;
    }
    else if ((DC_RELAY_OFF) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_ON;
    }
//    else if ((DC_OUT_FAULT) && (pstate_timer_elapsed))
//    {
//
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
//     int_err_code = DC_OUT_SHORT;
//    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;            // Status na EVSE_READY
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_CURRENT_DEMAND:
   {
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Za³¹czamy stycznik DC
     output(ON, DC_RELAY, EOF); delay_ms(50);
     // Za³¹czamy inwertery
     output(ON, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);

     S520.EVSEProcessing = FINISHED;             // Operacja  zakoñczona
     S520.EVSEProgState = PSTATE_CURRENT_DEMAND; // Podtwierdzamy stan PSTATE_CURRENT_DEMAND
     S520.EVSEStatusCode = EVSE_READY;           // Status na EVSE_READY

     test_period = sys_timer_count();
     restarts_enable = false;
     restart1_flag = restart1_soft = restart1_hard = false;
     restart2_flag = restart2_soft = restart2_hard = false;

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    if (pstate_timer_elapsed == false)
    {
     if (PSTATE_TIMER(TPS * 5)) pstate_timer_elapsed = true;
    }

    inv_U = S621.EVTargetVoltage / 10;
    if (inv_U > S621.EVMaximumVoltageLimit / 10) inv_U = S621.EVMaximumVoltageLimit / 10;
    inv_I = S621.EVTargetCurrent;
    if (inv_I > S621.EVMaximumCurrentLimit) inv_I = S621.EVMaximumCurrentLimit;

    // Aktualizujemy pomiary
    S521.EVSEPresentVoltage = MAX(real_U1, real_U2) * 10;
    S521.EVSEPresentCurrent = (real_I1 + real_I2) * 10;
    S521.EVSEMaximumCurrentLimit = EVSE_MAX_CURRENT * 10;
    S521.EVSEMaximumVoltageLimit = EVSE_MAX_VOLTAGE * 10;

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if (((AC1_RELAY_OFF) && (AC2_RELAY_OFF)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC12_NON;
    }
    else if (((INVERTER1_OFF) && (INVERTER2_OFF)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_INV12_NRDY;
    }
    else if ((DC_RELAY_OFF) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_ON;
    }
    else if ((DC_OUT_FAULT) && (pstate_timer_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_OUT_SHORT;
    }
    else if ((DC_ISO_FAULT) && (pstate_timer_elapsed))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     S520.EVSEIsolationStatus = ISO_FAULT;   // Stan FAULT
     int_err_code = DC_ISO_FAILER;
    }
    else if ((DC_ISO_WARN) && (pstate_timer_elapsed))
    {
     S520.EVSEIsolationStatus = ISO_WARNING; // Stan WARNING
     int_err_code = DC_ISO_WARNING;
    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;       // Status na EVSE_READY
     S520.EVSEIsolationStatus = ISO_VALID;   // Stan VALID

     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_STOP_CHARGE:
   {
    // Sekwencja wykonywana jednorazowo na wiejœciu do stanu
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Od³¹czamy inwerter
     output(OFF, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy stycznik DC
     output(OFF, DC_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy styczniki AC1, AC2
     output_off(2, AC1_RELAY, AC2_RELAY); delay_ms(50);

     S520.EVSEProcessing = FINISHED;              // Operacja  zakoñczona
     S520.EVSEProgState = PSTATE_STOP_CHARGE;     // Podtwierdzamy stan PSTATE_PRECHARGE
     S520.EVSEStatusCode = EVSE_READY;            // Status na EVSE_READY

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if (((AC1_RELAY_ON) || (AC2_RELAY_ON)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (AC1_RELAY_ON && AC2_RELAY_ON) ? CHECK_AC12_ON : (AC1_RELAY_ON) ? CHECK_AC1_ON : CHECK_AC2_ON;
    }
    else if (((INVERTER1_ON) || (INVERTER2_ON)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (INVERTER1_ON && INVERTER2_ON) ? CHECK_INV12_RDY : (INVERTER1_ON) ? CHECK_INV1_RDY : CHECK_INV2_RDY;
    }
    else if ((DC_RELAY_ON) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_OFF;
    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;            // Status na EVSE_READY
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_WELD_DETECTION:
   {
    // Sekwencja wykonywana jednorazowo na wiejœciu do stanu
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Od³¹czamy inwerter
     output(OFF, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy stycznik DC
     output(OFF, DC_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy styczniki AC1, AC2
     output_off(2, AC1_RELAY, AC2_RELAY); delay_ms(50);

     S520.EVSEProcessing = FINISHED;              // Operacja  zakoñczona
     S520.EVSEProgState = PSTATE_WELD_DETECTION;  // Potwierdzamy PSTATE_WELD_DETECTION
     S520.EVSEStatusCode = EVSE_READY;            // Status na EVSE_READY

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    S521.EVSEPresentVoltage = ((real_U1 > real_U2) ? real_U1 : real_U2) * 10;
    S521.EVSEPresentCurrent = (real_I1 + real_I2) * 10;
    S521.EVSEMaximumCurrentLimit = EVSE_MAX_CURRENT * 10;
    S521.EVSEMaximumVoltageLimit = EVSE_MAX_VOLTAGE * 10;

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if (((AC1_RELAY_ON) || (AC2_RELAY_ON)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (AC1_RELAY_ON && AC2_RELAY_ON) ? CHECK_AC12_ON : (AC1_RELAY_ON) ? CHECK_AC1_ON : CHECK_AC2_ON;
    }
    else if (((INVERTER1_ON) || (INVERTER2_ON)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (INVERTER1_ON && INVERTER2_ON) ? CHECK_INV12_RDY : (INVERTER1_ON) ? CHECK_INV1_RDY : CHECK_INV2_RDY;
    }
    else if ((DC_RELAY_ON) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_OFF;
    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;            // Status na EVSE_READY
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_SHUT_OFF:
   {
    // Sekwencja wykonywana jednorazowo na wiejœciu do stanu
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Od³¹czamy inwerter
     output(OFF, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy stycznik DC
     output(OFF, DC_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy styczniki AC1, AC2
     output_off(2, AC1_RELAY, AC2_RELAY); delay_ms(50);

     S520.EVSEProcessing = FINISHED;             // Operacja  zakoñczona
     S520.EVSEProgState = PSTATE_SHUT_OFF;       // Zg³asamy przejœcie do PSTATE_SHUT_OFF
     S520.EVSEStatusCode = EVSE_READY;           // Status na EVSE_READY

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    if (EMERGENCY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if (((AC1_RELAY_ON) || (AC2_RELAY_ON)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (AC1_RELAY_ON && AC2_RELAY_ON) ? CHECK_AC12_ON : (AC1_RELAY_ON) ? CHECK_AC1_ON : CHECK_AC2_ON;
    }
    else if (((INVERTER1_ON) || (INVERTER2_ON)) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = (INVERTER1_ON && INVERTER2_ON) ? CHECK_INV12_RDY : (INVERTER1_ON) ? CHECK_INV1_RDY : CHECK_INV2_RDY;
    }
    else if ((DC_RELAY_ON) && pstate_second_elapsed)
    {
     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     int_err_code = DC_RELAY_NOT_OFF;
    }
    else
    {
     S520.EVSEStatusCode = EVSE_READY;            // Status na EVSE_READY
     int_err_code = EVSE_NO_ERROR;
    }

    if (S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN)
    {
     if (EMERGENCY)  int_err_code = EMERGENCY_DOWN;
     else if (DOORS) int_err_code = DOORS_OPENED;
    }
   }
   break;

   case PSTATE_ERROR:
   {
    if (PSTATE_ENTER)
    {
     enter_state = master_state;

     // Od³¹czamy inwerter
     output(OFF, INV1_RELAY, INV2_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy stycznik DC
     output(OFF, DC_RELAY, EOF); delay_ms(50);
     // Wy³¹czamy styczniki AC1, AC2
     output_off(2, AC1_RELAY, AC2_RELAY); delay_ms(50);

     rpt.buf[0] = exit_state;
     rpt.buf[1] = int_err_code;
     rpt.len = 2;
     if (TSIP_SENDING) tsip_output_proc(BARSA_DEVICE_CODE, CLC_ERROR, rpt.len);

     PSTATE_TIMER_INIT;
     pstate_second_elapsed = false;
    }

    if (pstate_second_elapsed == false)
    {
     if (PSTATE_TIMER(TPS)) pstate_second_elapsed = true;
    }

    if (pstate_timer_elapsed == false)
    {
     if (PSTATE_TIMER(TPS * 5))
     {
      S520.EVSEProcessing = FINISHED;
      pstate_timer_elapsed = true;
     }
    }

    S520.EVSEProcessing = FINISHED;                // Operacja  zakoñczona
    S520.EVSEProgState = PSTATE_ERROR;             // Potwierdzamy PSTATE_ERROR
    S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;

    S521.EVSEPresentVoltage = ((real_U1 > real_U2) ? real_U1 : real_U2) * 10;
    S521.EVSEPresentCurrent = (real_I1 + real_I2) * 10;
    S521.EVSEMaximumCurrentLimit = EVSE_MAX_CURRENT * 10;
    S521.EVSEMaximumVoltageLimit = EVSE_MAX_VOLTAGE * 10;

    if (EMERGENCY)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = EMERGENCY_DOWN;
    }
    else if (DOORS)
    {
//    S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = DOORS_OPENED;
    }
    else if (CAN_TOUT)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = CAN_RX_TOUT;
    }
    else if (FIRE_ERROR)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = CHECK_FIRE_ERROR;
    }
    else if (FIRE_ALARM)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = CHECK_FIRE_ALARM;
    }
    else if (NO_AC_SUPPLY)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = CHECK_AC_SUPPLY;
    }
    else if (AC_RELAY_OFF)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = CHECK_AC_RELAY;
    }
    else if ((BIM_SET_1_ALR) || (BIM_SET_2_ALR))
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = (BIM_SET_1_ALR && BIM_SET_2_ALR) ? CHECK_BIM12_OVH : (BIM_SET_1_ALR) ? CHECK_BIM1_OVH : CHECK_BIM2_OVH;
    }
    else if ((AC1_RELAY_ON) || (AC2_RELAY_ON))
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = (AC1_RELAY_ON && AC2_RELAY_ON) ? CHECK_AC12_ON : (AC1_RELAY_ON) ? CHECK_AC1_ON : CHECK_AC2_ON;
    }
    else if ((INVERTER1_ON) || (INVERTER2_ON))
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = (INVERTER1_ON && INVERTER2_ON) ? CHECK_INV12_RDY : (INVERTER1_ON) ? CHECK_INV1_RDY : CHECK_INV2_RDY;
    }
    else if (DC_RELAY_ON)
    {
//     S520.EVSEStatusCode = EVSE_ENERGENCY_SHUTDOWN;
     if (pstate_timer_elapsed) int_err_code = DC_RELAY_NOT_OFF;
    }
    else
    {
     if ((S620.EVRequestProgState == PSTATE_DEFAULT) && pstate_timer_elapsed)
     {
      master_state = exit_state = S620.EVRequestProgState;
      S520.EVSEStatusCode = EVSE_READY;
      int_err_code = EVSE_NO_ERROR;
     }
    }
   }
   break;
  }

  if ((S520.EVSEStatusCode == EVSE_ENERGENCY_SHUTDOWN) && (master_state != PSTATE_ERROR))
  {
   exit_state = master_state;
   master_state = PSTATE_ERROR;
  }

 if (StopRequest) S520.EVSEStatusCode = EVSE_SHUTDOWN;

// if ((old_EVRequestProgState != S620.EVRequestProgState) || (sys_timer_elapsed_count(sending_period) >= TPS))
 if ((o_master_state != master_state) || (sys_timer_elapsed_count(sending_period) >= TPS))
 {
  sending_period = sys_timer_count();

  if (restart1_flag) int_wrr_code |= INV1_RST_GOING; else int_wrr_code &= ~INV1_RST_GOING;
  if (restart2_flag) int_wrr_code |= INV2_RST_GOING; else int_wrr_code &= ~INV2_RST_GOING;
  if (restart1_soft) int_wrr_code |= INV1_RST_SOFT;  else int_wrr_code &= ~INV1_RST_SOFT;
  if (restart2_soft) int_wrr_code |= INV2_RST_SOFT;  else int_wrr_code &= ~INV2_RST_SOFT;
  if (restart1_hard) int_wrr_code |= INV1_RST_HART;  else int_wrr_code &= ~INV1_RST_HART;
  if (restart2_hard) int_wrr_code |= INV2_RST_HART;  else int_wrr_code &= ~INV2_RST_HART;

  rpt.buf[0] = o_master_state;
  rpt.buf[1] = master_state;
  *(int *) &rpt.buf[2] = (real_U1 > real_U2) ? real_U1 : real_U2;
  *(int *) &rpt.buf[4] = (real_I1 + real_I2) * 10;
  *(int *) &rpt.buf[6] = inv_U;
  *(int *) &rpt.buf[8] = inv_I;
  rpt.buf[10] = inputs1;
  rpt.buf[11] = inputs2;
  rpt.buf[12] = inputs3;
  *(unsigned int *)  &rpt.buf[13] = relays;

  rpt.buf[15] = S620.EVRequestProgState;
  rpt.buf[16] = exit_state;
  rpt.buf[17] = int_err_code;
  rpt.buf[18] = int_wrr_code;  // Ostrze¿enia
  rpt.len = 19;
  if (TSIP_SENDING) tsip_output_proc(BARSA_DEVICE_CODE, CLC_STATECH, rpt.len);

  rpt.buf[0] = INVERTER1_RDY;
  rpt.buf[1] = INVERTER2_RDY;
  *(int *) &rpt.buf[2] = real_U1;
  *(int *) &rpt.buf[4] = real_U2;
  *(int *) &rpt.buf[6] = real_I1;
  *(int *) &rpt.buf[8] = real_I2;
  *(int *) &rpt.buf[10] = inv_U;
  *(int *) &rpt.buf[12] = inv_I;
  *(unsigned int *)  &rpt.buf[13] = relays;
  rpt.len = 15;

  if (TSIP_SENDING) tsip_output_proc(BARSA_DEVICE_CODE, CLC_INVERR, rpt.len);
  TX2_shot = RX_TX_OK_LTIME; LTX2_ON; update_leds();

  if (TXT_DEBUG)
  {
   cprintf(DEBUG_PORT, "\r\nEVps %p ", states[S620.EVRequestProgState]);
   cprintf(DEBUG_PORT, "EVSEps %p ", states[S520.EVSEProgState]);
   cprintf(DEBUG_PORT, "%p ", process[S520.EVSEProcessing]);
   cprintf(DEBUG_PORT, "Cd:%p ", status[S520.EVSEStatusCode]);
   cprintf(DEBUG_PORT, "%p ", errors[int_err_code]);
   cprintf(DEBUG_PORT, "Uq %3d ",inv_U);
   cprintf(DEBUG_PORT, "Iq %3d ",inv_I / 10);
   cprintf(DEBUG_PORT, "U1 %3d ",real_U1);
   cprintf(DEBUG_PORT, "I1 %3d ",real_I1);
   cprintf(DEBUG_PORT, "U2 %3d ",real_U2);
   cprintf(DEBUG_PORT, "I2 %3d ",real_I2);
   cprintf(DEBUG_PORT, "Ms: %p", states[master_state]);
   cprintf(DEBUG_PORT, "WR: %02x", int_wrr_code);
  }
 }

 if ((old_EVRequestProgState != S620.EVRequestProgState) || (old_EVSEStatusCode != S520.EVSEStatusCode) || (old_EVSEProcessing != S520.EVSEProcessing)
     || (old_err_code != int_err_code))
 {
  old_EVRequestProgState = S620.EVRequestProgState;
  old_EVSEStatusCode = S520.EVSEStatusCode;
  old_EVSEProcessing = S520.EVSEProcessing;
  old_err_code = int_err_code;

  if (TXT_DEBUG)
  {
   cprintf(DEBUG_PORT, "\r\nEVps %p ", states[S620.EVRequestProgState]);
   cprintf(DEBUG_PORT, "EVSEps %p ", states[S520.EVSEProgState]);
   cprintf(DEBUG_PORT, "%p ", process[S520.EVSEProcessing]);
   cprintf(DEBUG_PORT, "Cd:%p ", status[S520.EVSEStatusCode]);
   cprintf(DEBUG_PORT, "%p\r\n", errors[int_err_code]);
  }
 }

 if (MAINT_STATE) S520.EVSE_STATUS |= EVSE_STATUS_MAINT; else S520.EVSE_STATUS &= ~EVSE_STATUS_MAINT;
 if (EMERGENCY)   S520.EVSE_STATUS |= EVSE_STATUS_EMERG; else S520.EVSE_STATUS &= ~EVSE_STATUS_EMERG;
 if (DOORS)       S520.EVSE_STATUS |= EVSE_STATUS_DOORS; else S520.EVSE_STATUS &= ~EVSE_STATUS_DOORS;

 if ((master_state == PSTATE_ERROR) || (rem_Enable == false))
 {
  output(OFF, L_READY, EOF);
  output(OFF, L_CHARGE, EOF);
  output(ON, L_ERROR, EOF);
 }
 else if ((master_state == PSTATE_CABLE_CHECK) ||
          (master_state == PSTATE_PRECHARGE) ||
          (master_state == PSTATE_CURRENT_DEMAND))
 {
  output(OFF, L_READY, EOF);
  output(ON, L_CHARGE, EOF);
  output(OFF, L_ERROR, EOF);
 }
 else
 {
  output(ON, L_READY, EOF);
  output(OFF, L_CHARGE, EOF);
  output(OFF, L_ERROR, EOF);
 }

 if (o_master_state != master_state)
 {
  o_master_state = master_state;
 }
}

void main(void)
{
 // Declare your local variables here
 sys_init();
 input_init();
 output_init();
 sys_timer_init();

// init_adc();

 // Inicjacja portu RS_485 MODBUS
 uart0_init();
 // Inicjacja portu OnBoard
 uart1_init();
 // Inicjacja portu RJ12
 uart2_init();
 // Inicjacja portu z³¹cza DB9
 uart3_init();

 le0 = le1 = le2 = le3 = le4 = le5 = 0xff; update_leds();
 sys_timer_wait(TPS);
 le0 = le1 = le2 = le3 = le4 = le5 = 0x00; update_leds();
 sys_timer_wait(TPS);

 buzzer(true); sys_timer_wait(TPS / 10); buzzer(false);

 sending_period = test_period = sys_timer_count();

 if (can_init(CAN16MHZ_20KBPS) != CAN_OK) CAN_STATUS |= CAN_INIT_FAIL;
 if (can_setNormalMode() != CAN_OK)       CAN_STATUS |= CAN_TRANS_FAIL;

 while (1)
 {
  #asm("wdr")

  update_leds();

  while (rx_counter2) tsip_input_proc(getchar2());

  if(rpt.status == TSIP_PARSED_FULL)
  {
   le0 = le1 = le2 = le3 = le4 = le5 = 0xff;
   rpt.status = TSIP_PARSED_EMPTY;

   if ((rpt.code == 1) && (rpt.buf[0] == GET_CHIP_ID))
   {
    update_leds();
    while(true);
   }
  }

  MODBUS_Serial_tick ();

  can_rx_state = can_rx_process();
  prog_state_task();
  can_tx_process();

  // Ustawianie pr¹dów
  if ((INVERTER1_RDY) && (INVERTER2_RDY))
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if ((MBM_WriteReg(INVERTER1_ADDR, I_SET, inv_I / 2) == MB_SUCCESS) && (MBM_WriteReg(INVERTER2_ADDR, I_SET, inv_I / 2) == MB_SUCCESS))
   {
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_wr_inv_ecnt++;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  }
  else if (INVERTER1_RDY)
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if (MBM_WriteReg(INVERTER1_ADDR, I_SET, inv_I) == MB_SUCCESS)
   {
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_wr_inv_ecnt++;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  }
  else if (INVERTER2_RDY)
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if (MBM_WriteReg(INVERTER2_ADDR, I_SET, inv_I) == MB_SUCCESS)
   {
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_wr_inv_ecnt++;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  }

  can_rx_state = can_rx_process();
  prog_state_task();
  can_tx_process();

  // Ustawianie napiêcia
  if ((INVERTER1_RDY) && (INVERTER2_RDY))
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if ((MBM_WriteReg(INVERTER1_ADDR, U_SET, inv_U) == MB_SUCCESS) && (MBM_WriteReg(INVERTER2_ADDR, U_SET, inv_U) == MB_SUCCESS))
   {
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_wr_inv_ecnt++;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  }
  else if (INVERTER1_RDY)
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if (MBM_WriteReg(INVERTER1_ADDR, U_SET, inv_U) == MB_SUCCESS)
   {
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_wr_inv_ecnt++;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  }
  else if (INVERTER2_RDY)
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if (MBM_WriteReg(INVERTER2_ADDR, U_SET, inv_U) == MB_SUCCESS)
   {
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_wr_inv_ecnt++;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  }

  can_rx_state = can_rx_process();
  prog_state_task();
  can_tx_process();

  // Odczyt pr¹du i napiêcia  INV 1
  if (INVERTER1_RDY)
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if ((MBM_ReadRegs(INVERTER1_ADDR, &real_I1, I_REAL, 1) == MB_SUCCESS) && (MBM_ReadRegs(INVERTER1_ADDR, &real_U1, U_REAL, 1) ==  MB_SUCCESS))
   {
    health_INV1 = 0;
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_rd_inv_ecnt++;
    if (health_INV1 < 5) health_INV1++; else real_I1 = real_U1 = 0;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  } else real_I1 = real_U1 = 0;
  // Odczyt pr¹du i napiêcia  INV 2
  if (INVERTER2_RDY)
  {
   TX0_shot = RX_TX_OK_LTIME; LTX0_ON; update_leds();
   if ((MBM_ReadRegs(INVERTER2_ADDR, &real_I2, I_REAL, 1) == MB_SUCCESS) && (MBM_ReadRegs(INVERTER2_ADDR, &real_U2, U_REAL, 1) ==  MB_SUCCESS))
   {
    health_INV2 = 0;
    RX0_shot = RX_TX_OK_LTIME; LRX0_ON; update_leds();
   }
   else
   {
    mb_rd_inv_ecnt++;
    if (health_INV2 < 5) health_INV2++; else real_I2 = real_U2 = 0;
    E2_shot = RX_TX_OK_LTIME; LE2_ON; update_leds();
   }
  } else real_I2 = real_U2 = 0;

  can_rx_state = can_rx_process();
  prog_state_task();
  can_tx_process();


  if (sys_timer_elapsed_count(test_period) >= TPS * 10)
  {
   test_period = sys_timer_count();
   restarts_enable = true;

#define INVERTER_RESTART

#ifdef INVERTER_RESTART
   if (master_state == PSTATE_CURRENT_DEMAND)
   {
    if ((S621.EVTargetCurrent > 12 * 10) && (real_I1 < 3)) // * 10 bo dostajemy przemno¿one przez 10 czyli
    {
     if (! restart1_soft)
     {
      if (! restart1_flag)
      {
       restar1_time = sys_timer_count();
       restart1_flag = true;
       output_off (1, INV1_RELAY);
      }
      else
      {
       if (sys_timer_elapsed_count(restar1_time) >= TPS * 20)
       {
        if (INVERTER1_RDY) output_on (1, INV1_RELAY);

        restart1_flag = false;
        restart1_soft = true;
       }
      }
     }
     else if (! restart1_hard)
     {
      if (! restart1_flag)
      {
       restar1_time = sys_timer_count();
       restart1_flag = true;
       output_off (2, AC1_RELAY, INV1_RELAY);
      }
      else
      {
       if (sys_timer_elapsed_count(restar1_time) >= TPS * 90)
       {
        output_on (1, AC1_RELAY);
        restart1_flag = false;
        restart1_hard = true;
       }
      }
     }
     else
     {
      if (INVERTER1_RDY)
      {
       output_on (1, INV1_RELAY);
       restart1_soft = false;  // Po twardym restarcie dopuszczamy jeszcze jeden miêkki
      }
     }
    }

    if ((S621.EVTargetCurrent > 12 * 10) && (real_I2 < 3)) // * 10 bo dostajemy przemno¿one przez 10 czyli
    {
     if (! restart2_soft)
     {
      if (! restart2_flag)
      {
       restart2_time = sys_timer_count();
       restart2_flag = true;
       output_off (1, INV2_RELAY);
      }
      else
      {
       if (sys_timer_elapsed_count(restart2_time) >= TPS * 20)
       {
        if (INVERTER2_RDY) output_on (1, INV2_RELAY);

        restart2_flag = false;
        restart2_soft = true;
       }
      }
     }
     else if (! restart2_hard)
     {
      if (! restart2_flag)
      {
       restart2_time = sys_timer_count();
       restart2_flag = true;
       output_off (2, AC2_RELAY, INV2_RELAY);
      }
      else
      {
       if (sys_timer_elapsed_count(restart2_time) >= TPS * 90)
       {
        output_on (1, AC2_RELAY);
        restart2_flag = false;
        restart2_hard = true;
       }
      }
     }
     else
     {
      if (INVERTER2_RDY)
      {
       output_on (1, INV2_RELAY);
       restart2_soft = false;  // Po twardym restarcie dopuszczamy jeszcze jeden miêkki
      }
     }
    }
   }
#endif
  }
 }
}



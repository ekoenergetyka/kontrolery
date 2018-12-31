#define CCU620 0x0620
#define CCU621 0x0621
#define CCU630 0x0630

#define CLC520 0x0520
#define CLC521 0x0521
#define CLC404 0x0404

#define ONGOING  0x01
#define FINISHED 0x00

#define ISO_INVALID     0
#define ISO_VALID       1
#define ISO_WARNING     2
#define ISO_FAULT       3

#define EVReadyToChargeState 0x01
#define EVReady              0x02

#define EVSE_NOTREADY           0
#define EVSE_READY              1
#define EVSE_SHUTDOWN           2
#define EVSE_UINTEREVNT         3
#define EVSE_ISO_MONIT_ACTIVE   4
#define EVSE_ENERGENCY_SHUTDOWN 5
#define EVSE_MULFUNCTION        6

#define PSTATE_DEFAULT          0
#define PSTATE_ERROR            1
#define PSTATE_SHUT_OFF         2
#define PSTATE_CABLE_CHECK      3
#define PSTATE_PRECHARGE        4
#define PSTATE_CURRENT_DEMAND   5
#define PSTATE_WELD_DETECTION   6
#define PSTATE_STUNDBY          7
#define PSTATE_STOP_CHARGE      8

#define E_PSTATE_DEFAULT          (1<<0)
#define E_PSTATE_ERROR            (1<<1)
#define E_PSTATE_SHUT_OFF         (1<<2)
#define E_PSTATE_CABLE_CHECK      (1<<3)
#define E_PSTATE_PRECHARGE        (1<<4)
#define E_PSTATE_CURRENT_DEMAND   (1<<5)
#define E_PSTATE_WELD_DETECTION   (1<<6)
#define E_PSTATE_STUNDBY          (1<<7)
#define E_PSTATE_STOP_CHARGE      (1<<8)

#define NO_ERROR                             0x00
#define FAILED_RESSTemperatureInhibit        0x01
#define FAILED_EVShiftPosition               0x02
#define FAILED_ChargerConnectorLockFault     0x03
#define FAILED_EVRESSMalfunction             0x04
#define FAILED_ChargingCurrentdifferential   0x05
#define FAILED_ChargingVoltageOutOfRange     0x06
#define Reserved_A                           0x07
#define Reserved_B                           0x08
#define Reserved_C                           0x09
#define FAILED_ChargingSystemIncompatibility 0x0a
#define NoData                               0x0b

#define EVSE_STATUS_MAINT  (1<<0)
#define EVSE_STATUS_EMERG  (1<<1)
#define EVSE_STATUS_DOORS  (1<<2)

// 620
struct S_CCU620
{
 unsigned char EVReadySate;
 unsigned char EVRequestProgState;
 unsigned char EVSOC;
 unsigned char EVErrorCodeType;
};

struct S_CCU621
{
 unsigned int EVTargetCurrent;
 unsigned int EVTargetVoltage;
 unsigned int EVMaximumCurrentLimit;
 unsigned int EVMaximumVoltageLimit;
};

struct S_CLC520
{
 unsigned char EVSEProcessing;
 unsigned char EVSEProgState;
 unsigned char EVSEIsolationStatus;
 unsigned char EVSEStatusCode;

 unsigned char EVSE_STATUS;
};

struct S_CLC521
{
 unsigned int EVSEPresentCurrent;
 unsigned int EVSEPresentVoltage;
 unsigned int EVSEMaximumCurrentLimit;
 unsigned int EVSEMaximumVoltageLimit;
};

struct S_CLC522
{
 unsigned int ErrorProgState;
 unsigned int VSEPresentVoltage;
 unsigned int EVSEMaximumCurrentLimit;
 unsigned int EVSEMaximumVoltageLimit;
};

extern flash char *states[];
extern flash char *status[];
extern flash char *process[];

void unp_CCU620(unsigned char dta[], struct S_CCU620 *b);
void unp_CCU621(unsigned char dta[], struct S_CCU621 *b);
void pac_CLC520(unsigned char dta[], struct S_CLC520 *h);
void pac_CLC521(unsigned char dta[], struct S_CLC521 *h);


#include <stdbool.h>
#include "Insys.h"


flash char *states[] =
{
 "DEFAULT      ",
 "ERROR        ",
 "SHUT_OFF     ",
 "CABLE CHECK  ",
 "PRECHARGE    ",
 "CURR DEMAND  ",
 "WELD_DET     ",
 "STUNDBY      ",
 "STOP_CHARGE  "
};

flash char *status[] =
{
 "NOTREADY           ",
 "READY              ",
 "SHUTDOWN           ",
 "UINTEREVNT         ",
 "ISO_MONIT_ACTIVE   ",
 "ENERGENCY_SHUTDOWN ",
 "MULFUNCTION        "
};

flash char *process[] =
{
 "FINISHED",
 "ONGOING "
};


void unp_CCU620(unsigned char dta[], struct S_CCU620 *b)
{
 b->EVReadySate = dta[0];
 b->EVRequestProgState = dta[1];
 b->EVSOC = dta[2];
 b->EVErrorCodeType = dta[3];
}

void unp_CCU621(unsigned char dta[], struct S_CCU621 *b)
{
 unsigned int ui;

 ui = dta[0]; ui <<= 8; ui |= dta[1]; b->EVTargetCurrent = ui;
 ui = dta[2]; ui <<= 8; ui |= dta[3]; b->EVTargetVoltage = ui;
 ui = dta[4]; ui <<= 8; ui |= dta[5]; b->EVMaximumCurrentLimit = ui;
 ui = dta[6]; ui <<= 8; ui |= dta[7]; b->EVMaximumVoltageLimit = ui;
}


void pac_CLC520(unsigned char dta[], struct S_CLC520 *h)
{
 dta[0] = h->EVSEProcessing;
 dta[1] = h->EVSEProgState;
 dta[2] = h->EVSEIsolationStatus;
 dta[3] = h->EVSEStatusCode;
 dta[4] = h->EVSE_STATUS;
}

void pac_CLC521(unsigned char dta[], struct S_CLC521 *h)
{
 dta[0] = h->EVSEPresentCurrent >> 8;
 dta[1] = h->EVSEPresentCurrent;
 dta[2] = h->EVSEPresentVoltage >> 8;
 dta[3] = h->EVSEPresentVoltage;
 dta[4] = h->EVSEMaximumCurrentLimit >> 8;
 dta[5] = h->EVSEMaximumCurrentLimit;
 dta[6] = h->EVSEMaximumVoltageLimit >> 8;
 dta[7] = h->EVSEMaximumVoltageLimit;
}

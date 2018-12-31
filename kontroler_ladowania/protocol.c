#include "protocol.h"
#include "uart2.h"

TSIPPKT rpt;

int calcrc(char *ptr, int count)
{
 int crc;
 char i;

 crc = 0;
 while (--count >= 0)
 {
  crc = crc ^ (int) *ptr++ << 8;
  i = 8;
  do
  {
   if (crc & 0x8000) crc = crc << 1 ^ 0x1021; else crc = crc << 1;
  } while(--i);
 }
 return (crc);
}


int updatecrc(int crc, char c)
{
 char i;

 crc = crc ^ (int) c << 8;
 i = 8;
 do
 {
  if (crc & 0x8000) crc = crc << 1 ^ 0x1021; else crc = crc << 1;
 } while(--i);

 return crc;
}

// formats a command for sending to TSIP receiver
void tsip_output_proc(char code, char subcode, unsigned char len)
{
 unsigned char *cbuf, *cbufend;
 unsigned int crc;

 putchar2(DLE);
 putchar2(code);

 crc = 0;
 crc = updatecrc(crc, code);

 putchar2(subcode);
 crc = updatecrc(crc, code);

 cbufend = rpt.buf + len;

 for (cbuf = rpt.buf; cbuf < cbufend; cbuf++)
 {
  if (*cbuf == DLE) putchar2(DLE);

  putchar2(*cbuf);
  crc = updatecrc(crc, *cbuf);
 }

 putchar2(crc);
 if ((crc & 0x00ff) == DLE) putchar2(DLE);
 putchar2(crc / 256);
 if ((crc / 256) == DLE) putchar2(DLE);

 putchar2(DLE);
 putchar2(ETX);
}

// reads bytes until serial buffer is empty or a complete report
// has been received; end of report is signified by DLE ETX.
//
void tsip_input_proc(unsigned char newbyte)
{
 switch (rpt.status)
 {
  case TSIP_PARSED_DLE_1:
   switch (newbyte)
   {
    // illegal TSIP IDs
    case 0: case ETX: rpt.len = 0; rpt.status = TSIP_PARSED_EMPTY;	break;

    // try normal message start again
    case DLE: rpt.len = 0; rpt.status = TSIP_PARSED_DLE_1; break;

    // legal TSIP ID; start message
    default: rpt.code = newbyte; rpt.len = 0; rpt.status = TSIP_PARSED_DATA; break;
   }
   break;

  case TSIP_PARSED_DATA:
   switch (newbyte)
   {
    // expect DLE or ETX next
    case DLE: rpt.status = TSIP_PARSED_DLE_2; break;

    // normal data byte  no change in rpt->status
    default: rpt.buf[rpt.len] = newbyte; rpt.len++; break;
   }
   break;

  case TSIP_PARSED_DLE_2:
   switch (newbyte)
   {
    // normal data byte
    case DLE: rpt.buf[rpt.len] = newbyte; rpt.len++; rpt.status = TSIP_PARSED_DATA; break;

    // end of message; return TRUE here.
    case ETX: rpt.status = TSIP_PARSED_FULL; break;

    // error: treat as TSIP_PARSED_DLE_1; start new report packet
    default: rpt.code = newbyte; rpt.len = 0; rpt.status = TSIP_PARSED_DATA;
   }
   break;

  case TSIP_PARSED_FULL: case TSIP_PARSED_EMPTY: default:
   switch (newbyte)
   {
    // normal message start
    case DLE: rpt.len = 0; rpt.status = TSIP_PARSED_DLE_1; break;

    // error: ignore newbyte
    default: rpt.len = 0; rpt.status = TSIP_PARSED_EMPTY;
   }
   break;
 }

 if (rpt.len > MAX_RPTBUF)
 {
  // error: start new report packet
  rpt.status = TSIP_PARSED_EMPTY;
  rpt.len = 0;
 }
}
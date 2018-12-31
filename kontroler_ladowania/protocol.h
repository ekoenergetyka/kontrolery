// Kod komunikacyjny urz¹dzenia
#define VOSLOCH_DEVICE_CODE 20
#define MEDCOM_DEVICE_CODE  21
#define BARSA_DEVICE_CODE   22

#define CAN_CLC2BUS 21
#define CAN_BUS2CLC 22
#define CLC_STATECH 23
#define CLC_ERROR   24
#define CLC_INVERR  25
#define CLC_MB_ERR  26
#define CLC_INV_ENG 27

#define CLC_INV1_ENG 28
#define CLC_INV2_ENG 29
#define CLC_INV1_ERR 30
#define CLC_INV2_ERR 31

// Rozmar bufora komunikacyjnego
#define MAX_RPTBUF (64)

// control characters for DATA packets
#define DLE (0x10)
#define ETX (0x03)

// values of TSIPPKT.status
#define TSIP_PARSED_EMPTY 0
#define TSIP_PARSED_FULL  1
#define TSIP_PARSED_DLE_1 2
#define TSIP_PARSED_DATA  3
#define TSIP_PARSED_DLE_2 4

// TSIP packets have the following structure, whether report or command.
typedef struct
{
 unsigned char
  len,             // size of buf; < MAX_RPTBUF unsigned chars
  status,          // TSIP packet format/parse status
  code;            // TSIP code
 unsigned char
  buf[MAX_RPTBUF]; // report or command string
} TSIPPKT;

extern TSIPPKT rpt;

int calcrc(char *ptr, int count) ;
void tsip_input_proc(unsigned char newbyte);
void tsip_output_proc(char code, char subcode, unsigned char len);

// void tsip_output_buffer(char *ptr, unsigned int count);


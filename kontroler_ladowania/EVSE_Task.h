#define L_GREEN       R6
#define L_BLUE        R7
#define L_RED         R8

#define AC1_RELAY     R3     // R1  Sterowanie stycznika AC1
#define AC2_RELAY     R4     // R2  Sterowanie stycznika AC2
#define INV1_RELAY    R5     // R3  Sterowanie falownika 1
#define INV2_RELAY    R6     // R4  Sterowanie falownika 2
#define DC_RELAY      R7     // R5  Sterowanie stycznika wyjœciowego DC
#define L_READY       R8     // R6  Sterowanie lampka GOTOWA
#define L_CHARGE      R9     // R7  Sterowanie lampka £ADUJE
#define L_ERROR       R10    // R8  Sterowanie lampka B£¥D - MRUGA DLA B£¥D z AUTOBUSU
#define UPS_OFF       R11    // R9  Sterowanie zasilaczem podtrzymuj¹cym
#define GSM_OFF       R12    // R10 Sterowanie wy³¹czaniem modu³u GSM
#define CPENABLE      R13    // Sterowanie sygna³em enable dla SPM pilot module

#define AC1_STATE     I1     // Potwierdzenie stycznika AC1
#define AC2_STATE     I2     // Potwierdzenie stycznika AC2
#define INV1_STATE    I3     // Potwierdzenie gotowoœci falownika 1
#define INV2_STATE    I4     // Potwierdzenie gotowoœci falownika 2
#define DC_STATE      I5     // Potwierdzenie stycznika wyjœciowego DC
#define AC0_STATE     I6     // Potwierdzenie stycznika g³ównego AC
#define MAINT_STATE   I7     // Informacja o pracy w stanie MAINTENANCE
#define NOT_USED      I8     // NOT USED
#define TXT_DBG       I8     // NOT USED

#define EMERGENCY     (!(I15))    // Informacja o grzybku
#define DOORS         (!(I16))    // Informacja o otwarciu drzwi

#define DC_ISO_WRN    I17    // Ostrze¿enie od Bendera
#define DC_ISO_ALR    I18    // Alarm od Bendera
#define BIM_SET_1     I19    // Wejœcie sumaryczne od bimetali i innych ze sterownika zamka - sumaryczne
#define BIM_SET_2     I20    // Wejœcie sumaryczne od bimetali i innych ze sterownika zamka - sumaryczne
#define SUPPLY_CTRL   I21    // Informacja o poprawnoœci zasilania z czujnika FAZ
#define FIRE_ALR      I22    // Alarm po¿arowy
#define FIRE_ERR      I23    // B³¹d ukladu po¿arowego
#define DC_OUT_CTRL   I24    // Nieprawid³owe napiêcie na wyjœciu

#define FIRE_ERROR      ( (FIRE_ERR))
#define NO_FIRE_ERROR   (!(FIRE_ERR))
#define FIRE_ALARM      ( (FIRE_ALR))
#define NO_FIRE_ALARM   (!(FIRE_ALR))
#define AC_SUPPLY       ( (SUPPLY_CTRL))
#define NO_AC_SUPPLY    (!(SUPPLY_CTRL))
#define AC_RELAY_ON     ( (AC0_STATE))
#define AC_RELAY_OFF    (!(AC0_STATE))
#define AC1_RELAY_ON    (AC1_STATE)
#define AC1_RELAY_OFF   (!(AC1_STATE))
#define AC2_RELAY_ON    (AC2_STATE)
#define AC2_RELAY_OFF   (!(AC2_STATE))
#define BIM_SET_1_OK    (BIM_SET_1)
#define BIM_SET_1_ALR   (!(BIM_SET_1))
#define BIM_SET_2_OK    (BIM_SET_2)
#define BIM_SET_2_ALR   (!(BIM_SET_2))

#define INP_SUPPLY_OK   (AC0_STATE)
#define INP_FUSE_OK     (INP_FUSE_ST)
#define INP_RELAY_OK    (INP_RELAY_ST)
#define INVERTER_READY  (INVERTER_ST)
#define INT_SUPPLY_OK   (INT_SUPPLY_ST)
#define EVSE_TIMEOUT(A) (sys_timer_elapsed_count(timeout_period) >= (A))

#define MAIN_RELAY_ON   (MAIN_RELAY_ST)
#define MAIN_RELAY_OFF  (!(MAIN_RELAY_ST))
#define INVERTER1_RDY   (INV1_STATE)
#define INVERTER1_ON    (INV1_STATE)
#define INVERTER1_OFF   (!(INV1_STATE))
#define INVERTER2_RDY   (INV2_STATE)
#define INVERTER2_ON    (INV2_STATE)
#define INVERTER2_OFF   (!(INV2_STATE))
#define DC_RELAY_ON     (DC_STATE)
#define DC_RELAY_OFF    (!(DC_STATE))

#define DC_OUT_OK       (DC_OUT_CTRL)
#define DC_OUT_FAULT    (!(DC_OUT_CTRL))

#define DC_ISO_OK       (!(DC_ISO_ALR))
#define DC_ISO_FAULT    (DC_ISO_ALR)
#define DC_ISO_WARN     (DC_ISO_WRN)
#define TXT_DEBUG       (TXT_DBG)
#define TSIP_SENDING    (true)

#define DEBUG_PORT COM1

#define EVSE_NO_ERROR      0
#define CHECK_FIRE_ERROR   1
#define CHECK_FIRE_ALARM   2
#define CHECK_AC_SUPPLY    3
#define CHECK_AC_RELAY     4
#define CHECK_BIM1_OVH     5
#define CHECK_BIM2_OVH     6
#define CHECK_BIM12_OVH    7
#define CHECK_AC1_ON       8
#define CHECK_AC2_ON       9
#define CHECK_AC12_ON     10
#define CHECK_AC1_NON     11
#define CHECK_AC2_NON     12
#define CHECK_AC12_NON    13
#define CHECK_INV1_RDY    14
#define CHECK_INV2_RDY    15
#define CHECK_INV12_RDY   16
#define CHECK_INV1_NRDY   17
#define CHECK_INV2_NRDY   18
#define CHECK_INV12_NRDY  19
#define DC_RELAY_NOT_OFF  20
#define DC_RELAY_NOT_ON   21
#define DC_OUT_SHORT      22
#define DC_ISO_FAILER     23
#define DC_ISO_WARNING    24
#define CAN_RX_TOUT       25
#define EMERGENCY_DOWN    26
#define DOORS_OPENED      27

#define INV1_RST_GOING  (1<<0)
#define INV1_RST_SOFT   (1<<1)
#define INV1_RST_HART   (1<<2)
#define INV2_RST_GOING  (1<<3)
#define INV2_RST_SOFT   (1<<4)
#define INV2_RST_HART   (1<<5)

flash char *errors[] =
{
 "NO ERROR     0",
 "FIRE  ERROR  1",
 "FIRE  ALARM  2",
 "AC    ERROR  3",
 "AC    OFF    4",
 "BIM1  OVH    5",
 "BIM2  OVH    6",
 "BIM12 OVH    7",
 "AC1   ON     8",
 "AC2   ON     9",
 "AC12  ON    10",
 "AC1   OFF   11",
 "AC2   OFF   12",
 "AC12  OFF   13",
 "INV1  RDY   14",
 "INV2  RDY   15",
 "INV12 RDY   16",
 "INV1  NRDY  17",
 "INV2  NRDY  18",
 "INV12 NRDY  19",
 "DC    ON    20",
 "DC    OFF   21",
 "DC  SHORT   22",
 "ISO FAILER  23",
 "ISO WARNING 24",
 "CAN RX TOUT 25",
 "EMERGENCY   26",
 "DOORS OPEN  27",

 "UNDEFINED     "
};

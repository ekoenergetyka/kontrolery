#define TOUT(A, B) (sys_timer_elapsed_count(A) >= (B))
#define LOWBYTE(A) (*(unsigned char*) &A))
#define HIBYTE(A)  (*(unsigned char*) &A + 1))

#define LOW 0
#define HIGH 1

#define INPUT(port,pin) DDR ## port &= ~(1<<pin)
#define OUTPUT(port,pin) DDR ## port |= (1<<pin)
#define CLEAR(port,pin) PORT ## port &= ~(1<<pin)
#define SET(port,pin) PORT ## port |= (1<<pin)
#define TOGGLE(port,pin) PORT ## port ^= (1<<pin)
#define READ(port,pin) (PIN ## port & (1<<pin))

#define __bswap_constant_16(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

#define __bswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |               \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define Endian16_Swap(value) \
      ((((UInt16)((value) & 0x00FF)) << 8) | \
         (((UInt16)((value) & 0xFF00)) >> 8))

#define Endian32_Swap(value) \
         ((((UInt32)((value) & 0x000000FF)) << 24) | \
         (((UInt32)((value) & 0x0000FF00)) << 8) | \
         (((UInt32)((value) & 0x00FF0000)) >> 8) | \
         (((UInt32)((value) & 0xFF000000)) >> 24))

#define MIN(a,b) (((a)<=(b))?(a):(b))
#define MAX(a,b) (((a)>=(b))?(a):(b))
#define MIN3(a,b,c) ((a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c) )
#define MAX3(a,b,c) ((a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c) )
#define BETWEEN(a,b,c) ((a)<=(b) and (b)<=(c))
#define TIMEOUT(A,B)(sys_timer_elapsed_count(A) >= (B))

typedef unsigned char  BYTE;        // 8-bit unsigned entity
typedef unsigned short WORD;        // 16-bit unsigned number
typedef unsigned int   UINT;        // machine sized unsigned number (preferred)
//typedef long           LONG;      // 32-bit signed number
typedef unsigned long  DWORD;       // 32-bit unsigned number
//typedef short          BOOL;      // BOOLean (0 or !=0)
typedef void*          POSITION;    // abstract iteration position


/* Examples

OUTPUT(D, 3); // port D, pin 3 as output
SET(D, 3); // set port D pin 3 to HIGH
CLEAR(D, 3); // set it to LOW

INPUT(B, 5);
if (READ(B, 5) == HIGH)

*/


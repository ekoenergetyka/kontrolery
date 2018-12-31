
#define LOW 0
#define HIGH 1

#define INPUT(port,pin) DDR ## port &= ~(1<<pin)
#define OUTPUT(port,pin) DDR ## port |= (1<<pin)
#define CLEAR(port,pin) PORT ## port &= ~(1<<pin)
#define SET(port,pin) PORT ## port |= (1<<pin)
#define TOGGLE(port,pin) PORT ## port ^= (1<<pin)
#define READ(port,pin) (PIN ## port & (1<<pin))

#define SwapEndianShort(in) ((in & 0xFF00) >> 8) | ((in & 0x00FF) << 8)

/* Examples

OUTPUT(D, 3); // port D, pin 3 as output
SET(D, 3); // set port D pin 3 to HIGH
CLEAR(D, 3); // set it to LOW

INPUT(B, 5);
if (READ(B, 5) == HIGH)

*/


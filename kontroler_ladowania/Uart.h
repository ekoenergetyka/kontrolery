//UART.h : header file for the UART project
//

#ifndef _UART_H_
#define _UART_H_

#define TXB8 0
#define RXB8 1
#define UPE  2
#define OVR  3
#define FE   4
#define UDRE 5
#define TXC  6
#define RXC  7

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<OVR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define SHIFT_REGISTER_EMPTY (1 << TXC)
#define RX_COMPLETE (1<<RXC)

#define UART0_BAUD 9600
#define UART1_BAUD 115200
#define UART2_BAUD 115200
#define UART3_BAUD 115200

#define COM0 0
#define COM1 1
#define COM2 2
#define COM3 3
#define LCD  4

// RS485 UART PORT 0
#define RS485 COM0
#endif

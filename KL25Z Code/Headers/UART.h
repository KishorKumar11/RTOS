#ifndef UART_HEADER
#define UART_HEADER

#include "foundation.h"

#define BAUD_RATE 9600
#define UART_TX_PORTE 22
#define UART_RX_PORTE 23

extern volatile tasks dir;

void initUART2(uint32_t);

void UART2_IRQHandler(void);

#endif
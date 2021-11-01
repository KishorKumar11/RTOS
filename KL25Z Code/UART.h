#ifndef UART_H
#define UART_H

#include "basic.h"

/*For UART*/
#define BAUD_RATE 9600
#define UART_TX_PORTE 22
#define UART_RX_PORTE 23 

void initUART2(uint32_t baud_rate);

void UART2_IRQHandler(void);

#endif
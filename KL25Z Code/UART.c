#include "MKL25Z4.h"
#include "UART.h"

volatile tasks dir = STOP;

void initUART2(uint32_t baud_rate) {
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	PORTE->PCR[22] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[22] |= PORT_PCR_MUX(4);
	
	PORTE->PCR[23] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[23] |= PORT_PCR_MUX(4);
	
	UART2->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
	
	uint32_t divisor = DEFAULT_SYSTEM_CLOCK / (32 * baud_rate);
	UART2->BDH = UART_BDH_SBR(divisor >> 8);
	UART2->BDL = UART_BDL_SBR(divisor);
	
	UART2->C1 = 0;
	UART2->S2 = 0;
	UART2->C3 = 0;
	
	UART2->C2 |= (UART_C2_RIE_MASK | UART_C2_TIE_MASK);
	UART2->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK);
	
	NVIC_SetPriority(UART2_IRQn, 1);
	NVIC_ClearPendingIRQ(UART2_IRQn);
	NVIC_EnableIRQ(UART2_IRQn);
}

void UART2_IRQHandler() {
	NVIC_ClearPendingIRQ(UART2_IRQn);
	
	if (UART2_S1 & UART_S1_RDRF_MASK) {
		uint8_t serialValue = UART2->D;
		if ((serialValue & 0b11101111) <= 9) {
			dir = (serialValue & 0b11101111);
		} else {
			
		}
	}
}
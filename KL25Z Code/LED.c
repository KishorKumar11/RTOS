#include "LED.h"

void initGreenLED() {
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
  PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[0] |= PORT_PCR_MUX(1);
	PORTB->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[1] |= PORT_PCR_MUX(1);
	PORTB->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[2] |= PORT_PCR_MUX(1);
	PORTB->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[3] |= PORT_PCR_MUX(1);
	PORTB->PCR[8] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[8] |= PORT_PCR_MUX(1);
	PORTB->PCR[9] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[9] |= PORT_PCR_MUX(1);
	PORTB->PCR[10] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[10] |= PORT_PCR_MUX(1);
	PORTB->PCR[11] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[11] |= PORT_PCR_MUX(1);
	
	PTB->PDDR |= (0b1111 << 8) | 0b1111;
	PTB->PCOR |= (0b1111 << 8) | 0b1111;
}

void initRedLED() {
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	
	PORTC->PCR[7] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[7] |= PORT_PCR_MUX(1);
	
	PTC->PDDR |= 1 << 7;
	PTC->PCOR |= 1 << 7;
}

void flash(int rate) {
	PTC->PCOR |= 1 << 7;
	
	PTC->PSOR |= 1 << 7;
	osDelay(rate);
	PTC->PCOR |= 1 << 7;
	osDelay(rate);
}

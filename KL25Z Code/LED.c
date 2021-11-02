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
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
	
	PORTA->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[1] |= PORT_PCR_MUX(1);
	
	PTA->PDDR |= 0b10;
	PTA->PCOR |= 0b11;
}

void flash(int rate) {
	PTA->PCOR |= 0b11;
	
	PTA->PSOR |= 0b10;
	osDelay(rate);
	PTA->PCOR |= 0b11;
	osDelay(rate);
}

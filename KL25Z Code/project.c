#include "MKL25Z4.h"
#include "constants.h"

volatile directions dir = STOP;
volatile int speed = 5;

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

void initPWM1() {
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
	
	TPM0->SC &= ~(TPM_SC_CPWMS_MASK | TPM_SC_CMOD_MASK | TPM_SC_PS_MASK);
	TPM0->SC |= (TPM_SC_TOIE_MASK | TPM_SC_PS(2));
	TPM0->CNT = 0;
	TPM0->MOD = 255;
	
	TPM0_C0SC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
	TPM0_C0SC |= (TPM_CnSC_CHIE_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK);
	
	TPM0_C1SC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
	TPM0_C1SC |= (TPM_CnSC_CHIE_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK);
	
	TPM0_C2SC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
	TPM0_C2SC |= (TPM_CnSC_CHIE_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK);
	
	TPM0_C3SC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK);
	TPM0_C3SC |= (TPM_CnSC_CHIE_MASK | TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK);
	
	TPM0_C0V = speed * 50;
	TPM0_C1V = speed * 50;
	TPM0_C2V = speed * 50;
	TPM0_C3V = speed * 50;
	
	NVIC_SetPriority(TPM0_IRQn, 2);
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	NVIC_EnableIRQ(TPM0_IRQn);
}

void startPWM() {
	TPM0->SC |= TPM_SC_CMOD(1);
}

void TPM0_IRQHandler() {
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	uint8_t status = TPM0->STATUS;
	TPM0->STATUS = 319;
	if (status & 1) { //front right
		switch (dir) {
			case FORWARDSTRAIGHT:
			case FORWARDLEFT:
			case FORWARDRIGHT:
			case SPINLEFT:
				PTA->PDOR &= ~((1 << 17));
				PTA->PDOR |= (1 << 16);
			break;
			case REVERSESTRAIGHT:
			case REVERSELEFT:
			case REVERSERIGHT:
			case SPINRIGHT:
				PTA->PDOR &= ~((1 << 16));
				PTA->PDOR |= (1 << 17);
			break;
			default:
				PTA->PDOR &= ~(0b11 << 16);
		}
	}
	if (status & (1 << 1)) { //front left
		switch (dir) {
			case FORWARDSTRAIGHT:
			case FORWARDLEFT:
			case FORWARDRIGHT:
			case SPINRIGHT:
				PTC->PDOR &= ~((1 << 16));
				PTC->PDOR |= (1 << 17);
			break;
			case REVERSESTRAIGHT:
			case REVERSELEFT:
			case REVERSERIGHT:
			case SPINLEFT:
				PTC->PDOR &= ~((1 << 17));
				PTC->PDOR |= (1 << 16);
			break;
			default:
				PTC->PDOR &= ~(0b11 << 16);
		}
	}
	if (status & (1 << 2)) { //back right
		switch (dir) {
			case FORWARDSTRAIGHT:
			case FORWARDLEFT:
			case FORWARDRIGHT:
			case SPINLEFT:
				PTD->PDOR &= ~(1 << 3);
				PTD->PDOR |= (1 << 1);
			break;
			case REVERSESTRAIGHT:
			case REVERSELEFT:
			case REVERSERIGHT:
			case SPINRIGHT:
				PTD->PDOR &= ~(1 << 1);
				PTD->PDOR |= (1 << 3);
			break;
			default:
				PTD->PDOR &= ~(0b1010);
		}
	}
	if (status & (1 << 3)) { //back left
		switch (dir) {
			case FORWARDSTRAIGHT:
			case FORWARDLEFT:
			case FORWARDRIGHT:
			case SPINRIGHT:
				PTD->PDOR &= ~(1);
				PTD->PDOR |= (1 << 2);
			break;
			case REVERSESTRAIGHT:
			case REVERSELEFT:
			case REVERSERIGHT:
			case SPINLEFT:
				PTD->PDOR &= ~(1 << 2);
				PTD->PDOR |= (1);
			break;
			default:
				PTD->PDOR &= ~(0b101);
		}
	}
	if (status & (1 << 7)) { //overflow
		PTA->PDOR &= ~((1 << 16) | (1 << 17));
		PTC->PDOR &= ~((1 << 16) | (1 << 17));
		PTD->PDOR &= ~(0b1111);
	}
}

void initMotors() {
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK);
	
	PORTA->PCR[16] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[16] |= PORT_PCR_MUX(1);
	
	PORTA->PCR[17] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[17] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[16] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[16] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[17] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[17] |= PORT_PCR_MUX(1);
	
	PORTD->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[0] |= PORT_PCR_MUX(1);
	
	PORTD->PCR[1] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[1] |= PORT_PCR_MUX(1);
	
	PORTD->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[2] |= PORT_PCR_MUX(1);
	
	PORTD->PCR[3] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[3] |= PORT_PCR_MUX(1);
	
	PTA->PDDR |= ((1 << 16) | (1 << 17));
	PTC->PDDR |= ((1 << 16) | (1 << 17));
	PTD->PDDR |= (0b1111);
}

int main(void) {
	initUART2(9600);
	initPWM1();
	initMotors();
	startPWM();
	while(1) {
		
	}
	
	return 1;
}
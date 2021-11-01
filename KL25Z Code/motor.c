#include "motor.h"

void initPWM0() {
	// Enable CLock Gating for Timer 0
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	// Select Clock for TPM module
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); //MCGFLLCLK clock or MCGPLLCLK/2
	
	//Set Modulo Value (48000000 / 128) / 7500 = 50Hz MOD value = 7500
	TPM0->MOD = 7500;
	
	/*Edge-Aligned PWM*/
	
	//Update Status&Control Registers and set bits to CMOD:01, PS:111 (prescalar 128)
	TPM0->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM0->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(1));
	TPM0->SC &= ~(TPM_SC_CPWMS_MASK); //Set to upcount PWM
	
	//Enable PWM on TPM0 Channel 0 -> PTD0
	TPM0_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM0_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
	//Enable PWM on TPM2 Channel 1 -> PTD1
	TPM0_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM0_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
	//Enable PWM on TPM2 Channel 2 -> PTD2
	TPM0_C2SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM0_C2SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
	//Enable PWM on TPM2 Channel 3 -> PTD3
	TPM0_C3SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM0_C3SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
}

void initPortD() {
	// 1. Set Pins PORT D, enable clk gating
	
	//Enable clk gating for PORT D
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	
	//Set MUX for PORT D
	PORTD->PCR[0] &= ~(PORT_PCR_MUX_MASK);
	PORTD->PCR[0] |= PORT_PCR_MUX(4);
	PORTD->PCR[1] &= ~(PORT_PCR_MUX_MASK);
	PORTD->PCR[1] |= PORT_PCR_MUX(4);
	PORTD->PCR[2] &= ~(PORT_PCR_MUX_MASK);
	PORTD->PCR[2] |= PORT_PCR_MUX(4);
	PORTD->PCR[3] &= ~(PORT_PCR_MUX_MASK);
	PORTD->PCR[3] |= PORT_PCR_MUX(4);
}

void initPWMMotors() {
	initPWM0();
	initPortD();
}

void stop() {
	LF_Pin = 0;
	RF_Pin = 0; 
	LR_Pin = 0;
	RR_Pin = 0;
}
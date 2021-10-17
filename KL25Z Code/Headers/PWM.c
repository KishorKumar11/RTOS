#include "PWM.h"

void initPortD() {
	// 1. Set Pins PORT D, enable clk gating
	
	//Enable clk gating for PORT D
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	
	//Set MUX for PORT D
	for (int i = 0; i < TOTAL_NO_PTD_PINS; i++) {
		PORTD->PCR[PTD_PIN(i)] &= ~(PORT_PCR_MUX_MASK);
		PORTD->PCR[PTD_PIN(i)] |= PORT_PCR_MUX(4);
	}
	
	// 2. Enable CLock Gating for Timer 0 and Timer 2
	SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	// 3. Select Clock for TPM module
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
	//Enable PWM on TPM2 Channel 4 -> PTD4
	TPM0_C4SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM0_C4SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
	//Enable PWM on TPM2 Channel 5 -> PTD5
	TPM0_C5SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM0_C5SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
}

void initPortA(){
	// 1. Set Pins PORT A, enable clk gating
	
	//Enable clk gating for PORT A
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
	
	//Set MUX for PORT A and PORT D
	for (int i = 1; i <= TOTAL_NO_PTA_PINS; i++) {
		PORTA->PCR[PTA_PIN(i)] &= ~(PORT_PCR_MUX_MASK);
		PORTA->PCR[PTA_PIN(i)] |= PORT_PCR_MUX(3); 
	}
	
	// 2. Enable CLock Gating for Timer 2
	SIM_SCGC6 |= SIM_SCGC6_TPM2_MASK;
	
	// 3. Select Clock for TPM module
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); //MCGFLLCLK clock or MCGPLLCLK/2
	
	//Set Modulo Value (48000000 / 128) / 7500 = 50Hz MOD value = 7500
	TPM2->MOD = 7500;
	
	/*Edge-Aligned PWM*/
	
	//Update Status&Control Registers and set bits to CMOD:01, PS:111 (prescalar 128)
	TPM2->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
	TPM2->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(1));
	TPM2->SC &= ~(TPM_SC_CPWMS_MASK); //Set to upcount PWM
	
	//Enable PWM on TPM2 Channel 0 -> PTA1
	TPM2_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM2_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
	//Enable PWM on TPM2 Channel 1 -> PTA2
	TPM2_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM2_C1SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
}

void initPWMMotors() {
	initPortA();
	initPortD();
}
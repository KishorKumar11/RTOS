#include "PWM.h"

volatile int speed = 5;

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
}

void startPWM() {
	TPM0->SC |= TPM_SC_CMOD(1);
}
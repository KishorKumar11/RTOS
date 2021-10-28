/**
* TPM2_CH0 -> Rear-Left forward
* TPM2_CH1 -> Rear-Left reverse
* TPM0_CH4 -> Front-Left forward
* TPM0_CH5 -> Front-Left reverse
* TPM0_CH0 -> Rear-Right forward
* TPM0_CH2 -> Rear-Right reverse
* TPM0_CH3 -> Front-Right forward
* TPM0_CH1 -> Front-Right reverse
*/
 
#include "MKL25Z4.h"
/* Default Core Clk Freq is 20.97152MHz */
// Current code will run at 48 MHz core clk freq

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "constants.h"

/*For UART*/
#define BAUD_RATE 9600
#define UART_TX_PORTE 22
#define UART_RX_PORTE 23

/*For Motor*/
#define LF_Pin TPM0_C2V
#define LR_Pin TPM0_C0V
#define RF_Pin TPM0_C3V
#define RR_Pin TPM0_C1V

volatile direction dir = 0;
volatile int speed = 7000;

#define QUEUE_SIZE 3

osSemaphoreId_t mainSem;
osSemaphoreId_t moveSem;



const osThreadAttr_t veryHighPriority = {
		.priority = osPriorityHigh4
};

const osThreadAttr_t highPriority = {
		.priority = osPriorityAboveNormal
};

const osThreadAttr_t aboveNormalPriority = {
		.priority = osPriorityAboveNormal
};

const osThreadAttr_t normalPriority = {
		.priority = osPriorityAboveNormal
};

typedef struct {
	uint8_t message;
} Message_Object;

/*
static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}*/

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
	
	UART2->C2 |= (UART_C2_RIE_MASK);
	UART2->C2 |= (UART_C2_RE_MASK);
	
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
			dir = serialValue;
		}
	}
	osSemaphoreRelease(moveSem);
}

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


void tBrain (void* argument) {
	for(;;) {
		
		
	}
}

void stop() {
	LF_Pin = 0;
	RF_Pin = 0; 
	LR_Pin = 0;
	RR_Pin = 0;
}

void tMotorControl (void *argument) {
	//LF: PTD2 C2V
	//LR: PTD0 C0V
	//RF: PTD3 C3V
	//RR: PTD1 C1V
  for (;;) {
		osSemaphoreAcquire(moveSem, osWaitForever); 
		switch(dir) {
			case STOP:
				stop();
				break;
			case FORWARDSTRAIGHT:
				LF_Pin = speed; // 0xEA6 = 3750, basically half of 7500 for 50% duty cycle
				RF_Pin = speed;
				break;
			case FORWARDLEFT:
				LF_Pin = speed * 0.8;
				RF_Pin = speed;
				break;
			case FORWARDRIGHT:
				LF_Pin = speed;
				RF_Pin = speed * 0.8;
				break;
			case REVERSESTRAIGHT:
				LR_Pin = speed;
				RR_Pin = speed;
				break;
			case REVERSELEFT:
				LR_Pin = speed * 0.8;
				RR_Pin = speed;
				break;
			case REVERSERIGHT:
				LR_Pin = speed;
				RR_Pin = speed * 0.8;
				break;
			case SPINLEFT:
				LR_Pin = speed;
				RF_Pin = speed;
				break;
			case SPINRIGHT:
				LF_Pin = speed;
				RR_Pin = speed;
				break;
			default:
				stop();
				break;
		}
	}
}

void tLED (void* Argument) {
	
	
}

void tAudio (void* Argument) {
	
	
}




int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initUART2(BAUD_RATE);
	initPWMMotors();
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	
	osSemaphoreId_t moveSem = osSemaphoreNew(1, 0, NULL);
	
	osMessageQueueId_t brainMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(Message_Object), NULL);
	osMessageQueueId_t motorMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(Message_Object), NULL);
	osMessageQueueId_t redLedMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(Message_Object), NULL);
	osMessageQueueId_t greenLedMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(Message_Object), NULL);
	osMessageQueueId_t audioMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(Message_Object), NULL);

	osThreadNew(tBrain, NULL, &veryHighPriority);
	osThreadNew(tMotorControl, NULL, &highPriority);
	osThreadNew(tAudio, NULL, &aboveNormalPriority);
	osThreadNew(tLED, NULL, &normalPriority);
	
  osKernelStart();                      // Start thread execution
  for (;;) {}
}

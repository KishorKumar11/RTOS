#include "MKL25Z4.h"
/* Default Core Clk Freq is 20.97152MHz */
// Current code will run at 48 MHz core clk freq

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "constants.h"
#include "sound.h"

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

osMessageQueueId_t brainMessageQueue;
osMessageQueueId_t motorMessageQueue;
osMessageQueueId_t redLedMessageQueue;
osMessageQueueId_t greenLedMessageQueue;
osMessageQueueId_t audioMessageQueue;

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
} MessageObjectType;

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
		MessageObjectType messageObject;
		messageObject.message = serialValue;
		osMessageQueuePut(brainMessageQueue, &messageObject, 0, 0);
	}
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


void initLED() {
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

void tBrain (void* argument) {
	for(;;) {
		MessageObjectType messageObject;
		osMessageQueueGet(brainMessageQueue, &messageObject, NULL, osWaitForever);
		uint8_t message = messageObject.message;
		if ((message >> 4) == 0x1) { //motor control instructions
			MessageObjectType motorMessage;
			motorMessage.message = (message & 0x0F); //only the 4 LSB
			osMessageQueuePut(motorMessageQueue, &motorMessage, 0, 0);
			if (motorMessage.message) { //if its moving, todo add case for incresae/decrease speed
				MessageObjectType greenLedMessage;
				greenLedMessage.message = 0x2;
				osMessageQueuePut(greenLedMessageQueue, &greenLedMessage, 0, 0);
			} else {
				MessageObjectType greenLedMessage;
				greenLedMessage.message = 0x0;
				osMessageQueuePut(greenLedMessageQueue, &greenLedMessage, 0, 0);
			}
		}
		if ((message >> 4) == 0x2) { //internet connection message
			MessageObjectType greenLedMessage;
			greenLedMessage.message = 0x1;
			osMessageQueuePut(greenLedMessageQueue, &greenLedMessage, 0, 0);
			MessageObjectType audioMessage;
			audioMessage.message = 0x1;
			osMessageQueuePut(audioMessageQueue, &audioMessage, 0, 0);
		}
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
		MessageObjectType messageObject;
		osMessageQueueGet(motorMessageQueue, &messageObject, NULL, osWaitForever); 
		switch(messageObject.message) {
			case 0x0: //STOP
				stop();
				break;
			case 0x1: //FORWARDSTRAIGHT
				LF_Pin = speed; // 0xEA6 = 3750, basically half of 7500 for 50% duty cycle
				RF_Pin = speed;
				break;
			case 0x2: //FORWARDLEFT
				LF_Pin = speed * 0.8;
				RF_Pin = speed;
				break;
			case 0x3: //FORWARDRIGHT
				LF_Pin = speed;
				RF_Pin = speed * 0.8;
				break;
			case 0x5: //REVERSESTRAIGHT
				LR_Pin = speed;
				RR_Pin = speed;
				break;
			case 0x6: //REVERSELEFT
				LR_Pin = speed * 0.8;
				RR_Pin = speed;
				break;
			case 0x7: //REVERSERIGHT
				LR_Pin = speed;
				RR_Pin = speed * 0.8;
				break;
			case 0x8: //SPINLEFT
				LR_Pin = speed;
				RF_Pin = speed;
				break;
			case 0x9: //SPINRIGHT
				LF_Pin = speed;
				RR_Pin = speed;
				break;
			default:
				stop();
				break;
		}
	}
}

void tGreenLED (void* Argument) {
	int state = 0;
	int led = 0;
	for(;;) {
		MessageObjectType messageObject;
		osStatus_t messageStatus = osMessageQueueGet(greenLedMessageQueue, &messageObject, 0, 0);
		if (messageStatus == osOK) {
			state = messageObject.message;
		}
		switch(state) {
			case 0: //stationary
				PTB->PSOR |= (0b1111 << 8) | 0b1111;
			break;
			case 1: //wifi connected
				PTB->PCOR |= (0b1111 << 8) | 0b1111;
				osDelay(250);
				PTB->PSOR |= (0b1111 << 8) | 0b1111;
				osDelay(250);
				PTB->PCOR |= (0b1111 << 8) | 0b1111;
				state = 0;
			break;
			case 2: //moving (Order is 3,2,1,0,11,10,9,8)
				PTB->PCOR |= (0b1111 << 8) | 0b1111;
				switch(led) {
					case 0: 
						PTB->PSOR |= (1 << 3);
						break;
					case 1: 
						PTB->PSOR |= (1 << 2);
						break;
					case 2: 
						PTB->PSOR |= (1 << 1);
						break;
					case 3: 
						PTB->PSOR |= (1 << 0);
						break;
					case 4: 
						PTB->PSOR |= (1 << 11);
						break;
					case 5: 
						PTB->PSOR |= (1 << 10);
						break;
					case 6: 
						PTB->PSOR |= (1 << 9);
						break;
					case 7: 
						PTB->PSOR |= (1 << 8);
						break;
					default: led = 0;
				}
				led = (led + 1) % 8;
			break;
			default: 
				state = 0;
		}
		osDelay(250);
	}
}

void tRedLED (void* Argument) {
	
	
}

void tAudio (void* Argument) {
	int song = 0;
	for (;;) {
		MessageObjectType messageObject;
		osStatus_t messageStatus = osMessageQueueGet(audioMessageQueue, &messageObject, 0, 0);
		if (messageStatus == osOK) {
			song = messageObject.message;
		}
		
		switch (song) {
			case 1: 
				playDumbNotes();
				offSound();
				osDelay(500);
				song = 2;
				break;
			case 2:
				playPinkPantherMelody();
				//playNarutoThemeMelody();
				break;
			case 3:
				playPinkPantherMelody();
				song = 0;
				break;
			default:
				osDelay(250);
		}
	}	
}


int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initUART2(BAUD_RATE);
	initPWMMotors();
	initLED();
	initSound();
	initAudioPWM();
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	
	brainMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	motorMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	redLedMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	greenLedMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	audioMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);

	osThreadNew(tBrain, NULL, &veryHighPriority);
	osThreadNew(tMotorControl, NULL, &highPriority);
	osThreadNew(tAudio, NULL, &aboveNormalPriority);
	osThreadNew(tGreenLED, NULL, &normalPriority);
	osThreadNew(tRedLED, NULL, &normalPriority);
	
  osKernelStart();                      // Start thread execution
  for (;;) {}
}

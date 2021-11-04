#include "UART.h"
#include "motor.h"
#include "sound.h"
#include "LED.h"
#include "ultrasonic.h"

volatile direction dir = 0;
volatile int speed = 7000;
volatile int isDone = 0;

#define QUEUE_SIZE 3
#define TURN_MULTIPLIER 0.5;

const osThreadAttr_t brainPriority = {
		.priority = osPriorityHigh4
};

const osThreadAttr_t autoPriority = {
		.priority = osPriorityHigh
};

const osThreadAttr_t motorPriority = {
		.priority = osPriorityAboveNormal
};

const osThreadAttr_t ultrasonicPriority = {
		.priority = osPriorityNormal
};

const osThreadAttr_t audioPriority = {
		.priority = osPriorityBelowNormal
};

const osThreadAttr_t LEDPriority = {
		.priority = osPriorityLow
};

osMessageQueueId_t brainMessageQueue;
osMessageQueueId_t motorMessageQueue;
osMessageQueueId_t redLedMessageQueue;
osMessageQueueId_t greenLedMessageQueue;
osMessageQueueId_t audioMessageQueue;
osMessageQueueId_t ultrasonicMessageQueue;
osSemaphoreId_t ultrasonicSemaphore;
osSemaphoreId_t autoStartSemaphore;
osSemaphoreId_t autoStopSemaphore;

void tBrain (void* argument) {
	for(;;) {
		int isManualMode = 1;
		
		MessageObjectType messageObject;
		osMessageQueueGet(brainMessageQueue, &messageObject, NULL, osWaitForever);
		uint8_t message = messageObject.message;
		if (isManualMode) {
			if ((message >> 4) == 0x1) { //motor control instructions
				MessageObjectType motorMessage;
				motorMessage.message = (message & 0x0F); //only the 4 LSB
				osMessageQueuePut(motorMessageQueue, &motorMessage, 0, 0);
				if (motorMessage.message) { //if its moving, todo add case for incresae/decrease speed
					MessageObjectType greenLedMessage;
					greenLedMessage.message = 0x2;
					osMessageQueuePut(greenLedMessageQueue, &greenLedMessage, 0, 0);
					MessageObjectType redLedMessage;
					redLedMessage.message = 0x1;
					osMessageQueuePut(redLedMessageQueue, &redLedMessage, 0, 0);
				} else {
					MessageObjectType greenLedMessage;
					greenLedMessage.message = 0x0;
					osMessageQueuePut(greenLedMessageQueue, &greenLedMessage, 0, 0);
					MessageObjectType redLedMessage;
					redLedMessage.message = 0x0;
					osMessageQueuePut(redLedMessageQueue, &redLedMessage, 0, 0);
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
			if ((message >> 4) == 0x3) { //Finish Level message
				isDone = 1;
			}
			if ((message >> 4) == 0x4) { //Reset Level message
				isDone = 0;
			}
			if ((message >> 4) == 0x5) { //Change to auto driving mode
				isManualMode = 0;
				osSemaphoreRelease(autoStartSemaphore);
			}
		}
		else {
			if ((message >> 4) == 0x6) { //Change to manual driving mode
				isManualMode = 1;
				osSemaphoreRelease(autoStopSemaphore);
			}
		}
	}
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
				LF_Pin = speed * TURN_MULTIPLIER;
				RF_Pin = speed;
				break;
			case 0x3: //FORWARDRIGHT
				LF_Pin = speed;
				RF_Pin = speed * TURN_MULTIPLIER;
				break;
			case 0x5: //REVERSESTRAIGHT
				LR_Pin = speed;
				RR_Pin = speed;
				break;
			case 0x6: //REVERSELEFT
				LR_Pin = speed * TURN_MULTIPLIER;
				RR_Pin = speed;
				break;
			case 0x7: //REVERSERIGHT
				LR_Pin = speed;
				RR_Pin = speed * TURN_MULTIPLIER;
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
	int state = 0;
	
	for (;;) {
		MessageObjectType messageObject;
		osStatus_t messageStatus = osMessageQueueGet(redLedMessageQueue, &messageObject, 0, 0);
		if (messageStatus == osOK) {
			state = messageObject.message;
		}
		
		switch (state) {
			case 0: //stationary
				flash(250);
				break;
			case 1: //moving
				flash(500);
				break;
			default:
				state = 0;
		}
	}
	
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
				playConnectionMelody();
				offSound();
				osDelay(500);
				song = 2;
				break;
			case 2:
				song = playNarutoThemeMelody(&isDone);
				break;
			case 3:
				playPinkPantherMelody();
				offSound();
				osDelay(500);
				song = 0;
				break;
			default:
				osDelay(250);
		}
	}	
}

volatile int ultrasonicRising = 1;
volatile uint32_t ultrasonicReading = 0;
void TPM2_IRQHandler(void) {
	NVIC_ClearPendingIRQ(TPM2_IRQn);
	TPM2_STATUS |= TPM_STATUS_CH1F_MASK;
	if (ultrasonicRising) { //start of echo pin pulse
		TPM2_CNT = 0; 
		ultrasonicRising = 0;
		//Configure Input Compare Mode on Channel 1 to respond to falling edge
		TPM2_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
		TPM2_C1SC |= TPM_CnSC_ELSB_MASK;
		
	} else { //end of echo pin pulse
		ultrasonicReading = TPM2_C1V * 0.1143;
		ultrasonicRising = 1;
		NVIC_DisableIRQ(TPM2_IRQn);
	}
}

void tUltrasonic(void* Argument) {
	for (;;) {
		osSemaphoreAcquire(ultrasonicSemaphore, osWaitForever);
		//Stop timer
		TPM2_SC &= ~TPM_SC_CMOD_MASK;
		
		//Enable Output Compare Mode on Channel 0, to generate 10 microsec high pulse when timer starts
		TPM2_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); 
		TPM2_C0SC |= (TPM_CnSC_ELSB_MASK | TPM_CnSC_MSA_MASK);
		
		//Configure Input Compare Mode on Channel 1 to respond to rising edge
		TPM2_C1SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
		TPM2_C1SC |= TPM_CnSC_ELSA_MASK;
		
		//Reset timer count value
		TPM2_CNT = 0;
		
		ultrasonicRising = 1;
		ultrasonicReading = 0;
		NVIC_EnableIRQ(TPM2_IRQn);
		NVIC_ClearPendingIRQ(TPM2_IRQn);
		TPM2_SC |= TPM_SC_CMOD(1);
		
		osDelay(100);
		MessageObjectType messageObject;
		messageObject.message = ultrasonicReading;
		osMessageQueuePut(ultrasonicMessageQueue, &messageObject, 0,0);
	}
}

void tAutoMode(void* Argument) {
	for(;;) {
		osSemaphoreAcquire(autoStartSemaphore, osWaitForever);
		
		int isBreak = 0;
		uint32_t distanceReading = 900;
		
		//Go forward
		MessageObjectType motorMessage;
		motorMessage.message = (0x01); //only the 4 LSB
		osMessageQueuePut(motorMessageQueue, &motorMessage, 0, 0);
		
		//While not near obstacle
		while (distanceReading > 100) {
			osStatus_t status = osSemaphoreAcquire(autoStopSemaphore, 1);
			//if stop auto mode commnand was triggered
			if (status != osErrorTimeout) {
				motorMessage.message = (0x00);
				osMessageQueuePut(motorMessageQueue, &motorMessage, 0, 0);
				isBreak = 1;
				break;
			}
			//get ultrasonic reading
			MessageObjectType ultrasonicMessage;
			osSemaphoreRelease(ultrasonicSemaphore);
			osMessageQueueGet(ultrasonicMessageQueue, &ultrasonicMessage, 0, 125);
			distanceReading = ultrasonicMessage.message;
		}
		if (isBreak) {
			continue;
		}
		motorMessage.message = (0x00);
		osMessageQueuePut(motorMessageQueue, &motorMessage, 0, 0);
	}
}

int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initUART2(BAUD_RATE);
	initPWMMotors();
	initGreenLED();
	initRedLED();
	initSound();
	initAudioPWM();
	initTPM2();
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	
	brainMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	motorMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	redLedMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	greenLedMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	audioMessageQueue = osMessageQueueNew(QUEUE_SIZE, sizeof(MessageObjectType), NULL);
	ultrasonicMessageQueue = osMessageQueueNew(1, sizeof(MessageObjectType), NULL);
	ultrasonicSemaphore = osSemaphoreNew(1, 0, NULL);
	autoStartSemaphore = osSemaphoreNew(1, 0, NULL);
	autoStopSemaphore = osSemaphoreNew(1, 0, NULL);

	osThreadNew(tBrain, NULL, &brainPriority);
	osThreadNew(tAutoMode, NULL, &autoPriority);
	osThreadNew(tMotorControl, NULL, &motorPriority);
	osThreadNew(tUltrasonic, NULL, &ultrasonicPriority);
	osThreadNew(tAudio, NULL, &audioPriority);
	osThreadNew(tGreenLED, NULL, &LEDPriority);
	osThreadNew(tRedLED, NULL, &LEDPriority);
	
  osKernelStart();                      // Start thread execution
  for (;;);
}

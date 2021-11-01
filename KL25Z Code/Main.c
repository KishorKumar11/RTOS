#include "UART.h"
#include "motor.h"
#include "sound.h"
#include "LED.h"

volatile direction dir = 0;
volatile int speed = 7000;
volatile int isDone = 0;

#define QUEUE_SIZE 3

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

osMessageQueueId_t brainMessageQueue;
osMessageQueueId_t motorMessageQueue;
osMessageQueueId_t redLedMessageQueue;
osMessageQueueId_t greenLedMessageQueue;
osMessageQueueId_t audioMessageQueue;

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
		if ((message >> 4) == 0x3) { //Finish Level message
			isDone = 1;
		}
		if ((message >> 4) == 0x4) { //Reset Level message
			isDone = 0;
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


int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initUART2(BAUD_RATE);
	initPWMMotors();
	initGreenLED();
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

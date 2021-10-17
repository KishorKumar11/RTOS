#include "UART.h"
#include "PWM.h"
#include "motors.h"

const osThreadAttr_t maxPriority = {
		.priority = osPriorityRealtime
};

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {
  for (;;) {
		osSemaphoreAcquire(mainSem, osWaitForever);
		switch(dir) {
			case STOP:
			case FORWARDSTRAIGHT:
			case FORWARDLEFT:
			case FORWARDRIGHT:
			case REVERSESTRAIGHT:
			case REVERSELEFT:
			case REVERSERIGHT:
			case SPINLEFT:
			case SPINRIGHT:
				osSemaphoreRelease(moveSem);
				break;
			default:
				break;
		}
	}
}

void motor_thread (void *argument) {
	
	// ...
  for (;;) {
		osSemaphoreAcquire(moveSem, osWaitForever);
		switch(dir) {
			case STOP:
				stop();
				dir = STOP;
				break;
			case FORWARDSTRAIGHT:
				TPM0_C0V = 0x0EA6; // 0xEA6 = 3750, basically half of 7500 for 50% duty cycle
				break;
			case FORWARDLEFT:
				TPM0_C1V = 0x0EA6;
				break;
			case FORWARDRIGHT:
				TPM0_C2V = 0x0EA6;
				break;
			case REVERSESTRAIGHT:
				TPM0_C3V = 0x0EA6;
				break;
			case REVERSELEFT:
				TPM0_C4V = 0x0EA6;
				break;
			case REVERSERIGHT:
				TPM0_C5V = 0x0EA6;
				break;
			case SPINLEFT:
				TPM2_C0V = 0x0EA6;
				break;
			case SPINRIGHT:
				TPM2_C1V = 0x0EA6;
				break;
			default:
				stop();
				break;
		}
	}
}

int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
	initUART2(BAUD_RATE);
	initPWMMotors();
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	
	// mainSem only released by UART2_IRQ Receive Interrupt
  // other Semaphores only released by brain_thread
	osSemaphoreId_t mainSem = osSemaphoreNew(1, 0, NULL);
	osSemaphoreId_t moveSem = osSemaphoreNew(1, 0, NULL);
	
  osThreadNew(app_main, NULL, &maxPriority);    // Create application main thread
	osThreadNew(motor_thread, NULL, NULL);
	
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
 

 


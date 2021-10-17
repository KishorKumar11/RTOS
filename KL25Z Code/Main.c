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

/*For PWM*/

/**
PORTD 0-5 == TPM0_CH 0-5 alt4
PORTA 1-2 == TPM2_CH 0-1 alt3 
*/
#define PTA_PIN(x) x
#define TOTAL_NO_PTA_PINS 2
#define PTD_PIN(x) x
#define TOTAL_NO_PTD_PINS 6

osSemaphoreId_t mainSem;
osSemaphoreId_t moveSem;

volatile tasks dir;

const osThreadAttr_t maxPriority = {
		.priority = osPriorityRealtime
};

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

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
		osSemaphoreRelease(mainSem);
		delay(0x80000);
		if ((serialValue & 0b11101111) <= 9) {
			dir = (serialValue & 0b11101111);		
		} else {
			
		}
	}
	
	
}

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

void stop() {
	TPM2_C0V = 0;
	TPM2_C1V = 0;
	TPM0_C0V = 0;
	TPM0_C1V = 0; 
	TPM0_C2V = 0;
	TPM0_C3V = 0;
	TPM0_C4V = 0;
	TPM0_C5V = 0;
}
 
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
		osSemaphoreAcquire(mainSem, osWaitForever); 
		switch(dir) {
			case STOP:
				stop();
				break;
			case FORWARDSTRAIGHT:
				TPM0_C0V = 0x0EA6; // 0xEA6 = 3750, basically half of 7500 for 50% duty cycle
				osDelay(100);
				break;
			case FORWARDLEFT:
				TPM0_C1V = 0x0EA6;
				osDelay(100);
				break;
			case FORWARDRIGHT:
				TPM0_C2V = 0x0EA6;
				osDelay(100);
				break;
			case REVERSESTRAIGHT:
				TPM0_C3V = 0x0EA6;
				osDelay(100);
				break;
			case REVERSELEFT:
				TPM0_C4V = 0x0EA6;
				osDelay(100);
				break;
			case REVERSERIGHT:
				TPM0_C5V = 0x0EA6;
				osDelay(100);
				break;
			case SPINLEFT:
				TPM2_C0V = 0x0EA6;
				osDelay(100);
				break;
			case SPINRIGHT:
				TPM2_C1V = 0x0EA6;
				osDelay(100);
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
	//osSemaphoreId_t moveSem = osSemaphoreNew(1, 0, NULL);
	
  //osThreadNew(app_main, NULL, &maxPriority);    // Create application main thread
	osThreadNew(motor_thread, NULL, NULL);
	
  osKernelStart();                      // Start thread execution
  for (;;) {}
}

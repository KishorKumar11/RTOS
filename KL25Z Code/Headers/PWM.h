#ifndef PWM_HEADER
#define PWM_HEADER

#include "foundation.h"

#define PTA_PIN(x) x
#define TOTAL_NO_PTA_PINS 2
#define PTD_PIN(x) x
#define TOTAL_NO_PTD_PINS 6

/**
PORTD 0-5 == TPM0_CH 0-5 alt4
PORTA 1-2 == TPM2_CH 0-1 alt3 
*/

void initPortD(void);

void initPortA(void);

void initPWMMotors(void);

#endif
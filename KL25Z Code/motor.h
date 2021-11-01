#ifndef MOTOR_H
#define MOTOR_H

#include "basic.h"

/*For Motor*/
#define LF_Pin TPM0_C2V
#define LR_Pin TPM0_C0V
#define RF_Pin TPM0_C3V
#define RR_Pin TPM0_C1V

void initPWM0(void);

void initPortD(void);

void initPWMMotors(void);

void stop(void);

#endif
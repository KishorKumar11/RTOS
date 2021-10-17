#include "motors.h"

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
#ifndef FOUNDATION_HEADER
#define FOUNDATION_HEADER

#include "MKL25Z4.h"
/* Default Core Clk Freq is 20.97152MHz */
// Current code will run at 48 MHz core clk freq

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "constants.h"

extern osSemaphoreId_t mainSem;
extern osSemaphoreId_t moveSem;

#endif
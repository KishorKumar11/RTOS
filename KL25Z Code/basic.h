#ifndef BASIC_H
#define BASIC_H

#include "MKL25Z4.h"
/* Default Core Clk Freq is 20.97152MHz */
// Current code will run at 48 MHz core clk freq

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "constants.h"

typedef struct {
	uint8_t message;
} MessageObject_t;

typedef struct {
	uint32_t message;
} LongMessageObject_t;

extern osMessageQueueId_t brainMessageQueue;
extern osMessageQueueId_t motorMessageQueue;
extern osMessageQueueId_t redLedMessageQueue;
extern osMessageQueueId_t greenLedMessageQueue;
extern osMessageQueueId_t audioMessageQueue;

#endif
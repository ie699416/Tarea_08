/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    clase4.c
 * @brief   Application entry point.
 */

#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t hours_semaphore;
SemaphoreHandle_t minutes_semaphore;

void seconds_task(void *arg) {

	int8_t seconds_counter = 0;
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(1000);
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, xPeriod);
		seconds_counter++;
		if (60 == seconds_counter) {
			xSemaphoreGive(minutes_semaphore);
			seconds_counter = 0;

		}

	}
}

void minutes_task(void *arg) {

	int8_t minutes_counter = 0;
	for (;;) {
		xSemaphoreTake(minutes_semaphore, portMAX_DELAY);
		minutes_counter++;
		if (60 == minutes_counter) {
			xSemaphoreGive(hours_semaphore);
			minutes_counter = 0;

		}

	}
}

void hours_task(void *arg) {

	int8_t hours_counter = 0;
	for (;;) {
		xSemaphoreTake(hours_semaphore, portMAX_DELAY);
		hours_counter++;
		if (24 == hours_counter) {
			hours_counter = 0;

		}

	}
}

//
//void alarm_task(void *arg)
//{
//	for(;;)
//	{
//
//	}
//}

void print_task(void *arg) {

	for (;;) {


	}
}

int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	CLOCK_EnableClock(kCLOCK_PortB);
	CLOCK_EnableClock(kCLOCK_PortA);

	minutes_semaphore = xSemaphoreCreateBinary();
	hours_semaphore = xSemaphoreCreateBinary();

	xTaskCreate(seconds_task, "seconds task", configMINIMAL_STACK_SIZE, NULL,
			configMAX_PRIORITIES - 2, NULL);
	xTaskCreate(minutes_task, "minutes task", configMINIMAL_STACK_SIZE, NULL,
			configMAX_PRIORITIES - 3, NULL);
	xTaskCreate(hours_task, "hours task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 4, NULL);
//xTaskCreate(led_task, "alarm task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-1, NULL);
	//xTaskCreate(print_task, "print task", configMINIMAL_STACK_SIZE, NULL,configMAX_PRIORITIES - 5, NULL);
//xTaskCreate(dummy, "dummy task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);

	vTaskStartScheduler();
	while (1) {

	}
	return 0;
}

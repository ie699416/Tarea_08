/**
 * @file    alarm.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define EVENT_60_SECONDS (1<<0)
#define EVENT_60_MINUTES (1<<1)

#define EVENT_ALARM_SECONDS (1<<2)
#define EVENT_ALARM_MINUTES (1<<3)
#define EVENT_ALARM_HOURS (1<<4)

#define ALARM_VALUE_SECONDS 1
#define ALARM_VALUE_MINUTES 0
#define ALARM_VALUE_HOURS 1

#define ALARM_MASK 0x01C

EventGroupHandle_t g_time_events;

void seconds_task(void *arg) {
	TickType_t xLastWakeTime;

	const TickType_t xPeriod = pdMS_TO_TICKS(1000);
	xLastWakeTime = xTaskGetTickCount();

	uint8_t seconds = 55;

	for (;;) {

		if (ALARM_VALUE_SECONDS == seconds) {
			xEventGroupSetBits(g_time_events, EVENT_ALARM_SECONDS);
		} else {
			xEventGroupClearBits(g_time_events, EVENT_ALARM_SECONDS);
		}

		vTaskDelayUntil(&xLastWakeTime, xPeriod);
		seconds++;


		if (60 == seconds)

		{
			seconds = 0;
			xEventGroupSetBits(g_time_events, EVENT_60_SECONDS);
		}

	}
}
void minutes_task(void *arg) {
	uint8_t minutes = 59;
	for (;;) {

		if (ALARM_VALUE_MINUTES == minutes) {
			xEventGroupSetBits(g_time_events, EVENT_ALARM_MINUTES);
		} else {
			xEventGroupClearBits(g_time_events, EVENT_ALARM_MINUTES);
		}

		xEventGroupWaitBits(g_time_events, EVENT_60_SECONDS, pdTRUE, pdTRUE,
		portMAX_DELAY);
		minutes++;


		if (60 == minutes) {
			minutes = 0;
			xEventGroupSetBits(g_time_events, EVENT_60_MINUTES);

		}
	}
}
void hours_task(void *arg) {
	uint8_t hours = 0;
	for (;;) {

		if (ALARM_VALUE_HOURS == hours) {
			xEventGroupSetBits(g_time_events, EVENT_ALARM_HOURS);
		} else {
			xEventGroupClearBits(g_time_events, EVENT_ALARM_HOURS);
		}

		xEventGroupWaitBits(g_time_events, EVENT_60_MINUTES, pdTRUE, pdTRUE,
		portMAX_DELAY);
		hours++;


		if (24 == hours) {
			hours = 0;
		}
	}
}

void print_task(void *arg) {
	while (1) {
		vTaskDelay(1000);
	}
}

void alarm_task(void *arg) {
	EventBits_t alarm;
	while (1) {
		alarm = xEventGroupGetBits(  g_time_events );
		if (ALARM_MASK == (ALARM_MASK & alarm)){
			PRINTF("ALARM");
		}


	}
}
int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	g_time_events = xEventGroupCreate();

	xTaskCreate(seconds_task, "Seconds", 300, NULL, configMAX_PRIORITIES - 1,
	NULL);
	xTaskCreate(minutes_task, "Minutes", 300, NULL, configMAX_PRIORITIES - 1,
	NULL);
	xTaskCreate(hours_task, "Hours", 300, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(print_task, "Printer", 300, NULL, configMAX_PRIORITIES - 1,
	NULL);
	xTaskCreate(alarm_task, "Alarm", 300, NULL, configMAX_PRIORITIES - 3,
	NULL);
	vTaskStartScheduler();

	for (;;) {

	}

	return 0;
}

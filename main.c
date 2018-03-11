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
#include "queue.h"
#include "semphr.h"


#define EVENT_60_SECONDS (1<<0)
#define EVENT_60_MINUTES (1<<1)

#define EVENT_ALARM_SECONDS (1<<2)
#define EVENT_ALARM_MINUTES (1<<3)
#define EVENT_ALARM_HOURS (1<<4)

#define ALARM_VALUE_SECONDS 1
#define ALARM_VALUE_MINUTES 0
#define ALARM_VALUE_HOURS 1

#define QUEUE_LENGTH 3
#define QUEUE_ITEM_SIZE sizeof(time_msg_t)

#define ALARM_MASK 0x01C

SemaphoreHandle_t mutex;


typedef enum {
	seconds_type, minutes_type, hours_type
} time_types_t;

typedef struct {
	time_types_t time_type;
	uint8_t value;
} time_msg_t;

QueueHandle_t g_time_mailbox;
EventGroupHandle_t g_time_events;

void seconds_task(void *arg) {
	TickType_t xLastWakeTime;
	time_msg_t xMessage = { seconds_type, 55 };

	const TickType_t xPeriod = pdMS_TO_TICKS(1000);
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		xQueueSendToBack(g_time_mailbox, &xMessage, 10);

		if (ALARM_VALUE_SECONDS == xMessage.value) {
			xEventGroupSetBits(g_time_events, EVENT_ALARM_SECONDS);
		} else {
			xEventGroupClearBits(g_time_events, EVENT_ALARM_SECONDS);
		}

		vTaskDelayUntil(&xLastWakeTime, xPeriod);
		xMessage.value++;

		if (60 == xMessage.value)

		{
			xMessage.value = 0;
			xEventGroupSetBits(g_time_events, EVENT_60_SECONDS);
		}

	}
}
void minutes_task(void *arg) {
	time_msg_t xMessage = { minutes_type, 59 };
	for (;;) {


		xQueueSendToBack(g_time_mailbox, &xMessage, 10);



		if (ALARM_VALUE_MINUTES == xMessage.value) {
			xEventGroupSetBits(g_time_events, EVENT_ALARM_MINUTES);
		} else {
			xEventGroupClearBits(g_time_events, EVENT_ALARM_MINUTES);
		}

		xEventGroupWaitBits(g_time_events, EVENT_60_SECONDS, pdTRUE, pdTRUE,
		portMAX_DELAY);
		xMessage.value++;

		if (60 == xMessage.value) {
			xMessage.value = 0;
			xEventGroupSetBits(g_time_events, EVENT_60_MINUTES);

		}
	}
}
void hours_task(void *arg) {
	time_msg_t xMessage = { hours_type, 0 };
	for (;;) {



		xQueueSendToBack(g_time_mailbox, &xMessage, 10);




		if (ALARM_VALUE_HOURS == xMessage.value) {
			xEventGroupSetBits(g_time_events, EVENT_ALARM_HOURS);
		} else {
			xEventGroupClearBits(g_time_events, EVENT_ALARM_HOURS);
		}

		xEventGroupWaitBits(g_time_events, EVENT_60_MINUTES, pdTRUE, pdTRUE,
		portMAX_DELAY);
		xMessage.value++;

		if (24 == xMessage.value) {
			xMessage.value = 0;
		}
	}
}

void print_task(void *arg) {

	time_msg_t xMessage = { 0, 0 };
	time_msg_t TimeReceived[3] = { { seconds_type, 0 }, { minutes_type, 0 }, {
			hours_type, 0 } };

	while (1) {
		if ( xQueueReceive( g_time_mailbox, &xMessage, portMAX_DELAY ) != pdPASS) {
			/* Nothing was received from the queue – even after blocking to wait
			 for data to arrive. */
		} else {
			switch (xMessage.time_type) {
			case seconds_type: {
 				TimeReceived[0].value = xMessage.value;
 				PRINTF("\n%d:%d:%d", TimeReceived[2].value, TimeReceived[1].value,
 											TimeReceived[0].value);
 				break;

			}
			case minutes_type: {
				TimeReceived[1].value = xMessage.value;
				break;

			}
			case hours_type: {
				TimeReceived[2].value = xMessage.value;
				break;

			}

			}


		}


	}
}

void alarm_task(void *arg) {
	EventBits_t alarm;
	while (1) {
		alarm = xEventGroupGetBits(g_time_events);
		if (ALARM_MASK == (ALARM_MASK & alarm)) {

			xSemaphoreTake(mutex,portMAX_DELAY);
			PRINTF("\nALARM");
			xEventGroupClearBits(g_time_events, ALARM_MASK);
			xSemaphoreGive(mutex);

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

	 mutex = xSemaphoreCreateMutex();

	g_time_events = xEventGroupCreate();
	g_time_mailbox = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

	xTaskCreate(seconds_task, "Seconds", 300, NULL, configMAX_PRIORITIES -2,
	NULL);
	xTaskCreate(minutes_task, "Minutes", 300, NULL, configMAX_PRIORITIES-1,
	NULL);
	xTaskCreate(hours_task, "Hours", 300, NULL, configMAX_PRIORITIES  ,
	NULL);
	xTaskCreate(print_task, "Printer", 300, NULL, configMAX_PRIORITIES - 4,
	NULL);
	xTaskCreate(alarm_task, "Alarm", 300, NULL, configMAX_PRIORITIES - 3,
	NULL);
	vTaskStartScheduler();

	for (;;) {

	}

	return 0;
}

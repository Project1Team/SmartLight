/*
 * main_task.c
 *
 * Created on: Sep 29, 2017
 * Author: namnguyen
 */

#include "../include/light_task.h"
#include "../include/main_task.h"
#include "mailbox.h"
#include "marky_network_task.h"

xTaskHandle Main_Task_Init(void *pArg)
{
	xTaskHandle maintask;

	xTaskCreate(Main_Task, "Main_Task", configMINIMAL_STACK_SIZE, pArg, tskIDLE_PRIORITY, maintask);

	return maintask;
}

void Main_Task(void *pArg)
{
	//task's mailbox
	Task_Mailbox mailbox;

	//init light task
	//Light_Init(&mailbox);

	//init network task
	MARKYT_Init(&mailbox);

	while(1)
	{
		vTaskDelay(100);
	}
}


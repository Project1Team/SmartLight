/*
 * light_task.c
 *
 * Created on: Sep 29, 2017
 * Author: namnguyen
 * This will handle anythings about the light ring - WS2812
 */

#include "../include/light_task.h"
#include "esp8266_arduino_ws2812.h"

xTaskHandle Light_Init(void *pArg)
{
	xTaskHandle light_task;

	xTaskCreate(Light_Task, "Light_Task", configMINIMAL_STACK_SIZE*2, pArg, tskIDLE_PRIORITY, light_task);

	return light_task;
}

void Light_Task(void *pArg)
{
	//task's mailbox
	Task_Mailbox* mailbox;

	//light data
	Task_Mailbox_Light oldLightData[4];

	//get the task's mailbox
	mailbox = (Task_Mailbox*)pArg;

	//init WS2812
	//numbers of led on the ring, dataIN PIN(ESP) and freq
	WS2812_Init(12, 5, NEO_GRB + NEO_KHZ800);
	WS2812_Begin();

	//light up first
	mailbox->light[0] = 255,		//Br
	mailbox->light[1] = 255,		//W
	mailbox->light[2] = 255,		//B
	mailbox->light[3] = 255,		//G
	mailbox->light[4] = 255,		//R
	//mailbox->light[5] = 255;

	Light_Update(mailbox->light);
	CopyLight(oldLightData, mailbox->light);

	while(1)
	{
		if(Check_Light_Different(oldLightData, mailbox->light))
		{
			Light_Update(mailbox->light);

			CopyLight(oldLightData, mailbox->light);
		}

		vTaskDelay(10);
	}
}

void CopyLight(Task_Mailbox_Light* light1, Task_Mailbox_Light* light2)
{
	int i = 0;
	for(i = 0; i < 6; i++)
		light1[i] = light2[i];
}

bool Check_Light_Different(Task_Mailbox_Light* light1, Task_Mailbox_Light* light2)
{
	int i = 0;
	for(i = 0; i < 6; i++)
		if(light1[i] != light2[i])
			return true;

	return false;
}

void Light_Update(Task_Mailbox_Light* light)
{
	int i = 0;

	//analyze light data
	//light: red green blue white brightness
	for(i=0; i<12; i++)
		WS2812_SetPixelColor( i, light[4], light[3], light[2], light[1] );	//Swipe

	WS2812_SetBrightness( light[0] );

	//Update pixel color
	//TODO why this function have to be called twice
	WS2812_Show();
	WS2812_Show();
}

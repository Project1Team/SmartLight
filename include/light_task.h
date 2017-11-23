/*
 * light_task.h
 *
 * Created on: Sep 29, 2017
 * Author: namnguyen
 * This will handle anythings about the light ring - WS2812
 */

#ifndef INCLUDE_LIGHT_TASK_H_
#define INCLUDE_LIGHT_TASK_H_

#include "esp_common.h"
#include "freertos/FreeRTOS.h"

#include "mailbox.h"

/*
 *@brief	init light task
 *@param	pArg:	this will be passed to light task (Light_Task) as a argument
 *@return   light handler (task handler)
 */
xTaskHandle Light_Init(void *pArg);


/*
 *@brief	main light task
 *@param	pArg:	argument for handler stuff (optional)
 */
void Light_Task(void *pArg);


/*
 * @brief	copy light
 * @param	light1:		destination of copy
 * @param	light2:		source of copy
 */
void CopyLight(Task_Mailbox_Light* light1, Task_Mailbox_Light* light2);


/*
 * @brief	compare two light
 * @param	light1:		light to compare
 * @param	light2:		light to compare
 * @return	true if two light is different
 */
bool Check_Light_Different(Task_Mailbox_Light* light1, Task_Mailbox_Light* light2);


/*
 * @brief	update light
 * @param	light to update (array of 6 element)
 * 			- exp: [red, green, blue, white, brightness]
 */
void Light_Update(Task_Mailbox_Light* light);

#endif /* INCLUDE_LIGHT_TASK_H_ */

/*
 * main_task.h
 *
 * Created on: Sep 29, 2017
 * Author: namnguyen
 * This will handle the main task, the task which be called first
 */

#ifndef INCLUDE_MAIN_TASK_H_
#define INCLUDE_MAIN_TASK_H_

#include "esp_common.h"
#include "freertos/FreeRTOS.h"


/*
 *@brief	init main task
 *@param	pArg:	this will be passed to main task as a argument
 *@return
 *	- main handler (task handler)
 */
xTaskHandle Main_Task_Init(void *pArg);

/*
 *@brief	main task
 *@param	pArg:	argument for handler stuff (optional)
 */
void Main_Task(void *pArg);

#endif /* INCLUDE_MAIN_TASK_H_ */

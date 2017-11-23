/*
 * marky_network_task.h
 *
 * Created on: Nov 2, 2017
 * This will handle task about the websocket communication between server-client
 */

#ifndef INCLUDE_MARKY_NETWORK_TASK_H_
#define INCLUDE_MARKY_NETWORK_TASK_H_

#include "esp_common.h"
#include "freertos/FreeRTOS.h"

//-----------------------------------------------------------------------

/*
 * @brief	marky task operation mode
 */
typedef enum MARKYT_OPERATIONMODE
{
	MARKYT_OP_Startup,				/*!< check if network is ok */
	MARKYT_OP_ConnectToServer,		/*!< connect to server */
	MARKYT_OP_TalkToServer,			/*!< communicate to server */
	MARKYT_OP_MAX
} MARKYT_OperationMode;

//-----------------------------------------------------------------------

/*
 *@brief	init marky (network) task
 *@param	pArg:	this will be passed to marky task (MARKYT_Task) as a argument
 *@return
 *	- marky handler (task handler)
 */
xTaskHandle MARKYT_Init(void *pArg);

/*
 *@brief	main marky task
 *@param	pArg:	argument for handler stuff (optional)
 */
void MARKYT_Task(void *pArg);

#endif /* INCLUDE_MARKY_NETWORK_TASK_H_ */

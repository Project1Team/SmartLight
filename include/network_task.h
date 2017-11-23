/*
 * network_task.h
 *
 * Created on: Nov 2, 2017
 * This will handle anythings about the network task connection
 */


#ifndef INCLUDE_NETWORK_TASK_H_
#define INCLUDE_NETWORK_TASK_H_

#include "esp_common.h"
#include "freertos/FreeRTOS.h"

#define NETWORKT_Mode_Develop

//-----------------------------------------------------------------------

/*
 * @brief	operation mode of network task
 */
typedef enum NETWORKT_OPERATIONMODE
{
	NETWORKT_OP_Startup,					/*!< idle state */
	NETWORKT_OP_ST_Connect_Wifi,			/*!< try to connect to wifi, also listen to any device try to connect to network's server */
	NETWORKT_OP_AP_Have_Client,				/*!< once user's device try to change wifi for bulb to connect */
	NETWORKT_OP_MAX
} NETWORKT_OperationMode;

/*
 * @brief	network status
 */
typedef enum NETWORKT_STATUS
{
	NETWORKT_ST_Wifi_Connection = 0x01,		/*!< wifi connection register (1 if connected, 0 if not connected) */
	NETWORKT_ST_MAX
} NETWORKT_Status;

#ifdef NETWORKT_Mode_Develop
/*
 * @brief	name and pass wifi to connect to
 */
#define NETWORKT_SSID		"Look like a boss"	/*! name of wifi <*/
#define NETWORKT_PASS		"motherfucker"		/*!, password of wifi */
#endif /* NETWORKT_Mode_Debug */

/*
 * @brief	network's server address
 */
#define NETWORK_ESP_WS_Address				"192.168.4.1"
#define NETWORK_ESP_WS_ServerPort			1705

//-----------------------------------------------------------------------

/*
 *@brief	init network (network) task
 *@param	pArg:	this will be passed to marky task (NETWORKT_Task) as a argument
 *@return
 *	- network handler (task handler)
 */
xTaskHandle NETWORKT_Init(void *pArg);

/*
 *@brief	main network task
 *@param	pArg:	argument for handler stuff (optional)
 */
void NETWORKT_Task(void *pArg);

/*
 * @brief	get network status holder of network task
 * @return	pointer to status of network condition
 */
NETWORKT_Status* NETWORKT_GetStatus(void);

#endif /* INCLUDE_NETWORK_TASK_H_ */

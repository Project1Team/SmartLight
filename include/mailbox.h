/*
 * mailbox.h
 *
 * Created on: Oct 5, 2017
 * Author: namnguyen
 * This will handle a struct to broadcast data between server and the light
 */

#ifndef INCLUDE_MAILBOX_H_
#define INCLUDE_MAILBOX_H_

#include "esp_common.h"

//-----------------------------------------------------------------------

/*
 * @brief	each task mail box data type
 */
//typedef uint16_t Task_Mailbox_Marky;
//typedef uint16_t Task_Mailbox_Main;
typedef uint8_t Task_Mailbox_Light;

/*
 * @brief	task's mailbox
 */
typedef struct TASK_MAILBOX
{
	//Task_Mailbox_Marky marky;				/*!< marky mailbox */
	//Task_Mailbox_Main maintask;			/*!< main task mailbox */
	Task_Mailbox_Light light[4];			/*!< light mailbox */
} Task_Mailbox;

//-----------------------------------------------------------------------



#endif /* INCLUDE_MAILBOX_H_ */

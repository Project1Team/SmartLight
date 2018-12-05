/*
 * marky_network_task.c
 *
 * Created on: Nov 2, 2017
 * This will handle task about the websocket communication between server-client
 */

#include "marky_network_task.h"

#include "mailbox.h"
#include "marky_network.h"
#include "network_task.h"
#include "uart.h"

xTaskHandle MARKYT_Init(void *pArg)
{
	xTaskHandle marky;

	xTaskCreate(MARKYT_Task, "MARKYT_Task", configMINIMAL_STACK_SIZE*6, pArg, tskIDLE_PRIORITY, marky);

	return marky;
}

void MARKYT_Task(void *pArg)
{
	MARKYT_OperationMode mode = MARKYT_OP_Startup;
	//network
	NETWORKT_Status* networkStatus;
	//server connection
	noPollConn* conn = NULL;
	bool connStatus = false;
	//server communication
	Task_Mailbox_Light recvData;
	//task's mailbox
	Task_Mailbox* mailbox;

	//get the task's mailbox
	mailbox = (Task_Mailbox*)pArg;

	//init network operation
	NETWORKT_Init(NULL);

	while(1)
	{
		switch(mode)
		{
		case MARKYT_OP_Startup:
			//init things
			networkStatus = NETWORKT_GetStatus();
			if( !(networkStatus == NULL) )
				//check if network is ok
				if(*networkStatus & NETWORKT_ST_Wifi_Connection)
					//switch to MARKYT_OP_ConnectToServer
					mode = MARKYT_OP_ConnectToServer;

			break;
		case MARKYT_OP_ConnectToServer:
			//connect to server
			if(MARKY_WS_LogintoServer(&conn) == true)
			{
				//set_conn(conn);
				mode = MARKYT_OP_TalkToServer;
			}

			//check if wifi is ok
			if( !(*networkStatus & NETWORKT_ST_Wifi_Connection) )
			{
				//close connection
				MARKY_WS_Close(&conn);
				//set_conn(conn);
				mode = MARKYT_OP_Startup;
			}

			break;
		case MARKYT_OP_TalkToServer:
			//comunicate to server
			//TODO	should keep mailbox->deedee immediate updated???
			//if(MARKY_WS_Recv(conn, mailbox->deedee) == true)
			//{
				//TODO	add {"reply":"_ok"} to this???
				//mailbox->deedee = recvData;
			//}
//			printf("MARKYT: %d\n", system_get_free_heap_size());
			// if(getUartData() != 0x00)
			// {
			// 	MARKY_WS_sendData(conn, getUartData());
			// 	setUartData(0x00);
			// }

			MARKY_WS_Recv(conn, mailbox->light);

			//check connection
			if (! nopoll_conn_is_ready(conn))
			{
				//close connection
				MARKY_WS_Close(&conn);
				//set_conn(conn);
				mode = MARKYT_OP_Startup;
			}

			//check if wifi is ok
			if( !(*networkStatus & NETWORKT_ST_Wifi_Connection) )
			{
				//close connection
				MARKY_WS_Close(&conn);
				//set_conn(conn);
				mode = MARKYT_OP_Startup;
			}

			break;
		default:

			break;
		}

		vTaskDelay(10);
	}
}


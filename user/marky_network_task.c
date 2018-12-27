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

	uint8_t dataReceived[2] = {0,0};

	uint8 byte_start = 0xFE;
	uint8 item1_data_received = 0x00;
	uint8 item2_data_received = 0x00;
	uint8 data_send2nRF[3] = {0x00, 0x00, 0x00};

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

			// if(checkFlagBuffer() == true)
			// {
			// 	MARKY_WS_sendData(conn, getDataFromBuffer());
			// 	setFlagBuffer(false);
			// }

			if(checkFlagSend() == true)
			{
				MARKY_WS_send2Byte(conn, getDataItem(1), getDataItem(0));
				setFlagSend(false);
			}

			if(MARKY_WS_RecvData(conn, dataReceived))
			{
				// set status
				if(dataReceived[2] == 0)
				{
					item2_data_received = item2_data_received | 0x10;
				}
				else if(dataReceived[2] == 1)
				{
					item2_data_received = item2_data_received & 0x01;
				}

				// set number switch of switch board
				if(dataReceived[1] == 1)
				{
					item2_data_received = item2_data_received & 0x10;
					item2_data_received = item2_data_received | 0x01;
				}
				else if(dataReceived[1] == 2)
				{
					item2_data_received = item2_data_received & 0x10;
					item2_data_received = item2_data_received | 0x02;
				}
				else if(dataReceived[1] == 3)
				{
					item2_data_received = item2_data_received & 0x10;
					item2_data_received = item2_data_received | 0x03;
				}
				else if(dataReceived[1] == 4)
				{
					item2_data_received = item2_data_received & 0x10;
					item2_data_received = item2_data_received | 0x04;
				}
				// set number switch board
				if(dataReceived[0] == 0)
				{
					item1_data_received = 0x03;	
				}
				else if(dataReceived[0] == 1)
				{
					item1_data_received = 0x13;
				}
				else if(dataReceived[0] == ('f' - '0'))
				{
					item1_data_received = 0x46;
					item2_data_received = 0x00;

				}
				else if(dataReceived[0] == 'g' - '0')
				{
					item1_data_received = 0x47;
					item2_data_received = 0x00;
				}
				
				data_send2nRF[0] = byte_start;
				data_send2nRF[1] = item1_data_received;
				data_send2nRF[2] = item2_data_received;
				uart1_tx_buffer(data_send2nRF, 3);
			}

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


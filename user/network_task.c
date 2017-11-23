/*
 * network_task.c
 *
 * Created on: Nov 2, 2017
 * This will handle anythings about the network task connection
 */

#include "network_task.h"

#include "network.h"
#include "esp_wps.h"

NETWORKT_Status* networkStatus = NULL;

xTaskHandle NETWORKT_Init(void *pArg)
{
	xTaskHandle network;

	xTaskCreate(NETWORKT_Task, "NETWORKT_Task", configMINIMAL_STACK_SIZE*4, pArg, tskIDLE_PRIORITY, network);

	return network;
}

void NETWORKT_Task(void *pArg)
{
	NETWORKT_OperationMode mode = NETWORKT_OP_Startup;
	//wifi
	bool wifiStatus = false;
	//socket
	struct sockaddr_in server;
	int serverSocket = -1, clientSocket = -1;
	//wifi info
	Network_Info_Wifi infoWifi;

	networkStatus = (NETWORKT_Status*)calloc(1, sizeof(NETWORKT_Status));

	//connect to wifi that was stored in flash and generate wifi at the same time
	//TODO	check something went wrong when ap_pass ("uhome bulb") didn't have spacing (' ')
	NETWORK_Wifi_ConnectAndGenerate(NETWORKT_SSID, NETWORKT_PASS, "SmartLight", "smartlight");

	while(1)
	{
		switch(mode)
		{
		case NETWORKT_OP_Startup:
			if(serverSocket < 0)
			{
				serverSocket = NETWORK_Server_Create_Connection(&server, NETWORK_ESP_WS_ServerPort);
			}
			else
				//switch to NETWORKT_OP_ST_Connect_Wifi
				mode = NETWORKT_OP_ST_Connect_Wifi;

			break;
		case NETWORKT_OP_ST_Connect_Wifi:
			//try to connect to wifi, also listen to any device try to connect to network's server

			//check if wifi connected
			//TODO	network disconnected event
			//TODO	should make a network connect event?
			wifiStatus = NETWORK_WifiGotIp();
			if(!wifiStatus)
				*networkStatus &= ~NETWORKT_ST_Wifi_Connection;
			else
				//wifi is connected
				*networkStatus |= NETWORKT_ST_Wifi_Connection;

			if(NETWORK_TCP_CheckMailBox(serverSocket) == true)
			{
				clientSocket = NETWORK_TCP_AcceptClient(serverSocket, &server);
				if(clientSocket >= 0)
					mode = NETWORKT_OP_AP_Have_Client;
			}

			break;
		case NETWORKT_OP_AP_Have_Client:
			//check if client is valid and begins config procedure
			if(NETWORK_TCP_CheckMailBox(clientSocket) == true)
			{
				if(NETWORK_TCP_GetWifiData(clientSocket, &infoWifi) == true)
				{
					//connect to received wifi data
					NETWORK_ConnectWifi(infoWifi.ssid, infoWifi.pass);

					mode = NETWORKT_OP_ST_Connect_Wifi;
				}

				close(clientSocket);
			}

			break;
		default:
			break;
		}

		vTaskDelay(25);
	}
}

NETWORKT_Status* NETWORKT_GetStatus(void)
{
	return networkStatus;
}

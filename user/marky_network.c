/*
 * marky_network.c
 *
 * Created on: Nov 2, 2017
 * This will handle anythings about the websocket communication between server-client
 */

#include "marky_network.h"

#include "stdlib.h"
#include "nopoll/nopoll_private.h"
#include "nopoll/nopoll_ctx.h"

char* MARKY_WS_InitMessages_Login(void)
{
	uint8		macArray[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	char		macAddress[17];
	int messageLen = 0;
	char* message = NULL;
	int i = 0;

	messageLen = strlen(MARKY_WS_MS_Send_Login);
	message = (char*)calloc(messageLen, sizeof(char));

	strcpy(message, MARKY_WS_MS_Send_Login);

	//get mac address
	wifi_get_macaddr(STATION_IF, macArray);
	sprintf(macAddress, "%02x:%02x:%02x:%02x:%02x:%02x", macArray[5], macArray[4], macArray[3], macArray[2], macArray[1], macArray[0]);

	//pin it to message
	for(i = 0; i < 17; i++)
		message[34 + i] = macAddress[i];

	return message;
}

char* MARKY_WS_InitMessages_LightSet(uint16_t light)
{
	uint8		macArray[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	char		macAddress[17];
	int messageLen = 0;
	char* message = NULL;
	int i = 0;

	messageLen = strlen(MARKY_WS_MS_Send_Login);
	message = (char*)calloc(messageLen, sizeof(char));

	strcpy(message, MARKY_WS_MS_Send_LightSet);

	//get mac address
	wifi_get_macaddr(STATION_IF, macArray);
	sprintf(macAddress, "%02x:%02x:%02x:%02x:%02x:%02x", macArray[5], macArray[4], macArray[3], macArray[2], macArray[1], macArray[0]);

	//pin it to message
	for(i = 0; i < 17; i++)
		message[36 + i] = macAddress[i];
		message[58] = ((light / 1000) % 10) + '0';
		message[59] = ((light / 100) % 10) + '0';
		message[60] = ((light / 10) % 10) + '0';
		message[61] = ((light / 1) % 10) + '0';

	return message;
}

bool MARKY_WS_LogintoServer(noPollConn** conn)
{
	//TODO	store ctx, opts to free in other place
	//TODO	ssl & tls verify
	noPollCtx*  ctx;
	//noPollConnOpts* opts;
	noPollMsg* msg;
	int recvLen = strlen(MARKY_WS_MS_Recv_Ok);
	char recv[recvLen];
	char* send = NULL;
	int sendLen = 0;

	uint8		macArray[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	char		macAddress[17];
	wifi_get_macaddr(STATION_IF, macArray);
	sprintf(macAddress, "deviceConnect/%02x:%02x:%02x:%02x:%02x:%02x", macArray[5], macArray[4], macArray[3], macArray[2], macArray[1], macArray[0]);

	//connect to server
	ctx = (noPollCtx*)nopoll_ctx_new();
	//opts = nopoll_conn_opts_new ();
	//nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);
	//*conn = (noPollConn*)nopoll_conn_tls_new (ctx, opts, MARKY_SERVER_Ip, MARKY_SERVER_Port, NULL, MARKY_SERVER_SubUrl, NULL, NULL);

	*conn = (noPollConn*)nopoll_conn_new (ctx, MARKY_SERVER_Ip, MARKY_SERVER_Port, NULL, MARKY_SERVER_SubUrl, NULL, NULL);

	//send MAC Address
	nopoll_conn_send_text (*conn, macAddress ,strlen(macAddress));

	return true;
}

void MARKY_WS_sendData(noPollConn* conn, uint16 data)
{
	char		dataArray[17];
	uint8 data1 = 0x00;
	uint8 data2 = 0x00;

	data1 = (uint8)(data & 0x00FF);
	data2 = (uint8)((data >> 8) & 0x00FF);
	sprintf(dataArray, "data/%02x:%02x", data1, data2);
	nopoll_conn_send_text (conn, dataArray ,strlen(dataArray));
}

void MARKY_WS_send2Byte(noPollConn* conn, uint8 data1, uint8 data2)
{
	char		dataArray[17];
	int 		number_hundred = 0;
	int 		number_ten = 0;
	int 		number_unit = 0;
	uint8_t 	temperature = 0;
	if(data1 == 0x54)
	{
		temperature = data2;
		number_hundred = temperature / 100;
		number_ten = (temperature % 100) / 10;
		number_unit = (temperature % 100) % 10;
		if(number_hundred != 0)
			sprintf(dataArray, "temperature/%d%d%d", number_hundred, number_ten, number_unit);
		else if(number_ten != 0)
			sprintf(dataArray, "temperature/%d%d", number_ten, number_unit);
		else
			sprintf(dataArray, "temperature/%d", number_unit);
	}
	else if(data1 == 0x47)
	{
		if(data2 == 0x01)
			sprintf(dataArray, "gas/DETECTED");
		else
			sprintf(dataArray, "gas/SAFE");
	}
	else if(data1 == 0x46)
	{
		if(data2 == 0x01)
			sprintf(dataArray, "fire/DETECTED");
		else
			sprintf(dataArray, "fire/SAFE");
	}
	else
		return;
	nopoll_conn_send_text (conn, dataArray ,strlen(dataArray));
}

void MARKY_WS_Close(noPollConn** conn)
{
	noPollCtx*  ctx;

	ctx = nopoll_conn_ctx(*conn);

	nopoll_conn_close(*conn);
	nopoll_ctx_unref(ctx);

	*conn = NULL;
}

bool MARKY_WS_Send(noPollConn* conn, MARKY_SendType type, uint16_t sendData)
{
	char*		send = NULL;
	int         sendLen = 0;
	int recvLen = strlen(MARKY_WS_MS_Recv_Ok);
	char recv[recvLen];

	switch(type)
	{
		case MARKY_SendT_Light_Set:
			send = MARKY_WS_InitMessages_LightSet(sendData);
			sendLen = strlen(send);

			if (! nopoll_conn_is_ready (conn))
				return nopoll_false;

			if (nopoll_conn_send_text (conn, send, sendLen) <= 0)
				return nopoll_false;

			nopoll_conn_read (conn, recv, recvLen, nopoll_false, 5);
			if (strncmp(recv, MARKY_WS_MS_Recv_Ok, recvLen) != 0)
				return false;

			free(send);
			break;

		case MARKY_SendT_Detected_Person:
			//TODO	make this available
			//not supported yet
			return false;
			break;

		default:
			break;
	}
	return true;
}

bool MARKY_WS_Recv(noPollConn* conn, uint8_t* recvData)
{
	noPollMsg* msg;
	char* readData = NULL;
	char* changeColor = "changeColor/";
	char msgColor[100];
	if ( !nopoll_conn_is_ready (conn) )
		return false;

	if( MARKY_WS_CheckWSMailBox(conn) )
	{
		msg = nopoll_conn_get_msg(conn);
		if(msg == NULL)
			return false;

		readData = (char*)nopoll_msg_get_payload(msg);
		int i, j = 0;
		for (i = 0; i < 12; i++)
		{
			if(readData[i] != changeColor[i])
				return false;
		}

		for (i = 12; i < 100; i++)
		{
			msgColor[j] = readData[i];
			j++;
		}

		//red from client
		// if (strcmp(readData, red) == 0)
		// 	readData = "255000000255255";
		// else
		// //green from client
		// if (strcmp(readData, green) == 0)
		// 	readData = "000255000255255";
		// else
		// //blue from client
		// if (strcmp(readData, blue) == 0)
		// 	readData = "000000255255255";
		// else
		// //turn off from client
		// if (strcmp(readData, turn_off) == 0)
		// 	readData = "000000000000000";
		// else
		// //turn on from client
		// if (strcmp(readData, turn_on) == 0)
		// 	readData = "255255255255255";

		//012345678901234
		//255255255255255	//light on max brightness


		// R  G  B  W  Br
		//000000255255255	//blue
		//000255000255255	//green
		//255000000255255	//red
		//000255255255255	//mixed blue vs green
		//000000000000000	//off

		//TODO new algorithm
		//ASCII(a craft algorithm)

		//BRIGHTNESS
		recvData[0] = (msgColor[14]-'0');
		recvData[0] += (msgColor[13]-'0')*10;
		recvData[0] += (msgColor[12]-'0')*100;

		//WHITE
		recvData[1] = (msgColor[11]-'0');
		recvData[1] += (msgColor[10]-'0')*10;
		recvData[1] += (msgColor[9]-'0')*100;

		//BLUE
		recvData[2] = (msgColor[8]-'0');
		recvData[2] += (msgColor[7]-'0')*10;
		recvData[2] += (msgColor[6]-'0')*100;

		//GREEN
		recvData[3] = (msgColor[5]-'0');
		recvData[3] += (msgColor[4]-'0')*10;
		recvData[3] += (msgColor[3]-'0')*100;

		//RED
		recvData[4] = (msgColor[2]-'0');
		recvData[4] += (msgColor[1]-'0')*10;
		recvData[4] += (msgColor[0]-'0')*100;
		/*
		recvData[5] = (readData[2]-'0');
		recvData[5] += (readData[1]-'0')*10;
		recvData[5] += (readData[0]-'0')*100;*/

		free(readData);

		return true;
	}

	return false;
}

bool MARKY_WS_RecvData(noPollConn* conn, uint8_t* recvData)
{
	noPollMsg* msg;
	bool isSwitch = true;
	bool isFire = true;
	bool isGas = true;
	char* readData = NULL;
	char* change_status = "switch/";
	char* reset_fire = "resetFire";
	char* reset_gas = "resetGas";
	if ( !nopoll_conn_is_ready (conn) )
		return false;

	if( MARKY_WS_CheckWSMailBox(conn) )
	{
		msg = nopoll_conn_get_msg(conn);
		if(msg == NULL)
			return false;

		readData = (char*)nopoll_msg_get_payload(msg);
		int i = 0;
		for (i = 0; i < 7; i++)
		{
			if(readData[i] != change_status[i])
			{
				isSwitch = false;
				break;
			}
		}
		if(isSwitch == true)
		{
			recvData[0] = (readData[7] - '0');
			recvData[1] = (readData[8] - '0');
			recvData[2] = (readData[9] - '0');
			free(readData);
			return true;
		}
		for(i = 0; i < 9; i++)
		{
			if(readData[i] != reset_fire[i])
			{
				isFire = false;
				break;
			}
		}
		if(isFire == true)
		{
			recvData[0] = ('f' - '0');
			recvData[1] = ('i' - '0');
			recvData[2] = ('r' - '0');
			free(readData);
			return true;
		}
		for(i = 0; i < 8; i++)
		{
			if(readData[i] != reset_gas[i])
			{
				isGas = false;
				break;
			}
		}
		if(isGas == true)
		{
			recvData[0] = ('g' - '0');
			recvData[1] = ('a' - '0');
			recvData[2] = ('s' - '0');
			free(readData);
			return true;
		}
	}
	return false;
}
bool MARKY_WS_CheckWSMailBox(noPollConn* conn)
{
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(conn->session, &readfds);
		FD_SET(STDIN_FILENO, &readfds);

		select(conn->session+1, &readfds, NULL, (fd_set*)0, &tv);

		return FD_ISSET(conn->session, &readfds);
}

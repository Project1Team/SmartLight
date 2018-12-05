/*
 * network.c
 *
 * Created on: Nov 2, 2017
 * This will handle anythings about the network connection
 */

#include "network.h"

#include "esp8266-wifi.h"

#include "nopoll/nopoll_private.h"
#include "lwip/lwip/sockets.h"

//----------------------------------------------------------------------
/*
 * websocket communication functionality
 */

void NETWORK_Init(void)
{

}

#ifndef NETWORK_ConnectWifi

#endif

#ifndef NETWORK_WifiGotIp

#endif

bool NETWORK_WS_InitServer(noPollConn** listener, noPollCtx** ctx, const char* serverIp, const char* serverPort)
{
	uint8_t waitTime = 0;

	if(*ctx != NULL)
		nopoll_ctx_unref(*ctx);

	*ctx = (noPollCtx*)nopoll_ctx_new();
	*listener = (noPollConn*)nopoll_listener_new(*ctx, serverIp, serverPort);

	if (! nopoll_conn_is_ok(*listener))
	{
		nopoll_conn_close(*listener);
		nopoll_ctx_unref(*ctx);
		return false;
	}

	nopoll_ctx_set_on_msg(*ctx, NETWORK_WS_ListeneronMessage, NULL);
	nopoll_ctx_set_on_ready 	(*ctx, NETWORK_WS_Handler_NewConnection, NULL);

	return true;
}

nopoll_bool NETWORK_WS_Handler_NewConnection(noPollCtx * ctx, noPollConn * conn, noPollPtr user_data)
{
	//printf("new client\n");

	return nopoll_true;
}

void NETWORK_WS_ListeneronMessage(noPollCtx* ctx, noPollConn* conn, noPollMsg* msg, noPollPtr user_data)
{
	char* receiveData;
	printf("DEALER: on Message\n");

	// print the message (for debugging purposes) and reply
	/*
	printf ("Listener received (size: %d, ctx refs: %d): (first %d bytes, fragment: %d) '%s'\n",
	nopoll_msg_get_payload_size (msg),
	nopoll_ctx_ref_count (ctx), shown, nopoll_msg_is_fragment (msg), example);
	*/

	receiveData = (char*)nopoll_msg_get_payload(msg);
	printf("%s\n", receiveData);

	// reply to the message
	nopoll_conn_send_text (conn, "Message received", 16);

	return;
}

//----------------------------------------------------------------------
/*
 * TCP socket communication functionality
 */

int NETWORK_TCP_CreateConnection(struct sockaddr_in *SERVER, const char *IP, const int PORT)
{
	//create socket
	int status, socket_des;

	socket_des = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_des == -1)
	{
#ifdef NETWORK_TCP_Enable_Debug
		printf("Create socket error!!!\n");
#endif /* NETWORK_TCP_Enable_Debug */
		close(socket_des);
		return -1;
	}

#ifdef NETWORK_TCP_Enable_Debug
	printf("Socket created\n");
#endif /* NETWORK_TCP_Enable_Debug */

	//create tcp connection
	bzero(SERVER, sizeof(struct sockaddr_in));
	SERVER->sin_addr.s_addr = inet_addr(IP);
	SERVER->sin_family = AF_INET;
	SERVER->sin_port = htons(PORT);

	status = connect(socket_des, (struct sockaddr *)  SERVER,  sizeof(struct sockaddr));

	if (status != 0)
	{
#ifdef NETWORK_TCP_Enable_Debug
		printf("Connect error: %d\n", status);
#endif /* NETWORK_TCP_Enable_Debug */
		close(socket_des);
		return -2;
	}

	return socket_des;
}

int NETWORK_Server_Create_Connection(struct sockaddr_in *server, const int PORT)
{
	int socket;
	bool ret = false;

	//bzero(server, sizeof(struct sockaddr_in));
	memset(server, 0, sizeof(*server));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_len = sizeof(*server);
	server->sin_port = htons(PORT);

	//create socket for incoming connections
	socket = socket(AF_INET, SOCK_STREAM, 0);
	if (socket == -1)
	{
		close(socket);
		return -1;
	}

	do
	{
		ret = bind(socket, (struct sockaddr *)server, sizeof(*server));

		if(ret != 0)
		{
			vTaskDelay(100);
		}
	} while(ret != 0);

	do
	{
		ret = listen(socket, 2);
	} while(ret != 0);

	return socket;
}

void NETWORK_TCP_InitServer(struct sockaddr_in *server, int *sock, int PORT)
{
	bool ret = false;

	//bzero(server, sizeof(struct sockaddr_in));
	memset(server, 0, sizeof(*server));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = INADDR_ANY;
	server->sin_len = sizeof(*server);
	server->sin_port = htons(PORT);

	//create socket for incoming connections
	do
	{
		*sock = socket(AF_INET, SOCK_STREAM, 0);
		if(*sock == -1)
		{
#ifdef NETWORK_TCP_Enable_Debug
			//printf("ESP8266 TCP server task > socket error\n");
#endif /* NETWORK_TCP_Enable_Debug */
			vTaskDelay(100);
		}

	} while(*sock == -1);

#ifdef NETWORK_TCP_Enable_Debug
	printf("ESP8266 TCP server task > create socket\n");
#endif /* NETWORK_TCP_Enable_Debug */

	//bind to PORT
	do
	{
		ret = bind(*sock, (struct sockaddr *)server, sizeof(*server));

		if(ret != 0)
		{
#ifdef NETWORK_TCP_Enable_Debug
			printf("ESP8266 TCP server task > bind fail\n");
#endif /* NETWORK_TCP_Enable_Debug */
			vTaskDelay(100);
		}
	} while(ret != 0);

#ifdef NETWORK_TCP_Enable_Debug
	printf("ESP8266 TCP server task > port:%d\n",ntohs(server->sin_port));
#endif /* NETWORK_TCP_Enable_Debug */

	do
	{
		ret = listen(*sock, 2);
		if(ret != 0)
		{
#ifdef NETWORK_TCP_Enable_Debug
			printf("ESP8266 TCP server task > failed to set listen queue!\n");
#endif /* NETWORK_TCP_Enable_Debug */
			vTaskDelay(100);
		}
	} while(ret != 0);

#ifdef NETWORK_TCP_Enable_Debug
	printf("ESP8266 TCP server task > listen ok\n");
#endif /* NETWORK_TCP_Enable_Debug */
}

int NETWORK_TCP_AcceptClient(int serverSocket, struct sockaddr_in* server)
{
	int clientSocket;
	int32 len = sizeof(struct sockaddr_in);

	if((clientSocket = accept(serverSocket, (struct sockaddr *)server, (socklen_t *)&len)) < 0)
		return -1;

	return clientSocket;
}

char* NETWORK_TCP_ReadLine(int SOCKET_DES)
{
	int checker = true;
	char* recv_buf = (char *)calloc( 11, sizeof(char) );

	while (1)
	{
		char *recv_cha;
		recv_cha = (char *)calloc( 2, sizeof(char) );
		checker = read( SOCKET_DES, recv_cha, sizeof(char) );
		//recv_cha[checker] = 0;

		if (!checker)
			break;

		if (recv_cha[0] == '\n')
		{
			free(recv_cha);
			break;
		}

		strcat(recv_buf, recv_cha);

		free(recv_cha);
	}

	switch (checker)
	{
	case -1:
#ifdef NETWORK_TCP_Enable_Debug
		printf("Error\n");
#endif /* NETWORK_TCP_Enable_Debug */
		break;
	case 0:
#ifdef NETWORK_TCP_Enable_Debug
		printf("End of file or connection closed\n");
#endif /* NETWORK_TCP_Enable_Debug */
		break;
	case 1:
#ifdef NETWORK_TCP_Enable_Debug
		printf("Read complete!\n");
#endif /* NETWORK_TCP_Enable_Debug */
		break;
	default:
#ifdef NETWORK_TCP_Enable_Debug
		printf("Don't know!\n");
#endif /* NETWORK_TCP_Enable_Debug */
		break;
	}

	return recv_buf;
}

int NETWORK_TCP_WriteLine(int SOCKET_DES, char *write_buf)
{

	char *write;
	write = (char *)calloc(strlen(write_buf) + 1, sizeof(char));
	strcat(write, write_buf);
	strcat(write, "\n");

	if ( write(SOCKET_DES, write, strlen(write) ) < 0 )
	{
		close(SOCKET_DES);
		vTaskDelay(1000 / portTICK_RATE_MS);
#ifdef NETWORK_TCP_Enable_Debug
		printf("ESP8266 TCP client task > send fail\n");
#endif /* NETWORK_TCP_Enable_Debug */
		return 1;
	}
	/*
	if ( write(SOCKET_DES, "\n", strlen(write_buf) ) < 0 )
	{
		close(SOCKET_DES);
		vTaskDelay(1000 / portTICK_RATE_MS);
		printf("ESP8266 TCP client task > send fail\n");
		return 2;
	}
	*/

	free(write);
#ifdef NETWORK_TCP_Enable_Debug
	printf("ESP8266 TCP client task > send success\n");
#endif /* NETWORK_TCP_Enable_Debug */

	return 0;
}

bool NETWORK_TCP_CheckMailBox(int socket)
{
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);
	FD_SET(STDIN_FILENO, &readfds);

	select(socket+1, &readfds, NULL, (fd_set*)0, &tv);

	return FD_ISSET(socket, &readfds);
}

char* NETWORK_TCP_InitMessages_SignIn(void)
{
	uint8		macArray[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	char		macAddress[17];
	int messageLen = 0;
	char* message = NULL;
	int i = 0;

	messageLen = strlen(NETWORK_TCP_MS_SignIn);
	message = (char*)calloc(messageLen, sizeof(char));

	strcpy(message, NETWORK_TCP_MS_SignIn);

	//get mac address
	wifi_get_macaddr(STATION_IF, macArray);
	//sprintf(macAddress, "%02x:%02x:%02x:%02x:%02x:%02x", macArray[5], macArray[4], macArray[3], macArray[2], macArray[1], macArray[0]);

	//pin it to message
	for(i = 0; i < 17; i++)
		message[10 + i] = macAddress[i];

	return message;
}

bool NETWORK_TCP_GetWifiData(int clientSocket, Network_Info_Wifi *Wifi)
{
	//TODO	json data type for this shitty function
	char* signin = NULL;
	char* recv = NULL,* read = NULL;
	uint8_t signinLen = 0, repokLen = 0;
	int status = 0;

	memset(Wifi, 0, sizeof(*Wifi));
	recv = (char*)calloc(80, sizeof(char));

	//sign in procedure
	signin = NETWORK_TCP_InitMessages_SignIn();
	signinLen = strlen(signin);

	//reply ok
	repokLen = strlen(NETWORK_TCP_MS_Rep_Ok);

	status = read(clientSocket, recv, sizeof(char)*49);
//	status = recv(clientSocket, recv, 39, 0);
	if(status < 49)
	{
		write(clientSocket, NETWORK_TCP_MS_Rep_NotOk, repokLen);
		return false;
	}
	//TODO	python conversed json different form javascript (take a look at this when programming android app
	if(strncmp(signin, recv, signinLen) != 0)
	{
		write(clientSocket, NETWORK_TCP_MS_Rep_NotOk, repokLen);
		return false;
	}

	//reply ok
	write(clientSocket, NETWORK_TCP_MS_Rep_Ok, repokLen);

	//TODO	make this become json data type ( exp: {"ops":"change wifi", "change":{"ssid":"Lenovo A6000", "pass":"22031996"}} )
	//TODO	start
	//ssid
	read = NETWORK_TCP_ReadLine(clientSocket);
	if(read == NULL)
	{
		write(clientSocket, NETWORK_TCP_MS_Rep_NotOk, repokLen);
		return false;
	}

	strcpy(Wifi->ssid, read);
	free(read);
	read = NULL;

	//pass
	read = NETWORK_TCP_ReadLine(clientSocket);
	if(read == NULL)
		return false;
	strcpy(Wifi->pass, read);
	free(read);
	//TODO	end

	//reply ok
	write(clientSocket, NETWORK_TCP_MS_Rep_Ok, repokLen);

	return true;
}

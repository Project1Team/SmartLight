/*
 * network.h
 *
 * Created on: Nov 2, 2017
 * This will handle anythings about the network connection
 */

#ifndef INCLUDE_NETWORK_H_
#define INCLUDE_NETWORK_H_

#include "esp_common.h"

#include "nopoll/nopoll.h"

//----------------------------------------------------------------------

//#define NETWORK_TCP_MS_SignIn		"{\"ops\":\"sign in\",\"pass\":\"00:00:00:00:00:00_1\"}"
#define NETWORK_TCP_MS_SignIn		"{\"pass\": \"71:6f:06:7f:cf:5c_1\", \"ops\": \"sign in\"}"

#define NETWORK_TCP_MS_Rep_Ok		"{\"reply\": \"_ok\"}"

#define NETWORK_TCP_MS_Rep_NotOk	"{\"reply\": \"_no\"}"

#define NETWORK_TCP_MS_ChangeWifi

/*
 * data defination
 */

typedef struct NETWORK_INFO_WIFI
{
	char ssid[32];
	char pass[64];
} Network_Info_Wifi;

//----------------------------------------------------------------------
/*
 * wifi functionality
 */

/*
 *
 */
void NETWORK_Init(void);

/*
 *
 */
#define NETWORK_ConnectWifi(ssid, pass)		start_wifi_station(ssid, pass)
#ifndef NETWORK_ConnectWifi

#endif /*NETWORK_ConnectWifi*/

/*
 *
 */
#define NETWORK_WifiGotIp(void)	(wifi_station_get_connect_status()==STATION_GOT_IP)
#ifndef NETWORK_WifiGotIp

#endif /*NETWORK_WifiGotIp*/

#define NETWORK_Wifi_ConnectAndGenerate(st_ssid,st_pass, ap_ssid, ap_pass)			start_wifi_ap_station(st_ssid,st_pass, ap_ssid, ap_pass)
#ifndef NETWORK_Wifi_ConnectAndGenerate

#endif /*NETWORK_Wifi_ConnectAndGenerate*/

//----------------------------------------------------------------------
/*
 * websocket communication functionality
 */

/*
 * create a websocket server
 */
bool NETWORK_WS_InitServer(noPollConn** listener, noPollCtx** ctx, const char* serverIp, const char* serverPort);

/*
 *
 */
nopoll_bool NETWORK_WS_Handler_NewConnection(noPollCtx * ctx, noPollConn * conn, noPollPtr user_data);

/*
 *
 */
void NETWORK_WS_ListeneronMessage(noPollCtx* ctx, noPollConn* conn, noPollMsg* msg, noPollPtr user_data);

//----------------------------------------------------------------------
/*
 * TCP socket communication functionality
 */

//TODO	clear this mesh of making server
//TODO	start
/*
 * @brief	create a tcp connection to SERVER (IP, PORT) at socket socket_des
 * @param	SERVER:	server parameter
 * @param	IP:		is taken by SERVER for connection
 * @param	PORT:	is taken by SERVER for connection
 * @return	socket that is connected if success
 */
int NETWORK_TCP_CreateConnection(struct sockaddr_in *SERVER, const char *IP, const int PORT);

/*
 * @brief	create a tcp connection to SERVER (IP, PORT) at socket socket_des
 * @param	SERVER:	server parameter
 * @return	socket that is connected if success
 */
int NETWORK_Server_Create_Connection(struct sockaddr_in *server, const int PORT);

/*
 *
 */
void NETWORK_TCP_InitServer(struct sockaddr_in *server, int *sock, int PORT);
//TODO	end

/*
 * @brief	accept server
 * @param	serverSocket:	socket of esp server
 * @param	server:			config of server
 * @return	client socket (>=0) if success, else return <0
 */
int NETWORK_TCP_AcceptClient(int serverSocket, struct sockaddr_in* server);

/*
 * @brief	read a line from buffer
 * @param	SOCKET_DES:		socket
 * @return string of data was read from buffer
 */
char* NETWORK_TCP_ReadLine(int SOCKET_DES);

/*
 * @brief	write a line to buffer
 * 			WRITEDATA = write_buf + "\n"
 * @param	SOCKET_DES:		socket
 * @param	write_buf:		buffer
 * @return	0 if success
 */
int NETWORK_TCP_WriteLine(int SOCKET_DES, char *write_buf);

/*
 * @brief	check if have something on socket
 * @param	socket:		socket
 * @return	true if have something
 */
bool NETWORK_TCP_CheckMailBox(int socket);

/*
 * @brief	init message sign up for compare purpose
 */
char* NETWORK_TCP_InitMessages_SignIn(void);

/*
 * @brief	get wifi information (ssid, pass)
 * @param	clientSocket:	socket of client
 * @param	Wifi:			wifi info will be stored in here
 * @return	true if success
 */
bool NETWORK_TCP_GetWifiData(int clientSocket, Network_Info_Wifi *Wifi);

#endif /* INCLUDE_NETWORK_H_ */

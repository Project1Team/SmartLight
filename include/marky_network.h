/*
 * marky_network.h
 *
 * Created on: Nov 2, 2017
 * This will handle anythings about the websocket communication between server-client
 */

#ifndef INCLUDE_MARKY_NETWORK_H_
#define INCLUDE_MARKY_NETWORK_H_

#include "esp_common.h"

#include "nopoll/nopoll.h"




/*
 * @brief	server information
 */
#define MARKY_SERVER_Ip			"192.168.137.150"			//Local IP of wifi
#define MARKY_SERVER_Port		"8887"						//Local port
#define MARKY_SERVER_SubUrl		"/devices"					//ws enpoint


/*
 * @brief	send message to server
 */
#define MARKY_WS_MS_Send_Login			"{Logged in, connection established!}"		//Login signal

#define MARKY_WS_MS_Send_LightSet		"{\"ops\":\"light set\",\"DEVs\":[{\"dev\":[\"00:00:00:00:00:00_1\",\"0000\"]}]}"

#define MARKY_WS_MS_Send_DetectedPerson	"{\"ops\":\"detected person\",\"DEVs\":[{\"dev\":[\"id\",\"stt\"]}]}"

/*
 * @brief	receive message from server
 */
#define MARKY_WS_MS_Recv_Ok			"{\"reply\":\"_ok\"}"

#define MARKY_WS_MS_Recv_SetLight	"{\"ops\":\"set light\",\"DEVs\":[{\"dev\":[\"00:00:00:00:00:00_1\",\"0000\"]}]}"

/*
 * @brief	send parameter
 */
typedef enum MARKY_SENDTYPE
{
	MARKY_SendT_Light_Set,				/*!< 'light set' */
	MARKY_SendT_Detected_Person,		/*!< 'detected person' */
	MARKY_SendT_MAX
} MARKY_SendType;

//-----------------------------------------------------------------------

/*
 * @brief	get mac address of chip and implement to  messages login to server
 * @return	login message
 */
char* MARKY_WS_InitMessages_Login(void);

/*
 * @brief	get mac address of chip and implement to message light set
 * @return	light set message
 */
char* MARKY_WS_InitMessages_LightSet(uint16_t light);

/*
 * @brief	connect & login to server
 * @param	conn:	pointer of connection handler
 * @return	true if succeed
 */
bool MARKY_WS_LogintoServer(noPollConn** conn);

/*
 * @brief	close connection and free memory
 * @param	conn:	pointer of connection handler
 */
void MARKY_WS_Close(noPollConn** conn);

/*
 * @brief	send to server
 * @param	conn:		connection handler
 * @param	type:		type of send data
 * @param	sendData:	data for sending
 * @return	true if succeed
 */
bool MARKY_WS_Send(noPollConn* conn, MARKY_SendType type, uint16_t sendData);

/*
 * @brief	read command from server
 * @param	conn:		connection handler
 * @param	recvData:	received command from server will be store in here
 * @return	true if succed
 */
bool MARKY_WS_Recv(noPollConn* conn, uint8_t* recvData);

/*
 * @brief	connect to server
 * @param	conn:	connection handler
 * @return	true if succeed
 */
bool MARKY_WS_CheckWSMailBox(noPollConn* conn);

#endif /* INCLUDE_MARKY_NETWORK_H_ */

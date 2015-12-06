//---------------------------------------------------------------------------
// ����Ϣ [12/6/2015 2014]
#ifndef LOGIN_GATE_H_
#define LOGIN_GATE_H_

#include "Category.h"

#pragma pack(1)

enum
{
	GATE2SRV_KEEPALIVE,		//KEEPALIVE
	GATE2SRV_ADDSOCKET,		//���socket
	GATE2SRV_CLOSESOCKET,	//�ر�socket
	GATE2SRV_MSG,			//ת����Ϣ

	LOGINGATE_MAX,
};

struct LOGIN_GATE_GATE2SRV_KEEPALIVE_Data
{

};


struct LOGIN_GATE_GATE2SRV_ADDSOCKET_Data
{
	uint32 sock;					//sock
	char   szIp[20];				//ip
};

struct LOGIN_GATE_GATE2SRV_CLOSESOCKET_Data
{
	uint32 sock;					//sock
};

struct LOGIN_GATE_GATE2SRV_MSG_Data
{
	uint32 sock;					//sock
	char   szPacket[1];				//szPacket
};

#pragma pack()

#endif
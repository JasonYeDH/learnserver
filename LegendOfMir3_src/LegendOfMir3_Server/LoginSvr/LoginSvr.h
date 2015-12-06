#pragma once

#include "../Common/Common.h"

#define PACKET_KEEPALIVE				"%++$"

#define LOGPARAM_STR			1
#define LOGPARAM_INT			2

class CUserInfo
{
public:
	SOCKET		sock;
	TCHAR		szSockHandle[4];

	char		szUserID[11];				// User ID
	char		szAddress[20];				// User's local address 
	
	BYTE		btPayMode;
	
	int			nClientVersion;
	int			nCertification;
	BOOL		fVersionAccept;
	BOOL		fSelServerOk;

	int			nServerID;
};

class AccountUser;
class CGateInfo
{
public:
	SOCKET					sock;								//��LoginGate�ڷ�������socket

	CWHList<CUserInfo*>		xUserInfoList;						//ͨ����LoginGate
	
	// For Overlapped I/O
	OVERLAPPED				Overlapped;
	WSABUF					DataBuf;
	char					Buffer[DATA_BUFSIZE];
	uint32					bufLen;

	CWHQueue				g_SendToGateQ;

public:
	void	SendToGate(SOCKET cSock, char *pszPacket);

	//������Ϣ
	void	ReceiveSendUser(uint32 socket,char * szMsg,uint32 nLen);

	//�ر�user
	void	ReceiveCloseUser(uint32 sock);

	//���user
	void	ReceiveOpenUser(uint32 sock,char *szIp);

	void	ReceiveServerMsg(char *pszPacket);
	void	MakeNewUser(char *pszPacket);

	bool	ParseUserEntry( char *buf, AccountUser *userInfo );

	void	ProcAddUser(SOCKET s, char *szID,char * szPwd);
	void	ProcLogin(SOCKET s, char *pszData);
	void	ProcSelectServer(SOCKET s, WORD wServerIndex);

	void	Close();
	
	void	SendToGate(char * szData,int nLen);

	void	SendToGate(Packet *pPacket);
	
	//�����ĵ籨
	void	SendKeepAlivePacket();

	//�¼�����
	bool	PacketProcess(Packet*pPacket);

	//logingate
	bool	LoginGateProcess(Packet*pPacket);
	//loginsrv
	bool	LoginSrvProcess(Packet*pPacket);

	//ACCOUNT
	bool	AccountProcess(SOCKET sock,Packet*pPacket);

	CGateInfo()
	{
		bufLen	= 0;
	}

	int  Recv()
	{
		DWORD nRecvBytes = 0, nFlags = 0;

		DataBuf.len = DATA_BUFSIZE - bufLen;
		DataBuf.buf = Buffer + bufLen;

		memset( &Overlapped, 0, sizeof( Overlapped ) );

		return WSARecv( sock, &DataBuf, 1, &nRecvBytes, &nFlags, &Overlapped, 0 );
	}

	//�Ƿ��и������İ�
	bool HasCompletionPacket()
	{
		//return memchr( Buffer, '!', bufLen ) ? true : false;
		if (bufLen >= PHLen)	//��ͷ����
		{
			Packet *pHavePacket = (Packet*)Buffer;
			if (pHavePacket == NULL)
				return false;

			if(pHavePacket->ver != PHVer || pHavePacket->hlen != PHLen)
				return false;

			int crc		  = pHavePacket->crc;
			int packetLen = pHavePacket->dlen ^ crc;	//������

			if (packetLen + pHavePacket->hlen < bufLen)
				return false;

			return true;
		}
		else
			return false;
	}

	// recv չ����
	int ExtractPacket( char *pPacket )
	{
		Packet *pHavePacket = (Packet*)Buffer;
		if (pPacket== NULL)
			return NULL;

		if(pHavePacket->ver != PHVer || pHavePacket->hlen != PHLen)
			return NULL;

		int crc		  = pHavePacket->crc;
		int packetLen = pHavePacket->dlen ^ crc +  pHavePacket->hlen;	//������

		memcpy( pPacket, Buffer, packetLen );

		memmove( Buffer, Buffer + packetLen, DATA_BUFSIZE - packetLen );
		bufLen -= packetLen;

		return packetLen;
	}
};


struct GAMESERVERINFO
{
	int  index;
	char name[50];
	char ip[50];
	int  connCnt;	// ���� �� ����
};

// ORZ: ��� ����Ʈ���� ��� ��� ������
struct GATESERVERINFO
{
	char ip[50];
};


typedef struct tag_TSENDBUFF
{
	SOCKET			sock;
	int				nLen;
	char			szData[TCP_PACKET_SIZE+PHLen];
}_TSENDBUFF, *_LPTSENDBUFF;

void InsertLogMsg(UINT nID);
void InsertLogMsg(LPTSTR lpszMsg);
void InsertLogMsgParam(UINT nID, void *pParam, BYTE btFlags = LOGPARAM_STR);

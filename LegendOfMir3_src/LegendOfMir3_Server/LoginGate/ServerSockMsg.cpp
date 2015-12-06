#include "stdafx.h"
#include "../Common/Common.h"
#include "../packet/Category.h"
#include "../packet/logingate_protocol.h"

void SendExToServer(Packet*pPacket);
void SendExToServer(uint8 Category,uint8 protocol);

extern SOCKET		g_ssock;
extern SOCKET		g_csock;

extern HWND			g_hStatusBar;

extern HANDLE					g_hIOCP;

CWHList<CSessionInfo*>			g_xSessionList;

void UpdateStatusBar(BOOL fGrow)
{
	static LONG	nNumOfCurrSession = 0;

	TCHAR	szText[20];

	(fGrow ? InterlockedIncrement(&nNumOfCurrSession) : InterlockedDecrement(&nNumOfCurrSession));
	
	wsprintf(szText, _TEXT("%d Sessions"), nNumOfCurrSession);

	SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(3, 0), (LPARAM)szText);
}

/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��
bool _IsSocketAlive( SOCKET s )
{
	int nByteSent=send(s,"",0,0);
	if (-1 == nByteSent) return false;
	return true;
}


// ��ʾ��������ɶ˿��ϵĴ��� 0:�����˳���1���ͻ��˹ر� 2������������
int HandleError(SOCKET sock,const DWORD& dwErr )
{
	// ����ǳ�ʱ�ˣ����ټ����Ȱ�  
	if(WAIT_TIMEOUT == dwErr)  
	{  	
		// ȷ�Ͽͻ����Ƿ񻹻���...
		if( !_IsSocketAlive( sock ) )			//sendУ��
		{
			InsertLogMsg(_TEXT("��⵽�ͻ����쳣�˳���"));
			return 1;
		}
		else
		{
			InsertLogMsg( _TEXT("���������ʱ��������..."));
			return 2;
		}
	}  

	// �����ǿͻ����쳣�˳���
	else if( ERROR_NETNAME_DELETED==dwErr )		
	{
		InsertLogMsg(_TEXT( "��⵽�ͻ����쳣�˳���"));			//�ͻ����ǹ�Xֱ���˳�
		return 1;
	}

	else
	{
		TCHAR szMsg[1024];
		wsprintf(szMsg,_TEXT("��ɶ˿ڲ������ִ����߳��˳���������룺%d"),dwErr);
		InsertLogMsg(szMsg);
		return 0;
	}
}


//UINT WINAPI AcceptThread(LPVOID lpParameter)
//logingate�����߳�
DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int							nLen = sizeof(SOCKADDR_IN);
	char						szMsg[128] = {0};

	SOCKET						Accept;
	SOCKADDR_IN					Address;

	while (TRUE)
	{
		Accept = accept(g_ssock, (struct sockaddr FAR *)&Address, &nLen);

		if (g_fTerminated)
			return 0;

		CSessionInfo* pNewUserInfo = CSessionInfo::ObjPool().createObject();//(CSessionInfo*)GlobalAlloc(GPTR, sizeof(CSessionInfo));

		if (pNewUserInfo)
		{
			pNewUserInfo->sock				= Accept;

			CreateIoCompletionPort((HANDLE)pNewUserInfo->sock, g_hIOCP, (DWORD)pNewUserInfo, 0);

			if (g_xSessionList.AddNewNode(pNewUserInfo))
			{
				int zero = 0;
				setsockopt(pNewUserInfo->sock, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) );
				zero = 0;
				setsockopt( pNewUserInfo->sock, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero));

				int nodelay = 1;
				setsockopt( pNewUserInfo->sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay) );

				// ORZ:��������
				int retcode = pNewUserInfo->Recv();
				if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
				{
					closesocket(Accept);
					continue;
				}

				UpdateStatusBar(TRUE);

				BuildPacketEx(pPacket,LOGIN_GATE,GATE2SRV_ADDSOCKET,buffer,256);
				SET_DATA(pData,LOGIN_GATE,GATE2SRV_ADDSOCKET,pPacket);
				pData->sock = Accept;
				strcpy(pData->szIp,inet_ntoa(Address.sin_addr));
				//���͸�loginsrv����
				SendExToServer(pPacket);
			}
		}
	}

	return 0;
}

//���͸�loginsrv�ر�
void CloseSession(int s)
{
	BuildPacketEx(pPacket,LOGIN_GATE,GATE2SRV_CLOSESOCKET,buffer,256);
	SET_DATA(pData,LOGIN_GATE,GATE2SRV_CLOSESOCKET,pPacket);
	pData->sock = s;
	//���͸�loginsrv����
	SendExToServer(pPacket);

	closesocket(s);

	UpdateStatusBar(FALSE);
}

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	DWORD					dwBytesTransferred = 0;

	CSessionInfo*			pSessionInfo = NULL;
	_LPTCOMPLETIONPORT		lpPerIoData = NULL;

	char					szMsg[32] = {0};

	while (TRUE)
	{
		BOOL bRet =  GetQueuedCompletionStatus((HANDLE)CompletionPortID, &dwBytesTransferred, (LPDWORD)&pSessionInfo,(LPOVERLAPPED *)&lpPerIoData, INFINITE);
		
		if ( bRet == FALSE)
		{
			if (g_fTerminated)
				return 0;

			//��� *lpOverlappedΪ�ղ��Һ���û�д���ɶ˿�ȡ����ɰ�������ֵ��Ϊ0�������򲻻���lpNumberOfBytes and lpCompletionKey��ָ��Ĳ����д洢��Ϣ
			if (lpPerIoData == NULL)
				continue;

			//�˳�
			DWORD dwErr = GetLastError();

			if (pSessionInfo != NULL)
			{
				// ��ʾһ����ʾ��Ϣ,�˳�����true
				int nRet = HandleError( pSessionInfo->sock,dwErr ) ;
				if( nRet == 0)			
				{
					break;
				}

				//�ͻ��˶˿����ӻ�������ر�
				if (nRet == 1)
				{
					//�رտͻ���
					CloseSession(pSessionInfo->sock);

					g_xSessionList.RemoveNodeByData(pSessionInfo);

					//GlobalFree(pSessionInfo);
					CSessionInfo::ObjPool().reclaimObject(pSessionInfo);
				}

			}

			continue;
		}

		if (g_fTerminated)
			return 0;

		//���󣬹رշ�����
		if (dwBytesTransferred == 0)
		{
			BuildPacketEx(pPacket,LOGIN_GATE,GATE2SRV_CLOSESOCKET,buffer,64);
			SET_DATA(pData,LOGIN_GATE,GATE2SRV_CLOSESOCKET,pPacket);
			pData->sock = pSessionInfo->sock;
			//���͸�loginsrv����
			SendExToServer(pPacket);

			g_xSessionList.RemoveNodeByData(pSessionInfo);

			closesocket(pSessionInfo->sock);
			pSessionInfo->sock = INVALID_SOCKET;

			UpdateStatusBar(FALSE);

			//GlobalFree(pSessionInfo);
			CSessionInfo::ObjPool().reclaimObject(pSessionInfo);
			continue;
		}


		// ORZ:
		pSessionInfo->bufLen += dwBytesTransferred;

		while ( pSessionInfo->HasCompletionPacket() )
		{
			BuildPacketEx(pPacket,LOGIN_GATE,GATE2SRV_MSG,buf,3096);
			SET_DATA(pData,LOGIN_GATE,GATE2SRV_MSG,pPacket);
			pData->sock = pSessionInfo->sock;
			int nLen = pSessionInfo->ExtractPacket( pData->szPacket );
			pPacket->dlen += nLen;
			SendExToServer(pPacket);
		}

		// ORZ:
		if ( pSessionInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING )
		{
			InsertLogMsg(_TEXT("WSARecv() failed"));
			
			CloseSession(pSessionInfo->sock);
			continue;
		}
	}

	return 0;
}



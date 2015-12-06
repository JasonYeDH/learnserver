#include "stdafx.h"

extern HWND								g_hStatusBar;

extern SOCKET							g_gcSock;

UINT WINAPI ThreadFuncForMsg(LPVOID lpParameter);

extern HANDLE							g_hIOCP;
extern BOOL								g_fTerminated;

CWHList<CGateInfo*>						g_xGateList;
CWHList<GAMESERVERINFO*>				g_xGameServerList;
CWHList<GATESERVERINFO*>				g_xGateServerList;
char									g_szServerList[1024];

unsigned long							g_hThreadForMsg = 0;

void UpdateStatusBar(BOOL fGrow)
{
	static long	nNumOfCurrSession = 0;

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

BOOL InitServerThreadForMsg()
{
	UINT	dwThreadIDForMsg = 0;

	if (!g_hThreadForMsg)
	{
		g_hThreadForMsg	= _beginthreadex(NULL, 0, ThreadFuncForMsg,	NULL, 0, &dwThreadIDForMsg);

		if (g_hThreadForMsg)
			return TRUE;
	}

	return FALSE;
}

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	int					nLen = sizeof(SOCKADDR_IN);

	SOCKET				Accept;
	SOCKADDR_IN			Address;

	while (TRUE)
	{
		Accept = accept(g_gcSock, (struct sockaddr FAR *)&Address, &nLen);

		if (g_fTerminated)
			return 0;

		CGateInfo* pGateInfo = new CGateInfo;

		if (pGateInfo)
		{
			pGateInfo->sock = Accept;

			CreateIoCompletionPort((HANDLE)pGateInfo->sock, g_hIOCP, (DWORD)pGateInfo, 0);

			if (g_xGateList.AddNewNode(pGateInfo))
			{
				int zero = 0;
				setsockopt(pGateInfo->sock, SOL_SOCKET, SO_SNDBUF, (char *)&zero, sizeof(zero) );
				zero = 0;
				setsockopt( pGateInfo->sock, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero));

				int nodelay = 1;
				setsockopt( pGateInfo->sock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay) );

				int retcode = pGateInfo->Recv();

				if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING))
				{
					//����ܵ��쳣����Ϣ
					closesocket(Accept);	// ������ɶ˿�ʧ����Ϣ
					continue;
				}

				UpdateStatusBar(TRUE);

#ifdef _DEBUG
				TCHAR szGateIP[256];
				wsprintf(szGateIP, _T("%d.%d.%d.%d"), Address.sin_addr.s_net, Address.sin_addr.s_host, 
															Address.sin_addr.s_lh, Address.sin_addr.s_impno);

				InsertLogMsgParam(IDS_ACCEPT_GATESERVER, szGateIP);
#endif
			}
		}
	}

	return 0;
}

void CloseGate(CGateInfo* pGateInfo)
{
}

DWORD WINAPI ServerWorkerThread(LPVOID CompletionPortID)
{
	DWORD					dwBytesTransferred = 0;
	CGateInfo*				pGateInfo = NULL;
	LPOVERLAPPED			lpOverlapped = NULL;
	char					szTmp[TCP_PACKET_SIZE + PHLen] = {0};

	while (TRUE)
	{
		BOOL bRet =  GetQueuedCompletionStatus((HANDLE)CompletionPortID, &dwBytesTransferred, (LPDWORD)&pGateInfo, (LPOVERLAPPED *)&lpOverlapped,INFINITE);

		if (bRet == FALSE)
		{
			//��� *lpOverlappedΪ�ղ��Һ���û�д���ɶ˿�ȡ����ɰ�������ֵ��Ϊ0�������򲻻���lpNumberOfBytes and lpCompletionKey��ָ��Ĳ����д洢��Ϣ
			if (lpOverlapped == NULL)
				continue;

			//�˳�
			DWORD dwErr = GetLastError();

			if (pGateInfo != NULL)
			{
				// ��ʾһ����ʾ��Ϣ,�˳�����true
				int nRet = HandleError( pGateInfo->sock,dwErr ) ;
				
				//������������
				if( nRet == 0)			
				{
					break;
				}

				//�ͻ��˶˿����ӻ�������ر�
				if (nRet == 1)
				{
					//�رտͻ���
					pGateInfo->Close();

					//���б�ɾ��
					g_xGateList.RemoveNodeByData(pGateInfo);

					SAFE_DELETE(pGateInfo);
				}

			}

			continue;
		}
		

		//ȫ���ر�
		if (g_fTerminated)
		{
			PLISTNODE		pListNode;

			if (g_xGateList.GetCount())
			{
				pListNode = g_xGateList.GetHead();

				while (pListNode)
				{
					pGateInfo = g_xGateList.GetData(pListNode);

					if (pGateInfo)
						pGateInfo->Close();

					delete pGateInfo;
					pGateInfo = NULL;

					pListNode = g_xGateList.RemoveNode(pListNode);
				}
			}

			return 0;
		}
		
		if ( dwBytesTransferred == 0 )
		{
			pGateInfo->Close();
			continue;
		}

		pGateInfo->bufLen += dwBytesTransferred;

		//������Ϣ
		while ( pGateInfo->HasCompletionPacket() )
		{
			uint32 nLen = pGateInfo->ExtractPacket(szTmp);

			Packet *pPacket = (Packet*)szTmp;
			pPacket->dlen = pPacket->dlen ^ pPacket->crc;
			pGateInfo->PacketProcess(pPacket);
			//switch (pPacket->Category)
			//{
			//	case '-':
			//		pGateInfo->SendKeepAlivePacket();
			//		break;
			//	case 'A':
			//		//�������ӣ����������
			//		pGateInfo->ReceiveSendUser(szTmp);
			//		break;
			//	case 'O':
			//		//
			//		pGateInfo->ReceiveOpenUser(&szTmp[nPacketHeader+2]);
			//		break;
			//	case 'X':
			//		//�ر�
			//		pGateInfo->ReceiveCloseUser(&szTmp[nPacketHeader+2]);
			//		break;
			//	case 'S':
			//		pGateInfo->ReceiveServerMsg(&szTmp[nPacketHeader+2]);
			//		break;
			//	case 'M':
			//		//�½���ɫ
			//		pGateInfo->MakeNewUser(&szTmp[nPacketHeader+2]);
			//		break;
			//}
		}

		if ( pGateInfo->Recv() == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING )
		{
			InsertLogMsg(_TEXT("WSARecv() failed"));
			closesocket(pGateInfo->sock);	
			continue;
		}
	}

	return 0;
}

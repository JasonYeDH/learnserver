#include "stdafx.h"
#include "../Common/Common.h"
#include "../packet/Category.h"

extern HWND						g_hStatusBar;
extern CWHList<CGateInfo*>		g_xGateList;

//�������
UINT WINAPI ThreadFuncForMsg(LPVOID lpParameter)
{
	_TDEFAULTMESSAGE	DefaultMsg;
	char				*pszBegin, *pszEnd;
	int					nCount;
	PLISTNODE			pListNode;
	CGateInfo*			pGateInfo;

	while (TRUE)
	{
		if (g_xGateList.GetCount())
		{
			pListNode = g_xGateList.GetHead();

			while (pListNode)
			{
				pGateInfo = g_xGateList.GetData(pListNode);

				if (pGateInfo)
				{
					nCount = pGateInfo->g_SendToGateQ.GetCount();

					if (nCount)
					{
						for (int nLoop = 0; nLoop < nCount; nLoop++)
						{
							_LPTSENDBUFF pSendBuff = (_LPTSENDBUFF)pGateInfo->g_SendToGateQ.PopQ();

							//�Ƚ���
							if (pSendBuff)
							{
								int nLen = pSendBuff->nLen;

								pSendBuff->szData[nLen] = '\0';

								pszBegin = pSendBuff->szData;
								{
									//����
									Packet *pPacket = (Packet*)pSendBuff->szData;
									/*pPacket->dlen = pPacket->crc^ pPacket->dlen;*/

									switch(pPacket->Category)
									{
									case ACCOUNT:
										pGateInfo->AccountProcess(pSendBuff->sock,pPacket);
										break;
									default:
										break;
									}
									//switch (DefaultMsg.wIdent)
									//{
									//	case CM_PROTOCOL:
									//		break;
									//	case CM_IDPASSWORD:
									//		pGateInfo->ProcLogin(pSendBuff->sock, pszBegin + DEFBLOCKSIZE + 2);
									//		break;
									//	case CM_SELECTSERVER:
									//		pGateInfo->ProcSelectServer(pSendBuff->sock, DefaultMsg.wParam);
									//		break;
									//	case CM_ADDNEWUSER:
									//		pGateInfo->ProcAddUser(pSendBuff->sock, pszBegin + DEFBLOCKSIZE + 2);
									//		break;
									//	case CM_UPDATEUSER:
									//		break;
									//	case CM_CHANGEPASSWORD:
									//		break;
									//}
								}

								delete pSendBuff;
								pSendBuff = NULL;
							}
						}
					}
				}

				pListNode = g_xGateList.GetNext(pListNode);
			}
		}

		SleepEx(1, TRUE);	
	}

	return 0;
}

/************************************************************************/
/* Account													
*/
/************************************************************************/
#ifndef ACCOUNT_PROTOCOL_H_
#define ACCOUNT_PROROTOL_H_

#include "Category.h"

#pragma pack(1)
enum
{
	REQ_REGISTER,			//ע��
	RES_REGISTERRLT,		//���

	REQ_LOGIN,			//ע��
	RES_LOGINRLT,		//���

	ACCOUNT_MAX,
};

#define SM_CERTIFICATION_FAIL	1				//֤��
#define SM_ID_NOTFOUND			2				//δ�ҵ�
#define SM_PASSWD_FAIL			3				//��֤ʧ��
#define SM_NEWID_SUCCESS        4				//��ӳɹ�
#define SM_NEWID_FAIL           5				//��idʧ��
#define SM_NEWID_EXISTS			6				//��id�Ѵ���
#define SM_PASSOK_SELECTSERVER	7				//
#define SM_SELECTSERVER_OK		8				

struct ACCOUNT_REQ_REGISTER_Data
{
	char szID[50];
	char szPwd[16];
};

struct ACCOUNT_RES_REGISTERRLT_Data
{
	uint8 nRlt;
};

struct ACCOUNT_REQ_LOGIN_Data
{
	char szID[50];
	char szPwd[16];
};

struct ACCOUNT_RES_LOGINRLT_Data
{
	uint8 nRlt;
};

#pragma pack()
#endif
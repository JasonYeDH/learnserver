/************************************************************************/
/* rc5,crc,md5    ��֤�ͼ���											*/
/************************************************************************/
#ifndef ENCRYPT_H_
#define ENCRYPT_H_

#include "Common.h"

//md5help
class MD5Helper
{
public:
	//ȡ�ļ�md5
	static std::string FileMd5(std::string filename);

	//ȡ����md5
	static std::string DataMd5(uint8 *bytes, int32 nLen);
};

//crc
class CrcHelper
{
public:
	//crc8
	static uint8	GetCrc8(uint8 *bytes,int32 nLen);	

	//crc16
	static uint16	GetCrc16(uint8*bytes, int32 nLen);

	//crc32
	static uint32	GetCrc32(uint8*bytes, int32 nLen);
};


//����㷨
class CRandom
{
public:
	CRandom();
	~CRandom();

	void	Random_Seed (uint32 seed);
	int32	Random_Int(int32 min, int32 max);
	int32   Random_Int  ();
private:
	uint32 m_dwSeed;
};


//����
class  CEncrypt
{
public:
	// ��򵥵�,�����(���2�ξͻ�ԭֵ)
	static void	Encrypt_Simple( uint8* pCode, int32 nSize, uint32 dwKEY );// ��򵥵����
	static void	Encrypt_Simple( uint8* pCode, int32 nSize, uint16 wKEY  );
	static void	Encrypt_Simple( uint8* pCode, int32 nSize, uint8  byKEY );

	// KEYΪ���������
	// ������DWORD, WORD, BYTE ����KEY
	static void	Encrypt_Random( uint8* pCode, int32 nSize, uint32 dwKEY );

	// �������+crc16 key��֤,wKEY_CRC(0:����,��ֵ:����+��֤)������ֵ:������֤��ͨ����ʧ��
	static bool	Encrypt_CRC16R( uint8* pCode, int32 nSize, uint16& wKEY_CRC);


	//������Կ
	static void	SetRc6Key(uint8 *pKey,uint32 nLen);

	//����	
	static bool Encrypt_RC6(uint8* pData,uint32 nLen);

	//����
	static bool Decrypt_RC6(uint8* pData,uint32 nLen);
private:
	static uint8*	m_Key;				//��Կ 0-256������16��24��32
	static uint32   m_nLen;				//����
	static bool		m_bInitKey;			//�Ƿ��ʼ��
};
#endif
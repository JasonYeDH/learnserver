/************************************************************************/
/* DBProtect��������                                                                     */
/************************************************************************/
#ifndef _DBPROTECT_H_
#define _DBPROTECT_H_

#include "Common.h"

namespace db
{

//db���ݱ���
//����������ģ��
class CDBBLOB
{
public:
	CDBBLOB(char * pData, uint32 nLen);
	~CDBBLOB();

	char* m_pData;	//����
	uint32 m_nLen;	//����
};

//db����
class CDBOperation;
//����
class CDBProtect;	

typedef std::list<CDBBLOB*> CDBBLOBList;
typedef std::list<CDBOperation*> CDBOperationList;

//����
class CDBOperation
{
public:
	CDBOperation();
	virtual ~CDBOperation();

	//���ö������ı�
	virtual void SetSQL(char* szSQL,...);

	//��Ӷ���������
	virtual void AddBlob(char *pData,uint32 nLen);

	//����Ͷ�ȡSQL��¼
	virtual uint32 Load(FILE * fp);
	virtual void   Save(FILE * fp);
	
	//ִ��
	virtual bool Excute(CDBProtect * pProtect) = 0;
protected:
	char * m_szSQL;
	CDBBLOBList m_listBLOB;
};

//dbproect
class CDBProtect
{
public:
	CDBProtect(char * szFileName);
	~CDBProtect();

	//���SQL ��¼
	virtual CDBOperation * CreateOperation() = 0;
	void AddOperation(CDBOperation * pOperation);

	//����Ͷ�ȡoperation
	bool LoadOperation();
	bool SaveOperation();

	//ִ��
	virtual bool Excute();

private:
	//����
	bool _BackUpOperation();

	CDBOperationList m_listOperation;
	char * m_szFileName;
};

}
#endif
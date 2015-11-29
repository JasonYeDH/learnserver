/************************************************************************/
/* ����������                                                                     */
/************************************************************************/
#ifndef SERVERCONFIG_H_
#define SERVERCONFIG_H_

#include  "Common.h"
#include "singleton.h"

// ���������Ϣ�ṹ��
typedef struct EngineComponentInfo
{
	
	std::string extip;										// ��������к�����ip
	uint32		extport;									// ��������к�����Ķ˿�
	std::string intip;										// �������ip
	uint32		intport;									// ��������к�����Ķ˿�
	COMPONENT_ID id;										// ���id

	std::string		dbip;			//IP
	uint32			dbport;		//�˿�
	std::string		dbuser;		//�û���
	std::string		dbpwd;		//����
	std::string		dbname;		//���ݿ���
	uint32			dbconnectcnt; //������

}ENGINE_COMPONENT_INFO;

class ServerConfig:public CSingleton<ServerConfig>
{
public:
	ServerConfig();
	~ServerConfig();
	
	//��ȡ����
	bool LoadServerConfig(char * szXml);

	ENGINE_COMPONENT_INFO& getDBSrvInfo();
	ENGINE_COMPONENT_INFO& getLoginGateInfo();
	ENGINE_COMPONENT_INFO& getLoginSrvInfo();
	ENGINE_COMPONENT_INFO& getSelGateInfo();
	ENGINE_COMPONENT_INFO& getGameGateInfo();
	ENGINE_COMPONENT_INFO& getGameSrvInfo();

private:
	ENGINE_COMPONENT_INFO m_dbSrvInfo;			//dbsrv
	ENGINE_COMPONENT_INFO m_logingateInfo;		//logingate
	ENGINE_COMPONENT_INFO m_loginsrvInfo;		//loginsrv
	ENGINE_COMPONENT_INFO m_selgateInfo;		//selgate
	ENGINE_COMPONENT_INFO m_gamegateInfo;		//gamegate
	ENGINE_COMPONENT_INFO m_gamesrvInfo;		//gamesrv
};

#define g_SeverConfig ServerConfig::GetInstance()
#endif
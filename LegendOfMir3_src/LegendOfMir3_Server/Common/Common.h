#ifndef COMMON_H_
#define COMMON_H_

#include "platform.h"
#include "format.h"

#define INLINE inline

/** ��ȫ���ͷ�һ��ָ���ڴ� */
#define SAFE_DELETE(i)										\
	if (i)													\
		{													\
		delete i;										\
		i = NULL;										\
		}

/** ��ȫ���ͷ�һ��ָ�������ڴ� */
#define SAFE_DELETE_ARRAY(i)								\
	if (i)													\
		{													\
		delete[] i;										\
		i = NULL;										\
		}

/** ��ȫ���ͷ�һ��ָ���ڴ� */
#define SAFE_FREE(i)										\
	if (i)													\
		{													\
		free(i);										\
		i = NULL;										\
		}



void printmsg(std::string str);

void printmsg(char* str);

#define ERROR_MSG(str) printmsg(str)
#define INFO_MSG(str) printmsg(str)
#define WARNING_MSG(str) printmsg(str)
#define DEBUG_MSG(str) printmsg(str)

/** ����������������� */
enum COMPONENT_TYPE
{
	UNKNOWN_COMPONENT_TYPE	= 0,
	DBSRV_TYPE				= 1,
	LOGINGATE_TYPE			= 2,
	LOGINSRV_TYPE			= 3,
	SELGATE_TYPE			= 4,
	GAMEGATE_TYPE			= 5,
	GAMESRV_TYPE			= 6,
	COMPONENT_END_TYPE		= 7,
};

/** ��ǰ�������������ID */
extern COMPONENT_TYPE g_componentType;

/** ������������������ */
const char COMPONENT_NAME[][255] = {
	"unknown",
	"dbsrv",
	"logingate",
	"loginsrv",
	"selgate",
	"gamegate",
	"gamesrv",
};

#endif
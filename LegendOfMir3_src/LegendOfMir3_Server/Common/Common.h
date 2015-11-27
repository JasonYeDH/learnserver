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

#endif
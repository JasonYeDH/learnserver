#ifndef RC6_H_
#define RC6_H_

#include "stdafx.h"

//0-256bit,����Ϊ16��24��32�ֽ�
void rc6_key_setup(unsigned char *K, int b);

//�ԳƼ��ܣ����ı䳤�ȵ�
//ע�⣺nLen һ���� 4�ı�����������0���
//����
void rc6_encrypt(unsigned char* K,int nLen);

//����
void rc6_decrypt(unsigned char* K,int nLen); 

#endif
/**************************************************************************
* Copyright (c)  ΢�߼�(WEILUOJI). All rights reserved.                   *
**************************************************************************/

/*****************************************************
�ļ�˵����Bootloader
�汾�ţ�  1.0.0
����ʱ�ڣ�2003-10-23
���ߣ�    AlexZeng
�޸ļ�¼��
******************************************************/
#include <ewindows.h>


/*Ϊ�˼�СBootLoader�Ĵ���,û�е��ÿ⺯�� */ 

/**************************************************
������size_t str_len( const char *string )
������
	IN	const char *string--��Ҫ���㳤�ȵĵ��ַ���
	OUT
	IN/OUT

����ֵ��  �ַ����ĳ���
�����������õ��ַ����ĳ���
����: 
************************************************/
size_t str_len( const char *string )
{	
	int i=0;
				
	while(*string++){
		i++;	
	}
	return i;
}

/**************************************************
������void *mem_cpy( void *dest, const void *src, size_t count )
������
	IN	  *src--Դ��ַ��ָ��  count--��С(�ֽ�)
	OUT	 *dest--Ŀ�ĵ�ַ��ָ��
	IN/OUT  

����ֵ��  ��
�����������ڴ濽����СΪcount���ַ���
����: 
************************************************/
void *mem_cpy( void *dest, const void *src, size_t count )
{
	register char * pd = (char*)dest;
	register const char * ps = (const char*)src;

	while( count )
	{
		*pd++ = *ps++;
		count--;
	}
	return dest;
}

/**************************************************
������void *mem_set( void *dest, int c, unsigned count )
������
	IN  	  c--ָ������,  count--��С(�ֽ�)
	OUT	   *dest--Ŀ�ĵ�ַ��ָ��	
	IN/OUT	

����ֵ����
������������ָ�����ݳ�ʼ���ڴ��
����: 
************************************************/
/*
void *mem_set( void *dest, int c, unsigned count )
{
	register char * pd = (char*)dest;
	while( count )
	{
		*pd++ = c; count--;
	}
	return dest;
}
*/
/**************************************************
������char *str_cpy( char *strDestination, const char *strSource )
������
	IN		*strSource--Դ��ַ��ָ��
	OUT	   *strDestination--Ŀ�ĵ�ַ��ָ�� 	
	IN/OUT	

����ֵ������Ŀ�ĵ�ַ��ָ��
���������������ַ���������
����: 
************************************************/
char *str_cpy( char *strDestination, const char *strSource )
{
	int i;
				
	i = strlen(strSource);
	memcpy((void*)strDestination, (void*)strSource, i);
	strDestination[i] = 0x0; 
				
	return strDestination;
}

/**************************************************
������char *str_cat( char *strDestination, const char *strSource )
������
	IN		*strSource--Դ��ַ��ָ��
	OUT	   *strDestination--Ŀ�ĵ�ַ��ָ�� 	
	IN/OUT	

����ֵ������Ŀ�ĵ�ַ��ָ��
��������������ַ�������һ�ַ�����ĩβ
����: 
************************************************/
char *str_cat( char *strDestination, const char *strSource )
{
	int i;
				
	i = strlen(strDestination);
	return strcpy((void*)(strDestination+i), (void*)strSource);	
}

/**************************************************
������int mem_cmp( const void *buf1, const void *buf2, size_t count )
������
	IN		*buf1--�ַ�ָ��1, *buf2--�ַ�ָ��2, count--��С
	OUT
	IN/OUT	

����ֵ��-1(С��)�� 0(����), 1(����)
�����������Ƚ��ַ����Ĵ�С
����: 
************************************************/
int mem_cmp( const void *buf1, const void *buf2, size_t count )
{
	int i;
	unsigned char *p1, *p2;
				
	p1 = (unsigned char *)buf1;
	p2 = (unsigned char *)buf2;
	for(i=0; i<count; i++){
		if( *(p1+i) > *(p2+i) ){
			return 1;
		}else if( *(p1+i) < *(p2+i) ){
			return -1;
		}
	}
	return 0;	
}

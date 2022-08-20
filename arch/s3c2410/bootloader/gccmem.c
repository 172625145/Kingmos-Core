/**************************************************************************
* Copyright (c)  微逻辑(WEILUOJI). All rights reserved.                   *
**************************************************************************/

/*****************************************************
文件说明：Bootloader
版本号：  1.0.0
开发时期：2003-10-23
作者：    AlexZeng
修改记录：
******************************************************/
#include <ewindows.h>


/*为了减小BootLoader的代码,没有调用库函数 */ 

/**************************************************
声明：size_t str_len( const char *string )
参数：
	IN	const char *string--需要计算长度的的字符串
	OUT
	IN/OUT

返回值：  字符串的长度
功能描述：得到字符串的长度
引用: 
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
声明：void *mem_cpy( void *dest, const void *src, size_t count )
参数：
	IN	  *src--源地址的指针  count--大小(字节)
	OUT	 *dest--目的地址的指针
	IN/OUT  

返回值：  无
功能描述：内存拷贝大小为count的字符串
引用: 
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
声明：void *mem_set( void *dest, int c, unsigned count )
参数：
	IN  	  c--指定内容,  count--大小(字节)
	OUT	   *dest--目的地址的指针	
	IN/OUT	

返回值：无
功能描述：用指定内容初始化内存块
引用: 
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
声明：char *str_cpy( char *strDestination, const char *strSource )
参数：
	IN		*strSource--源地址的指针
	OUT	   *strDestination--目的地址的指针 	
	IN/OUT	

返回值：返回目的地址的指针
功能描述：复制字符串的内容
引用: 
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
声明：char *str_cat( char *strDestination, const char *strSource )
参数：
	IN		*strSource--源地址的指针
	OUT	   *strDestination--目的地址的指针 	
	IN/OUT	

返回值：返回目的地址的指针
功能描述：添加字符串到另一字符串的末尾
引用: 
************************************************/
char *str_cat( char *strDestination, const char *strSource )
{
	int i;
				
	i = strlen(strDestination);
	return strcpy((void*)(strDestination+i), (void*)strSource);	
}

/**************************************************
声明：int mem_cmp( const void *buf1, const void *buf2, size_t count )
参数：
	IN		*buf1--字符指针1, *buf2--字符指针2, count--大小
	OUT
	IN/OUT	

返回值：-1(小于)， 0(等于), 1(大于)
功能描述：比较字符串的大小
引用: 
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

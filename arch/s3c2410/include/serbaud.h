/**************************************************************************
* Copyright (c)  微逻辑(WEILUOJI). All rights reserved.                   *
**************************************************************************/

#ifndef		_SERBAUD_H_
#define		_SERBAUD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	__BAUDTBL
{
	ULONG	dwBaud;		//BaudRate
	ULONG	dwDivisor;	//BaudRate Divisor

} BAUDTBL, *PBAUDTBL;

//使用的波特率表
static const	BAUDTBL	LS_BaudTable[] =
{
	{50,	    4608-1},
	{75,	    3072-1},
	{110,	    2095-1},
	{135,	    1707-1},
	{150,	    1536-1},
	{300,	    768 -1},
	{600,	    384 -1},
	{1200,	    192 -1},
	{1800,	    128 -1},
	{2000,	    115 -1},
	{2400,	    96  -1},
	{3600,	    64  -1},
	{4800,	    48  -1},
	{7200,	    32  -1},
	{9600,	    24  -1},
	{12800,	    18  -1},
	{14400,	    16  -1},
	{19200,     12  -1},
	{23040,     10  -1},
	{28800,     8   -1},
	{38400,     6   -1},
	{57600,     4   -1},
	{115200,    2   -1},
	{230400,    1   -1}
};
#define	BAUD_TABLE_SIZE		(sizeof(LS_BaudTable)/sizeof(LS_BaudTable[0]))

#ifdef __cplusplus
}	
#endif

#endif	//	_SERBAUD_H_

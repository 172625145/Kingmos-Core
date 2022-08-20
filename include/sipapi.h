/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/

#ifndef __SIPAPI_H
#define __SIPAPI_H

#ifndef __EVERSION_H
#include <eversion.h>
#endif

#include <softkey.h>

#ifdef __cplusplus
extern "C" {
#endif      /* __cplusplus */

#define SIPF_ON		0x0001
#define SIPF_OFF	0x0002

#define SipShowIM	Kingmos_SipShowIM
BOOL Kingmos_SipShowIM( DWORD dwFlag );


//	UINT uKBType  == KB_SPELL | KB_ENGLISH | KB_SYMBOL | KB_HANDWRITE
//	UINT uSubKeyType == KB_NUMBERIC | KB_SYMBOL1 | KB_SYMBOL2

#define SipSelect	Kingmos_SipSelect
BOOL Kingmos_SipSelect( UINT uKBType, UINT uSubKeyType );

// ��������
// ����ֵ����ǰ�м�����ʾ���򷵻�SIPF_ON�����򷵻�SIPF_OFF
#define GetSipStatus	Kingmos_GetSipStatus
DWORD Kingmos_GetSipStatus( void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  // __SIPAPI_H


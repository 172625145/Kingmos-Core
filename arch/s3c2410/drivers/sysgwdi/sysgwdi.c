/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/

/*****************************************************
�ļ�˵�������л�����ʾ�����������ʼ�����
�汾�ţ�1.0.0
����ʱ�ڣ�2000-04-02
���ߣ�ln
�޸ļ�¼��
******************************************************/

#include <eframe.h>
#include <gwmeobj.h>

extern DWORD CALLBACK GWDI_DisplayEnter( UINT msg, DWORD dwParam, LPVOID lParam );
extern DWORD CALLBACK GWDI_KeyEnter( UINT msg, DWORD dwParam, LPVOID lParam );
extern DWORD CALLBACK GWDI_TouchPanelEnter( UINT msg, DWORD dwParam, LPVOID lParam );

// *****************************************************************
//������BOOL _InitDefaultGwdi( void );
//������
//	IN - ��
//����ֵ��
//	�ɹ�������TRUE;ʧ�ܣ�����FALSE
//������������������ʾ�����������þ�̬���ӵķ�ʽʱ�������������������
//          �Լ���ͼ���豸�ӿں���
//
//����: ��ͼ�δ�����Ϣ�¼�ϵͳ����
// *****************************************************************
BOOL _InitDefaultGwdi( void )
{  
	// ��ʾ��������
	lpGwdiDisplayEnter = GWDI_DisplayEnter;
	// ��λ������������꣬������
	lpGwdiPosEnter = GWDI_TouchPanelEnter;
	// ������������
//	lpGwdiKeyEnter = GWDI_KeyEnter;
    return TRUE;
}


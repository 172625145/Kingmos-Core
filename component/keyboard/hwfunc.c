/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/
/*****************************************************
�ļ�˵������д����  ��д����
�汾�ţ�1.0.0
����ʱ�ڣ�2004-05-21
���ߣ��½��� Jami chen
�޸ļ�¼��
******************************************************/
#include <eWindows.h>
#include <ecomctrl.h>
#include <gwmeobj.h>
#include "hwfunc.h"

static HWND g_hHWWnd = NULL; // Ҫ����ʶ�Ľ�����͵ķ���

#define MAX_POINTNUM	1024

#define RECOGIZE_DELAY	1000
#define SYSTEMMESSAGE_DELAY 100  // �ڶ೤ʱ���ڱ�û���ƶ�������Ϊϵͳ��Ϣ����

static POINT g_pointDown[MAX_POINTNUM + 10];  //�������λ��
static int g_iPointNum = 0;  // ��ǰ�����Ŀ
static COLORREF cl_Pen = RGB(255,0,0);  // �ʵ���ɫ
static int g_iPenWidth = 2;  // �ʵĿ��

static LPPOS_CALLBACK lpSysPosEventFun = NULL;

static BOOL g_bHWMessage = FALSE; // �Ƿ��Ѿ���ʼ����д�ֹ���

static UINT g_uRecogizeDelay = 0;

static BOOL g_bExit = FALSE;
static HPEN g_hDrawPen = NULL;

static DWORD g_dwCurFlags = 0; // ��ǰ�ʵ�״̬

// �����һ�ε��±�״̬
static DWORD g_dwDownFlags;
static DWORD g_Downdx;
static DWORD g_Downdy;
static DWORD g_dwDownData;
static DWORD g_dwDownExtraInfo;

static BOOL CALLBACK HWPosEventFun( DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, DWORD dwExtraInfo );
static int WINAPI RecogizeThread( LPVOID lParam );
// **************************************************
// ������BOOL InitialHWFunction(HWND hWnd)
// ������
//	IN hWnd -- ϵͳ��Ҫ�ѱ�ʶ�Ľ�����͵Ĵ��ھ��
// 
// ����ֵ���ɹ�����TRUE�����򷵻�FALSE
// ������������ʼ����ʶϵͳ��
// ����: 
// **************************************************
BOOL InitialHWFunction(HWND hWnd)
{
	g_hHWWnd = hWnd; // ϵͳ��Ҫ�ѱ�ʶ�Ľ�����͵Ĵ��ھ��
	g_bExit = FALSE;
	StartHWFunction();
	CreateThread( NULL, 0, RecogizeThread, NULL, 0, NULL );
	return TRUE;
}


// **************************************************
// ������BOOL DeinitialHWFunction(void)
// ������
// 
// ����ֵ���ɹ�����TRUE�����򷵻�FALSE
// ����������ֹͣ��ʶϵͳ��
// ����: 
// **************************************************
BOOL DeinitialHWFunction(void)
{
	g_bExit = TRUE;
	StopHWFunction();
	return TRUE;
}

// **************************************************
// ������BOOL StartHWFunction(void)
// ������
// 
// ����ֵ���ɹ�����TRUE�����򷵻�FALSE
// ������������ʼ��ʶ���ܡ�
// ����: 
// **************************************************
BOOL StartHWFunction(void)
{
	if (lpSysPosEventFun == NULL)
		lpSysPosEventFun =  _SetPosEventCallBack( HWPosEventFun );
	return TRUE;
}

// **************************************************
// ������BOOL StopHWFunction(void)
// ������
// 
// ����ֵ���ɹ�����TRUE�����򷵻�FALSE
// ����������ֹͣ��ʶ���ܡ�
// ����: 
// **************************************************
BOOL StopHWFunction(void)
{
	if (lpSysPosEventFun )
	{
		_SetPosEventCallBack( lpSysPosEventFun ); // �ָ�ϵͳ�����������
		lpSysPosEventFun = NULL;
	}
	return FALSE;
}


// **************************************************
// ������static BOOL CALLBACK HWPosEventFun( DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, DWORD dwExtraInfo )
// ������
// IN dwFlags -- ��ǰ����״̬
// IN dx -- X����
// IN dy -- Y����
// IN dwData -- ����
// IN dwExtraInfo -- ����
// 
// ����ֵ���ɹ�����TRUE�����򷵻�FALSE
// ��������������ϵͳ�����Ϣ��
// ����: 
// **************************************************
static BOOL CALLBACK HWPosEventFun( DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, DWORD dwExtraInfo )
{
	HDC hdc;
	POINT pointDown;
//	RECT rectKbd;


	g_uRecogizeDelay = 0; 
	if (dwFlags & MOUSEEVENTF_LEFTDOWN)
	{
		// ����±� LBUTTONDOAN
//		RETAILMSG(1,(TEXT("DOWN (%d,%d)"),dx,dy));
		pointDown.x = dx;
		pointDown.y = dy;

		if (g_iPointNum == 0 && ((g_dwCurFlags & MOUSEEVENTF_LEFTDOWN ) == 0))
		{
			// ��һ���±ʣ�������һ���±ʵ�״̬
			g_dwDownFlags = dwFlags;
			g_Downdx = dx;
			g_Downdy = dy;
			g_dwDownData = dwData;
			g_dwDownExtraInfo = dwExtraInfo;
			
			// ����һ���赽��Ļ�����
			g_pointDown[g_iPointNum].x = dx; // ���浱ǰ�ĵ�
			g_pointDown[g_iPointNum].y = dy; // ���浱ǰ�ĵ�
			g_iPointNum ++ ; // ����һ����
			g_bHWMessage = TRUE; // ��ʼ��������
		}
		else
		{
			// ���ǵ�һ���±�
			if (g_bHWMessage == FALSE)
			{
				// ��ǰ�ıʵ���Ϣ��ϵͳ��Ϣ
				if (lpSysPosEventFun)
					lpSysPosEventFun(dwFlags,dx,dy,dwData,dwExtraInfo);
				return TRUE;
			}
			else
			{
				// ����д����Ϣ
				if (g_iPointNum < MAX_POINTNUM)
				{
					// �õ���һ�αʵ�λ��        
					pointDown = g_pointDown[g_iPointNum-1];
					if (pointDown.x != -1 && pointDown.x != -1)
					{  // �������
						// ���߶�
						hdc = GetDC(NULL); // �õ��豸���

						if (g_hDrawPen == NULL)
						{ // ��û�д���д�ֵı�
							g_hDrawPen = CreatePen(PS_SOLID,g_iPenWidth,cl_Pen);
						}
						if (g_hDrawPen)
						{
							SelectObject(hdc,g_hDrawPen);
						}

						MoveTo(hdc,
							  (short)(pointDown.x),
							  (short)(pointDown.y));
						LineTo(hdc,
							  (short)(dx),
							  (short)(dy));
						ReleaseDC(NULL,hdc); // �ͷž��
					}
					g_pointDown[g_iPointNum].x = dx; // ���浱ǰ�ĵ�
					g_pointDown[g_iPointNum].y = dy; // ���浱ǰ�ĵ�
					g_iPointNum ++ ; // ����һ����
					g_bHWMessage = TRUE; // ��ʼ��������

				}
			}
		}
		g_dwCurFlags = dwFlags;
	}
	else if (dwFlags & MOUSEEVENTF_LEFTUP)
	{
		// ���̧�� LBUTTONUP
//		RETAILMSG(1,(TEXT("UP (%d,%d)"),dx,dy));
		g_dwCurFlags = 0; // �Ѿ�̧��
		if (g_bHWMessage == FALSE)
		{  // ��ϵͳ��Ϣ
			if (lpSysPosEventFun)
				lpSysPosEventFun(dwFlags,dx,dy,dwData,dwExtraInfo); // ϵͳ������Ϣ
			return TRUE;
		}
		if (g_iPointNum == 1)
		{
			// ��̧��ʱֻ���±ʣ�û������������������Ϊϵͳ��Ϣ
			if (lpSysPosEventFun)
			{
				// ����һ�����Ϣ���͸�ϵͳ
				lpSysPosEventFun(g_dwDownFlags,g_Downdx,g_Downdy,g_dwDownData,g_dwDownExtraInfo);
				// ���͵�ǰ����Ϣ
				lpSysPosEventFun(dwFlags,dx,dy,dwData,dwExtraInfo); // ϵͳ������Ϣ
			}
			// �������һ�����Ϣ
			g_iPointNum = 0 ; 
			
			// ���õ�ǰ��Ϣ��ϵͳ��Ϣ
			g_bHWMessage = FALSE; 
			g_uRecogizeDelay = 0;

			return 0;
		}
		if (g_iPointNum < MAX_POINTNUM)
		{
			// �õ��ʵ�λ��

			pointDown = g_pointDown[g_iPointNum-1];
			// ���߶�
			hdc = GetDC(NULL); // �õ��豸���

			if (g_hDrawPen == NULL)
			{ // ��û�д���д�ֵı�
				g_hDrawPen = CreatePen(PS_SOLID,g_iPenWidth,cl_Pen);
			}
			if (g_hDrawPen)
			{
				SelectObject(hdc,g_hDrawPen);
			}

			MoveTo(hdc,
				  (short)(pointDown.x),
				  (short)(pointDown.y));
			LineTo(hdc,
				  (short)(dx),
				  (short)(dy));
			ReleaseDC(NULL,hdc); // �ͷž��

			g_pointDown[g_iPointNum].x = dx; // ���浱ǰ�ĵ�
			g_pointDown[g_iPointNum].y = dy; // ���浱ǰ�ĵ�
			g_iPointNum ++ ; // ����һ����

			// �߶��յ�
			g_pointDown[g_iPointNum].x = -1; // ���浱ǰ�ĵ�
			g_pointDown[g_iPointNum].y = -1; // ���浱ǰ�ĵ�
			g_iPointNum ++ ; // ����һ����
		}
//		g_bHWMessage = FALSE;
	}
	return TRUE;
}


// **************************************************
// ������static int WINAPI RecogizeThread( LPVOID lParam )
// ������
// IN lParam -- �߳���ڲ���
// 
// ����ֵ���ɹ�����TRUE�����򷵻�FALSE
// ������������ʶ�̡߳�
// ����: 
// **************************************************
static int WINAPI RecogizeThread( LPVOID lParam )
{
	int iNum;
	WORD Recognize[10];

	while(1)
	{
		if (g_bExit == TRUE)
			return 0;
		if (g_iPointNum == 1 && g_uRecogizeDelay >= SYSTEMMESSAGE_DELAY)
		{
			// ��ϵͳ�ȴ�ʱ����ֻ���±ʣ�û������������������Ϊϵͳ��Ϣ
			// ����һ�����Ϣ���͸�ϵͳ
			if (lpSysPosEventFun)
				lpSysPosEventFun(g_dwDownFlags,g_Downdx,g_Downdy,g_dwDownData,g_dwDownExtraInfo);
			
			// �������һ�����Ϣ
			g_iPointNum = 0 ; 
			
			// ���õ�ǰ��Ϣ��ϵͳ��Ϣ
			g_bHWMessage = FALSE; 
			g_uRecogizeDelay = 0;
		}

		if (g_dwCurFlags == 0)
		{  //ֻ����̧��״̬�²ſ��Ա�ʶ
			if (g_iPointNum && g_uRecogizeDelay >= RECOGIZE_DELAY)
			{// ��ʶ�ӳ��ѵ�,���бʼ�����
				//��ʼ��ʶ

				// ���ͱ�ʶ���ݸ�������
				iNum = 9;
				strcpy((LPTSTR)Recognize,(LPTSTR)"�ҷ����С���ⲻ��");

				SendMessage(g_hHWWnd,KM_RECOGNIZE,(WPARAM)iNum,(LPARAM)Recognize);
				g_iPointNum = 0;
				InvalidateRect(NULL,NULL,TRUE);

				// �Ѿ���ʶ��ɣ����õ�ǰ��Ϣ������д��Ϣ
				g_bHWMessage = FALSE; 
			}
		}

		Sleep(10);
		g_uRecogizeDelay += 10;
	}
	return 0;
}

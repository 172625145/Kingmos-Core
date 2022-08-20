/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/
/*****************************************************
文件说明：手写键盘  手写功能
版本号：1.0.0
开发时期：2004-05-21
作者：陈建明 Jami chen
修改记录：
******************************************************/
#include <eWindows.h>
#include <ecomctrl.h>
#include <gwmeobj.h>
#include "hwfunc.h"

static HWND g_hHWWnd = NULL; // 要将辩识的结果发送的方向

#define MAX_POINTNUM	1024

#define RECOGIZE_DELAY	1000
#define SYSTEMMESSAGE_DELAY 100  // 在多长时间内笔没有移动，则作为系统消息发送

static POINT g_pointDown[MAX_POINTNUM + 10];  //保留点的位置
static int g_iPointNum = 0;  // 当前点的数目
static COLORREF cl_Pen = RGB(255,0,0);  // 笔的颜色
static int g_iPenWidth = 2;  // 笔的宽度

static LPPOS_CALLBACK lpSysPosEventFun = NULL;

static BOOL g_bHWMessage = FALSE; // 是否已经开始处理写字功能

static UINT g_uRecogizeDelay = 0;

static BOOL g_bExit = FALSE;
static HPEN g_hDrawPen = NULL;

static DWORD g_dwCurFlags = 0; // 当前笔的状态

// 保存第一次的下笔状态
static DWORD g_dwDownFlags;
static DWORD g_Downdx;
static DWORD g_Downdy;
static DWORD g_dwDownData;
static DWORD g_dwDownExtraInfo;

static BOOL CALLBACK HWPosEventFun( DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, DWORD dwExtraInfo );
static int WINAPI RecogizeThread( LPVOID lParam );
// **************************************************
// 声明：BOOL InitialHWFunction(HWND hWnd)
// 参数：
//	IN hWnd -- 系统将要把辩识的结果发送的窗口句柄
// 
// 返回值：成功返回TRUE，否则返回FALSE
// 功能描述：初始化辩识系统。
// 引用: 
// **************************************************
BOOL InitialHWFunction(HWND hWnd)
{
	g_hHWWnd = hWnd; // 系统将要把辩识的结果发送的窗口句柄
	g_bExit = FALSE;
	StartHWFunction();
	CreateThread( NULL, 0, RecogizeThread, NULL, 0, NULL );
	return TRUE;
}


// **************************************************
// 声明：BOOL DeinitialHWFunction(void)
// 参数：
// 
// 返回值：成功返回TRUE，否则返回FALSE
// 功能描述：停止辩识系统。
// 引用: 
// **************************************************
BOOL DeinitialHWFunction(void)
{
	g_bExit = TRUE;
	StopHWFunction();
	return TRUE;
}

// **************************************************
// 声明：BOOL StartHWFunction(void)
// 参数：
// 
// 返回值：成功返回TRUE，否则返回FALSE
// 功能描述：开始辩识功能。
// 引用: 
// **************************************************
BOOL StartHWFunction(void)
{
	if (lpSysPosEventFun == NULL)
		lpSysPosEventFun =  _SetPosEventCallBack( HWPosEventFun );
	return TRUE;
}

// **************************************************
// 声明：BOOL StopHWFunction(void)
// 参数：
// 
// 返回值：成功返回TRUE，否则返回FALSE
// 功能描述：停止辩识功能。
// 引用: 
// **************************************************
BOOL StopHWFunction(void)
{
	if (lpSysPosEventFun )
	{
		_SetPosEventCallBack( lpSysPosEventFun ); // 恢复系统控制鼠标输入
		lpSysPosEventFun = NULL;
	}
	return FALSE;
}


// **************************************************
// 声明：static BOOL CALLBACK HWPosEventFun( DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, DWORD dwExtraInfo )
// 参数：
// IN dwFlags -- 当前鼠标的状态
// IN dx -- X坐标
// IN dy -- Y坐标
// IN dwData -- 保留
// IN dwExtraInfo -- 保留
// 
// 返回值：成功返回TRUE，否则返回FALSE
// 功能描述：处理系统鼠标消息。
// 引用: 
// **************************************************
static BOOL CALLBACK HWPosEventFun( DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, DWORD dwExtraInfo )
{
	HDC hdc;
	POINT pointDown;
//	RECT rectKbd;


	g_uRecogizeDelay = 0; 
	if (dwFlags & MOUSEEVENTF_LEFTDOWN)
	{
		// 鼠标下笔 LBUTTONDOAN
//		RETAILMSG(1,(TEXT("DOWN (%d,%d)"),dx,dy));
		pointDown.x = dx;
		pointDown.y = dy;

		if (g_iPointNum == 0 && ((g_dwCurFlags & MOUSEEVENTF_LEFTDOWN ) == 0))
		{
			// 第一次下笔，保留第一次下笔的状态
			g_dwDownFlags = dwFlags;
			g_Downdx = dx;
			g_Downdy = dy;
			g_dwDownData = dwData;
			g_dwDownExtraInfo = dwExtraInfo;
			
			// 将第一点设到点的缓存中
			g_pointDown[g_iPointNum].x = dx; // 保存当前的点
			g_pointDown[g_iPointNum].y = dy; // 保存当前的点
			g_iPointNum ++ ; // 增加一个点
			g_bHWMessage = TRUE; // 开始绘制数据
		}
		else
		{
			// 不是第一次下笔
			if (g_bHWMessage == FALSE)
			{
				// 当前的笔的消息是系统消息
				if (lpSysPosEventFun)
					lpSysPosEventFun(dwFlags,dx,dy,dwData,dwExtraInfo);
				return TRUE;
			}
			else
			{
				// 是手写笔消息
				if (g_iPointNum < MAX_POINTNUM)
				{
					// 得到上一次笔的位置        
					pointDown = g_pointDown[g_iPointNum-1];
					if (pointDown.x != -1 && pointDown.x != -1)
					{  // 不是起笔
						// 画线段
						hdc = GetDC(NULL); // 得到设备句柄

						if (g_hDrawPen == NULL)
						{ // 还没有创建写字的笔
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
						ReleaseDC(NULL,hdc); // 释放句柄
					}
					g_pointDown[g_iPointNum].x = dx; // 保存当前的点
					g_pointDown[g_iPointNum].y = dy; // 保存当前的点
					g_iPointNum ++ ; // 增加一个点
					g_bHWMessage = TRUE; // 开始绘制数据

				}
			}
		}
		g_dwCurFlags = dwFlags;
	}
	else if (dwFlags & MOUSEEVENTF_LEFTUP)
	{
		// 鼠标抬笔 LBUTTONUP
//		RETAILMSG(1,(TEXT("UP (%d,%d)"),dx,dy));
		g_dwCurFlags = 0; // 已经抬笔
		if (g_bHWMessage == FALSE)
		{  // 是系统消息
			if (lpSysPosEventFun)
				lpSysPosEventFun(dwFlags,dx,dy,dwData,dwExtraInfo); // 系统处理消息
			return TRUE;
		}
		if (g_iPointNum == 1)
		{
			// 在抬笔时只有下笔，没有做过其他动作，作为系统消息
			if (lpSysPosEventFun)
			{
				// 将第一点的信息发送给系统
				lpSysPosEventFun(g_dwDownFlags,g_Downdx,g_Downdy,g_dwDownData,g_dwDownExtraInfo);
				// 发送当前的信息
				lpSysPosEventFun(dwFlags,dx,dy,dwData,dwExtraInfo); // 系统处理消息
			}
			// 清除将第一点的信息
			g_iPointNum = 0 ; 
			
			// 设置当前消息是系统消息
			g_bHWMessage = FALSE; 
			g_uRecogizeDelay = 0;

			return 0;
		}
		if (g_iPointNum < MAX_POINTNUM)
		{
			// 得到笔的位置

			pointDown = g_pointDown[g_iPointNum-1];
			// 画线段
			hdc = GetDC(NULL); // 得到设备句柄

			if (g_hDrawPen == NULL)
			{ // 还没有创建写字的笔
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
			ReleaseDC(NULL,hdc); // 释放句柄

			g_pointDown[g_iPointNum].x = dx; // 保存当前的点
			g_pointDown[g_iPointNum].y = dy; // 保存当前的点
			g_iPointNum ++ ; // 增加一个点

			// 线段终点
			g_pointDown[g_iPointNum].x = -1; // 保存当前的点
			g_pointDown[g_iPointNum].y = -1; // 保存当前的点
			g_iPointNum ++ ; // 增加一个点
		}
//		g_bHWMessage = FALSE;
	}
	return TRUE;
}


// **************************************************
// 声明：static int WINAPI RecogizeThread( LPVOID lParam )
// 参数：
// IN lParam -- 线程入口参数
// 
// 返回值：成功返回TRUE，否则返回FALSE
// 功能描述：辩识线程。
// 引用: 
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
			// 在系统等待时间内只有下笔，没有做过其他动作，作为系统消息
			// 将第一点的信息发送给系统
			if (lpSysPosEventFun)
				lpSysPosEventFun(g_dwDownFlags,g_Downdx,g_Downdy,g_dwDownData,g_dwDownExtraInfo);
			
			// 清除将第一点的信息
			g_iPointNum = 0 ; 
			
			// 设置当前消息是系统消息
			g_bHWMessage = FALSE; 
			g_uRecogizeDelay = 0;
		}

		if (g_dwCurFlags == 0)
		{  //只有在抬笔状态下才可以辩识
			if (g_iPointNum && g_uRecogizeDelay >= RECOGIZE_DELAY)
			{// 辩识延迟已到,且有笔记数据
				//开始辩识

				// 发送辩识内容给父窗口
				iNum = 9;
				strcpy((LPTSTR)Recognize,(LPTSTR)"我方你大小里外不是");

				SendMessage(g_hHWWnd,KM_RECOGNIZE,(WPARAM)iNum,(LPARAM)Recognize);
				g_iPointNum = 0;
				InvalidateRect(NULL,NULL,TRUE);

				// 已经辩识完成，设置当前消息不是手写消息
				g_bHWMessage = FALSE; 
			}
		}

		Sleep(10);
		g_uRecogizeDelay += 10;
	}
	return 0;
}

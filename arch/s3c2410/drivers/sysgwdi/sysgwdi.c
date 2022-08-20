/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

/*****************************************************
文件说明：所有基于显示的驱动程序初始化入口
版本号：1.0.0
开发时期：2000-04-02
作者：ln
修改记录：
******************************************************/

#include <eframe.h>
#include <gwmeobj.h>

extern DWORD CALLBACK GWDI_DisplayEnter( UINT msg, DWORD dwParam, LPVOID lParam );
extern DWORD CALLBACK GWDI_KeyEnter( UINT msg, DWORD dwParam, LPVOID lParam );
extern DWORD CALLBACK GWDI_TouchPanelEnter( UINT msg, DWORD dwParam, LPVOID lParam );

// *****************************************************************
//声明：BOOL _InitDefaultGwdi( void );
//参数：
//	IN - 无
//返回值：
//	成功，返回TRUE;失败，返回FALSE
//功能描述：当基于显示的驱动程序用静态连接的方式时，驱动程序在这里加入
//          自己的图形设备接口函数
//
//引用: 被图形窗口消息事件系统调用
// *****************************************************************
BOOL _InitDefaultGwdi( void )
{  
	// 显示驱动程序
	lpGwdiDisplayEnter = GWDI_DisplayEnter;
	// 定位驱动程序，如鼠标，触摸屏
	lpGwdiPosEnter = GWDI_TouchPanelEnter;
	// 键盘驱动程序
//	lpGwdiKeyEnter = GWDI_KeyEnter;
    return TRUE;
}


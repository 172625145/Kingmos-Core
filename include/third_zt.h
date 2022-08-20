/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/

#ifndef __THIRD_ZT_H
#define __THIRD_ZT_H

#ifdef __cplusplus
extern "C" {
#endif      //__cplusplus

#include <ecomctrl.h>
#include <titlebar.h>
#include <combotoolbar.h>
#include <ztmsgbox.h>
extern const char classTitleBar[];
extern const char classComboToolBar[];
extern const char classZTBUTTON[];

#define BIGGER_ZTBUTTON_LENGTH	80
#define BIGGER_ZTBUTTON_HEIGHT	30

#define SMALLER_ZTBUTTON_LENGTH	60
#define SMALLER_ZTBUTTON_HEIGHT	30

#define ITC_TITLEBAR_CLASS		0x00000001
#define ITC_COMBOTOOLBAR_CLASS	0x00000002
#define ITC_ZTBUTTON_CLASS		0x00000003

//状态条高度
#define STATE_BAR_HEIGHT   26 
#define STATE_BAR_WIDTH    240
#define STATE_BAR_STARTX   0
#define STATE_BAR_STARTY   0
//工作域是指出了顶部的状态条外的其它部分
//屏幕坐标
#define WORK_AREA_HEIGHT   (320-STATE_BAR_HEIGHT)
#define WORK_AREA_WIDTH    240
#define WORK_AREA_STARTX   0
#define WORK_AREA_STARTY   STATE_BAR_HEIGHT

#define TITLE_BAR_HEIGHT   26
#define TITLE_BAR_WIDTH    240

#define TAB_CTRL_HEIGHT    30

//中间文本行高度，上方空2个点，下方空一个点
#define TEXT_LINE_HEIGHT   26

//客户坐标（相对于WORD_AREA）
#define TITLE_BAR_STARTX   0
#define TITLE_BAR_STARTY   0    //STATE_BAR_HEIGHT

//客户坐标（相对于WORD_AREA）
//在工作域中出了标题条的部分
#define AP_AREA_HEIGHT     (WORK_AREA_HEIGHT - TITLE_BAR_HEIGHT)
#define AP_AREA_WIDTH      240
#define AP_AREA_STARTX     0
#define AP_AREA_STARTY     (TITLE_BAR_STARTX + TITLE_BAR_HEIGHT)

#define TOOL_BAR_HEIGHT    30
#define TOOL_BAR_WIDTH     240
#define TOOL_BAR_STARTX    0
#define TOOL_BAR_STARTY    (WORK_AREA_HEIGHT - TOOL_BAR_HEIGHT)
#define TOOL_BAR_ICON_WIDTH  44
#define TOOL_BAR_ICON_HEIGHT  24

#define KEYBOARDWIDTH  240
#define KEYBOARDHEIGHT 104



#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // __THIRD_ZT_H

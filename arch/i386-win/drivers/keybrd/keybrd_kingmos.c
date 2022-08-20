#include <ewindows.h>
//#include <edef.h>
#include <isr.h>
#include <epintr.h>
#include <w32intr.h>
#include <ekeydrv.h>
#include <w32cfg.h>
#include <oalintr.h>

static HANDLE hKeyEvent;

static DWORD CALLBACK Keyboard_ISR( LPVOID lp )//DWORD dwISRHandle )
{
    LPMSG lpmsg = &IntrMsg[ID_INTR_KEYBOARD];

//    KEYRECORD keyRecord;
    
    while( 1 )
	{
		UINT uiFlag;
		WaitForSingleObject( hKeyEvent, INFINITE );
/*
		keyRecord.dwKeyValue = lpmsg->wParam;
		keyRecord.wCount = 1;
		if( lpmsg->message == WM_KEYDOWN )
			keyRecord.wState = KS_KEYDOWN;
		//!!!! Add By Jami Chen In 2001.06.29
		else if( lpmsg->message == WM_CHAR )
			keyRecord.wState = KS_CHAR;
		//!!!! Add ENd By Jami Chen In 2001.06.29
		else if( lpmsg->message == WM_KEYUP )
			keyRecord.wState = KS_KEYUP;
		else
		{
			INTR_Done( SYSINTR_KEYBOARD );
			continue;
		}

		__PutKeybrdEvent( &keyRecord );
*/
		uiFlag = 0;
		//if( lpmsg->message == WM_KEYDOWN )
		//	uiFlag = KS_KEYDOWN;
		//!!!! Add By Jami Chen In 2001.06.29

		if( lpmsg->message == WM_CHAR )
			uiFlag = KEYEVENTF_CHAR;

		//!!!! Add ENd By Jami Chen In 2001.06.29
		else if( lpmsg->message == WM_KEYUP )
			uiFlag = KEYEVENTF_KEYUP;

		keybd_event( (UCHAR)lpmsg->wParam, 0, uiFlag, 0  );
		
		INTR_Done( SYSINTR_KEYBOARD );    
	}
}


BOOL InstallKeyboardDevice( void )
{
//    return INTR_Initialize( ISR_KEYBOARD, Keyboard_ISR, 0, 
  //                           SYSINTR_KEYBOARD, 0, 0 );
	hKeyEvent = CreateEvent( NULL, NULL, NULL, NULL );
	//ISR_Register( ISR_TOUCH, TouchPanel_ISR, 0 );
	INTR_Init( SYSINTR_KEYBOARD, hKeyEvent, 0, 0 );

	CreateThread(NULL, 0, Keyboard_ISR, 0, 0, 0 );
	return TRUE;

}




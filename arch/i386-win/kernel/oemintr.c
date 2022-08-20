#include <ewindows.h>
#include <w32intr.h>
#include <oalintr.h>
//#include <isr.h>
#include <cpu.h>



/*
	@func	BOOL | OEM_InterruptEnable | Enable a hardware interrupt
	@rdesc	Returns TRUE if valid interrupt ID or FALSE if invalid ID.
	@comm 	This function is called by the Kernel when a device driver
			calls <f InterruptInitialize>. The system is not preemptible when this
			function is called.
*/

BOOL OEM_InterruptEnable (
    DWORD idInt,       // @parm Interrupt ID to be enabled. See 
                       //  <l Interrupt ID's.Interrupt ID's>  for a list of possble values.
    LPVOID pvData,     // @parm ptr to data passed in in the <f InterruptInitialize> call
    DWORD cbData       // @parm Size of data pointed to be <p pvData>
)
{
    BOOL bRetv = TRUE; 

    INTR_OFF();
	
    switch (idInt) 
	{
	case SYSINTR_RESCHED:		
		intrMask.timer0  = 1;
		break;
	case SYSINTR_KEYBOARD:
        //intrMask.mask[ID_INTR_KEYBOARD] = 1;// enable intr
		intrMask.keyboard = 1;// enable intr
		break;
	case SYSINTR_MOUSE:
        intrMask.mouse = 1;// enable intr
		//intrMask.mask[ID_INTR_MOUSE] = 1;// enable intr
		break;
	//case SYSINTR_TOUCH_CHANGED:
	//	break;
	case SYSINTR_TIMER3:
		intrMask.timer1  = 1;
		break;	
    default:
		bRetv = FALSE; 
    }

    INTR_ON();
    return bRetv;
}

/*
	@func	BOOL | OEMInterruptDisable | Disable a hardware interrupt
	@rdesc	none
	@comm 	OEMInterruptDisable is called by the Kernel when a device driver
			calls <f InterruptDisable>. The system is not preemtible when this
			function is called.
*/

void OEM_InterruptDisable( DWORD idInt )
{
    INTR_OFF();

    switch (idInt) 
	{
	case SYSINTR_GSM:
		break;		
   	case SYSINTR_TIMER3:
		break;		
	case SYSINTR_KEYBOARD:
        intrMask.keyboard = 0;// disable intr
		//intrMask.mask[ID_INTR_KEYBOARD] = 1;// enable intr
		break;		
	case SYSINTR_MOUSE:
        intrMask.mouse = 0;// disable intr
		//intrMask.mask[ID_INTR_MOUSE] = 1;// enable intr
		break;		
	//case SYSINTR_TOUCH_CHANGED:
	//	break;
	}
    INTR_ON();
}

void OEM_InterruptDone( DWORD idInt )
{
    INTR_OFF();

    switch (idInt) {
	case SYSINTR_RESCHED:
		break;		
    case SYSINTR_TIMER3:
		break;
	case SYSINTR_KEYBOARD:
        intrMask.keyboard = 1;// enable intr
		//intrMask.mask[ID_INTR_KEYBOARD] = 1;// enable intr
		break;		
	case SYSINTR_MOUSE:
        intrMask.mouse = 1;// enable intr
		//intrMask.mask[ID_INTR_MOUSE] = 1;// enable intr
		break;		
	//case SYSINTR_TOUCH_CHANGED:
	//	break;
	}
    INTR_ON();
}


static UINT ulTickCount=0;

int OEM_InterruptHandler( int dumy )
{
    ///if( intrBits.intr[ID_INTR_TIMER0] )
    //else if( intrBits.intr[ID_INTR_MOUSE] )

	if( intrBits.mouse )
    {
        intrBits.mouse = 0; // clear intr state bit
		//intrBits.intr[ID_INTR_MOUSE] = 0; // clear intr state bit
        intrMask.mouse = 0;// disable intr
		//intrMask.mask[ID_INTR_MOUSE] = 0; // disable intr

        //ISR_Active( ISR_MOUSE );
		//RETAILMSG( 1, ( "mouse.\r\n" ) );
        return SYSINTR_MOUSE;
    }
    //else if( intrBits.intr[ID_INTR_KEYBOARD] )
	else if( intrBits.keyboard )
    {
        intrBits.keyboard = 0; // clear intr state bit
		//intrBits.intr[ID_INTR_KEYBOARD] = 0;
        intrMask.keyboard = 0;// disable intr
		//intrMask.mask[ID_INTR_KEYBOARD] = 0;

        //ISR_Active( ISR_KEYBOARD );
//		RETAILMSG( 1, ( "key.\r\n" ) );
        return SYSINTR_KEYBOARD;
    }
	else if( intrBits.timer0 )
    {
        intrBits.timer0 = 0; // clear intr state bit
		//intrMask.mask[ID_INTR_KEYBOARD] = 1;// enable intr
		//intrBits.intr[ID_INTR_TIMER0] = 0; // clear intr state bit
        ulTickCount++;
		if( (ulTickCount % 1000) == 0 )
		    RETAILMSG( 1, ( "timer0=%d.\r\n", ulTickCount ) );
        return SYSINTR_RESCHED;
    }
    //else if( intrBits.intr[ID_INTR_TIMER1] )
	else if( intrBits.timer1 )
    {
        intrBits.timer1 = 0; // clear intr state bit
		//intrBits.intr[ID_INTR_TIMER1] = 0; // clear intr state bit
        //ulTickCount++;
        return SYSINTR_RTC_ALARM;
    }
	ASSERT(0);

	return SYSINTR_NOP;
}






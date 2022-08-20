#include <ewindows.h> 

void usWait(unsigned);
void msWait(unsigned); 

void usWait(unsigned usVal)
{
	#define USEC_DELAY_SCALER 50
	volatile DWORD dwScaledSecs = (usVal * USEC_DELAY_SCALER);

	while (dwScaledSecs)
	{
		dwScaledSecs--;
	}
	
}

void msWait(unsigned msVal) 
{
	msVal *= 1000;
	while( msVal-- )
		usWait(1);
}




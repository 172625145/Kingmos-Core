/*********************************************************************

Author:     Dale Roberts
Date:       8/30/95
Program:    TSTIO.EXE
Compile:    cl -DWIN32 tstio.c

Purpose:    Test the GIVEIO device driver by doing some direct
            port I/O.  We access the PC's internal speaker.

*********************************************************************/
#include <stdio.h>
#include <windows.h>
#include <math.h>
#include <conio.h>

typedef struct {
	short int pitch;
	short int duration;
} NOTE;

/*
 *  Table of notes.  Given in half steps.  It's a communication from
 * the "other side."
 */
NOTE notes[] = {{14, 500}, {16, 500}, {12, 500}, {0, 500}, {7, 1000}};

/*********************************************************************
  Set PC's speaker frequency in Hz.  The speaker is controlled by an
Intel 8253/8254 timer at I/O port addresses 0x40-0x43.
*********************************************************************/
void setfreq(int hz)
{
	hz = 1193180 / hz;						// clocked at 1.19MHz
	_outp(0x43, 0xb6);						// timer 2, square wave
	_outp(0x42, hz);
	_outp(0x42, hz >> 8);
}

/*********************************************************************
  Pass a note, in half steps relative to 400 Hz.  The 12 step scale
is an exponential thing.  The speaker control is at port 0x61.
Setting the lowest two bits enables timer 2 of the 8253/8254 timer
and turns on the speaker.
*********************************************************************/
void playnote(NOTE note)
{
	_outp(0x61, _inp(0x61) | 0x03);			// start speaker going
	setfreq((int)(400 * pow(2, note.pitch / 12.0)));
	Sleep(note.duration);
	_outp(0x61, _inp(0x61) & ~0x03);		// stop that racket!
}

/*********************************************************************
  Open and close the GIVEIO device.  This should give us direct I/O
access.  Then try it out by playin' our tune.
*********************************************************************/
int main()
{
	int i;
	HANDLE h;

    h = CreateFile("\\\\.\\giveio", GENERIC_READ, 0, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE) {
        printf("Couldn't access giveio device\n");
        return -1;
    }
	CloseHandle(h);

	for(i=0; i < sizeof(notes)/sizeof(int); ++i)
		playnote(notes[i]);

	return 0;
}


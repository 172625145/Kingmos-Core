
#include <edef.h>
#include <estdlib.h>
//#include <ewindows.h>

//typedef unsigned size_t;

//UINT	mymem[1024];

//#define TOUPPER( c ) ( ( (c) >= 'a' && (c) <= 'z' ) ? ((c) - 'a' + 'A') : (c) )
//#define TOLOWER( c ) ( ( (c) >= 'A' && (c) <= 'Z' ) ? ((c) - 'A' + 'a') : (c) )
//#define size_t unsigned 
void *memcpy( void *dest, const void *src, size_t count )
{
	register char * pd = (char*)dest;
	register const char * ps = (const char*)src;

	while( count )
	{
		*pd++ = *ps++;
		count--;
	}
	return dest;
}


void *memset( void *dest, int c, unsigned count )
{
	register char * pd = (char*)dest;
	while( count )
	{
		*pd++ = c; count--;
	}
	return dest;
}




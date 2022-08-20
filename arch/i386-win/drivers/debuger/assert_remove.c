#include <edef.h>
#include <estring.h>
#include <estdlib.h>
#include <eassert.h>
#include <stdarg.h>
#include <eframe.h>

void  ExitEsoftSystem(void);
//static HDC hdcDebug;

BOOL _InitDebug(void)
{
	return 1;
}
/*
void __ShowDebugStr( int x, int y, char *lpstr )
{
    if( hdcDebug )
        TextOut( hdcDebug, (short)x, (short)y, lpstr, strlen( lpstr ) );
}

void __ShowDebugValue( int x, int y, int v )
{
    char buf[16];

    ltoa( v, buf, 10 );
    __ShowDebugStr( x, y, buf );
}
*/

static char bufAssert[512];

void __AssertFail( char *__msg, char *__cond, char *__file, int __line, char * __notify )
{
    extern void WinDebug( void );
/*
    int l = 0;
    RECT rect;
    
	if( __notify )
	{
		strcpy( bufAssert+l, __notify );
		strcat( bufAssert, " at, " );
		l = strlen( bufAssert );
	}

	strcpy( bufAssert+l, __file );
	strcat( bufAssert, ":" );

	l = strlen( bufAssert );
	ltoa( __line, &bufAssert[l], 10 );
	rect.left = rect.top = 0;
	rect.right = 120;
	rect.bottom = 100;
	
	//DrawText( hdcDebug, buf, strlen( buf ), &rect, DT_WORDBREAK );
	// goto win32 debuger
	DbgOutString( bufAssert );
*/
	if( __notify )
	{
		EdbgOutputDebugString( "%s", __notify );
	}

	EdbgOutputDebugString(__msg, __cond, __file, __line);

	WinDebug();
}

//================format output ================================
static char strFormat[1024];
static int indexFormat = 0;
static void pOutputByte( unsigned char c )
{
	strFormat[indexFormat++] = c;
}

static void pOutputNumDecimal( unsigned long n )
{
    if (n >= 10) {
        pOutputNumDecimal(n / 10);
        n %= 10;
    }
    pOutputByte((unsigned char)(n + '0'));
}

static void pOutputNumHex( unsigned long n, long depth )
{
    if (depth) {
        depth--;
    }
    
    if ((n & ~0xf) || depth) {
        pOutputNumHex(n >> 4, depth);
        n &= 0xf;
    }
    
    if (n < 10) {
        pOutputByte((unsigned char)(n + '0'));
    } else { 
		pOutputByte((unsigned char)(n - 10 + 'A'));
	}
}

static void pOutputString( const unsigned char *s )
{
    while( *s )
    {
   	    strFormat[indexFormat++] = *s++;
    }
}
extern unsigned int _out_stream( unsigned char *pbuf, const unsigned char *format, va_list vl );
void EdbgOutputDebugString( const unsigned char *sz, ... )
{
	static int in = 0;
	//static char strbuf[1024];
	int i;
    
    in++;	

	if( in == 1 )
	{
        va_list         vl;
		va_start(vl, sz);
		if( (i = _out_stream( strFormat, sz, vl )) != 0 )
		{
			strFormat[i] = 0;
			DbgOutString( strFormat );
		}
	}	

	in--;	
}

void EdbgOutputWarnString( const unsigned char *sz, ... )
{
	extern void WarnSound( void );
	static int in = 0;
    
	//static char strbuf[1024];
	int i;
    
    in++;	

	if( in == 1 )
	{
        va_list         vl;
		va_start(vl, sz);
		if( (i = _out_stream( strFormat, sz, vl )) != 0 )
		{
			strFormat[i] = 0;
			DbgOutString( strFormat );
		}
	}

	in--;
	WarnSound();
	
}

/*
void RETAILMSG( int iCond, const unsigned char *lpsz, ... )
{
	static int in = 0;
	//static char strbuf[1024];
    
    in++;	

	if( in == 1 && iCond )
	{
        va_list         vl;
		va_start(vl, lpsz);
		if( _out_stream( strFormat, lpsz, vl ) )
			DbgOutString( strFormat );
	}	

	in--;	
}
*/

/*
void EdbgOutputDebugString( const unsigned char *sz, ... )
{
    unsigned char    c;
    va_list         vl;
    
    va_start(vl, sz);
    indexFormat = 0;
    while (*sz) {
        c = *sz++;
        switch (c) { 
		case (unsigned char)'%':
            c = *sz++;
            switch (c) { 
			case 'x':
                pOutputNumHex(va_arg(vl, unsigned long), 0);
                break;
			case 'B':
                pOutputNumHex(va_arg(vl, unsigned long), 2);
                break;
			case 'H':
                pOutputNumHex(va_arg(vl, unsigned long), 4);
                break;
			case 'X':
                pOutputNumHex(va_arg(vl, unsigned long), 8);
                break;
			case 'd': 
				{
					long    l;
					
					l = va_arg(vl, long);
					if (l < 0) 
					{ 
						pOutputByte('-');
						l = - l;
					}
					pOutputNumDecimal((unsigned long)l);
				}
                break;
			case 'u':
                pOutputNumDecimal(va_arg(vl, unsigned long));
                break;
			case 's':
                pOutputString(va_arg(vl, char *));
                break;
			case '%':
                pOutputByte('%');
                break;
			case 'c':
                c = va_arg(vl, unsigned char);
                pOutputByte(c);
                break;
                
			default:
                pOutputByte(' ');
                break;
            }
            break;
            case '\n':
				pOutputByte('\r');
				// fall through
            default:
				pOutputByte(c);
        }
    }
    
    strFormat[indexFormat]=0;
    DbgOutString( strFormat );
   
    va_end(vl);
}
*/

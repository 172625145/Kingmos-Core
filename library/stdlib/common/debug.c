#include <eframe.h>
#include <estring.h>
#include <estdlib.h>
#include <eassert.h>
#include <stdarg.h>

BOOL _InitDebug(void)
{
	return 1;
}

/*****************************************************************************
*
*
*   @func   void    |   pOutputByte | Sends a byte out of the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned int |   c |
*               Byte to send.
*
*/

static void pOutputByte( unsigned char c )
{
	TCHAR sz[2] = { c, 0 }
	DbgOutString( sz );
}


/*****************************************************************************
*
*
*   @func   void    |   pOutputNumHex | Print the hex representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*   @parm   long | depth |
*               Minimum number of digits to print.
*
*/

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


/*****************************************************************************
*
*
*   @func   void    |   pOutputNumDecimal | Print the decimal representation of a number through the monitor port.
*
*   @rdesc  none
*
*   @parm   unsigned long |   n |
*               The number to print.
*
*/
static void pOutputNumDecimal( unsigned long n )
{
    if (n >= 10) {
        pOutputNumDecimal(n / 10);
        n %= 10;
    }
    pOutputByte((unsigned char)(n + '0'));
}

/*****************************************************************************
*
*
*   @func   void    |   OutputString | Sends an unformatted string to the monitor port.
*
*   @rdesc  none
*
*   @parm   const unsigned char * |   s |
*               points to the string to be printed.
*
*   @comm
*           backslash n is converted to backslash r backslash n
*/
static void OutputString( const unsigned char *s )
{
	if( s )
	{
		while (*s) {		
			if (*s == '\n') {
				pOutputByte('\r');
			}
			pOutputByte(*s++);
		}
	}
}


// This routine will take a binary IP address as represent here and return a dotted decimal version of it
static char *inet_ntoa( unsigned long dwIP ) {

    static char szDottedD[16];

    FormatString( szDottedD, "%u.%u.%u.%u",
        (BYTE)dwIP, (BYTE)(dwIP >> 8), (BYTE)(dwIP >> 16), (BYTE)(dwIP >> 24) );

    return szDottedD;

} // inet_ntoa()

// This routine will take a dotted decimal IP address as represent here and return a binary version of it
static unsigned long inet_addr( char *pszDottedD ) {

    unsigned long dwIP = 0;
    unsigned long cBytes;
    char *pszLastNum;
    int atoi (const char *s);
    
    // Replace the dots with NULL terminators
    pszLastNum = pszDottedD;
    for( cBytes = 0; cBytes < 4; cBytes++ ) {
        while(*pszDottedD != '.' && *pszDottedD != '\0')
            pszDottedD++;
        if (pszDottedD == '\0' && cBytes != 3)
            return 0;
        *pszDottedD = '\0';
        dwIP |= (atoi(pszLastNum) & 0xFF) << (8*cBytes);
        pszLastNum = ++pszDottedD;
    }

    return dwIP;

} // inet_ntoa()


/*++

Notes: 
    supported, with no frills: x d s u X, H, and B
    backslash n is converted to backslash n backslash r.

    X is equivalent to 08x; B, 02x; and H, 04x in printf format.

--*/

//#include <nkintr.h>

//
// Functional Prototypes
//
static void pOutputByte(unsigned char c);
static void pOutputNumHex(unsigned long n,long depth);
static void pOutputNumDecimal(unsigned long n);
static void OutputString(const unsigned char *s);

char *szSprintf;

//
// Routine starts
//
/*****************************************************************************
*
*
*   @func   void    |   OutputFormatString | Simple formatted output string routine
*
*   @rdesc  none
*
*   @parm   const unsigned char * |   sz,... |
*               Format String:
*
*               @flag Format string | type
*               @flag u | unsigned
*               @flag d | int
*               @flag c | char
*               @flag s | string
*               @flag x | 4-bit hex number
*               @flag B | 8-bit hex number
*               @flag H | 16-bit hex number
*               @flag X | 32-bit hex number
*
*   @comm
*           Same as FormatString, but output to serial port instead of buffer.
*/
//static BOOL bInitCS = FALSE;

void EdbgOutputDebugString( const unsigned char *sz, ... )
{
    unsigned char    c;
    va_list         vl;
    
    va_start(vl, sz);

   
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
                OutputString(va_arg(vl, char *));
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
    
    va_end(vl);
}

/*****************************************************************************
*
*
*   @func   void    |   FormatString | Simple formatted output string routine
*
*   @rdesc  Returns length of formatted string
*
*   @parm   unsigned char * |   pBuf |
*               Pointer to string to return formatted output.  User must ensure
*               that buffer is large enough.
*
*   @parm   const unsigned char * |   sz,... |
*               Format String:
*
*               @flag Format string | type
*               @flag u | unsigned
*               @flag d | int
*               @flag c | char
*               @flag s | string
*               @flag x | 4-bit hex number
*               @flag B | 8-bit hex number
*               @flag H | 16-bit hex number
*               @flag X | 32-bit hex number
*
*   @comm
*           Same as OutputFormatString, but output to buffer instead of serial port.
*/
static unsigned int FormatString( unsigned char *pBuf, const unsigned char *sz, ... )
{
    unsigned char    c;
    va_list         vl;
    
    va_start(vl, sz);
    
    szSprintf = pBuf;
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
			case 'd': {
                long    l;
                
                l = va_arg(vl, long);
                if (l < 0) { 
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
                OutputString(va_arg(vl, char *));
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
    pOutputByte(0);
    c = szSprintf - pBuf;
    szSprintf = 0;
    va_end(vl);
    return (c);
}


/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/

/*****************************************************
�ļ�˵�����ں��жϴ���
�汾�ţ�1.0.0
����ʱ�ڣ�2003-04-04
���ߣ�����
�޸ļ�¼��
******************************************************/

#include <estdarg.h>
#include <ewindows.h>
#include <oemfunc.h>

static void pOutputByte(unsigned char c);

static void OutputNumHex(unsigned long n,long depth);
static void OutputNumDecimal(unsigned long n);
static void OutputString(const unsigned char *s);

static char * szSprintf;

// ********************************************************************
//������void EdbgOutputDebugString( const unsigned char *lpszFormat, ... )
//������
//	IN lpszFormat-�ַ�����ʽ
//����ֵ����
//������������ʽ���ַ���������Կ����
//          ֧����������
//            %u = unsigned
//            %d = int
//            %c = char
//            %s = string
//            %x = 4-bit hex number
//            %B = 8-bit hex number
//            %H = 16-bit hex number
//            %X = 32-bit hex number
//����: 
// ********************************************************************

void EdbgOutputDebugString( const char * lpszFormat, ... )
{
    unsigned char   c;
    va_list         vl;
    
    va_start( vl, lpszFormat );
   
    while( *lpszFormat )
	{
        c = *lpszFormat++;
        switch (c) { 
		case (unsigned char)'%':
            c = *lpszFormat++;
            switch (c) { 
			case 'x':
                OutputNumHex(va_arg(vl, unsigned long), 0);
                break;
			case 'B':
                OutputNumHex(va_arg(vl, unsigned long), 2);
                break;
			case 'H':
                OutputNumHex(va_arg(vl, unsigned long), 4);
                break;
			case 'X':
                OutputNumHex(va_arg(vl, unsigned long), 8);
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
					OutputNumDecimal((unsigned long)l);
				}
                break;
			case 'u':
                OutputNumDecimal(va_arg(vl, unsigned long));
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


// ********************************************************************
//������void pOutputByte( unsigned char c )
//������
//	IN c-�ַ�
//����ֵ����
//�������������Կ����һ���ַ�
//����: 
// ********************************************************************

static void pOutputByte( unsigned char c )
{
	OEM_WriteDebugByte(c);
}

// ********************************************************************
//������void OutputNumHex( unsigned long n, long depth )
//������
//	IN n-��Ҫ�������ֵ
//  IN depth-��Ҫ�������ֵ�ĳ��ȣ��� 08 �� 0008
//����ֵ����
//�������������16������ֵ
//����: 
// ********************************************************************

static void OutputNumHex( unsigned long n, long depth )
{
    if( depth )
        depth--;    
    if ( ( n & ~0xf ) || depth )
	{
        OutputNumHex( n >> 4, depth );
        n &= 0xf;
    }    
    if( n < 10 )
        pOutputByte( (unsigned char)(n + '0') );
	else
		pOutputByte( (unsigned char)(n - 10 + 'A') );
}


// ********************************************************************
//������void OutputNumDecimal( unsigned long n )
//������
//	IN n-��Ҫ�������ֵ
//����ֵ����
//�������������10������ֵ
//����: 
// ********************************************************************

static void OutputNumDecimal( unsigned long n )
{
    if( n >= 10 )
	{
        OutputNumDecimal( n / 10 );
        n %= 10;
    }
    pOutputByte( (unsigned char)(n + '0') );
}

// ********************************************************************
//������void OutputString( const unsigned char *lpsz )
//������
//	IN lpsz-��Ҫ������ַ���ָ��
//����ֵ����
//��������������ַ���
//����: 
// ********************************************************************
static void OutputString( const unsigned char *lpsz )
{
	if( lpsz )
	{
		while( *lpsz )
		{		
			if ( *lpsz == '\n' )
			{
				pOutputByte('\r');
			}
			pOutputByte( *lpsz++ );
		}
	}
}





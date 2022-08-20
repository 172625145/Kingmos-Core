/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/


/*****************************************************
�ļ�˵����ϵͳ������ģ��������
�汾�ţ�1.0.0
����ʱ�ڣ�2000
���ߣ�����
�޸ļ�¼��

******************************************************/
#include <eframe.h>
#include <gwmeobj.h>
#include <eobjtype.h>
#include "hzk16.h"

static DWORD SYSFont16_Init(void);
static BOOL SYSFont16_Deinit( DWORD dwData );
static BOOL SYSFont16_InstallFont( DWORD dwData, LPCTSTR lpszPathName );
static HANDLE SYSFont16_CreateFont( DWORD dwData, const LOGFONT *lplf );
static BOOL SYSFont16_DeleteFont( HANDLE );
static int SYSFont16_MaxHeight( HANDLE );
static int SYSFont16_MaxWidth( HANDLE );
static int SYSFont16_WordLength( HANDLE, const BYTE FAR* lpText );
static int SYSFont16_WordHeight( HANDLE, WORD aWord );
static int SYSFont16_WordWidth( HANDLE, WORD aWord );
static int SYSFont16_WordMask( HANDLE, const BYTE FAR* lpText, _LPBITMAPDATA );
static int SYSFont16_TextWidth( HANDLE, const BYTE FAR* lpText, int aLimiteWidth );
static int SYSFont16_TextHeight( HANDLE, const BYTE FAR* lpText, int aLineWidth );
static const BYTE FAR* SYSFont16_NextWord( HANDLE, const BYTE FAR* lpText );

static LPCBYTE _GetChineseMask( WORD );
//	��������ӿں���
const _FONTDRV FAR _SYSFont16Drv = {
                      SYSFont16_Init,
                      SYSFont16_Deinit,
	                  SYSFont16_InstallFont,
	                  SYSFont16_CreateFont,
	                  SYSFont16_DeleteFont,
                      SYSFont16_MaxHeight,
                      SYSFont16_MaxWidth,
                      SYSFont16_WordLength,
                      SYSFont16_WordHeight,
                      SYSFont16_WordWidth,
                      SYSFont16_WordMask,
                      SYSFont16_TextWidth,
                      SYSFont16_TextHeight,
                      SYSFont16_NextWord 
};


enum{
    USE_NOFONT = 0,
    USE_ROMFONT = 1,
    USE_FILEFONT = 2
};

#define MK_FARP( seg, ofs ) ( (void * )MAKELONG( ofs, seg ) )

#define IS_CHINESE( lpText ) ( (*(lpText) & 0x80) && ( *((lpText) + 1) & 0x80 ) )
#define IS_TEXT_MARK( v ) ( (v) == TEXT_SOFT_BREAK || (v) == TEXT_KEY_BREAK || (v) == TEXT_EOF )
#define _GET_CHINESE( lpText ) ( (WORD)( (((WORD)*(lpText)) << 8) | *((lpText) + 1) ) )
#define _GET_ENGLISH( lpText ) ( *(lpText) )
#define FONT_HEIGHT 16
#define CHINESE_WIDTH 16
#define ENGLISH_WIDTH 8


// *****************************************************************
// ������static DWORD SYSFont16_Init( void )
// ������
//	��
// ����ֵ��
//	����ɹ������ض����������򣬷���NULL
// ����������
//	��ʼ��������������
// ����: 
//	��������ӿں���
// *****************************************************************
static DWORD SYSFont16_Init( void )
{
	return 1;
}

// *****************************************************************
// ������static BOOL SYSFont16_Deinit( DWORD dwData )
// ������
//	IN dwData - ��SYSFont16_Init���صĶ�����
// ����ֵ��
//	����ɹ�������TRUE�����򣬷���FALSE
// ����������
//	�ͷŶ���
// ����: 
//	��������ӿں���
// *****************************************************************
static BOOL SYSFont16_Deinit( DWORD dwData )
{
	return 1;
}

// *****************************************************************
// ������BOOL SYSFont16_InstallFont( DWORD dwData, LPCTSTR lpszPathName )
// ������
//	IN dwData - ��ROM8x8_Init���صĶ�����
//	IN lpszPathName - �ļ�·�����������ļ���
// ����ֵ��
//	���������ļ�������������ʶ�𣬷���TRUE; ���򣬷���FALSE
// ����������
//	��װһ������
// ����: 
//	��������ӿں���
// *****************************************************************
BOOL SYSFont16_InstallFont( DWORD dwData, LPCTSTR lpszPathName )
{
	return FALSE;
}

// *****************************************************************
// ������HANDLE SYSFont16_CreateFont( DWORD dwData, const LOGFONT *lplf )
// ������
//	IN dwData - ��SYSFont16_Init���صĶ�����
//	IN lplf - LOGFONT �ṹָ�룬�����߼���������
// ����ֵ��
//	����ɹ������ش򿪵��߼����������
// ����������
//	�����߼��������
// ����: 
//	��������ӿں���
// *****************************************************************
HANDLE SYSFont16_CreateFont( DWORD dwData, const LOGFONT *lplf )
{
	return (HANDLE)1;
}

// *****************************************************************
// ������BOOL SYSFont16_DeleteFont( HANDLE handle )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
// ����ֵ��
//	����ɹ�������TRUE; ���򣬷���FALSE
// ����������
//	ɾ���߼��������
// ����: 
//	��������ӿں���
// *****************************************************************

BOOL SYSFont16_DeleteFont( HANDLE handle )
{
	return TRUE;
}

// *****************************************************************
// ������static int SYSFont16_MaxHeight( HANDLE handle )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
// ����ֵ��
//	�����߼�����ĸ߶�
// ����������
//	�õ��߼�����ĸ߶�
// ����: 
//	��������ӿں���
// *****************************************************************
static int SYSFont16_MaxHeight( HANDLE handle )
{
    return FONT_HEIGHT;
}

// *****************************************************************
// ������static int SYSFont16_MaxWidth( HANDLE handle )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
// ����ֵ��
//	�����߼�����Ŀ���
// ����������
//	�õ��߼�����Ŀ���
// ����: 
//	��������ӿں���
// *****************************************************************
static int SYSFont16_MaxWidth( HANDLE handle )
{
    return 8;
}

// *****************************************************************
// ������static int SYSFont16_WordHeight( HANDLE handle, WORD aWord )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN aWord - �ַ�����
// ����ֵ��
//	�����ַ��ĸ߶�
// ����������
//	�õ��ַ��ĸ߶�
// ����: 
//	��������ӿں���
// *****************************************************************
static int SYSFont16_WordHeight( HANDLE handle, WORD aWord )
{
    return FONT_HEIGHT;
}

// *****************************************************************
// ������static int SYSFont16_WordWidth( HANDLE handle, WORD aWord )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN aWord - �ַ�����
// ����ֵ��
//	�����ַ��Ŀ���
// ����������
//	�õ��ַ��Ŀ���
// ����: 
//	��������ӿں���
// *****************************************************************
static int SYSFont16_WordWidth( HANDLE handle, WORD aWord )
{
    return aWord < 0xff ? ENGLISH_WIDTH : CHINESE_WIDTH;
}

// *****************************************************************
// ������static int SYSFont16_WordMask( HANDLE handle, const BYTE FAR* lpText, _LPBITMAPDATA lpMask )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN lpText - �ı�ָ��
//	OUT lpMask - ���ڽ�����ģ�Ľṹָ��
// ����ֵ��
//	�����ַ�������ֽ��������ַ�Ϊ2����lpMask�������ַ��ĳ�������ģ��ַ
// ����������
//	�õ��ַ�������ֽ��������ַ�Ϊ2�����ַ��ĳ�������ģ��ַ
// ����: 
//	��������ӿں���
// *****************************************************************
static int SYSFont16_WordMask( HANDLE handle, LPCBYTE lpText, _LPBITMAPDATA lpMask )
{
#ifdef EML_DOS
    static const WORD FAR chineseMask[16] =
    { 0xfffe,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0x8002,
      0xfffe,
      0x0000 };
    static const BYTE FAR englishMask[16] =
    { 0xfe,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0x82,
      0xfe,
      0x00 };

   if( IS_CHINESE( lpText ) )
   {
       lpMask->bmWidth = CHINESE_WIDTH;
       lpMask->bmHeight = FONT_HEIGHT;
       lpMask->bmWidthBytes = 2;
       lpMask->bmBits = (BYTE FAR*)chineseMask;
       return 2;
   }
   else
   {
       lpMask->bmWidth = ENGLISH_WIDTH;
       lpMask->bmHeight = FONT_HEIGHT;
       lpMask->bmWidthBytes = 1;
       lpMask->bmBits = englishMask;
       return 1;
   }
#else
   //��ģ
   extern const unsigned char eng16Mask[];
   if( IS_CHINESE( lpText ) )
   {	//����
       lpMask->bmWidth = 16;
       lpMask->bmHeight = 16;
       lpMask->bmWidthBytes = 2;
       lpMask->bmBits = (LPBYTE)_GetChineseMask( _GET_CHINESE( lpText ) );
       return 2;
   }
   else
   {	//ascii
       lpMask->bmWidth = 8;
       lpMask->bmHeight = 16;
       lpMask->bmWidthBytes = 1;
       lpMask->bmBits = (LPBYTE)(eng16Mask+_GET_ENGLISH( lpText )*16);//_ptrInRomEFont+ _GET_ENGLISH( lpText )*16;
       return 1;
   }
#endif
}

// *****************************************************************
// ������static int SYSFont16_TextWidth( HANDLE handle, const BYTE FAR* lpText, int len )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN lpText - �ı�ָ��
//	IN len - ��Ҫͳ�Ƶ��ı�����
// ����ֵ��
//	�ı������س���
// ����������
//	�õ��ı������س��ȣ�����ı����ȣ�len���м���������ַ�����ͳ�Ƶ������ַ�λ��Ϊֹ��
// ����: 
//	��������ӿں���
// *****************************************************************

static int SYSFont16_TextWidth( HANDLE handle, LPCBYTE lpText, int len )
{
   int  w = 0;

   if( lpText )
   {
       while( !IS_TEXT_MARK( *lpText ) && *lpText && len )
       {	//���� �� �ѵ����ƿ���
           if( IS_CHINESE( lpText ) )
           {	//����
               w += SYSFont16_WordWidth( handle, _GET_CHINESE( lpText ) );
               lpText += 2;
               if( len > 0 )
                   len -= 2;
           }
           else
           {	//ascii 
               w += SYSFont16_WordWidth( handle, _GET_ENGLISH( lpText ) );
               lpText += 1;
               if( len > 0 )
                   len--;
           }
       }
   }
   return w;

}

// *****************************************************************
// ������static int SYSFont16_TextHeight( HANDLE handle, const BYTE FAR* lpText, int aLineWidth )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN lpText - �ı�ָ��
//	IN aLineWidth - ��Ҫͳ�Ƶ����������ȣ����Ϊ0����û�п�������
// ����ֵ��
//	�ı������ظ߶�
// ����������
//	�õ��ı������ظ߶ȣ�����ı����س��� ���ڵ���aLineWidth����������߶ȣ�
//	����ı��м���������ַ�������������߶ȡ�
//	���aLineWidthΪ0����û�п������ƣ��Ի����ַ�ȷ����һ��
// ����: 
//	��������ӿں���
// *****************************************************************
static int SYSFont16_TextHeight( HANDLE handle, LPCBYTE lpText, int aLineWidth )
{
    int h = 0, w = 0;

    if( lpText )
    {
        h = FONT_HEIGHT;	//Ĭ�ϸ߶�
        do  {
            if( IS_CHINESE( lpText ) )
            {	//����
                if( aLineWidth > 0 && w + CHINESE_WIDTH >= aLineWidth )
                {	//��������
                    h += FONT_HEIGHT;
                    w = 0;
                }
                else
                    w += CHINESE_WIDTH;
				//�����ַ��ֽ���Ϊ2 ����һ���ַ�
                lpText += 2;	
            }
            else
            {  //ascii�ַ� english font
                if( (aLineWidth > 0 && w + ENGLISH_WIDTH >= aLineWidth) ||
                    IS_TEXT_MARK( *lpText ) )
                {	//�������� �� �л����ַ�
                    h += FONT_HEIGHT;
                    w = 0;
                }
                else
                    w += ENGLISH_WIDTH;
                lpText++;
            }
        }while( *lpText );
    }
    return h;
}

// *****************************************************************
// ������const BYTE FAR* SYSFont16_NextWord( HANDLE handle, const BYTE FAR* lpText )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN lpText - �ı�ָ��
// ����ֵ��
//	�ı�����һ���ַ���ַָ��
// ����������
//	�õ��ı�����һ���ַ���ַָ��
// ����: 
//	��������ӿں���
// *****************************************************************
static LPCBYTE SYSFont16_NextWord( HANDLE handle, LPCBYTE lpText )
{
    return IS_CHINESE( lpText ) ? (lpText + 2) : (lpText + 1);
}

// *****************************************************************
// ������int SYSFont16_WordLength( HANDLE handle, const BYTE FAR* lpText )
// ������
//	IN handle - �� SYSFont16_CreateFont ���ص��߼����������
//	IN lpText - �ı�ָ��
// ����ֵ��
//	�ı������ַ�Ϊ��λ������
// ����������
//	�õ��ı������ַ�Ϊ��λ������
// ����: 
//	��������ӿں���
// *****************************************************************
int SYSFont16_WordLength( HANDLE handle, LPCBYTE lpText )
{
     int l = 0;
     if( lpText )
     {
         while( *lpText )
         {
             if( IS_CHINESE( lpText ) )
			 {
                 lpText += 2;
				 //l+= 2;
			 }
             else
                 lpText++;//l++;
             //lpText++;
			 l++;
         }
     }
     return l;
}

// *****************************************************************
// ������static LPCBYTE _GetChineseMask( WORD aWord )
// ������
//	IN aWord - ���ִ���
// ����ֵ��
//	���ַ�����ģ
// ����������
//	�õ�������ģ
// ����: 
//	��������ӿں���
// *****************************************************************

static LPCBYTE _GetChineseMask( WORD aWord )
{
    const static BYTE undefMask[32] = {
        0x00,0x00,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x40,0x02,
        0x00,0x00
    };

#ifdef EML_DOS
    long offset;
    BYTE hi = (BYTE)(aWord >> 8);
    BYTE lo = (BYTE)aWord;

    offset = ( ( (long)hi - 0xa1L )*94L + (long)lo - 0xa1L ) << 5;  // * 32
/*
    if( _ptrInXMSFont )
    {
        XMSSeek( _ptrInXMSFont, Offset, XMS_BEG );
        XMSRead( _ptrInXMSFont, Mask, 16, 1 );
    }
    else if( _ptrInFileFont )
    {
        fseek( _ptrInFileFont, Offset, SEEK_SET );
        fread( Mask, 32, 1, _ptrInFileFont );
    }
*/
    return (BYTE FAR*)undefMask;

#else
	//����������ģ������
    extern const unsigned char hzk16Mask[];

    long offset;
    BYTE hi = (BYTE)(aWord >> 8);
    BYTE lo = (BYTE)aWord;
	//�õ����ƫ��
    offset = ( ( (long)hi - 0xa1L )*94L + (long)lo - 0xa1L ) << 5;  // * 32
	if( offset <= 267616 - 32 )
		return (LPCBYTE)(hzk16Mask+offset);	
	else
		return undefMask;	//δ����
#endif
}
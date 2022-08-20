/******************************************************
Copyright(c) ��Ȩ���У�1998-2003΢�߼�����������Ȩ����
******************************************************/


/*****************************************************
�ļ�˵����ͼ���豸��������(BLT & PAT)
�汾�ţ�3.0.0
����ʱ�ڣ�2000-9-06
���ߣ�����
�޸ļ�¼��
     1.LN 2003-05-06, Hatch brush�д���

******************************************************/

#ifndef FUNCTION
    #error not define FUNCTION!
#endif

#ifndef BLT_ROP
    #error not define BLT_ROP
#endif

static BOOL FUNCTION( _LPBLKBITBLT lpData )
{
    LPWORD lpDestStart, lpDest;
    LPCBYTE lpPattern;
    WORD clrFore, clrBack;
    int i, j, n,  rows, cols, shift;
    BYTE mask, bitMask;
	int iDestWidthBytes = lpData->lpDestImage->bmWidthBytes;

    lpDestStart = (LPWORD)( lpData->lpDestImage->bmBits + lpData->lprcDest->top * iDestWidthBytes ) + lpData->lprcDest->left;
    lpPattern = lpData->lpBrush->pattern;
    rows = lpData->lprcDest->bottom - lpData->lprcDest->top;
    cols = lpData->lprcDest->right - lpData->lprcDest->left;
    shift = lpData->lprcMask->left & 0x07;
    clrFore = (WORD)lpData->lpBrush->color;
    clrBack = (WORD)lpData->solidBkColor;
    n = lpData->lprcDest->top;
    for( i = 0; i < rows; i++ )
    {
        mask = *(lpPattern+(n&0x07));
		n++; //LN 2003-05-06,����
        bitMask = 0x80 >> shift;
        lpDest = lpDestStart;
        for( j = 0; j < cols; j++ )
        {
            if( bitMask == 0 )
            {
                bitMask = 0x80;
            }
            if( mask & bitMask )
            {
                *lpDest = BLT_ROP( *lpDest, (WORD)clrFore );
            }
            else
                *lpDest = BLT_ROP( *lpDest, (WORD)clrBack );
            lpDest++;
            bitMask >>= 1;
        }
        lpDestStart = (LPWORD)( (LPBYTE)lpDestStart + iDestWidthBytes );//(LPWORD)( (LPBYTE)lpDestStart + lpData->lpDestImage->bmWidthBytes );
    }
    return TRUE;
}


/******************************************************
Copyright(c) 版权所有，1998-2003微逻辑。保留所有权利。
******************************************************/


/*****************************************************
文件说明：图形设备驱动程序(BLT & MERGE)
版本号：3.0.0
开发时期：2000-9-06
作者：李林
修改记录：

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
	LPWORD lpSrcStart, lpSrc;
    LPCBYTE lpPattern;
    WORD clrFore, clrBack;
    int i, j, n,  rows, cols, shift;
    BYTE mask, bitMask;
	int iSrcWidthBytes, iDestWidthBytes;
    
	iSrcWidthBytes = lpData->lpSrcImage->bmWidthBytes;
	iDestWidthBytes = lpData->lpDestImage->bmWidthBytes;

    lpDestStart = (LPWORD)( lpData->lpDestImage->bmBits + lpData->lprcDest->top * iDestWidthBytes ) + lpData->lprcDest->left;
	lpSrcStart = (LPWORD)( lpData->lpSrcImage->bmBits + lpData->lprcSrc->top * iSrcWidthBytes ) + lpData->lprcSrc->left;

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
		n++; //LN 2003-05-06,增加
        bitMask = 0x80 >> shift;
        lpDest = lpDestStart;
		lpSrc = lpSrcStart;
        for( j = 0; j < cols; j++ )
        {
            if( bitMask == 0 )
            {
                bitMask = 0x80;
            }
            if( mask & bitMask )
            {
                *lpDest = BLT_ROP( *lpSrc, (WORD)clrFore );
            }
            else
                *lpDest = BLT_ROP( *lpSrc, (WORD)clrBack );
            lpDest++;
			lpSrc++;
            bitMask >>= 1;
        }
        lpDestStart = (LPWORD)( (LPBYTE)lpDestStart + iDestWidthBytes );
		lpSrcStart = (LPWORD)( (LPBYTE)lpSrcStart + iSrcWidthBytes );
    }
    return TRUE;
}


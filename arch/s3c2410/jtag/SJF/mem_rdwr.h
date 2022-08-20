#ifndef __MEM_RDWR_H__
#define __MEM_RDWR_H__

#include "def.h"
void MRW_Menu(void);

//************** JTAG dependent functions **************
void MRW_JtagInit(void);
int S2410_Addr2Bank(U32 addr);
void S2410_Assert_nGCS(U32 addr);
void S2410_Deassert_nGCS(U32 addr);

U8 MRW_Rd8(U32 addr);
U16 MRW_Rd16(U32 addr,int en_nBE,U32 bs);
U32 MRW_Rd32(U32 addr,int en_nBE,U32 bs);
void MRW_Wr8(U32 addr,U8 data);
void MRW_Wr16(U32 addr,U16 data,int en_nBE,U32 bs);
void MRW_Wr32(U32 addr,U32 data,int en_nBE,U32 bs);
//*******************************************************


#endif //__MEM_RDWR_H__


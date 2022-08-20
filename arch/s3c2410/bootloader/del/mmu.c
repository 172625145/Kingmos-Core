/************************************************ 
  NAME    : MMU.C
  DESC	  :
  Revision: 2001.3.15: purnnamu: draft ver 0.0
 ************************************************/

#include "def.h"
#include "option.h"
#include "24x.h"
#include "24xlib.h"
#include "24xslib.h"
#include "mmu.h"

// 1) Only the section table is used. 
// 2) The cachable/non-cachable area can be changed by MMT_DEFAULT value.
//    The section size is 1MB.



void MMU_Init(void)
{
    int i,j;
    //========================== IMPORTANT NOTE =========================
    //The current stack and code area can't be re-mapped in this routine.
    //If you want memory map mapped freely, your own sophiscated MMU
    //initialization code is needed.
    //===================================================================

    MMU_DisableDCache();
    MMU_DisableICache();

    //If write-back is used,the DCache should be cleared.
    for(i=0;i<64;i++)
    	for(j=0;j<8;j++)
    	    MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
    MMU_InvalidateICache();
    
    #if 0
    //To complete MMU_Init() fast, Icache may be turned on here.
    MMU_EnableICache(); 
    #endif
    
    MMU_DisableMMU();
    MMU_InvalidateTLB();

    MMU_SetMTT(0x00000000,0x01f00000,0x00000000,RW_CNB);  //bank0
    MMU_SetMTT(0x02000000,0x03f00000,0x02000000,RW_CNB);  //bank1
    MMU_SetMTT(0x04000000,0x05f00000,0x04000000,RW_NCNB); //bank2
    MMU_SetMTT(0x06000000,0x07f00000,0x06000000,RW_NCNB); //bank3
    MMU_SetMTT(0x08000000,0x09f00000,0x08000000,RW_NCNB); //bank4
    MMU_SetMTT(0x0a000000,0x0bf00000,0x0a000000,RW_NCNB); //bank5
    //MMU_SetMTT(0x0c000000,0x0cf00000,0x0c000000,RW_CB);   //bank6-1
    //MMU_SetMTT(0x0d000000,0x0df00000,0x0d000000,RW_NCNB); //bank6-2
    MMU_SetMTT(0x0c000000,0x0de00000,0x0c000000,RW_NCNB); //download 31MB
    MMU_SetMTT(0x0df00000,0x0df00000,0x0df00000,RW_CB); //used by U24XMON 
    MMU_SetMTT(0x0e000000,0x0ff00000,0x0e000000,RW_NCNB); //bank7
    MMU_SetMTT(0x10000000,0x13f00000,0x10000000,RW_FAULT);//not used
    MMU_SetMTT(0x14000000,0x15f00000,0x14000000,RW_NCNB); //SFR
    MMU_SetMTT(0x16000000,0xfff00000,0x16000000,RW_FAULT);//not used

    MMU_SetTTBase(_MMUTT_STARTADDRESS);
    MMU_SetDomain(0x55555550|DOMAIN1_ATTR|DOMAIN0_ATTR); 
    	//DOMAIN1: no_access, DOMAIN0,2~15=client(AP is checked)
    MMU_SetProcessId(0x0);
    MMU_EnableAlignFault();
    	
    MMU_EnableMMU();
    MMU_EnableICache();
    MMU_EnableDCache(); //DCache should be turned on after MMU is turned on.
}    


// attr=RW_CB,RW_CNB,RW_NCNB,RW_FAULT
void ChangeRomCacheStatus(int attr)
{
    int i,j;
    MMU_DisableDCache();
    MMU_DisableICache();
    //If write-back is used,the DCache should be cleared.
    for(i=0;i<64;i++)
    	for(j=0;j<8;j++)
    	    MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
    MMU_InvalidateICache();
    MMU_DisableMMU();
    MMU_InvalidateTLB();
    MMU_SetMTT(0x00000000,0x01f00000,0x00000000,attr);	//bank0
    MMU_SetMTT(0x02000000,0x03f00000,0x02000000,attr);	//bank1
    MMU_EnableMMU();
    MMU_EnableICache();
    MMU_EnableDCache();
}    
    

void MMU_SetMTT(int vaddrStart,int vaddrEnd,int paddrStart,int attr)
{
    U32 *pTT;
    int i,nSec;
    pTT=(U32 *)_MMUTT_STARTADDRESS+(vaddrStart>>20);
    nSec=(vaddrEnd>>20)-(vaddrStart>>20);
    for(i=0;i<=nSec;i++)*pTT++=attr |(((paddrStart>>20)+i)<<20);
}







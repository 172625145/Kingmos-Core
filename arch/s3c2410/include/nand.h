/******************************************************************************
 *
 * System On Chip(SOC)
 *
 * Copyright (c) 2002 Software Center, Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is the confidential and proprietary information of Samsung 
 * Electronics, Inc("Confidential Information"). You Shall not disclose such 
 * Confidential Information and shall use it only in accordance with the terms 
 * of the license agreement you entered into Samsung.
 *
 *-----------------------------------------------------------------------------
 *
 *	S3C2410 BSP (WinCE3.0 & PocketPC2002)
 *
 * nand.h : S3C2410 NAND Smart Media READ/WRITE Routine Header File
 *
 * @author	zartoven@samsung.com (SOC, SWC, SAMSUNG Electronics)
 *
 * @date	2002/05/14
 * 
 * Log:
 *	2002/05/14  Start(with SOC's S3C2410 Boot Loader NAND Routines)
 *      
 ******************************************************************************
 */

#ifndef __NAND_H__
#define __NAND_H__

#define NAND_BLOCK_CNT          (1024 * 4)      /* Each Plane has 1024 Blocks   */
#define NAND_PAGE_CNT           (32)            /* Each Block has 32 Pages      */
#define NAND_PAGE_SIZE          (512)           /* Each Page has 512 Bytes      */
#define NAND_BLOCK_SIZE         (NAND_PAGE_CNT * NAND_PAGE_SIZE)

#define CMD_READID                  0x90        //  ReadID
#define CMD_READ                    0x00        //  Read
#define CMD_READ2                   0x50        //  Read2
#define CMD_RESET                   0xff        //  Reset
#define CMD_ERASE                   0x60        //  Erase phase 1
#define CMD_ERASE2                  0xd0        //  Erase phase 2
#define CMD_WRITE                   0x80        //  Write phase 1
#define CMD_WRITE2                  0x10        //  Write phase 2

/* !!! Maximum Delay Setting, Please Adjust these value to Optimize */
#define TACLS               7      
#define TWRPH0              7 
#define TWRPH1              7 

#define NF_CMD(cmd)	        {s2410NAND->rNFCMD   =  (cmd);}
#define NF_ADDR(addr)	    {s2410NAND->rNFADDR  =  (volatile unsigned char)(addr);}	
#define NF_nFCE_L()	        {s2410NAND->rNFCONF &= ~(1 << 11);}
#define NF_nFCE_H()	        {s2410NAND->rNFCONF |=  (1 << 11);}
#define NF_RSTECC()	        {s2410NAND->rNFCONF |=  (1 << 12);}
#define NF_RDDATA() 	    (s2410NAND->rNFDATA)
#define NF_WRDATA(data)     {s2410NAND->rNFDATA  =  (data);}
#define NF_WAITRB()         {while (!(s2410NAND->rNFSTAT & (1 << 0)));} 

#endif

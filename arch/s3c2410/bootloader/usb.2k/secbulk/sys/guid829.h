/*++

Copyright (c) 1997-1998  Microsoft Corporation

Module Name:

    GUID829.h

Abstract:

 The below GUID is used to generate symbolic links to
  driver instances created from user mode

Environment:

    Kernel & user mode

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

  Copyright (c) 1997-1998 Microsoft Corporation.  All Rights Reserved.

Revision History:

    11/18/97 : created

--*/
#ifndef GUID829H_INC
#define GUID829H_INC

#include <initguid.h>

/*
// {00873FDF-61A8-11d1-AA5E-00C04FB1728B}  for BULKUSB.SYS
DEFINE_GUID(GUID_CLASS_I82930_BULK, 
0x873fdf, 0x61a8, 0x11d1, 0xaa, 0x5e, 0x0, 0xc0, 0x4f, 0xb1, 0x72, 0x8b);
*/

//New GUID by purnnamu
// {8E120C45-4968-4188-BA19-9A82361C8FA8}

DEFINE_GUID(GUID_CLASS_I82930_BULK, 
0x8e120c45, 0x4968, 0x4188, 0xba, 0x19, 0x9a, 0x82, 0x36, 0x1c, 0x8f, 0xa8);


#endif // end, #ifndef GUID829H_INC


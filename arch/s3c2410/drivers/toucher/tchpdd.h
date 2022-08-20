/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-2000 Microsoft Corporation.  All rights reserved.

Module Name:  

  tchpdd.h

Abstract:  

  This module contains all the definitions for the PDD for the touch
  panel device.

Notes:

Revision History:

  John Lindquist 7/2/95

--*/

#ifndef __TCHPDD_H__
#define __TCHPDD_H__


//
// Digitizer related definitions.
//

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | DELTA_X_COORD_VARIANCE |
// Maximum allowed variance in the X coordinate samples.
//

#define DELTA_X_COORD_VARIANCE          0x8

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | DELTA_Y_COORD_VARIANCE |
// Maximum allowed variance in the X coordinate samples.
//

#define DELTA_Y_COORD_VARIANCE          0x10

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | MIN_X_DIGITIZER_COORD |
// Minimum X coordinate.
//

#define MIN_X_DIGITIZER_COORD           0

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | MAX_X_DIGITIZER_COORD |
// Maximum X coordinate.
//

#define MAX_X_DIGITIZER_COORD           (1<<10)

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | MIN_Y_DIGITIZER_COORD |
// Minimum Y coordinate.
//

#define MIN_Y_DIGITIZER_COORD           0

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | MAX_Y_DIGITIZER_COORD |
// Maximum Y coordinate.
//

#define MAX_Y_DIGITIZER_COORD           (1<<10)

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | MAX_NOISE_COUNT |
// Maximum noise count.
//

#define MAX_NOISE_COUNT                 4

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | TOUCHPANEL_SAMPLE_RATE_LOW |
// Low sampling rate.
//

#define TOUCHPANEL_SAMPLE_RATE_LOW                 100

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | TOUCHPANEL_SAMPLE_RATE_HIGH |
// High sampling rate.
//

#define TOUCHPANEL_SAMPLE_RATE_HIGH                 100

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | TOUCH_NUM_REJECT |
// Number of initial samples to reject.
//

#define TOUCH_NUM_REJECT                 1

//
// Internal Function Prototypes
//

#endif //__TCHPDD_H__

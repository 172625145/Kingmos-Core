#ifndef __REG_H
#define __REG_H

//
// Definitions for the penDataFormat, e.g. the point samples DMA'ed into memory.
//

//#define penDataFormatValidMask  0x0fff

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @const ULONG | NUMBER_SAMPLES_PER_POINT |
//
// Defines the number of samples per point sampled by the ADC.
//

#define NUMBER_SAMPLES_PER_POINT    3

//
// Status field valid values
//

//
// Timeout when talking to UCB
//
//#define TOUCH_UCB_TIMEOUT 10000

//
// @doc INTERNAL DRIVERS PDD TOUCH_PANEL
// @type TOUCHPANEL_POINT_SAMPLES |
// Array of NUMBER_SAMPLES_PER_POINT samples.
// 

typedef TOUCHPANEL_POINT_SAMPLE TOUCHPANEL_POINT_SAMPLES[ NUMBER_SAMPLES_PER_POINT ];

#endif

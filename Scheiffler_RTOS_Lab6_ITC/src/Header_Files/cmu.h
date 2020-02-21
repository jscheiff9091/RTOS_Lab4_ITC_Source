#ifndef _CMU_H
#define _CMU_h

// Include files
#include "em_cmu.h"

// Macros


//Type definitions


// Global Variables


// Function Prototypes
void CMU_InitHFClocks();

void CMU_RouteGPIOClock();

void CMU_LETIMERClockInit();

#endif

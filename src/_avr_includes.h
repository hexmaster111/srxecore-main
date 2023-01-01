/* ************************************************************************************
* File:	_avr_includes.h
* Date:	2020.05.04
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## AVR Includes
a convenience for included the necessary header files in the correct order

--------------------------------------------------------------------------

--- */


#ifndef __SRXE_AVR_INCLUDES_
#define __SRXE_AVR_INCLUDES_

#ifdef F_CPU
#undef F_CPU
#endif
#define F_CPU 16000000UL // define it now as 16 MHz unsigned long

/* ---
**NOTE:** There are some dependencies across the function set of the library.

The following AVR headers and standard headers are used by the SRXE library functions:
```C
*/
#include <avr/power.h>      // this must be included first
#include <avr/interrupt.h>  // needed for the timers and interupt handlers
#include <avr/io.h>
#include <avr/pgmspace.h>	// needed becasue we store fonts and bitmaps in program memory space
#include <avr/sleep.h>		// needed for the power functions
#include <util/atomic.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
```
--- */


#endif // __AVR_INCLUDES_

/* ************************************************************************************
* File:	_srxe_includes.h
* Date:	2020.05.04
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## SRXE Library Includes

a convenience for included the necessary header files in the correct order

--------------------------------------------------------------------------

--- */


#ifndef __SRXE_ALL_INCLUDES_
#define __SRXE_ALL_INCLUDES_

// a few convenience macros used for debugging
#define OUTHIGH(dir, port, pin) {dir |= (1<<pin); port |= (1<<pin);}
#define OUTLOW(dir, port, pin) {dir |= (1<<pin); port &= ~(1<<pin);}
#define INUP(dir, port, pin) {dir |= (1<<pin); port |= (1<<pin); dir &= ~(1<<pin);}

// we need to forward declare this so it may be used anywhere
// ideally this would all be in the printf.h file

typedef enum {
	PRINT_NONE = 0,
	PRINT_LCD,
	PRINT_RF,
	PRINT_UART
} DESTINATIONS;

int printDevicePrintf(uint8_t device, const char *format, ...);

#ifndef DEFAULT_MENU_FONT
#define DEFAULT_MENU_FONT FONT2
#endif


// reduce some code and memory usage for capabilities we do not use from printf.h
#define PRINTF_DISABLE_SUPPORT_FLOAT
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#define PRINTF_DISABLE_SUPPORT_LONG_LONG
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T

/* ---

The following order of SRXE functions is recommended:
```C
*/
#include "uart.h"       // (optional) used with the **Enigma Interface Board** for debugging
#include "leds.h"       // (optional) used with the **Enigma Interface Board** for debugging

#include "clock.h"      // convenience reference timer
#include "power.h"      // handles sleep mode and battery status
#include "eeprom.h"     // access to EEPROM storage
#include "flash.h"      // access to the tiny 128KB FLASH chip
#include "rf.h"         // RF Transceiver I/O
#include "random.h"     // pseudo random number generator (must be after RF)
#include "lcdbase.h"    // the supporting functions for the remaining LCD functions
#include "lcddraw.h"    // the basic draw primatives
#include "lcdtext.h"    // text output to the LCD
#include "keyboard.h"   // Keyboard scanning
#include "ui.h"      	// composite UI elements (requires LCD and keyboard)

#include "printf.h"     // tiny printf() capabilities with selectable output targets (RF, LCD, or UART)
/*
```
You may include only what you will use.

**Note:** To include the UART and LED functions define `SRXECORE_DEBUG` before including the library header files.
Otherwise, the UART and LED functions will be compiled out.

Including them all will not increase your final code size if you are not using the functions.

--------------------------------------------------------------------------
--- */

#endif // __SRXE_ALL_INCLUDES_

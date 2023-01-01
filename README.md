
# SMART Response XE Core Library

The SMART Response XE (SRXE) is an existing hardware device from around 2011.
It was used to a greater or lesser extent in schools as a tool for educators
to send questions to students, and the students - each with an SRXE - would
respond. The educator could see who answered, tabular correct / wrong answers, etc.

The basic hardware consists of a device, approximately 120mm * 90mm * 20mm in size.
- It is powered by 4 AAA batteries.
- It has a 384x136 LCD display with 4 levels of greyscale.
- It has a full chicklet keyboard.
- It uses an ATMega128RFA1 chip with 128KB of FLASH.
- Because it uses the ATMega128RFA1, there is also a low power 2.4GHz RF transceiver.
- There is also a 128KB FLASH chip for storing extra data.

While the specification for the RF transceiver suggests a range up to 30m,
real world testing of the SRXE in a device-to-device configuration provides a
reliable range of only 7.5m.

The hardware was short lived. A large number of SRXE devices are on the used market.
When acquiring SRXE hardware, its very common to find old batteries and corrosion.
Used SRXE devices need to be thuroughly cleaned and tested.

_See the example code in [smoketest.h](#smart-response-xe-device-smoketest-and-demo) for a suitable hardware test._

**Why Another Library:** This library was created for a specifc project using multiple existing open source projects.
However, nearly all of those projects were poorly documented and had bugs or undocumented assumptions.

This library integrates content from those other sources. It also adds detailed documentation and a significant example.
Further, the example is a comprehensive hardware smoketest for the SMART Response XE. Given many of the devices may have hidden
corrosion damage, this smoketest is a valuble resource in and of itself. This library also adds helpful UI builder functions
such as menus for the soft keys, and pop-up message boxes. The UI builder functions will likely grow over time.

The SRXEcore library includes support for the optional **Enigma Development Adapter** - a custom PCB to easily program the SRXE device.
The adapter also provides LEDs and access to 4 extra GPIO pads directly accessable through the battery compartment.

![Enigma Development Adapter](https://gitlab.com/bradanlane/srxecore/-/raw/main/docs/pcb_512.png)

_Files related to the **Enigma Development Adapter** are located in the `docs` folder._

_The [font_gen.py](#smart-response-xe-font-generation) and [bitmap_gen.py](#smart-response-xe-bitmaps) tools are documented at the end._

Feel free to jump to a specific section:
[Clock](#clock) [Power](#power) [EEPROM](#eeprom) [FLASH](#flash) [RF Transceiver](#rf) [Keyboard](#keyboard)
[LCD](#lcd) [PrintF](#printf) [UART](#uart) [LEDs](#leds)

Complete Table of Contents:

[TOC]

--------------------------------------------------------------------------


## AVR Includes
a convenience for included the necessary header files in the correct order

--------------------------------------------------------------------------


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


## SRXE Library Includes

a convenience for included the necessary header files in the correct order

--------------------------------------------------------------------------



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


## common.h - common helper functions used by multiple modules of the SRXEcore

The SRXE core has been derived from the work of multiple sources including:
 - Original keyboard and LCD code - BitBank Software, Inc. / Larry Bank bitbank@pobox.com
 - Updates to keyboard mapping and LCD scroll code - fdufnews fdufnews@free.fr
 - Original RF code SparkFun Electronics / Jim Lindblom

The code has been extensively refactored.
 - The font generation is all new to support more flexibility in font choices.
 - The text drawing is all new to match the new font generation.
 - The bitmap handling has been updated to handle bitmaps of any width * height up to that of the LCD.
 - High level UI drawing operations have been added.
 - Debugging capabilities have been added (using the 10-pins accessed from the battery compartment):
 	- BitBang UART (9600-8N1) using the exposed TDI pin
    - LEDs may be connected to TDK, TMS, TDO, and TDI

_The debugging capabilities are generally available and have been tested with the **Enigma Development Adapter**._

--------------------------------------------------------------------------


### COMMON PIN HANDLING

For convenience and simplicity of writing code, internal code uses a `pincode` syntax.

A pin code is an 8 bit value with the top 4 bits representing the PORT and the bottom 4 bits as the PIN.
The Port mapping is a bit unusual since the ATMega128RFA1 exposes PORT B, D, E, F, and G;
but obviously there is no HEX 'G'.

Please use the following definitions for creating pin codes.

```C
*/
#define SRXE_PORTB 0xB0
#define SRXE_PORTD 0xD0
#define SRXE_PORTE 0xE0
#define SRXE_PORTF 0xF0
#define SRXE_PORTG 0xA0
/*
```
Pins are _zero based_ so 0=`PIN0`, 1=`PIN1`, ... 7=`PIN7`.


#### uint8_t srxePinMapper(...)

Parameters:
 - uint8_t pincode - _see common pin handling _(above)_ for using macros to create pin codes
 - *volatile uint8_t \*\*ddr - a pointer for returning the AVR direction register
 - volatile uint8_t \*\*port - a pointer for returning the AVR port register
 - bool in - flag to indicate the pin is for input

This function is really an internal function but it is so cool an idea, it deserves to be documented!

It will assign the **ddr** and **port** with the corresponding AVR registers and returns the **pin**.

#### void srxePinMode(...)

Parameters:
- uint8_t pincode - a pincode _(as detailed above)_
- uint8_t mode - one of `INPUT`, `INPUT_PULLUP`, or `OUTPUT`

Set the specified AVR pin to the mode.

#### void srxeDigitalWrite(uint8_t pincode, uint8_t value)

This is similar to the Arduino `digitalWrite()` but uses the pin code syntax.

Use the available `#define` values of `HIGH` or `LOW`.

#### uint8_t srxeDigitalRead(uint8_t pincode)

This is similar to the Arduino `digitalRead()` but uses the pin code syntax.

Will return one of the `#define` values of `HIGH` or `LOW`.

#### long srxeMap(long val, long in_min, long in_max, long out_min, long out_max)

Re-map a number from one range to another.

The return value is the input value mapped using the min/max mapping from in to out.


## Clock
**A 1 Millisecond Reference Timer**

The clock module uses TIMER2 and the COMPA vector to establish a timer and exposes it with 1 millisecond
accuracy. This is useful for performing timed operations, animations, detecting timeouts, etc.

**Note:** The timer runs at 10Khz and every 10th interupt we increment the milliseconds counter.
It is possible to use this time for other tasks.
A similar design is used in [ecccore](https://gitlab.com/bradanlane/ecccore) where
the charliePlexed LEDs are hooked onto the clock timer.

--------------------------------------------------------------------------

#### void clockInit()

Initialize the clock.

This function must be called prior to using any other clock functions.

#### uint32_t clockMillis()

Return the current counter as a 32bit unsigned integer. The counter starts at 0 when clockInit() is first called.

#### void clockDelay(uint32_t duration)

Delay for the specified number of milliseconds.
This exists since the low level `_delay_ms()` requires durations at compile time
whereas this function lets the duration be determined at runtime.


## Power
**Power Button and Voltage Reporting**

### Power Management and Battery Life ...

|FIRMWARE|SLEEP|IDLE|+RF|+UART|
|:-----|-----:|-----:|-----:|-----:|
|Original|450uA|1.3mA|13.5mA|N/A|
|fdufnews|*440uA|8.9mA|N/A|N/A|
|SRXEcore|440uA|8.5mA|20.0mA|32mA|

* _fdufnews reports his sleep measurement as 250uA_

The LDO is 90% efficient as sleep current levels. The AAA*4 batteries supply 6V @ 1000mA.
Therefore, calculations yield a maximum sleep time of approximately 150 days.

It is strongly suggested to remove batteries whenever the SRXE will not be used for more than a couple of weeks.

--------------------------------------------------------------------------

#### void powerInit()

Initialize the power system. This function must be called prior to using any other power functions.

#### bool powerButtonState()

Report if the power button is pressed or not.

The function will return `false` if the button is not pressed.

This function may be used to create custom features based on the state of the power button including detecting presses of varying duration.

#### bool powerButtonPressed()

Report if the power button has a long press.
The function will return `false` if the button is not pressed or if it is not a long press.
If it detects a long press, it will wait for the release before returning `true`.

#### void powerSleep()

Place the SRXE into _powerdown sleep_ state.

The SRXE will exit _powerdown sleep_ state when the power button is pressed.
This function will not return while the SRXE is in its _sleep_ state.

**Note:** the calling code is responsible for restoring any system capabilities.

Here is a sample of the pre/post `powerSleep()` code which is executed when `powerButtonPressed()` returns `true`:
```C
void my_sleep_function(uint8_t rf_state) {
    rfTerm();       // turn off RF transceiver
    uartTerm();     // turn off UART if using the bit-bang module
    ledsOff();      // turn off all LEDs if using the LEDs module
    lcdSleep();     // turn off LCD
    powerSleep();   // performa actual sleep
    lcdWake();      // restart LCD
    uartInit();     // re-initialize UART if using the bit-bang module
    rfInit(1);      // re-initialize RF Transceiver on channel 1

    // re-paint display as needed
}
```

#### void powerSleepUpdate()

Restart activity timeout before sleep

#### void powerSleepConditionally()

power down and sleep if the power button is pressed or the idle timer has run out

_Typically called from your main program loop._


#### uint16_t powerBatteryLevel(void)

Return the battery level in millivolts as a 16 bit integer


## EEPROM
**read/write interface to EEPROM memory**

The module provides basic read/write of the microcontroller eeprom.

**Tip:** Writing a specific bytecode to a specific location of the EEPROM is an easy way to know if
it has been used before or if initial data should be written. If the bytecode is missing or incorrect,
then it can be considered "unformatted" or "corrupted" and any initial data should be written.

--------------------------------------------------------------------------

#### void eepromInit()

Initialization of the EEPROM functions.

This function must be called prior to using any other EEPROM functions.

#### uint8_t eepromIsReady()

Returns `true` if the eeprom is ready and `false` if it busy.

#### void eepromWriteByte()

Write a single byte to an eeprom memory location.
The write is ignored if the address is outside the EEPROM storage as defined by `EEPROM_MAX_ADDRESS`.

void eepromReadByte() - read a single byte from eeprom memory relative to the eeprom base address
The read is ignored if the address is outside the EEPROM storage as defined by `EEPROM_MAX_ADDRESS` and will return 0.

#### char* eepromSignature()

Returns a pointer to a 6 character static buffer with the MCU signature ID converted to an ASCII string.


#### int eepromAddID(char \*new_code)

Store a 6 character code, checking if the `new_code` has previously been stored.

_Useful for tracking interations with other similar devices such as RF traffic._



## Random
**Pseudo Random Number Generator**

**NOTE:** This code uses the random number generator build into the ATMega128RFA1.

--------------------------------------------------------------------------

#### void randomInit()

Initialization of the pseudo random number generator.

This function must be called prior to using any other random functions.

**Note:** This function uses the ATMega128RFA1 RF subsystem to create the random seed value.
If the RF transceiver is not available, the seed will not be random.


#### uint16_t randomGetSeed()

return seed value being used by the pseudo random number generator.


#### uint16_t randomNumGet(uint16_t max)

return a pseudo random number between 0 and `max` (not including `max`).


#### uint8_t randomByteGet()

return a pseudo random byte value between 0x00 and 0xFF.


#### uint16_t randomWordGet()

return a pseudo random word value between 0x0000 and 0xFFFF.


#### char randomCharGet()

return a pseudo random upper case letter (A through Z inclusive).



## FLASH
**I/O access to the tiny 128KB FLASH chip included with the SRXE**

The SRXE has a small 128KB FLASH chip _(yes, that is 128 kilobytes)_.
This FLASH is useful for persistent data storage.

The chip is the MX25L1005C - [Macronix 1Mbit/128KB serial flash](http://www1.futureelectronics.com/doc/MACRONIX/MX25L1005CZUI-12GTR.pdf) memory chip.

**Warning:** all write operations must occur as a 256 byte page and on a page boundary.
A page must be in an erased state prior to a write operation.
Pages are stored in sectors.
A sector is 4KB and must be erased as a complete unit.
Thus, to re-write a 256 page, it's entire sector must be erased.
Any page/sector management is left as a tedious exercise for the developer.

--------------------------------------------------------------------------

#### void flashInit()

Initialization of the FLASH chip functions.

This function must be called prior to using any other FLASH functions.

#### int flashEraseSector(uint32_t addr, bool wait)

Erase the specified 4KB sector, with an option to wait for it to complete.

Returns `false` if the operation failed or timed out.

**Notes:**
 - It will wait no more than 100ms.
 - If you intend to perform a write operations immediately after the erase, then use `wait`.

#### int flashWritePage(uint32_t addr, uint8_t* data)

Write a page (up to 256 bytes) of data.

Returns `false` if the operation failed.

**Note:** It will wait no more than 25ms.

#### int flashWritePage(uint32_t addr, uint8_t* buffer, uint16_t count)

Read `count` bytes of data from FLASH.


## RF
**I/O Functions for the RF Transceiver**

The SRXE uses the ATMega128RFA1 with includes a 2.4GHz RF transceiver.
This library provides only the most basic read/write functions.

It is possible to build protocol specific communications on top
of these functions.

The RF transceiver has a hardware level 128 byte buffer.
Data transmission functions are limited to this buffer size.

The RF transceiver uses approximately 12.5-14.5mA of power.

--------------------------------------------------------------------------

There are two internal byte values available for debugging RF related issues.
They are:
```C
*/

//int8_t _rf_tx_debug;   // = -length of tx data while transmitting; = length of transmitted data when finished
//int8_t _rf_rx_debug;   // = 0 while receiving data; = length of received data when finished

/*
```

#### rfInit(uint8_t channel)

Initialize the RF transceiver on the specified channel.

The RF Transceiver has 16 possible channels (1 .. 16)

Must be called to initialize the RF transceiver prior to using any other RF functions.

#### rfTerm()

Close and powerdown the RF transceiver.

Use this function when the RF transceiver is not actively needed to conserve battery power.

Use rfInit(_channel_) to begin using the RF transceiver again.

#### uint8_t rfInited()

Return the RF channel if inited, otherwise return 0.

This is helpful to perform shutdown/wake efficiently and for controlling the RF radio only when needed

### Helper Functions

#### void rfFlushReceiveBuffer()

Flush any pending data in the receive buffer (useful if you are waiting on a specifc message and have detected it is corrupted).

#### bool rfReceiveBufferOverflow()

 Returns `true` if the receive buffer has filled up.
 This is a good indication that subsequent data is unreliable.

### Read Functions

#### int rfAvailable()

Return the number of unread bytes in receive buffer.

#### int rfGetByte()

Return a single byte from the receive buffer (getchar-style) returns -1 if no byte is available.

#### int rfGetBuffer(uint8_t *data, uint8_t maxlen)

Return up to `maxlen` bytes from the receive buffer into the `data` buffer.
Returns the number of bytes stored in the `data` buffer.

### Write Functions

#### void rfTransmitNow()

Transmit any data which hs been put into the TX buffer.

This function is often used after a series of `rfPutByte()`, `rfPutBuffer()`, or `rfPutString()` calls.
to transmit all of the data.
-- */
void rfTransmitNow() {
	if (!_rf_obj.inited)
		return;

	RF_TX_FRAME();
}

/* ---
#### int rfPutByte(uint8_t txData)

Stores a byte of data in the transmit buffer.

Does not actually transmit the data.
Use `rfTransmitNow()` to begin transmitting the data.
If the transmit buffer reaches `HW_FRAME_TX_SIZE` bytes, it will automatically transmit.
-- */
int rfPutByte(uint8_t txData) {
	if (!_rf_obj.inited)
		return -1;

	int rtn = bufferPut(&(_rf_obj.txBuffer), txData);

	if (_rf_obj.txBuffer.length >= (HW_FRAME_TX_SIZE)) {
		RF_TX_FRAME();
	}

	return rtn;
}


/* ---
#### int rfPutBuffer(uint8_t* data, uint8_t len)

Stores a byte of data in the transmit buffer.

Does not actually transmit the data.
Use `rfTransmitNow()` to begin transmitting the data.
If the transmit buffer reaches `HW_FRAME_TX_SIZE` bytes, it will automatically transmit.

#### int rfPutByte(uint8_t txData)

Stores a byte of data in the transmit buffer.

Does not actually transmit the data.
Use `rfTransmitNow()` to begin transmitting the data.
If the transmit buffer reaches `HW_FRAME_TX_SIZE` bytes, it will automatically transmit.
-- */
int rfPutByte(uint8_t txData) {
	if (!_rf_obj.inited)
		return -1;

	int rtn = bufferPut(&(_rf_obj.txBuffer), txData);

	if (_rf_obj.txBuffer.length >= (HW_FRAME_TX_SIZE)) {
		RF_TX_FRAME();
	}

	return rtn;
}


/* ---
#### int rfPutBuffer(uint8_t* data, uint8_t len)

Stores a byte of data in the transmit buffer.

Does not actually transmit the data.
Use `rfTransmitNow()` to begin transmitting the data.
If the transmit buffer reaches `HW_FRAME_TX_SIZE` bytes, it will automatically transmit.

#### int rfPutBuffer(uint8_t* data, uint8_t len)

Stores a byte of data in the transmit buffer.

Does not actually transmit the data.
Use `rfTransmitNow()` to begin transmitting the data.
If the transmit buffer reaches `HW_FRAME_TX_SIZE` bytes, it will automatically transmit.

#### int rfPutString(char* data)

Stores the string in the transmit buffer and transmit it.


## Keyboard
**Keyboard input**

The SRXE keyboard has three mode - **normal**, **shift**, and **symbol**.
The keyboard also includes the ten soft menu keys. These are located five on either side of the LCD screen,
and the is a four-way button in the lower right.

The original keyboard code from **fdufnews** have been extended with **shift** and **symbol** menus as well
as mappings for most available keys. It is strongly recommended to use the key definitions to avoid confusion.
The mapping of ENTER and DEL have been restored.

**FYI:** The ENTER key is not marked on the SRXE keyboard.
It is located in the center of the 4-way navigation pad.
Pressing the center of the NAV pad will generate the ENTER key.

--------------------------------------------------------------------------

There are definitions for the various special keys. They are:
```C
*/
#define KEY_NOP             0
#define KEY_SHIFT           KEY_NOP
#define KEY_SYM             KEY_NOP

#define KEY_DEL             0x08
#define KEY_ENTER           0x0D
#define KEY_ESC             0x1B

#define KEY_MENU1           0x80
#define KEY_MENU2           0x81
#define KEY_MENU3           0x82
#define KEY_MENU4           0x83
#define KEY_MENU5           0x84
#define KEY_MENU6           0x85
#define KEY_MENU7           0x86
#define KEY_MENU8           0x87
#define KEY_MENU9           0x88
#define KEY_MENU10          0x89
#define KEY_LEFT            0x8A
#define KEY_RIGHT           0x8B
#define KEY_UP              0x8C
#define KEY_DOWN            0x8D

#define KEY_MENU1SH         0x90
#define KEY_MENU2SH         0x91
#define KEY_MENU3SH         0x92
#define KEY_MENU4SH         0x93
#define KEY_MENU5SH         0x94
#define KEY_MENU6SH         0x95
#define KEY_MENU7SH         0x96
#define KEY_MENU8SH         0x97
#define KEY_MENU9SH         0x98
#define KEY_MENU10SH        0x99
#define KEY_PGUP            0x9A
#define KEY_PGDN            0x9B
#define KEY_HOME            0x9C
#define KEY_END             0x9D

#define KEY_MENU1SY         0xA0
#define KEY_MENU2SY         0xA1
#define KEY_MENU3SY         0xA2
#define KEY_MENU4SY         0xA3
#define KEY_MENU5SY         0xA4
#define KEY_MENU6SY         0xA5
#define KEY_MENU7SY         0xA6
#define KEY_MENU8SY         0xA7
#define KEY_MENU9SY         0xA8
#define KEY_MENU10SY        0xA9


#define KEY_FRAC            0xB0
#define KEY_ROOT            0xB1
#define KEY_EXPO            0xB2
#define KEY_ROOX            0xB3
#define KEY_BASE            0xB4
#define KEY_PI              0xB5
#define KEY_THETA           0xB6
#define KEY_DEG             0xB7
#define KEY_LE              0xB8
#define KEY_GE              0xB9

#define KEY_GRAB            0xFC    // when SCREEN_GRABBER has been defined
#define KEY_GRABON          0xFD    // when SCREEN_GRABBER has been defined
#define KEY_GRABOFF         0xFE    // when SCREEN_GRABBER has been defined

#define KEY_MENU            0xFF
/*
```

There are keyboard maps for normal, with-shift, and with-symbol keys. They are:
```C
*/
uint8_t _kbd_normal_keys[] = {
	'1',        '2',        '3',        '4',        '5',        '6',        '7',        '8',        '9',        '0',
	'q',        'w',        'e',        'r',        't',        'y',        'u',        'i',        'o',        'p',
	'a',        's',        'd',        'f',        'g',        'h',        'j',        'k',        'l',        KEY_DEL,
	KEY_SHIFT,  'z',        'x',        'c',        'v',        'b',        'n',        KEY_DOWN,   KEY_ENTER,  KEY_UP,
	KEY_SYM,    KEY_FRAC,   KEY_ROOT,   KEY_EXPO,   ' ',        ',',        '.',        'm',        KEY_LEFT,   KEY_RIGHT,
	KEY_MENU1,  KEY_MENU2,  KEY_MENU3,  KEY_MENU4,  KEY_MENU5,  KEY_MENU6,  KEY_MENU7,  KEY_MENU8,  KEY_MENU9,  KEY_MENU10
};

uint8_t _kbd_shift_keys[] = {
	'1',        '2',        '3',        '4',        '5',        '6',        '7',        '8',        '9',        '0',
	'Q',        'W',        'E',        'R',        'T',        'Y',        'U',        'I',        'O',        'P',
	'A',        'S',        'D',        'F',        'G',        'H',        'J',        'K',        'L',        KEY_ESC,
	KEY_SHIFT,  'Z',        'X',        'C',        'V',        'B',        'N',        KEY_PGDN,   KEY_ENTER,  KEY_PGUP,
	KEY_SYM,    KEY_GRABOFF,KEY_GRAB,   KEY_GRABON ,'_',        ',',        '.',        'M',        KEY_HOME,   KEY_END,
	KEY_MENU1SH,KEY_MENU2SH,KEY_MENU3SH,KEY_MENU4SH,KEY_MENU5SH,KEY_MENU6SH,KEY_MENU7SH,KEY_MENU8SH,KEY_MENU9SH,KEY_MENU10SH
};

uint8_t _kbd_symbol_keys[] = {
	'!',        KEY_PI,     KEY_THETA,  '$',        '%',        KEY_DEG,    '\'',       '\"',       '(',        ')',
	'Q',        'W',        'E',        'R',        'T',        'Y',        'U',        ';',        '[',        ']',
	'=',        '+',        '-',        'F',        'G',        'H',        'J',        ':',        '?',        KEY_ESC,
	KEY_SHIFT,  '*',        '/',        'C',        'V',        'B',        KEY_LE,     KEY_PGDN,   KEY_ENTER,  KEY_PGUP,
	KEY_SYM,    KEY_FRAC,   KEY_ROOX,   KEY_BASE,   KEY_MENU,   '<',        '>',        KEY_GE,     KEY_HOME,   KEY_END,
	KEY_MENU1SY,KEY_MENU2SY,KEY_MENU3SY,KEY_MENU4SY,KEY_MENU5SY,KEY_MENU6SY,KEY_MENU7SY,KEY_MENU8SY,KEY_MENU9SY,KEY_MENU10SY
};
/*
```

#### void kbdInit()

Initialize the keyboard for scanning.

Must be called to initialize the keyboard prior to using any other keyboard functions.

#### uint8_t kbdGetKeyDetails()

Return a 16 bit value with the low 8 bits representing the most current key press.
The high 8 bits are 4 bits for the column and 4 bits for the row.
This will ignore the current key if it has already been reported.
Returns KEY_NOP if no key _(or no new key)_ is pressed.

**Note:** The row and column values are 1's based to allow zero to represent no data.


#### uint8_t kbdGetKey()

Return the most current key press. This will ignore the current key if it has already been reported.
Returns KEY_NOP if no key _(or no new key)_ is pressed.


#### uint8_t kbdGetKeyWait()

Return the current key press.  This will ignore the current key if it has already been reported.
if no key _(or no new key)_ is pressed, it will wait.



## LCD
**Draw Primatives, Text, and Composite UI**

The SRXE LCD is 384\*136 pixels with four levels of greyscale.
The LCD Driver operates on three pixels at a time, stored in a single byte.
This code refers to a three-pixel byte as a **triplet**.

The physical LCD is 384\*136. However, it is only addressable by triplets.
All of the exposed functions in this library operate on a screen
coordinate system of 128\*136.

There is one issue with this hardware implementation - all vertical lines are _fat_.
The `lcdRectangle()` and `lcdVerticalLine()` code attempts to addressing this error.
_You can judge for yourself if they are successful._

Use `LCD_WIDTH` and `LCD_HEIGHT` for the extents of the screen.

There are definitions for the colors to improve code:
`LCD_BLACK`, `LCD_DARK`, `LCD_LIGHT`, and `LCD_WHITE`.

--------------------------------------------------------------------------

#### void lcdFill(uint8_t color)

Fill the entire screen with the specified color.


#### void lcdClearScreen()

Fill the entire screen with the current background color.

#### void lcdColorSet(uint8_t fore_color, uint8_t back_color)

Set the foreground and background colors used for subsequent operations.


#### uint8_t lcdColorTripletGetF()

Get the current foreground color as a three-byte triplet.

#### uint8_t lcdColorTripletGetB()

Get the current background color as a three-byte triplet.

#### void lcdPositionSet(uint8_t x, uint8_t y)

Set the current position on the screen used for subsequent operations.

_Note: Text draw operations will update the horizontal position._

**Notes:**
Horizontal dimensions are in display triplets, not real pixels.
Vertical dimensions are always in real pixels.

#### uint8_t lcdPositionGetX()

Get the current horizontal position on the screen.

**Note:** horizontal dimensions are in display triplets, not real pixels._

#### uint8_t lcdPositionGetY()

Get the current vertical position on the screen.

**Note:** vertical dimensions are always in real pixels._

#### uint8_t lcdContrastGet()

Get the current LCD contrast level (a value between 1 and 10)


#### void lcdContrastSetRaw()

Set the LCD contrast to a specified level (0 .. 256)

This is a special case function and probably is not the one you want.

#### void lcdContrastSet()

Set the LCD contrast to a specified level ( 1 .. 10)

There are definitions for the contrast to improve code:
`LCD_CONTRAST_MIN`, `LCD_CONTRAST_DEFAULT`, and `LCD_CONTRAST_MAX`.

#### void lcdContrastReset()

Reset the LCD to the `LCD_CONTRAST_DEFAULT` level.

#### void lcdContrastDecrease()

Decrease the LCD contrast one level.

#### void lcdContrastIncrease()

Increase the LCD contrast one level.

#### void lcdWake()

Perform wake-up operations after LCD has been put to sleep.
This is used as part of a re-initialization sequence after the SRXE has returned from its power-off sleep state.

#### void lcdSleep()

Perform shutdown operations of the LCD.
This is used ahead of the SRXE entering power-off sleep state.

#### bool lcdInit()

Must be called to initialize the LCD prior to using any other LCD functions.


### LCD Draw
**Draw Graphic Primatives**

--------------------------------------------------------------------------

#### void lcdHorizontalLine(int x, int y, int length, int thickness)

Draw a horizontal line with a given thickness.

**Note:** Horizontal dimensions are in display triplets, not real pixels.

#### void lcdVerticalLine(int x, int y, int length, int thickness)

Draw a vertical line. Thickness from 1 to 3 is supported.

**Note:** Vertical dimensions are always in real pixels.

#### void lcdRectangle(int x, int y, int width, int height, bool filled)

Draw a rectangle - hollow, filled, or erase area.

**Notes:**
A _filled_ rectangle is filled with the background color.
Horizontal dimensions are in display triplets, not real pixels.
Vertical dimensions are always in real pixels.

#### void lcdBitmap(int x, int y, const uint8_t *bitmap, bool invert)

Draw a bitmap. The color map may be inverted.

The bitmap must be run-length-encoded using the `bitmap_gen.py` tool.
The tool provides instructions on how to create a source graphic which will converted correctly.

**Notes:**
Horizontal dimensions are in display triplets, not real pixels.
Vertical dimensions are always in real pixels.

#### void lcdScrollSet(...)

Initialize an area for vertical scrolling. The scrolling operation is performed with `lcdScrollLines()`.

The parameters are:
 - int TA - number of pixel lines for the top fixed area
 - int SA - number of pixel lines for the scroll area
 - int BA - number of pixel lines for the bottom fixed area

The sum of TA + SA + BA must equal 160. 160 is the LCD driver height (not the LCD height).

**Warning:** This code has not been tested.

#### void lcdScrollLines(int count)

Scroll the _scroll area_ a given number of pixel lines. The _scroll area_, `SA`,  must already be set using `lcdScrollSet()`.

**Warning:** This code has not been tested.

#### void lcdScrollReset(int count)

Reset the scroll area to the whole screen.

**Warning:** This code has not been tested.


### LCD Text
**Draw Text**

The text functions of the SRXEcore use bitmap fonts which have been generated with the `font_gen.py` tool.
The tool is documented at the end of this README.
There are four available slots for fonts.
While an application may use more than four fonts, only four may be active at a time.
The font slots are identified as `FONT1`, `FONT2`, `FONT3`, and `FONT4`.

To specify your own fonts, add `#define CUSTOM_FONTS` before including the library code. If you define your own fonts,
you will need to sue the `lcdFontConfig()` function and optionally use `lcdFontClone()`.
If `CUSTOM_FONTS` is not defined, the SRXEcore will load default fonts.

--------------------------------------------------------------------------

#### void lcdFontConfig(...)

Fonts are managed by an internal `FONTOBJECT`. Up to four fonts (FONTS_MAX) may be active at any time.
This function initializes one of the four slots.

The input parameters are:
- uint8_t font_ID - one of `FONT1`, `FONT2`, `FONT3`, or `FONT4`
- const unsigned char* data - the PROGMEM data from a font `.h` file
- uint8_t width - defined in the font `.h` file
- uint8_t height - defined in the font `.h` file
- uint8_t width_bytes - defined in the font `.h` file
- uint8_t char_bytes - defined in the font `.h` file
- uint8_t scale - `FONT_DEFAULT_SCALE`, `FONT_DOUBLE_WIDTH`, `FONT_DOUBLE_HEIGHT`, or `FONT_DOUBLED`

**Notes:
Font dimension parameters are in real pixels, not display triplets.
Vertical dimensions are always in real pixels.

#### void lcdFontClone(...)

One font may use an existing font to save data.
This is referred to as font cloning.
The only difference between the two fonts will be any scaling.

This function initializes one of the four slots using the data of another slot.

In theory, three font slots could all be clones of a singel slot.
In practice, often the `FONT1` and `FONT2` slots are real data
with `FONT3` being a clone of `FONT1` and `FONT4` being a clone of `FONT2`.

The input parameters are:
- uint8_t target_ID - one of `FONT1`, `FONT2`, `FONT3`, or `FONT4` which will share configuration from another font
- uint8_t source_ID - one of `FONT1`, `FONT2`, `FONT3`, or `FONT4` which provides the source configuration
- uint8_t scale - `FONT_DEFAULT_SCALE`, `FONT_DOUBLE_WIDTH`, `FONT_DOUBLE_HEIGHT`, or `FONT_DOUBLED` forthe new font

#### void lcdFontSet(uint8_t font_id)

Set the active font for subsequent text operations.

There are definitions for the font IDs to improve code:
`FONT1`, `FONT2`, `FONT3`, and `FONT4`.

#### uint8_t lcdFontGet()

Get the active font ID being used for text operations.

The return value used definitions for the font IDs to improve code:
`FONT1`, `FONT2`, `FONT3`, and `FONT4`.

#### uint8_t lcdFontWidthGet()

Get the width of a character from the active font.

**Note:** width is in display triplets, not real pixels._

#### uint8_t lcdFontHeightGet()

Get the height of a character from the active font.

**Note:** vertical dimensions are always in real pixels._

#### uint8_t lcdTextWidthGet(const char* text)

Get the width of the text using characters from the active font.

**Note:** width is in display triplets, not real pixels._

#### int lcdPutChar(char c)

Display a character at the current LCD position, using the current font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the character displayed.

Use `lcdPositionSet()`, `lcdFontSet()`, and `lcdColorSet()` as necessary, prior to using the function.

The current position is updated by this function.

#### int lcdPutString(char* string)

Display a string at the current LCD position, using the current font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the string displayed.

Use `lcdPositionSet()`, `lcdFontSet()`, and `lcdColorSet()` as necessary, prior to using the function.

The current position is updated by this function.

#### int lcdPutStringAt(char* string, int x, int y)

Display a string at the specified position, using the current font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the string displayed.

Use `lcdFontSet()`, and `lcdColorSet()` as necessary, prior to using the function.

The current position is updated by this function.

__This is a convenience function which combines `lcdPositionSet()` and `lcdPutString()`.__

#### int lcdPutStringAtWith(char* string, int x, int y, uint8_t font_id, uint8_t fg, uint8_t bg)

Display a string at the specified position, using the specified font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the string displayed.

The current position is updated by this function.

__This is a convenience function which combines `lcdPositionSet()`, `lcdFontSet()`, `lcdColorSet()`, and `lcdPutString()`.__


### LCD UI
**Draw Composite UI Elements**

The SRXE LCD functions are broken into layers. The UI layer is to top most set of
functions and is intended to make building user interfaces easier and consistent.

**FYI:** The portion of SRXECore is likely to evolve as the first developers start
using the library and new UI functions are identified.

--------------------------------------------------------------------------

#### void uiMenu()

Render soft menu labels adjacent to the 5 buttons on either side of the screen.

The input parameters are:
- const char* menus[] - an array of 10 pointers to strings to be used as menu labels
- const char* title - optional title to display top center; use NULL to omit the title
- *uint8_t menu_shape - one of `UI_MENU_CLEAR`, `UI_MENU_RECTANGLE`, `UI_MENU_ROUND_END`, or `UI_MENU_ROUNDED`
- bool clear - flag to clear an area slightly larger than the space used for the soft menus

Returns the width used for rendering a menu item

**Notes:**

The array must contain 10 elements.
An element may be a NULL pointer to indicate no menu for that position.
The menu positions are number from top to bottom on the left 0..4 and on the right from 5..9

The function used DEFAULT_MENU_FONT._

#### void uiTextBox()

Render multi-line text.

The input parameters are:
- char* buffer[] - buffer to receive user input
- char* label[] - label to proceed input field (NULL to ignore)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint8_t w - width of input area
- uint8_t h - height of input area
- uint8_t render_flags - flags to control how user input is displayed

Render flags (_not all implemented_):
```C
*/
#define UI_TEXTBOX_WRAP           0x01    // wrap input onto multiple lines (currently assumed)
#define UI_TEXTBOX_INLINE_LABEL   0x02    // label will render at start of line (currently assumed)
/*
```

**Note:** the wrap feature does not use any word break algorithm.

_The function uses the current font and color._

#### void uiLinesBox()

Render multi-line text.

The input parameters are:
- char* lines[] - array of text lines (last array item must be NULL)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint8_t w - width of input area
- uint8_t h - height of input area

**Notes:** There are some special markup supported.
- FONT3 and Black on White text is used
- lines begining with the vertical bar (|) will be centered
- lines begining with the pound symbol (#) will use FONT2
- lines begining with the underscore symbol (_) will be at the bottom of the screen
- special markup may be used together


#### void uiInputBox()

Render multi-line input field.

The input parameters are:
- char* buffer[] - buffer to receive user input
- uint8_t size - length of buffer
- char* label[] - label to proceed input field (NULL to ignore)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint8_t w - width of input area
- uint8_t h - height of input area
- uint8_t render_flags - flags to control how user input is displayed - see `uiTextBox()`
- char_callback todo - an call back to perform actions on the current keyboard character (NULL to ignore)

Returns the number of characters input.

**Notes:**

Input ends when size-1 is reached or user hits ENTER or ESC.
Buffer is erased at start of input.
Buffer is erased if user hits ESC.
If no flags restrict input, then all input is allows. Flags may be combined.

_The function uses the current font and color._

#### void uiInputField()

Render multi-line input field.

The input parameters are:
- char* buffer[] - buffer to receive user input
- uint8_t size - length of buffer
- char* label[] - label to proceed input field (NULL to ignore)
- char* initial[] - initial value (NULL to ignore)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint16_t flags - TBD
- char_callback todo - an call back to perform actions on the current keyboard character (NULL to ignore)

Render flags:
```C
*/
#define UI_INPUTFIELD_FIXED       0x01    // field is fixed length and will automatically return when full
/*
```
Returns the number of characters input.

**Notes:**

Input ends when size-1 is reached, input matches `len` or user hits ESC.
Buffer is erased at start of input and optionally initialized with `default`.

_The function uses the current font and color._


## PrintF
**Formatted Print to the Available Output Devices**

These functions use tiny `printf` by (c) Marco Paland (info@paland.com) / PALANDesign Hannover, Germany.

These print functions are interfaced to work with the **RF**, **UART**, and **LCD** text functions.

**Note:** This code must be included after any of the output destinations - `lcdtext.h`, `rf.h`, and/or `uart.h`.

--------------------------------------------------------------------------

There are a set of predefine options for the output device for printf() functions.

The options are:
```C

typedef enum {
    PRINT_NONE = 0, // no output
    PRINT_LCD,      // output to the LCD at the current set position with the current font and color
    PRINT_RF,       // output to the RF transceiver; output is buffered through the transceiver FRAME buffer
    PRINT_UART      // output to the UART; intended for use with the Enigma Development Adapter
} DESTINATIONS;

```

#### void printSetDevice(uint8_t device)

Set the output device.

**Warning:** no attempt is made to validate the requested device is available.
If a device is not available, no output will occur.

#### int printDevicePrintf(uint8_t device, const char* fmt, ...)

replacement for stdlib printf() function with the output going to the specified device.

When used for output to LCD, this function will used the current LCD position, font, and colors. Do not use any newline or linefeed characters with the LCD.

When used for output to the UART, linefeed and newline are not automatically added and must be part of the `fmt` string as appropriate.

When used for output to the RF transceiver, this function will perform `rfTransmitNow()` prior to returning.


#### int printBufferPrintf(char* buffer, size_t size, const char* fmt, ...)

replacement for stdlib snprintf() function with the output going to the provided buffer.



## LEDs
**LEDs connected to the JTAG pads - handy for debugging**

The SRXE has a 10 pin pad accessible through the battery compartment.

The **Enigma Development Adapter** board has LEDs connected to these pins. It also provides a 0.1" header to the pins so they
may be used as general I/O pins.

The pads are labeled with the JTAG identifiers `TDK`, `TMS`, `TDO`, and `TDI` which correspond to `PORTF` `PIN4` thru `PIN7`.

**Note:** To use the 4 pins - and by extension the LEDs or UART - the JTAG interface must be disabled via the associated fuse bit.

Here is a handy [AVR FUSE Calculator](https://www.engbedded.com/fusecalc/).

The SRXE fuses are `LOW:0:EE HIGH:1:92 EXT:2:FC PROT:3:FF`

The SRXE fuses, with JTAG disabled, are: `LOW:0:EE HIGH:1:D2 EXT:2:FC PROT:3:FF`

**Warning:** the `TDI` (`PORTF` `PIN7`) is used by the bitbag UART functions. Do not attempt to use this LED if you will also use UART.

--------------------------------------------------------------------------

#### void ledsInit()

Initialize the LEDs.

Must call this function before using any other LED functions.

#### void ledOn(uint8_t num)

Turn the numbered LED on.

#### void ledOff(uint8_t num)

Turn the numbered LED off.

#### void ledsOn()

Turn all LEDs on.

#### void ledOff()

Turn all LEDs off.

#### void ledsTest()

This is a convenience function to sequence all of the LEDs on and then off.


# SMART Response XE Bitmaps

Bitmaps are memory intensive but when used judiciously, they can be smaller than attempted to create the same results as draw functions.
Bitmaps are also great for icons and other small graphics.

This `bitmap_gen.py` program generates run-length-encoded (RLE) data as a header file.
While RLE is pretty common, this program required the source bitmap to be correctly formatted.

### Source Bitmaps

The general workflow for generating a new bitmap file is as follows _(these instructions use GIMP)_:
 - The maximum graphic is 384x136. _Smaller graphics are fine._
 - Create your graphic. It's OK to use layers. Graphics layers can not use transparency.
 - Keep text in black & white.
 - Convert any image layers layers to grayscale using **Color -> Desaturate -> Desaturate**.
 - Increase contrast of any image image layers using **Colors -> Levels** and shifting the center node to the right and the right node to the left. _This will be somewhat of a trial and error process_.
 - Set image color mode to indexed using **Image -> Mode -> Indexed** with **Maximum Colors 4** and dithering set to **Floyd-Steinberg Reduced Color Bleeding**.
 - Export file with `.raw` extension and use **Standard** and **Normal** options.
 - Delete and `.pal` files.

**Note:** The SRXE requires draw operations on a 3-pixel boundary. If the RAW bitmap width is not a multiple of 3, it will be padded with background.

### Usage

`python3 bitmap_gen.py` _source-file_ _output-file_ _width_ _height_

|PARAMETER|DESCRIPTION|
|:-----|:-----|
|source file|4 indexed monochrome bitmap file in RAW format|
|output file| header file output|
|width|pixel width of source bitmap|
|height|pixel height of source bitmap|

### Example

`python3 bitmap_gen.py` menu_ball18.raw ball18.h 18 18`

--------------------------------------------------------------------------


# SMART Response XE Font Generation

Getting fonts to look good on the SRXE LCD is 25% trial and error, 33% subjective, 50% rendering engine, and 10% your code.

This `font_gen.py` program generates header files that are specific to the SRXEcore text renderer. It was not written to
match any previous format. The program will output a series of header files with bitmap data for a font at a designated W*H.
The output data is _width first_ to match the way the rendering engine on the SRXEcore works.
Also, _width first_ is how the LCD bitmap on a SRXE is formatted.

The general workflow for generating new font files is as follows:
 - find a font that will look good at the resolution you need. The best source is [Old School Fonts](https://int10h.org/oldschool-pc-fonts/)
 - adjust the parameters (ID number, point size, y offset, width, and height) until the resulting `PNG` looks good
 - include the generated header file(s) in your code

The SRXEcore will compile out any rendering engine code not used so only include the generated headers you will actually use.

### FONTS

Only use monospace fonts. This code does not attempt to clean up and space out proportional fonts ... at least not very well.
Ideally, the font width will be a multiple of 3. This will look best on the SRXE LCD display. The rendering engine will add whitespace
between characters to pad out to the 3-pixel boundary required by the LCD driver.

Fonts are somewhat subjective - especially at small sizes. Some people may have difficulty reading some fonts.
Here are the best fonts I was able to find:

|LABEL|WxH|DESCRIPTION|
|:-----|:-----:|:-----|
|TINY|6x8|The HP100LX is possible the best tiny font available|
|SMALL|8x12|the Toshiba 8x14 can be tweeked quite a bit to make an excellent small font|
|SMALL|9x15|the IBM XGA AI 7x15 is a clean contemporary looking font|
|MEDIUM|12x18|the IBM XGA AI 12x23 makes a clean contemporary looking medium font|
|LARGE|15x28|the IBM XGA AI 7x15 (doubled) actually looks a bit better that the 12x23|

**Memory Usage:** If memory usage is a concern, the the `TINY` may be doubled to make a `MEDIUM` and the `SMALL` may be doubled to make a `LARGE`.
The Toshiba font is better for doubling than the IBM font.

_See the `lcdFontConfig()` and `lcdFontClone()` functions for details on doubling.

### Notes

Most fonts drop the underscore character to the very bottom of the font cell and below any other glyph pixels.
This results in the need to make a font taller than it would otherwise need to be. For this reason, this program
forces the underscore to the bottom most available row of the font. This hack means its OK to clip the bottom of the
font, if the only thing that will be lost is the underscore.

The `font_gen.py` program is based on the work by Jared Sanson (jared@jared.geek.nz).
This program requires `PIL` (Python Imaging Library) to generate a `PNG` of the characters.
The `PNG` is chunked to make the bitmaps for each character.
Only the characters from SPACE (32) through Tilda (127) are rasterized.

--------------------------------------------------------------------------


# SMART Response XE Screen Grabber

Monitor the UART output for screen data and write it to PGM image files.

This `screen_grabber.py` works in conjunction with the SRXEcore when it has been compiled with `SCREEN_GRABBER` flag defined.
The SRXEcore - _when compiled with `SCREEN_GRABBER`_ - will stream buffer events over UART as well as stream the data written to the LCD.

By default the screen data streaming is automatic when `SCREEN_GRABBER` has been defined.

Define `SCREEN_GRABBER_MANUAL` to compile in functionality but have it remain idle.
Use `LCD_SCREEN_GRABBER_ACTIVATE()` and `LCD_SCREEN_GRABBER_DEACTIVATE()` to manually enable/disable the functionality.
This is most useful if tied to menu or hot keys on the keyboard.
It may also be helpful to tie `LCD_SCREEN_GRABBER_GRAB()` to a menu or hot key to trigger `screen_grabber.py`.

**CAUTION:** The data streaming **really really really** slows down the screen operations _(did we say really? we mean really)_
so this capability should only be used to generate screen shots and then then the SRXEcore should be recompiled without the `SCREEN_GRABBER` flag
or use the `SCREEN_GRABBER_MANUAL` option.

The UI for this program is rough but _it-is-what-it-is_. There are only two commands: **G** and **Q** - both of which must be followed by `[ENTER]`
to be processed.

The system will capture the current screen prior to `lcdClearScreen()`. Alternatively, the current screen may be saved using the **G** command.

**NOTE:** In addition to the UI being lazy, the Serial port selection is coded. You will need to edit `screen_grabber.py` if the default device does
not match your environment.

--------------------------------------------------------------------------


# SMART Response XE  Device Smoketest and Demo

This demo is a non-exhaustive demonstration of the SRXEcore library.
It includes the LEDs and UART capabilities exposed by the optional **Enigma Development Adapter**.
The adapter is not required to run this program.

In addition to demonstration of the SRXEcore library, this example is also a suitable test of the
SRXE hardware.

_The SMART Response XE (SRXE) is an existing hardware device from around 2011.
There are a lot of these on the used market.
Single devices and batches of devices are often listed on eBay.
It's not uncommon when buying a batch of devices to learn that the devices were
stored for a long period with cheap old batteries.
The biggest problem with these devices is corrosion.
When acquiring SRXE hardware, its very common to find old batteries and corrosion.
Used SRXE devices need to be thuroughly cleaned and tested.
This demo provides a convenient test of all functions of the SRXE.__

**Note:** The documentation does not attempt to explain the demo _line-by-line_.
It provides a high level explanation of the demo. For a full understanding of the demo,
and to see examples of SRXEcore library functions being used, please read through the actual code.

--------------------------------------------------------------------------

## Demo Code


#### Arduino-like code

The SRXEcore and the sample smoketest are developed and maintained using VSCode+PlatformIO.
For users of the Arduino IDE, the following code is broken out to mimic the structure of
Arduino projects.

`smoketest_setup()` contains the contents found in Arduino `setup()` and
`smoketest_loop()` contains the contents found in Arduino `loop()`.
There are also a number of global variables to persist data between `setup` and `loop`
as well between calls to `loop`.


#### Initialization of the Functional Areas

Before using any of the SRXEcore functions, the _init_ procedure for each subsystem must be called.
```C
*/
	ledsInit(); // uses the JTAG pads directly or via the Enigma Development Adapter
	uartInit(); // uses the JTAG pads directly or via the Enigma Development Adapter

	clockInit();
	powerInit();
	rfInit(1);
	randomInit(); // (must be after RF)
	kbdInit();
	lcdInit();
/*
```

#### Display the Screen Contents

Once the various systems have been initialized, we may start to display content on the LCD.

In this demo, we render a keyboard map.
This is later used to visually verify each key is functioning.

We also render some status information including
the build date of the firmware,
battery status,
RF transceiver status,
current TX and RX messages,
and a 1-second counter.

To demonstrate the graphics, we render text in severals sizes and a box for each color value.

We also use the composite UI function to render a menu with `UI_MENU_ROUND_END` styling.

The status bar combines a filled rectangle and bitmap circles for a rounded-rectangle result.


For every iteration of the loop, we check to see if there has been a long-press of the power button.
In the demo, this event is used to indicate the device should be put to sleep. It will wake when the button is pressed again.

The code performs a series of pre-sleep tasks to conserve power,
including shutting down the RF transceiver,
instructing the LCD to power down, and turning off any LEDs.
Then it initiates the system level sleep.
When it wakes, it will reverse the steps as part of its post-sleep tasks.


There are two counters with use the library clock to perform actions at given intervals.

The first counter performs updates to the status information once each 1 second (1000 milliseconds).

This set of actions include updating the battery, RF transceiver state, and counter, in the status bar.
It also will transmit the current counter depending on the state of the RF transceiver.

During this interval, the demo will also optionally transmit its counter value with the RF transceiver.

The demo lets the user set the RF Transceiver to one of four modes:
**OFF**, **RX only**, **RX with TX**, and **Echo**
(where the code will retransmit whatever it has received).

_When a new device powers up, the initial state is **RX with TX**.
With another devcie in **Echo**, the display of the RF messages on the new device will have
the RX message the same as the TX message. This is a quick and easy way to test the RF hardware on a device._


The second counter performs keyboard scanning and keyboard related actions once each 10 milliseconds.

If a key has been pressed, the first task is to update the keyboard map we displayed at the start.
For each key press, we blank out the corresponding location on the map.
Pressing every key on the keyboard will blank every location on the map.
If an location is not cleared, then it indicates that key has malfunctioned and
there may be internal damage to the SRXE circuit board.

The **UP** and **DOWN** of the four-way button on the keyboard is used to control the LCD contrast.

The **LEFT** and **RIGHT** of the four-way button on the keyboard is used to change the RF transceiver mode.



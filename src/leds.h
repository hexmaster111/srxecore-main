
/* ************************************************************************************
* File:    leds.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

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
--- */

#ifndef __SRXE_LEDS_
#define __SRXE_LEDS_

#ifdef SRXECORE_DEBUG

#include "common.h"


// --------------------------------------------------------------------------
// some 'convenience' LED code for the adapter board (only for debugging)
// --------------------------------------------------------------------------

static uint8_t _srxe_leds[] = {(SRXE_PORTF | PIN4), (SRXE_PORTF | PIN5), (SRXE_PORTF | PIN6), (SRXE_PORTF | PIN7)};
static uint8_t _srxe_leds_inited;

#define LEDS_COUNT 4


/* ---
#### void ledsInit()

Initialize the LEDs.

Must call this function before using any other LED functions.
--- */

void ledsInit() {
	// not sure we need to do anything
	_srxe_leds_inited = true;
}


/* ---
#### void ledOn(uint8_t num)

Turn the numbered LED on.
--- */
void ledOn(uint8_t num) {
	if (!_srxe_leds_inited) return;

	num = num % LEDS_COUNT;
	srxePinMode(_srxe_leds[num], OUTPUT);
	srxeDigitalWrite(_srxe_leds[num], HIGH);
}


/* ---
#### void ledOff(uint8_t num)

Turn the numbered LED off.
--- */
void ledOff(uint8_t num) {
	if (!_srxe_leds_inited) return;

	num = num % LEDS_COUNT;
	srxePinMode(_srxe_leds[num], OUTPUT);
	srxeDigitalWrite(_srxe_leds[num], LOW);
}


/* ---
#### void ledsOn()

Turn all LEDs on.
--- */
void ledsOn() {
	if (!_srxe_leds_inited) return;

	for (int i = 0; i < LEDS_COUNT; i++)
		ledOn(i);
}


/* ---
#### void ledOff()

Turn all LEDs off.
--- */
void ledsOff() {
	if (!_srxe_leds_inited) return;

	for (int i = 0; i < LEDS_COUNT; i++)
		ledOff(i);
}


/* ---
#### void ledsTest()

This is a convenience function to sequence all of the LEDs on and then off.
--- */
void ledsTest() {
	if (!_srxe_leds_inited) return;

	ledsOff();
	for (int i = 0; i < LEDS_COUNT; i++) {
		ledOn(i);
		_delay_ms(500);
	}
	for (int i = 0; i < LEDS_COUNT; i++) {
		ledOff((LEDS_COUNT - 1) - i);
		_delay_ms(100);
	}
}


#else //SRXECORE_DEBUG

#define ledsInit()
#define ledOn(num)
#define ledOff(num)
#define ledsOn()
#define ledsOff()
#define ledsTest()

#endif // SRXECORE_DEBUG



#endif // __SRXE_LEDS_

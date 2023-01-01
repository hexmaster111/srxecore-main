/* ************************************************************************************
* File:	random.h
* Date:	2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## Random
**Pseudo Random Number Generator**

**NOTE:** This code uses the random number generator build into the ATMega128RFA1.

--------------------------------------------------------------------------
--- */

#ifndef __RANDOM_H_
#define __RANDOM_H_

uint16_t _seed;


/* ---
#### void randomInit()

Initialization of the pseudo random number generator.

This function must be called prior to using any other random functions.

**Note:** This function uses the ATMega128RFA1 RF subsystem to create the random seed value.
If the RF transceiver is not available, the seed will not be random.

--- */

void randomInit() {
	uint8_t rf = rfInited();
	if (!rf) rfInit(1);	// if the RF is not active, turn it on

	// use RF data
	_seed = 0;
	for (uint8_t i = 0; i < 8; i++) {
		_seed = (_seed << 2) | ((PHY_RSSI >> 5) & 0x3);
		_delay_ms(1);
	}
	srand(_seed);

	if (!rf) rfTerm();	// if the RF was not active, turn it back off
}

/* ---
#### uint16_t randomGetSeed()

return seed value being used by the pseudo random number generator.

--- */
uint16_t randomGetSeed() {
	return _seed;
}


/* ---
#### uint16_t randomNumGet(uint16_t max)

return a pseudo random number between 0 and `max` (not including `max`).

--- */

uint16_t randomNumGet(uint16_t max) {
	return rand() % max;
}

/* ---
#### uint8_t randomByteGet()

return a pseudo random byte value between 0x00 and 0xFF.

--- */
uint8_t randomBytGet() {
	return rand() % 0xFF;
}

/* ---
#### uint16_t randomWordGet()

return a pseudo random word value between 0x0000 and 0xFFFF.

--- */
uint16_t randomWordGet() {
	return rand();
}

/* ---
#### char randomCharGet()

return a pseudo random upper case letter (A through Z inclusive).

--- */
char randomCharGet() {
	return 'A' + (rand() % 26);
}

#endif // __RANDOM_H_


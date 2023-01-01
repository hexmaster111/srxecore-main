/* ************************************************************************************
* File:    eeprom.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## EEPROM
**read/write interface to EEPROM memory**

The module provides basic read/write of the microcontroller eeprom.

**Tip:** Writing a specific bytecode to a specific location of the EEPROM is an easy way to know if
it has been used before or if initial data should be written. If the bytecode is missing or incorrect,
then it can be considered "unformatted" or "corrupted" and any initial data should be written.

--------------------------------------------------------------------------
--- */

#ifndef __EEPROM_
#define __EEPROM_

#include <avr/boot.h>
#include <avr/eeprom.h>

// all of the following are unsigned byte data entries in the eeprom space
#define EEPROM_FIRST_AVAILABLE		0x0000	// could be 0x01 but we are being generous
#define EEPROM_LAST_AVAILABLE		0x0BFF
#define EEPROM_ID_COUNT				0x0C00	// number of IDs we have stored
#define EEPROM_ID_STORAGE			0x0C01	// first slot used for storing received IDs
#define EEPROM_MAX_ADDRESS			0x1000	// 4KB

#define EEPROM_ID_SIZE				6		// a reasonable unique identifier

// by default, this code uses 6 bytes of the chip signature as an ID

char _eeprom_sig[EEPROM_ID_SIZE+1];	// sotrage buffer for the computed signature string

// forward declarations to order the functions more logically
uint8_t eepromReadByte(uint16_t addr);
void eepromWriteByte(uint16_t addr, uint8_t data);

/* ---
#### void eepromInit()

Initialization of the EEPROM functions.

This function must be called prior to using any other EEPROM functions.
--- */
void eepromInit() {

	// load up the ascii representation of the chip ID for general availability
	// the choice of order insures maximum uniqueness when using the first 4 letters

	uint8_t n;
	n = boot_signature_byte_get(0x0E + 2);
	_eeprom_sig[0] = 'A' + (n % 26);
	n = boot_signature_byte_get(0x0E + 5);
	_eeprom_sig[1] = 'A' + (n % 26);
	n = boot_signature_byte_get(0x0E + 7);
	_eeprom_sig[2] = 'A' + (n % 26);
	n = boot_signature_byte_get(0x0E + 8);
	_eeprom_sig[3] = 'A' + (n % 26);
	n = boot_signature_byte_get(0x0E + 4);
	_eeprom_sig[4] = 'A' + (n % 26);
	n = boot_signature_byte_get(0x0E + 3);
	_eeprom_sig[5] = 'A' + (n % 26);
	_eeprom_sig[6] = 0;
}

/* ---
#### uint8_t eepromIsReady()

Returns `true` if the eeprom is ready and `false` if it busy.
--- */
uint8_t eepromIsReady() {
	if (EECR & (1 << EEPE))
		return false;
	return true;
}


/* ---
#### void eepromWriteByte()

Write a single byte to an eeprom memory location.
The write is ignored if the address is outside the EEPROM storage as defined by `EEPROM_MAX_ADDRESS`.
--- */
void eepromWriteByte(uint16_t addr, uint8_t data) {
	if (addr >= EEPROM_MAX_ADDRESS)
		return;

	cli();

	while (EECR & (1 << EEPE)) /*wait until previous write any*/
		;

	EEAR = addr;

#if 1	// atomic operation
	EEDR = data;
	EECR = (1 << EEMPE);
	EECR |= (1 << EEPE);			// atomic operation
#else	// erase/write operation
	EEDR = 255;
	EECR = (1<<EEMPE) + (1<<EEPM0); // erase operation
	EECR |= (1<<EEPE);
	while(EECR & (1<<EEPE))
		;
	EEDR = data;
	EECR = (1<<EEMPE) + (2<<EEPM0);	// write operation
	EECR |= (1<<EEPE);
#endif
	sei();
}

/* ---
void eepromReadByte() - read a single byte from eeprom memory relative to the eeprom base address
The read is ignored if the address is outside the EEPROM storage as defined by `EEPROM_MAX_ADDRESS` and will return 0.
--- */
uint8_t eepromReadByte(uint16_t addr) {
	if (addr >= EEPROM_MAX_ADDRESS)
		return 0;

	uint8_t b = 0;
	cli();
	while (EECR & (1 << EEPE)) /*wait until previous write any*/
		;
	EEAR = addr;
	EECR |= (1 << EERE);
	b = EEDR;
	sei();
	return b;
}

/* ---
#### char* eepromSignature()

Returns a pointer to a 6 character static buffer with the MCU signature ID converted to an ASCII string.

--- */

char *eepromSignature() {
	return _eeprom_sig;
}

/* ---
#### int eepromAddID(char \*new_code)

Store a 6 character code, checking if the `new_code` has previously been stored.

_Useful for tracking interations with other similar devices such as RF traffic._

--- */

int eepromAddID (char *new_code) {
	if (new_code == NULL)
		return -1;

	// we need to check if we have already received this ID
	uint8_t count = eepromReadByte(EEPROM_ID_COUNT);
	uint16_t slot_start;
	uint8_t matched = false;

	matched = true;

	// test for our own ID
	char* sig = eepromSignature();
	for (uint8_t i = 0; i < EEPROM_ID_SIZE; i++)
		if (new_code[i] != sig[i])
			matched = false;						// the current test is not a match

	if (matched)									// we found ourself ?!
		return -1;

	// test all of the IDs we have already stored
	for (uint8_t slot_num = 0; slot_num < count; slot_num++) {
		slot_start = EEPROM_ID_STORAGE + (slot_num * EEPROM_ID_SIZE);

		// while unlikely, we want to avoid running off the end of the eeprom space
		if ((slot_start + EEPROM_ID_SIZE) >= EEPROM_MAX_ADDRESS)
			return -1;

		matched = true; // assume we will find a match
		for (uint8_t i = 0; i < EEPROM_ID_SIZE; i++)
			if (new_code[i] != eepromReadByte(slot_start+i))
				matched = false;						// the current slot is not a match
		if (matched)									// no need to continue if we found a match
			break;
		matched = false;								// we assume not a match in case the for loop is done
	}

	if (matched)
		return -1;

	// we never found a match so store the new ID
	slot_start = EEPROM_ID_STORAGE + (count * EEPROM_ID_SIZE);
	for (uint8_t i = 0; i < EEPROM_ID_SIZE; i++)
		eepromWriteByte(slot_start+i, new_code[i]);
	count++;
	eepromWriteByte(EEPROM_ID_COUNT, count);

	return count;
}

#endif // __EEPROM_


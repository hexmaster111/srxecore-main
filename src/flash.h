/* ************************************************************************************
* File:    flash.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

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
--- */



#ifndef __SRXE_FLASH_
#define __SRXE_FLASH_

#include "common.h"


/*
	FYI:
	The CS line must be set low before each command and then set high after.
	If you try to send 2 commands with the CS line left low, they won't execute.
*/


/* ---
#### void flashInit()

Initialization of the FLASH chip functions.

This function must be called prior to using any other FLASH functions.
--- */
void flashInit() {
	_srxe_spi_init();
	srxePinMode(FLASH_CS, OUTPUT); // in case we want to use the SPI flash
	srxeDigitalWrite(FLASH_CS, HIGH); // in case we want to use the SPI flash
}


/* ---
#### int flashEraseSector(uint32_t addr, bool wait)

Erase the specified 4KB sector, with an option to wait for it to complete.

Returns `false` if the operation failed or timed out.

**Notes:**
 - It will wait no more than 100ms.
 - If you intend to perform a write operations immediately after the erase, then use `wait`.
--- */
bool flashEraseSector(uint32_t addr, int wait) {
	if (addr & 4095L) // invalid address
		return false;

	uint8_t rc;
	int timeout;

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x05); // read status register
	rc = _srxe_spi_transfer(0);
	srxeDigitalWrite(FLASH_CS, HIGH);

	if (rc & 1) // indicates the chip is busy in a write operation
		return false; // fail

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x06); // WREN - Write enable
	srxeDigitalWrite(FLASH_CS, HIGH);

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x20); // Sector Erase
	// send 3-uint8_t address (big-endian order)
	_srxe_spi_transfer((uint8_t)(addr >> 16));	// AD1
	_srxe_spi_transfer((uint8_t)(addr >> 8));	// AD2
	_srxe_spi_transfer((uint8_t)addr);		 	// AD3
	srxeDigitalWrite(FLASH_CS, HIGH);

	// wait for the erase to complete
	if (wait) {
		rc = 1;
		timeout = 0;
		srxeDigitalWrite(FLASH_CS, LOW);
		while (rc & 1) {
			_srxe_spi_transfer(0x05); // read status register
			rc = _srxe_spi_transfer(0);
			_delay_ms(1);
			timeout++;
			if (timeout >= 100) { // took too long, bail out
				srxeDigitalWrite(FLASH_CS, HIGH);
				return false;
			}
		}
		srxeDigitalWrite(FLASH_CS, HIGH);
	} // if asked to wait
	return true;
} /* SRXEFlashEraseSector() */


/* ---
#### int flashWritePage(uint32_t addr, uint8_t* data)

Write a page (up to 256 bytes) of data.

Returns `false` if the operation failed.

**Note:** It will wait no more than 25ms.
--- */
bool flashWritePage(uint32_t addr, uint8_t *data) {
	if (addr & 255L) // invalid address
		return false;

	int i, timeout;
	uint8_t rc;

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x05); // read status register
	rc = _srxe_spi_transfer(0);
	srxeDigitalWrite(FLASH_CS, HIGH);
	if (rc & 1) // indicates the chip is busy in a write operation
		return false; // fail

	// Disable write protect by clearing the status bits
	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x01); // WRSR - write status register
	_srxe_spi_transfer(0x00); // set all bits to 0
	srxeDigitalWrite(FLASH_CS, HIGH);

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x06); // WREN - Write enable
	srxeDigitalWrite(FLASH_CS, HIGH);

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x02); // PP - page program
	// send 3-uint8_t address (big-endian order)
	_srxe_spi_transfer((uint8_t)(addr >> 16)); // AD1
	_srxe_spi_transfer((uint8_t)(addr >> 8));	 // AD2
	_srxe_spi_transfer((uint8_t)addr);		 // AD3
	for (i = 0; i < 256; i++)
		_srxe_spi_transfer(data[i]); // write the 256 data uint8_ts

	srxeDigitalWrite(FLASH_CS, HIGH); // this executes the command internally
	// wait for the write to complete
	rc = 1;
	timeout = 0;
	srxeDigitalWrite(FLASH_CS, LOW);

	while (rc & 1) {
		_srxe_spi_transfer(0x05); // read status register
		rc = _srxe_spi_transfer(0);
		_delay_ms(1);
		timeout++;
		if (timeout >= 25) { // took too long, bail out
			srxeDigitalWrite(FLASH_CS, HIGH);
			return false;
		}
	}
	srxeDigitalWrite(FLASH_CS, HIGH);
	return true;
}


/* ---
#### int flashWritePage(uint32_t addr, uint8_t* buffer, uint16_t count)

Read `count` bytes of data from FLASH.
--- */

bool SRXEFlashRead(uint32_t addr, uint8_t *buffer, uint16_t count) {
	int i;

	srxeDigitalWrite(FLASH_CS, LOW);
	_srxe_spi_transfer(0x03); // issue read instruction
	// send 3-uint8_t address (big-endian order)
	_srxe_spi_transfer((uint8_t)(addr >> 16)); // AD1
	_srxe_spi_transfer((uint8_t)(addr >> 8));	 // AD2
	_srxe_spi_transfer((uint8_t)addr);		 // AD3
	for (i = 0; i < count; i++)					 // read the uint8_ts out
		*buffer++ = _srxe_spi_transfer(0); // need to write something to read from SPI

	srxeDigitalWrite(FLASH_CS, HIGH); // de-activate
	return true;
}


#endif // __SRXE_FLASH_

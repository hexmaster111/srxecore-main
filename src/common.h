/* ************************************************************************************
* File:    common.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

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
--- */

#ifndef __SRXE_COMMON_
#define __SRXE_COMMON_

#ifndef HIGH
#define HIGH		 1
#define LOW			 0
#define INPUT		 0
#define INPUT_PULLUP 1
#define OUTPUT		 2
#endif

/* ---

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

--- */



/* ---
#### uint8_t srxePinMapper(...)

Parameters:
 - uint8_t pincode - _see common pin handling _(above)_ for using macros to create pin codes
 - *volatile uint8_t \*\*ddr - a pointer for returning the AVR direction register
 - volatile uint8_t \*\*port - a pointer for returning the AVR port register
 - bool in - flag to indicate the pin is for input

This function is really an internal function but it is so cool an idea, it deserves to be documented!

It will assign the **ddr** and **port** with the corresponding AVR registers and returns the **pin**.
--- */

uint8_t srxePinMapper(uint8_t pincode, volatile uint8_t **ddr, volatile uint8_t **port, bool in) {
	uint8_t port_code, bit_code;

	if ((ddr == NULL) || (port == NULL))
		return 0;

	port_code = (pincode & 0xf0); // hex port (A,B,D,E,F)
	bit_code = pincode & 0x7;
	switch (port_code) {
		case SRXE_PORTG: // really port G
			*port = (in) ? &PING : &PORTG;
			*ddr = &DDRG;
			break;
		case SRXE_PORTB:
			*port = (in) ? &PINB : &PORTB;
			*ddr = &DDRB;
			break;
		case SRXE_PORTD:
			*port = (in) ? &PIND : &PORTD;
			*ddr = &DDRD;
			break;
		case SRXE_PORTE:
			*port = (in) ? &PINE : &PORTE;
			*ddr = &DDRE;
			break;
		case SRXE_PORTF:
			*port = (in) ? &PINF : &PORTF;
			*ddr = &DDRF;
			break;
	}
	return bit_code;
}

/* ---
#### void srxePinMode(...)

Parameters:
- uint8_t pincode - a pincode _(as detailed above)_
- uint8_t mode - one of `INPUT`, `INPUT_PULLUP`, or `OUTPUT`

Set the specified AVR pin to the mode.
--- */

void srxePinMode(uint8_t pincode, uint8_t mode) {
	uint8_t bit;
	volatile uint8_t *port, *ddr;

	bit = srxePinMapper(pincode, &ddr, &port, 0);

	switch (mode) {
		case INPUT:
			*ddr &= ~(1 << bit);
			break;
		case INPUT_PULLUP:
			*ddr |= (1 << bit);
			*port |= (1 << bit); // set the output high, then set it as an input
			*ddr &= ~(1 << bit);
			break;
		case OUTPUT:
			*ddr |= (1 << bit);
			break;
	}
} /* srxePinMode() */


/* ---
#### void srxeDigitalWrite(uint8_t pincode, uint8_t value)

This is similar to the Arduino `digitalWrite()` but uses the pin code syntax.

Use the available `#define` values of `HIGH` or `LOW`.
--- */
void srxeDigitalWrite(uint8_t pincode, uint8_t value) {
	uint8_t bit;
	volatile uint8_t *port, *ddr;

	bit = srxePinMapper(pincode, &ddr, &port, 0);
	if (value == LOW) {
		*port &= ~(1 << bit);
	} else {
		*port |= (1 << bit);
	}
}


/* ---
#### uint8_t srxeDigitalRead(uint8_t pincode)

This is similar to the Arduino `digitalRead()` but uses the pin code syntax.

Will return one of the `#define` values of `HIGH` or `LOW`.
--- */
uint8_t srxeDigitalRead(uint8_t pincode) {
	uint8_t bit;
	volatile uint8_t *port, *ddr;

	bit = srxePinMapper(pincode, &ddr, &port, 1);
	if (*port & (1 << bit))
		return HIGH;
	else
		return LOW;
} /* srxeDigitalRead() */

/* ---
#### long srxeMap(long val, long in_min, long in_max, long out_min, long out_max)

Re-map a number from one range to another.

The return value is the input value mapped using the min/max mapping from in to out.
--- */
long srxeMap(long val, long in_min, long in_max, long out_min, long out_max) {
	return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



/*

### COMMON SPI HANDLING

The SRXE uses SPI to communicate with both the LCD controller and the FLASH chip on the PCB.

Each SPI device needs an `SS` _select_ pin. By default, this pin is `HIGH`. It is pulled `LOW` to activate the device.
after the SPI operation(s) have been performed, the `SS` is pulled `HIGH` again so the SPI interface is available for any other attached device.
*/

// the CS is uses so multiple devices may share the SPI - Set CS to HIGH so a connected chip will be "deselected" by default

#define SPI_CS		(SRXE_PORTB | PIN0)
#define SPI_SCK		(SRXE_PORTB | PIN1)
#define SPI_MOSI	(SRXE_PORTB | PIN2)
#define SPI_MISO	(SRXE_PORTB | PIN3)

#define LCD_CS	 	(SRXE_PORTE | PIN7)
#define LCD_DC	 	(SRXE_PORTD | PIN6)
#define LCD_RESET	(SRXE_PORTG | PIN2)

// Chip select for the external 1Mbit flash module -  _(that's not a typo; it really is only 128 kilo bytes)_
#define FLASH_CS 	(SRXE_PORTD | PIN3)
#define FLASH_DC 	(SRXE_PORTD | PIN6)

/*
void _srxe_spi_init() - initialize SPI using direct register access
*/
void _srxe_spi_init(void) {
	// Initialize SPI

	srxeDigitalWrite(SPI_CS, HIGH);

	// When the SS pin is set as OUTPUT, it can be used as a general purpose output port
	// (it doesn't influence SPI operations).
	srxePinMode(SPI_CS, OUTPUT);

	// SPCR = 01010000
	//interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
	//*fdufnews 11/2019 ****************  modification to speed SPI transfert  *********************
	//sample on leading edge of clk,system clock/2 rate

	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR = (1 << SPI2X);

	uint8_t temp;
	temp = SPSR; // reading these registers will clear any old data
	temp = SPDR;
	(void)temp;	// this is just to silence the compiler warning

	srxePinMode(SPI_SCK, OUTPUT);
	srxePinMode(SPI_MOSI, OUTPUT);

} /* _srxe_spi_init() */


/*
uint8_t _srxe_spi_transfer(volatile uint8_t data) - transfer one byte to the active/selected SPI device

Returns the response byte from the SPI device.
*/
uint8_t _srxe_spi_transfer(volatile uint8_t data) {
	SPDR = data; // Start the transmission

	/*
		trick from arduboy code:
			The following NOP introduces a small delay that can prevent the wait
			loop from iterating when running at the maximum speed. This gives
			about 10% more speed, even if it seems counter-intuitive. At lower
			speeds it is unnoticed.
	*/
	asm volatile("nop");

	while (!(SPSR & (1 << SPIF))) { // Wait for the end of the transmission
	}

	return SPDR; // return the received uint8_t
} /* _srxe_spi_transfer() */


#endif // __SRXE_COMMON_


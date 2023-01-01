
/* ************************************************************************************
* File:    uart.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## UART
**TX-only UART for debugging via the JTAG `TDI` pad**

The SRXE has a 10 pin pad accessable through the battery compartment.

The **Enigma Development Adapter** board has LEDs as well as provides a 0.1" header to the pins so they may be used as general I/O pins.

The pads are labeled with the JTAG identifiers `TDK`, `TMS`, `TDO`, and `TDI` which correspond to `PORTF` `PIN4` thru `PIN7`.

The `TDI` (`PORTF` `PIN7`) pad is used for very basic UART functions via software _bit banging_ the pin.
The UART is set for 9600-8N1.

**Note:** To use the 4 pins - and by extension the LEDs or UART - the JTAG interface must be disabled via the associated fuse bit.

Here is a handy [AVR FUSE Calculator](https://www.engbedded.com/fusecalc/).

The SRXE fuses are `LOW:0:EE HIGH:1:92 EXT:2:FC PROT:3:FF`

The SRXE fuses, with JTAG disabled, are: `LOW:0:EE HIGH:1:D2 EXT:2:FC PROT:3:FF`

**NOTE:** This will increase power consumption by 15mA. Disable for production or when not needed.

--------------------------------------------------------------------------
--- */

#ifndef __SRXE_UART_
#define __SRXE_UART_

#ifdef SRXECORE_DEBUG

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// UART "bit bang" TX only
// --------------------------------------------------------------------------

// change these to use another pin
#define UART_TX_PORT		PORTF
#define UART_TX_PIN			PF7
#define UART_TX_DDR			DDRF
#define UART_TX_DDR_PIN		DDF7

volatile uint16_t _uart_data = 0;
volatile uint8_t _uart_active = 0;
static uint8_t _uart_inited = 0;

//timer0 compare A match interrupt
ISR(TIMER0_COMPA_vect) {
	uint16_t local_uart_data = _uart_data;
	//output LSB of the TX shift register at the TX pin
	if (local_uart_data & 0x01) {
		UART_TX_PORT |= (1 << UART_TX_PIN);
	} else {
		UART_TX_PORT &= ~(1 << UART_TX_PIN);
	}
	//shift the TX shift register one bit to the right
	local_uart_data >>= 1;
	_uart_data = local_uart_data;
	//if the stop bit has been sent, the shift register will be 0
	//and the transmission is completed, so we can stop & reset timer0
	if (!local_uart_data) {
		TCCR0B = 0;
		TCNT0 = 0;
	}
}


/* ---
#### void uartInit()

Initialize the UART.

Must be called before using any other UART functions.
--- */
void uartInit() {
	//set TX pin as output HIGH
	UART_TX_DDR |= (1 << UART_TX_DDR_PIN);
	UART_TX_PORT |= (1 << UART_TX_PIN);
	//set timer0 to CTC mode
	TCCR0A = (1 << WGM01);
	//enable output compare 0 A interrupt
	TIMSK0 |= (1 << OCF0A);

	// with the 8MHz/8=1MHz timer0 clock: set compare value to 103 to achieve a 9600 baud rate (i.e. 104µs)
	/*
		NOTE: since the internal 8MHz oscillator is not very accurate, this value can be tuned
    	to achieve the desired baud rate, so if it doesn't work with the nominal value (103), try
     	increasing or decreasing the value by 1 or 2
		OCR0A = 103;
	*/
	// at 16MHz, we just double the value for OCR0A
	OCR0A = 204;

	//enable interrupts
	sei();
	_uart_inited = 1;
}


/* ---
#### void uartTerm()

Call if the UART is not longer needed.
--- */
void uartTerm() {
	UART_TX_DDR |= (1 << UART_TX_DDR_PIN);
	UART_TX_PORT &= ~(1 << UART_TX_PIN);
	TCCR0A = 0;
	TCCR0B = 0;
	_uart_inited = 0;
}

/* ---
#### bool uartInited()

True is the UART support hs been initialized.

This is helpful to perform shutdown/wake efficiently
--- */
bool uartInited() {
	return _uart_inited;
}


/* ---
#### int uartPutByteNoWait(uint8_t b)

Transmit a byte and do not wait for the write to complete.

Returns -1 if the UART is busy.

_This function should be used with extreme caution._
--- */
int uartPutByteNoWait(uint8_t b) {
	if (!_uart_inited) return -1;

	uint16_t local_uart_data = _uart_data;

	//if sending the previous character is not yet finished, return
	//transmission is finished when _uart_data == 0
	if (local_uart_data)
		return -1;

	//fill the TX shift register with the character to be sent and the start & stop bits (start bit (1<<0) is already 0)
	local_uart_data = (b << 1) | (1 << 9); //stop bit (1<<9)
	_uart_data = local_uart_data;

	//start timer0 with a prescaler of 8
	TCCR0B = (1 << CS01);

	return b;
}

/* ---
#### void uartPutByte(uint8_t b)

Transmit a byte over the UART

This function will not return until the UART is idle.

**Warning:** This function does not attempt to timeout.
--- */
int uartPutByte(uint8_t b) {
	if (!_uart_inited) return -1;
	// TODO needs a timer to be able to give up
	while (_uart_data)
		;

	uartPutByteNoWait(b);

	//wait until transmission is finished
	while (_uart_data)
		;

	return b;
}

#define uartPutChar uartPutByte

/* ---
#### void uartPutBytes(uint8_t* data, uint16_t len)

Transmit `len` bytes of `data`

This function will not return until the UART is no longer busy.

**Warning:** This function does not attempt to timeout.
--- */
void uartPutBytes(uint8_t *data, uint16_t len) {
	if (!_uart_inited) return;
	for (uint16_t i = 0; i < len; i++)
		uartPutByte(data[i]);
}


/* ---
#### void uartPutString(char* string)

Transmit a null terminated string.

This function will not return until the UART is no longer busy.

**Warning:** This function does not attempt to timeout.
--- */
void uartPutString(char *string) {
	if (!_uart_inited) return;
	uartPutBytes((uint8_t*)string, strlen(string));
}

/* ---
#### void uartPutStringNL(char* string)

Transmit a null terminated string with a trailing newline

This function will not return until the UART is no longer busy.

**Warning:** This function does not attempt to timeout.
--- */
void uartPutStringNL(char *string) {
	if (!_uart_inited) return;
	uartPutBytes((uint8_t*)string, strlen(string));
	uartPutByte('\n');
}

#else // SRXECORE_DEBUG

#define uartInit() ((void)0)
#define uartTerm() ((void)0)
#define uartInited() (false)
#define uartPutByteNoWait(b) ((void)-1)
#define uartPutByte(b) ((void)-1)
#define uartPutChar(b) ((void)-1)
#define uartPutBytes(data, len) ((void)0)
#define uartPutString(string) ((void)0)
#define uartPutStringNL(string) ((void)0)

#endif // SRXECORE_DEBUG

#endif // __SRXE_UART_

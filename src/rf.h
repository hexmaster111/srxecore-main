/* ************************************************************************************
* File:    rf.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

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
--- */

#ifndef __SRXE_RF_
#define __SRXE_RF_

// --------------------------------------------------------------------------
// RF RECEPTION DEBUGGING ---------------------------------------------------
// --------------------------------------------------------------------------

/* ---
There are two internal byte values available for debugging RF related issues.
They are:
```C
*/

//int8_t _rf_tx_debug;   // = -length of tx data while transmitting; = length of transmitted data when finished
//int8_t _rf_rx_debug;   // = 0 while receiving data; = length of received data when finished

/*
```
--- */

#define RF_CHANNEL_MIN 1
#define RF_CHANNEL_MAX 16

// the RF code is only for the ATMEGA128RFA1 chip
#ifndef CHIP_ATMEGA128RFA1
#define CHIP_ATMEGA128RFA1
#endif


//Interrupt related macros // TODO these should really use the official bit offset definitions
#if 1

//TRXPR MASK DEFINES
#define TRX_SLEEP 0x01
#define TRX_RESET 0x02

//TRX_STATUS MASK DEFINES
#define STAT_BUSY_RX					  0x01
#define STAT_BUSY_TX					  0x02
#define STAT_RX_ON						  0X06
#define STAT_TRX_OFF					  0X08
#define STAT_PLL_ON						  0X09
#define STAT_SLEEP						  0x0F
#define STAT_BUSY_RX_AACK				  0X11
#define STAT_BUSY_UART_TX_ARET			  0X12
#define STAT_RX_AACK_ON					  0X16
#define STAT_UART_TX_ARET_ON			  0X19
#define STAT_STATE_TRANSITION_IN_PROGRESS 0X1F
#define STAT_CCA_STATUS_IDLE			  0X40
#define STAT_CCA_DONE					  0x80

//TRX_STATE REGISTER MASK DEFINES
//TRAC_STATUS BITS MASK DEFINES
#define STAT_TRX_SUCCESS				0x00
#define STAT_SUCCESS_DATA_PENDING		0x20
#define STAT_SUCCESS_WAIT_FOR_ACK		0x40
#define STAT_CHANNEL_ACCESS_FAILURE 	0x60
#define STAT_NO_ACK						0xA0
#define STAT_INVALID					0xE0
//TRX_CMD BITS MASK DEFINES
#define CMD_TRX_CLEAR 					0xE0
//#define CMD_NOP						0x00
//#define CMD_TX_START					0x02
//#define CMD_FORCE_TRX_OFF				0x03
//#define CMD_FORCE_PLL_ON				0x04
//#define CMD_RX_ON						0x06
//#define CMD_TRX_OFF					0x08
//#define CMD_PLL_ON					0x09
#define CMD_TX_ON 						0x09
//#define CMD_RX_AACK_ON				0x16
//#define CMD_TX_ARET_ON				0x19

//IRQ_MASK REGISTER (ENABLE) MASK DEFINES
#define INT_TRX							0XFF
#define INT_AWAKE						0x80
#define INT_TX_END						0x40
#define INT_AMI							0x20
#define INT_CCA_ED_DONE 				0x10
#define INT_RX_END						0x08
#define INT_RX_START					0x04
#define INT_PLL_UNLOCK					0x02
#define INT_PLL_LOCK					0x01

//TRX_CTRL_1 REGISTER MASK DEFINES
#define CTRL_PA_EXT_EN					0x80
#define CTRL_IRQ_2_EXT_EN				0x40 //This bit is not to be enabled (set to 1) if Antenna diversity is being used as they share a pin
#define CTRL_TX_AUTO_CRC_ON 			0x20

//PHY_CC_CCA REGISTER MASK DEFINES
#define PHY_CCA_REQUEST	   				0x80
#define PHY_CCA_MODE1	   				0X40
#define PHY_CCA_MODE0	   				0x20
#define PHY_RESET_CHANNELS 				0xE0

//Macros

//Interrupt related macros
#define INT_TRX_ENABLE					IRQ_MASK |= INT_TRX
#define INT_TRX_DISABLE					IRQ_MASK &= ~(INT_TRX)
#define INT_AWAKE_ENABLE				IRQ_MASK |= INT_AWAKE
#define INT_AWAKE_DISABLE				IRQ_MASK &= ~(INT_AWAKE)
#define INT_TX_END_ENABLE				IRQ_MASK |= INT_TX_END
#define INT_TX_END_DISABLE				IRQ_MASK &= ~(INT_TX_END)
#define INT_AMI_ENABLE					IRQ_MASK |= INT_AMI
#define INT_AMI_DISABLE					IRQ_MASK &= ~(INT_AMI)
#define INT_CCA_ED_DONE_ENABLE			IRQ_MASK |= INT_CCA_ED_DONE
#define INT_CCA_ED_DONE_DISABLE 		IRQ_MASK & ~(INT_CCA_ED_DONE)
#define INT_RX_END_ENABLE				IRQ_MASK |= INT_RX_END
#define INT_RX_END_DISABLE				IRQ_MASK &= ~(INT_RX_END)
#define INT_RX_START_ENABLE				IRQ_MASK |= INT_RX_START
#define INT_RX_START_DISABLE			IRQ_MASK &= ~(INT_RX_START)
#define INT_PLL_UNLOCK_ENABLE			IRQ_MASK |= INT_PLL_UNLOCK
#define INT_PLL_UNLOCK_DISABLE			IRQ_MASK &= ~(INT_PLL_UNLOCK)
#define INT_PLL_LOCK_ENABLE				IRQ_MASK |= INT_PLL_LOCK
#define INT_PLL_LOCK_DISABLE			IRQ_MASK &= ~(INT_PLL_LOCK)

#endif // our AVR macros and definitions



//#define HW_FRAME_BUFFER	 	((uint8 *)(&TRXFBST + 1))	// this uses the hardware frame buffer directly
#define HW_FRAME_RX_SIZE		128
#define HW_FRAME_TX_SIZE		127							// TX uses a byte for the length


#define RF_TX_BUFFER_SIZE (HW_FRAME_TX_SIZE+1)				// could be larger but the current code does not need it
#define RF_RX_BUFFER_SIZE (HW_FRAME_RX_SIZE * 2)			// it only needs to be larger than HW_FRAME_BUFFER_SIZE to allow for more than a single message to arrive before being read

static uint8_t rfRxData[RF_RX_BUFFER_SIZE];
static uint8_t rfTxData[RF_TX_BUFFER_SIZE];

#include "cbuffer.h"

static cBufferObj _rf_obj;

//#define IO_DEVICE_RF 0	// this is legacy

// --------------------------------------------------------------------------
// RF INTERUPT VECTORS (ATMEGA128RFA1) --------------------------------------
// --------------------------------------------------------------------------
static uint8_t _rf_signal; // reusable byte access from the INT vectors



// RF TX is not handled by an interrupt. We process data synchronously to the frame buffer and then let it do it's thing.
void RF_LOAD_FRAME() {
	uint8_t length = 0;
	int c;
	uint8_t *bp = (uint8_t *)(&TRXFBST + 1);

	while (length < (HW_FRAME_TX_SIZE - 1)) {
		if ((c = bufferGet(&(_rf_obj.txBuffer))) < 0)
			break;
		bp[length++] = c;
	}
	bp[length++] = 0;

	// length is the number of bytes we have loaded into the hardware frame buffer

	//_rf_tx_debug = 0 - length; // we do this to be able to track state while debugging
	//The Transceiver State Control Register -- TRX_STATE controls the states of the radio.
	// Setting the PLL state to PLL_ON begins the TX.

	TRXFBST = 2 + length; // length (byte) +  n bytes of data
}

void RF_TX_FRAME() {
	TRX_STATE = (TRX_STATE & 0xE0) | PLL_ON; // Set to TX start state
	while (!(TRX_STATUS & PLL_ON))
		; // Wait for PLL to lock

	RF_LOAD_FRAME();

	// The start of frame buffer - TRXFBST is the first byte of the 128 byte frame. It should contain the length of the transmission.

	TRX_STATE |= CMD_TX_START; // initiate TX
	TRXPR |= (1 << SLPTR);	   // Setting SLPTR high will start the TX.
	TRXPR &= ~(1 << SLPTR);	   // Setting SLPTR low will end the TX.

	_delay_ms(1); // not sure if this needed

	// After the byte is sent the radio is set back into the RX waiting state.
	TRX_STATE = (TRX_STATE & 0xE0) | RX_ON;
}


// This interrupt is called when radio TX is complete. We'll just
// ISR(TRX24_TX_END_vect) { }	// not used

ISR(TRX24_TX_END_vect) {
	//_rf_tx_debug = 0 - _rf_tx_debug; // used to track state for debugging RF issues
}

// This interrupt is called when data is received by the radio. It gives us an opportunity to grab signal strength
ISR(TRX24_RX_START_vect) {
	/*
		RSSI - Received Signal Strength Indicator:
		·   RSSI is a 5-bit value and  varies from 0 to 28.
			It varies as linear scale in logarithmic input power scale (dBm) with a resolution of 3 dB.
			The dynamic range of RSSI value is 81 db.
		·   RSSI value is updated every 2 µs in receive states in the PHY_RSSI register.
		eg: Power = -90 + 3*(RSSI-1) dbm
		·   RSSI = 0 (Indicates power lower than -90 dBm which is the RSSI_BASE_VAL)
		·   RSSI = 28 (Indicates power higher or equal to -10 dbm)
	*/
	_rf_signal = PHY_RSSI; // Read in the received signal strength
	//_rf_rx_debug = 0;
}

// This interrupt is called at the end of data receipt.
// We can now get the data received and store it in the receive buffer.
ISR(TRX24_RX_END_vect) {
	// The received signal must be above a certain threshold.
	if (_rf_signal & RX_CRC_VALID) {
		uint8_t length;
		uint8_t frame[RF_RX_BUFFER_SIZE];

		length = TST_RX_LENGTH;						 // first byte is length of received bytes
		memcpy(&frame[0], (void *)&TRXFBST, length); // remaining bytes are the data

		// there are 2 extra bytes; we know one is the LQI; the other might(?) be the CRC? ... not sure
		// copy from to our receive buffer
		for (int i = 0; i < (length - 2); i++) {
			if (bufferPut(&(_rf_obj.rxBuffer), frame[i]) < 0)
				_rf_obj.rxOverflow++; // no space in buffer; count overflow
		}
		//_rf_rx_debug = length;
	}
}

bool _rf_off_state() {
	// Transceiver State Control Register (TRX_STATE) controls the states of the radio
	// Transceiver Status Register (TRX_STATUS) contains the present state of the radio

	// First, we'll set it to the TRX_OFF state.
	TRX_STATE = (TRX_STATE & 0xE0) | TRX_OFF; // Set to TRX_OFF state
	_delay_ms(1);

	// After telling it to go to the TRX_OFF state, we'll make sure it's actually there
	if ((TRX_STATUS & 0x1F) != TRX_OFF) { // Check to make sure state is correct
		//uartPutStringNL("RF: TX_OFF incorrect");
		TRX_STATE |= (TRX_STATE & 0xE0) | CMD_FORCE_TRX_OFF;
		_delay_ms(1);
		if ((TRX_STATUS & 0x1F) != TRX_OFF) { // Check to make sure state is correct
			//uartPutStringNL("RF: TX_OFF still incorrect");
			TRXPR &= ~(1 << SLPTR); // if the transceiver state is SLEEP then wake
			_delay_ms(1);
			if ((TRX_STATUS & 0x1F) != TRX_OFF) { // Check to make sure state is correct
				//uartPutStringNL("RF: TX_OFF failed to set");
				return false; // Error, TRX isn't off
			}
		}
		//uartPutStringNL("RF: TX_OFF corrected");
	}
	return true;
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

/* ---
#### rfInit(uint8_t channel)

Initialize the RF transceiver on the specified channel.

The RF Transceiver has 16 possible channels (1 .. 16)

Must be called to initialize the RF transceiver prior to using any other RF functions.
--- */
void rfInit(uint8_t channel) {

	//_rf_obj.id = IO_DEVICE_RF;
	_rf_obj.inited = 0;

	// for usability the input is a channel from 1..16
	if ((channel < RF_CHANNEL_MIN) || (channel > RF_CHANNEL_MAX))
		channel = 1;

	// the physical values are 11..26
	uint8_t physical_channel = channel + 10;

	// initialize the buffers
	bufferReset(&(_rf_obj.rxBuffer), rfRxData, RF_RX_BUFFER_SIZE); // initialize the receive buffer
	bufferReset(&(_rf_obj.txBuffer), rfTxData, RF_TX_BUFFER_SIZE); // initialize the transmit buffer

	//cli(); // prevent interrupts

	// do nothing while STATE_TRANSITION_IN_PROGRESS
	while (TRX_STATUS == STATE_TRANSITION_IN_PROGRESS)
		_delay_ms(1);

	if ((TRX_STATUS & 0x1F) == SLEEP) {
		TRXPR &= ~(1 << SLPTR); // if the transceiver state is SLEEP then wake
	}
	TRXPR |= (1 << TRXRST);
	IRQ_MASK = 0;

	if (!_rf_off_state())
		return;

	// Transceiver Control Register 1 - TRX_CTRL_1
	// We'll use this register to turn on automatic CRC calculations.
	TRX_CTRL_1 |= (1 << TX_AUTO_CRC_ON); // Enable automatic CRC calc.

	// Enable RX start/end and TX end interrupts
	IRQ_MASK = (1 << RX_START_EN) | (1 << RX_END_EN) | (1 << TX_END_EN);

	// Transceiver Clear Channel Assessment (CCA) -- PHY_CC_CCA
	// This register is used to set the channel. CCA_MODE should default
	// to Energy Above Threshold Mode.
	// Channel should be between 11 and 26 (2405 MHz to 2480 MHz)

	PHY_CC_CCA = (PHY_CC_CCA & 0xE0) | physical_channel; // Set the channel (default is 11)

	// set power

	PHY_TX_PWR &= ~(TX_PWR3 | TX_PWR2 | TX_PWR1 | TX_PWR0); // clear any existing bits
	//PHY_UART_TX_PWR |= ();	// set the bits we want; all bits 0 = full power

	// tweak timing of power amplifier switch receive and transmit
	//PHY_UART_TX_PWR |= ((1 << PA_LT1) | (1 << PA_LT0)); // set the bits we want; all bits 0 = full power
	// tweak timing of power amplifier relative to transmit
	//PHY_UART_TX_PWR |= ((1 << PA_BUF_LT1) | (1 << PA_BUF_LT0));

	// Finally, we'll enter into the RX_ON state. Now waiting for radio RX's, unless
	// we go into a transmitting state.
	TRX_STATE = (TRX_STATE & 0xE0) | RX_ON; // Default to receiver

	// sei();

	// establish our initial state
	_rf_obj.rxOverflow = 0;
	_rf_obj.txIdle = true;

	_rf_obj.inited = channel;
}


/* ---
#### rfTerm()

Close and powerdown the RF transceiver.

Use this function when the RF transceiver is not actively needed to conserve battery power.

Use rfInit(_channel_) to begin using the RF transceiver again.
--- */
void rfTerm() {
	if (!_rf_obj.inited) return;

	//_rf_obj.id = IO_DEVICE_RF;
	_rf_obj.inited = false;

	_rf_obj.rxOverflow = 0;
	_rf_obj.txIdle = true;

	_rf_off_state();

	TRXPR = 1 << SLPTR; // if the transceiver state is TRX_OFF then sleep

	IRQ_MASK = 0;

	return;
}

/* ---
#### uint8_t rfInited()

Return the RF channel if inited, otherwise return 0.

This is helpful to perform shutdown/wake efficiently and for controlling the RF radio only when needed
--- */
uint8_t rfInited() {
	return _rf_obj.inited;
}




/* ---
### Helper Functions
--- */

/* ---
#### void rfFlushReceiveBuffer()

Flush any pending data in the receive buffer (useful if you are waiting on a specifc message and have detected it is corrupted).
--- */
void rfFlushReceiveBuffer() {
	// flush all data from receive buffer
	bufferFlush(&(_rf_obj.rxBuffer));
	_rf_obj.rxOverflow = 0;
}


/* ---
#### bool rfReceiveBufferOverflow()

 Returns `true` if the receive buffer has filled up.
 This is a good indication that subsequent data is unreliable.
--- */
uint8_t rfReceiveBufferOverflow() {
	return _rf_obj.rxOverflow; // no space in buffer; count overflow
}


/* ---
### Read Functions
--- */

/* ---
#### int rfAvailable()

Return the number of unread bytes in receive buffer.
--- */
int rfAvailable() {
	if (!_rf_obj.inited)
		return -1;
	return _rf_obj.rxBuffer.length;
}


/* ---
#### int rfGetByte()

Return a single byte from the receive buffer (getchar-style) returns -1 if no byte is available.
--- */
int rfGetByte() {
	if (!_rf_obj.inited)
		return -1;
	return bufferGet(&(_rf_obj.rxBuffer));
}


/* ---
#### int rfGetBuffer(uint8_t *data, uint8_t maxlen)

Return up to `maxlen` bytes from the receive buffer into the `data` buffer.
Returns the number of bytes stored in the `data` buffer.
--- */
int rfGetBuffer(uint8_t *data, uint8_t maxlen) {
	if (!_rf_obj.inited)
		return -1;

	memset(data, 0, maxlen);

	int avail = rfAvailable();
	if (avail > maxlen)
		avail = maxlen;
	for (int i = 0; i < avail; i++) {
		data[i] = bufferGet(&(_rf_obj.rxBuffer));
	}
	return avail;
}


/* ---
### Write Functions
--- */

/* ---
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
--- */
int rfPutBuffer(uint8_t *data, uint8_t len) {
	if (!_rf_obj.inited)
		return -1;

	for (uint8_t i = 0; i < len; i++)
		bufferPut(&(_rf_obj.txBuffer), data[i]);	// bufferPut() will ignore any bytes that will not fit

	if (_rf_obj.txBuffer.length >= (HW_FRAME_TX_SIZE)) {
		RF_TX_FRAME();
	}

	return len;
}


/* ---
#### int rfPutString(char* data)

Stores the string in the transmit buffer and transmit it.
--- */
int rfPutString(char* data) {
	if (!_rf_obj.inited)
		return -1;

	uint8_t len = strlen(data);	// does not include the null terminator (so strings may be concatanated within the buffer)
	int rtn = rfPutBuffer((uint8_t *)data, len);
	rfTransmitNow();
	return rtn;
}


#endif // __SRXE_RF_

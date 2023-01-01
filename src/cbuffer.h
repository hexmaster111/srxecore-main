/* ************************************************************************************
* File:    cbuffer.h
* Date:    2021.07.01
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## cbuffer.h - circular buffer support

For all intents and purposes, you can ignore this. The functions are used internally
by various capabilities within the library.

--------------------------------------------------------------------------
--- */


#ifndef __CIRCULAR_BUFFER_CODE // Circular Buffer Code
#define __CIRCULAR_BUFFER_CODE

#if defined(CHIP_ATMEGA4809)
#define CRITICAL_SECTION_START	cli()
#define CRITICAL_SECTION_END	sei()
#endif

#if defined(CHIP_ATMEGA328PB) || defined(CHIP_ATMEGA128RFA1)
#define CRITICAL_SECTION_START  \
	unsigned char _sreg = SREG; \
	cli()
#define CRITICAL_SECTION_END SREG = _sreg
#endif

#define _cli()
#define _sei()
//#define _cli()	cli()
//#define _sei()	sei()

// cBuffer structure
typedef struct struct_cBuffer {
	unsigned char *data;				  // the physical memory for the buffer
	unsigned short size;				  // the allocated size of the buffer
	unsigned short length;				  // the length of the data currently in the buffer
	unsigned short current;				  // the index into the buffer where the data starts
} cBuffer;

// buffer object structure
typedef struct struct_cBufferObject {
	uint8_t id;					// deprecated? which usart (0 or 1); we track this to optimize some code
	cBuffer rxBuffer; 			// UART receive buffer
	cBuffer txBuffer; 			// UART transmit buffer
	unsigned short rxOverflow;	// receive overflow counter
	uint8_t txIdle;				// the TX buffer was emptied and the interrupt will need a jump start
	uint8_t inited;				// typically a bool but can be any value where 0 means not inited
} cBufferObj;


void bufferReset(cBuffer *buffer, uint8_t *data, uint16_t size) {
	// begin critical section
	CRITICAL_SECTION_START;
	buffer->data = data;
	buffer->size = size;
	// initialize index and length
	buffer->current = 0;
	buffer->length = 0;
	// end critical section
	CRITICAL_SECTION_END;
}

// access routines
int bufferPeek(cBuffer *buffer) {
	int rtn = -1;
	// begin critical section
	CRITICAL_SECTION_START;
	// check to see if there's data in the buffer
	if (buffer->length) {
		// get the first character from buffer
		rtn = buffer->data[buffer->current];
	}
	// end critical section
	CRITICAL_SECTION_END;
	// return
	return rtn;
}

// access routines
int bufferGet(cBuffer *buffer) {
	int rtn = -1;
	// begin critical section
	CRITICAL_SECTION_START;
	// check to see if there's data in the buffer
	if (buffer->length) {
		// get the first character from buffer
		rtn = buffer->data[buffer->current];
		// move index down and decrement length
		buffer->current++;
		if (buffer->current >= buffer->size)
			buffer->current -= buffer->size;
		buffer->length--;
	}
	// end critical section
	CRITICAL_SECTION_END;
	// return
	return rtn;
}

int bufferPut(cBuffer *buffer, uint8_t data) {
	int rtn = -1;

	// begin critical section
	CRITICAL_SECTION_START;
	// make sure the buffer has room
	if (buffer->length < buffer->size) {
		// save data byte at end of buffer
		buffer->data[(buffer->current + buffer->length) % buffer->size] = data;
		// increment the length
		buffer->length++;
		rtn = data;
	}
	// end critical section
	CRITICAL_SECTION_END;
	// return the original data character or failure
	return rtn;
}

int bufferPutWait(cBuffer *buffer, uint8_t data) {
	int rtn = -1;
	uint8_t attempts = 50;	// 50 * 5 ms = 250ms is the maximum we will wait to load the data
	while ((rtn = bufferPut(buffer, data)) < 0) {
		attempts--;
		if (!attempts)
			break;
		_delay_ms(5);
	}
	return rtn;
}

unsigned short bufferCapacity(cBuffer *buffer) {
	// begin critical section
	CRITICAL_SECTION_START;
	// check to see if the buffer has room
	// return true if there is room
	unsigned short bytesleft = (buffer->size - buffer->length);
	// end critical section
	CRITICAL_SECTION_END;
	return bytesleft;
}

unsigned short bufferEmpty(cBuffer *buffer) {
	// begin critical section
	CRITICAL_SECTION_START;
	// check to see if the buffer has room
	// return true if there is room
	unsigned short empty = true;
	if (buffer->length)
		empty = false;
	// end critical section
	CRITICAL_SECTION_END;
	return empty;
}

void bufferFlush(cBuffer *buffer) {
	// begin critical section
	CRITICAL_SECTION_START;
	// flush contents of the buffer
	buffer->length = 0;
	// end critical section
	CRITICAL_SECTION_END;
}

#endif // __CIRCULAR_BUFFER_CODE // Circular Buffer Code

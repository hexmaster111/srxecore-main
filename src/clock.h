/* ************************************************************************************
* File:    clock.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## Clock
**A 1 Millisecond Reference Timer**

The clock module uses TIMER2 and the COMPA vector to establish a timer and exposes it with 1 millisecond
accuracy. This is useful for performing timed operations, animations, detecting timeouts, etc.

**Note:** The timer runs at 10Khz and every 10th interupt we increment the milliseconds counter.
It is possible to use this time for other tasks.
A similar design is used in [ecccore](https://gitlab.com/bradanlane/ecccore) where
the charliePlexed LEDs are hooked onto the clock timer.

--------------------------------------------------------------------------
--- */

// Ref Material: https://www.avrfreaks.net/comment/2416971#comment-2416971


#ifndef __SRXE_CLOCK_
#define __SRXE_CLOCK_

// the follow variable is used within the interrupt to maintain a clock for animation
volatile uint8_t _clock_ticks; // when the ISR is at 10Khz, each tick = 0.0001 = 0.1ms = 100us
volatile uint32_t _clock_ms;   // external calls often want millisecond accuracy

// the ATMega code only works at 16Mhz or 2Mhz and for 10Khz and 2Khz

#define TIMER_FREQ 10000				// Khz timer
#define TIMER_INTERVAL_COMPENSATION (0)	// compensation for code execution time (positive value speeds up interrupt); requires oscilloscope and GPIO to calibrate

#define INTERRUPTS_PER_MILLIS (TIMER_FREQ / 1000)	// 1ms reference available to core functions
#define TIMER_INTERVAL ((F_CPU / TIMER_FREQ) - 1)	// computed COMP value

ISR(TIMER2_COMPA_vect) {
	// update our animation clock
	_clock_ticks++;
	if (_clock_ticks >= INTERRUPTS_PER_MILLIS) {	// 10 is for 10000 Hz while 2 is for 2000 Hz interrupt frequency
		_clock_ticks = 0;
		_clock_ms++;
	}
}

/* ---
#### void clockInit()

Initialize the clock.

This function must be called prior to using any other clock functions.
--- */

void clockInit() {

	// Timer Code Generator
	// WARNING: the code generated by http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
	// has a mistake = the CTC Mode should be set on TCCR2A not TCCR2B

	cli();												// stop interrupts
	TCCR2A = 0; 										// clear the mode register
	TCCR2B = 0; 										// clear the prescaler
	TCNT2 = 0;  										// initialize counter value to 0

#if (F_CPU == 16000000UL)
#if (TIMER_FREQ == 10000)
	OCR2A = 199 - TIMER_INTERVAL_COMPENSATION;			// = 16000000 / (8 * 10000) - 1 (must be <256)
	TCCR2B |= (0 << CS22) | (1 << CS21) | (0 << CS20);	// Set CS22, CS21 and CS20 bits for 8 prescaler
#endif
#if (TIMER_FREQ == 2000)
	OCR2A = 249 - TIMER_INTERVAL_COMPENSATION; 			// = 16000000 / (8 * 2000) - 1 (must be <256)
	TCCR2B |= (0 << CS22) | (1 << CS21) | (1 << CS20);	// Set CS22, CS21 and CS20 bits for 32 prescaler
#endif
#endif
#if (F_CPU == 2000000UL)
#if (TIMER_FREQ == 10000)
	OCR2A = 199 - TIMER_INTERVAL_COMPENSATION; 			// = 2000000 / (8 * 10000) - 1 (must be <256)
	TCCR2B |= (0 << CS22) | (0 << CS21) | (1 << CS20);	// Set CS22, CS21 and CS20 bits for 1 prescaler
#endif
#if (TIMER_FREQ == 2000)
	OCR2A = 124 - TIMER_INTERVAL_COMPENSATION; 			// = 2000000 / (8 * 2000) - 1 (must be <256)
	TCCR2B |= (0 << CS22) | (1 << CS21) | (0 << CS20);	// Set CS22, CS21 and CS20 bits for 8 prescaler
#endif
#endif
	TCCR2A |= (1 << WGM21);								// turn on CTC mode

	TIMSK2 |= (1 << OCIE2A);							// enable the timer compare interrupt
	sei();												// allow interrupts
}


/* ---
#### uint32_t clockMillis()

Return the current counter as a 32bit unsigned integer. The counter starts at 0 when clockInit() is first called.
--- */

uint32_t clockMillis() {
	return _clock_ms;
}


/* ---
#### void clockDelay(uint32_t duration)

Delay for the specified number of milliseconds.
This exists since the low level `_delay_ms()` requires durations at compile time
whereas this function lets the duration be determined at runtime.
--- */

void clockDelay(uint32_t duration) {
	for (; duration > 0; duration--)
		_delay_ms(1);
}

#endif // __SRXE_CLOCK_

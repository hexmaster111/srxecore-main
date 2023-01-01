/* ************************************************************************************
* File:	power.h
* Date:	2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

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
--- */

#ifndef __SRXE_POWER_
#define __SRXE_POWER_

#include "common.h"

#define POWER_PPIN 0xD2

#define LONG_PRESS			1000   // 1 seconds
#define SLEEP_IDLE_DURATION 10000     //300000 // 5 minutes

static unsigned long _power_button_timer;
static uint32_t _power_sleep_timer;

//
// Called when the power button is pressed to wake up the system
// Power up the display
//
ISR(INT2_vect) {
	// cancel sleep as a precaution
	sleep_disable();
}

/* ---
#### void powerInit()

Initialize the power system. This function must be called prior to using any other power functions.
--- */

void powerInit() {
	_power_button_timer = 0;
	// enable power button

	DDRD &= ~(1 << PORTD2); // PIN INT2 as input
	PORTD |= (1 << PORTD2); // pull-up resistor, the pin is forced to 1 if nothing is connected

	// 500uA this sets CS HIGH for the U2 chip which puts it to sleep
	OUTHIGH(DDRD, PORTD, PIN3);

	//MCUCR |= (1 << JTD);	// 300uA disable JTAG debugging; better to use the FUSE bit to disable JTAG

	// setup battery monitoring

	// Select 1.6V ref voltage
	// Select A0 as input
	ADMUX = 0xC0;  // Int ref 1.6V
	ADCSRA = 0x87; // Enable ADC
	ADCSRB = 0x00; // MUX5= 0, freerun
	ADCSRC = 0x54; // Default value
	ADCSRA = 0x97; // Enable ADC
	//clockDelay(5);
	ADCSRA |= (1 << ADSC); // start conversion

	_power_sleep_timer = clockMillis() + SLEEP_IDLE_DURATION;
}

/* ---
#### bool powerButtonState()

Report if the power button is pressed or not.

The function will return `false` if the button is not pressed.

This function may be used to create custom features based on the state of the power button including detecting presses of varying duration.
--- */

bool powerButtonState() {
	// if button is pressed LOW ...
	if (!srxeDigitalRead(POWER_PPIN))
		return true;
	return false;
}

/* ---
#### bool powerButtonPressed()

Report if the power button has a long press.
The function will return `false` if the button is not pressed or if it is not a long press.
If it detects a long press, it will wait for the release before returning `true`.
--- */

bool powerButtonPressed() {
	// if button is pressed LOW ...
	if (!srxeDigitalRead(POWER_PPIN)) {
		// if it's the initial press ...
		if (!_power_button_timer) {
			_power_button_timer = clockMillis() + LONG_PRESS;
			return false;
		}
		// else determine duration of pressed
		if (clockMillis() > _power_button_timer) {
			// wait for button release to return
			while (!srxeDigitalRead(POWER_PPIN))
				;
			_power_button_timer = 0;
			return true;
		}
	} else
		_power_button_timer = 0;
	return false;
}
/* ---
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
--- */
void powerSleep(void) {

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	cli();

	TRXPR = 1 << SLPTR; // send transceiver to sleep

	ADCSRA = 0; // disable ADC

#if 1
	uint8_t db, pb, dd, pd, de, pe, df, pf, dg, pg;

	db = DDRB;
	pb = PORTB;
	dd = DDRD;
	pd = PORTD;
	de = DDRE;
	pe = PORTE;
	df = DDRF;
	pf = PORTF;
	dg = DDRG;
	pg = PORTG;

	//DDRB  = 0;
	//PORTB = 0;
	//DDRD  = 0;
	//PORTD = 0;
	//DDRE  = 0;
	//PORTE = 0;
	//DDRF  = 0;
	//PORTF = 0;
	//DDRG  = 0;
	//PORTG = 0;
#endif

	//OUTHIGH(DDRD, PORTD, PIN3);
	//MCUCR |= (1 << JTD);	// 300uA disable JTAG debugging; better to use the FUSE bit to disable JTAG

	// NOTE: PD2 is our wake button when connected to GND

	DDRD &= ~(1 << PORTD2); // PIN INT2 as input
	//PORTD &= ~(1 << PORTD2);				 // no pull-up resistor, the pin is floating but we think the PCB has a 4.7K pull-up already
	PORTD |= (1 << PORTD2); // pull-up resistor, the pin is forced to 1 if nothing is connected

	EIMSK &= ~(1 << INT2);					 // disabling interrupt on INT2
	EICRA &= ~((1 << ISC21) | (1 << ISC20)); // low level triggers interrupt
	EIFR |= (1 << INTF2);					 // clear interrupt flag
	EIMSK |= (1 << INT2);					 // enabling interrupt flag on INT2

	sei();

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();

	// We are guaranteed that the sleep_cpu call will be done
	// as the processor executes the next instruction after
	// interrupts are turned on.

	sleep_cpu(); // one cycle

	sleep_disable();

	cli();

	EIFR |= (1 << INTF2);  //clear interrupt flag
	EIMSK &= ~(1 << INT2); //disabling interrupt on INT2

#if 1
	DDRB = db;
	PORTB = pb;
	DDRD = dd;
	PORTD = pd;
	DDRE = de;
	PORTE = pe;
	DDRF = df;
	PORTF = pf;
	DDRG = dg;
	PORTG = pg;
#endif

	sei();

} /* SRXESleep() */

/* ---
#### void powerSleepUpdate()

Restart activity timeout before sleep
--- */
void powerSleepUpdate() {
	_power_sleep_timer = clockMillis() + SLEEP_IDLE_DURATION;
}

/* ---
#### void powerSleepConditionally()

power down and sleep if the power button is pressed or the idle timer has run out

_Typically called from your main program loop._

--- */
uint8_t powerSleepConditionally() {
	// check if WAKE button is pressed
	if (powerButtonPressed()) {
		// activate sleep
		//powerSleep();
		// we do not actually activate sleep; we return a boolean so the caller may do pre-sleep activities
		return true;
	}

	// if there has been no user action for a while, we go to sleep
	uint32_t now = clockMillis();
	if (now > _power_sleep_timer) {
		//powerSleep();
		// we do not actually activate sleep; we return a boolean so the caller may do pre-sleep activities
		return true;
	}
	return false;
}

/* ---
#### uint16_t powerBatteryLevel(void)

Return the battery level in millivolts as a 16 bit integer
--- */
uint16_t powerBatteryLevel(void) {
	// Battery is connected through a resistor divider (825k and 300k) with gain of 0.266666
	// ADC ref is 1.6V with 1024 steps
	// to simplify the calculation we will neglect 1024 and round it to 1000
	// so we will use a conversion factor of 1.6/0.26666 which is 6
	// This will give an error of 24/1000 which is acceptable in this case
	uint16_t low, high;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC))
		;
	low = ADCL;
	high = ADCH;

	return ((high << 8) + low) * 6;
}

#endif // __SRXE_POWER_

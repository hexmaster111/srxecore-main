/* ************************************************************************************
* File:    smoketest.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

# SMART Response XE  Device Smoketest and Demo

This demo is a non-exhaustive demonstration of the SRXEcore library.
It includes the LEDs and UART capabilities exposed by the optional **Enigma Development Adapter**.
The adapter is not required to run this program.

In addition to demonstration of the SRXEcore library, this example is also a suitable test of the
SRXE hardware.

_The SMART Response XE (SRXE) is an existing hardware device from around 2011.
There are a lot of these on the used market.
Single devices and batches of devices are often listed on eBay.
It's not uncommon when buying a batch of devices to learn that the devices were
stored for a long period with cheap old batteries.
The biggest problem with these devices is corrosion.
When acquiring SRXE hardware, its very common to find old batteries and corrosion.
Used SRXE devices need to be thuroughly cleaned and tested.
This demo provides a convenient test of all functions of the SRXE.__

**Note:** The documentation does not attempt to explain the demo _line-by-line_.
It provides a high level explanation of the demo. For a full understanding of the demo,
and to see examples of SRXEcore library functions being used, please read through the actual code.

--------------------------------------------------------------------------

## Demo Code

--- */

#include "_avr_includes.h"
#include "_srxe_includes.h"


#define MENU_BALL10_WIDTH 12
#define MENU_BALL10_HEIGHT 10

const uint8_t menu_ball10[] PROGMEM ={
	0x0c, 0x00, // image width=12
	0x0a, 0x00, // image height=10
	0X01,0x00,0X01,0x52,0X01,0x40,0X01,0x00,0X01,0x0b,0X01,0xff,0X01,0xfd,0X01,0x00,
	0X01,0x1f,0X02,0xff,0X01,0x00,0X01,0x5f,0X02,0xff,0X01,0x40,0X01,0x9f,0X02,0xff,
	0X01,0x80,0X01,0x9f,0X02,0xff,0X01,0x80,0X01,0x5f,0X02,0xff,0X01,0x40,0X01,0x1f,
	0X02,0xff,0X01,0x00,0X01,0x0b,0X01,0xff,0X01,0xfd,0X02,0x00,0X01,0x52,0X01,0x40,
	0X01,0x00,0X00,0x00
};
// Total bytes from original bitmap: 120
// Total bytes to RLE datafrom original bitmap: 32




#define PERIODIC_INTERVAL 1000
#define KEYSCAN_RATE	  10 // milliseconds between keyboard scans


uint8_t* rf_receive_test() {
	static uint8_t rx_buffer[HW_FRAME_TX_SIZE + 1];

	memset(rx_buffer, 0, HW_FRAME_TX_SIZE+1);
	if (rfAvailable() > 0) {
		if (rfGetBuffer((uint8_t*) rx_buffer, HW_FRAME_TX_SIZE) > 0)
			return rx_buffer;
	}
	return NULL;
}

// button text must have 10 lines but a NULL means to leave that slot blank; they are order top to bottom, left then right
const char *_test_menus[] = { "LB-a",	"LB-b",	"LB-c",	"LB-d",	"LB-e",	"RB-f",	"RB-g",	"RB-h",	"RB-i",	"RB-j"	};

uint8_t _test_col1, _test_col2;

// Draw display background
void _initial_display_content(void) {
	uint8_t top;

	lcdClearScreen();

	lcdFontSet(FONT3);
	lcdColorSet(LCD_BLACK, LCD_WHITE);

	#define KB_BOX_WIDTH  (lcdFontWidthGet() * 10) // medium font width * 10 characters
	#define KB_BOX_HEIGHT (lcdFontHeightGet() * 6) // medium font width * 6 ROWS
	#define KB_BOX_OFFSET 2
	#define KB_TOP (2 + TRIPLET_TO_ACTUAL(KB_BOX_OFFSET))

	top = KB_TOP;
	_test_col1 = ((lcdFontWidthGet() * 5)) + 1; // to be clear of the menus we already displayed
	_test_col2 = _test_col1 + KB_BOX_WIDTH + 2;

	// display the keyboard characters
	lcdPutStringAt("1234567890", _test_col1, top);
	top += lcdFontHeightGet();
	lcdPutStringAt("QWERTYUIOP", _test_col1, top);
	top += lcdFontHeightGet();
	lcdPutStringAt("ASDFGHJKL*", _test_col1, top);
	top += lcdFontHeightGet();
	lcdPutStringAt("*ZXCVBNv ^", _test_col1, top);
	top += lcdFontHeightGet();
	lcdPutStringAt("*fre_,.M<>", _test_col1, top);
	top += lcdFontHeightGet();
	lcdPutStringAt("abcdefghij", _test_col1, top);
	top += lcdFontHeightGet();
	//top += KB_BOX_OFFSET;


	// Next, you can draw some text or rectangles. The color can be 0-3 (0=off, 3=fully on)

	top = KB_TOP;
	lcdFontSet(FONT2);
	lcdPutStringAt("Ver: ", _test_col2, top); // (int x, int y, char *szMsg, int iSize, int iFGColor, int iBGColor)
	// horizontal position has already been updated by text operation
	printDevicePrintf(PRINT_LCD, "%d.%02d%02d", ECC_VERSION_YEAR, ECC_VERSION_MAJOR, ECC_VERSION_MINOR);

	top += lcdFontHeightGet() + 2;

	// Draw some rectangles with various fill and color
	#define TEST_BOX_WIDTH 10
	#define TEST_BOX_HEIGHT 20
	lcdColorSet(LCD_BLACK, LCD_WHITE);
	lcdRectangle(_test_col2 + (TEST_BOX_WIDTH * 0), top, TEST_BOX_WIDTH, TEST_BOX_HEIGHT, LCD_FILLED);
	lcdColorSet(LCD_BLACK, LCD_LIGHT);
	lcdRectangle(_test_col2 + (TEST_BOX_WIDTH * 1), top, TEST_BOX_WIDTH, TEST_BOX_HEIGHT, LCD_FILLED);
	lcdColorSet(LCD_BLACK, LCD_DARK);
	lcdRectangle(_test_col2 + (TEST_BOX_WIDTH * 2), top, TEST_BOX_WIDTH, TEST_BOX_HEIGHT, LCD_FILLED);
	lcdColorSet(LCD_BLACK, LCD_BLACK);
	lcdRectangle(_test_col2 + (TEST_BOX_WIDTH * 3), top, TEST_BOX_WIDTH, TEST_BOX_HEIGHT, LCD_FILLED);

	top += TEST_BOX_HEIGHT + 2;

	lcdColorSet(LCD_BLACK, LCD_WHITE);

	#define TEST_STRING "AaBeGqKw@o0_#-1234567890"
	lcdFontSet(FONT1);
	lcdPutStringAt(TEST_STRING, _test_col2, top);
	top += lcdFontHeightGet() + 1;

	lcdFontSet(FONT2);
	lcdPutStringAt(TEST_STRING, _test_col2, top);
	top += lcdFontHeightGet() + 1;

	lcdFontSet(FONT3);
	lcdPutStringAt(TEST_STRING, _test_col2, top);
	top += lcdFontHeightGet() + 1;

	lcdFontSet(FONT4);
	lcdPutStringAt(TEST_STRING, _test_col2, top);
	top += lcdFontHeightGet() + 1;

	// draw footer status area
	#define BAT_POSITION (_test_col1 + 3)
	#define NUM_POSITION (BAT_POSITION + 27)
	#define RFX_POSITION (NUM_POSITION + 27)

	lcdFontSet(FONT2);
	uiMenu(_test_menus, NULL, UI_MENU_ROUND_END, true);

	lcdFontSet(FONT1);
	// the ball is 9 pixels tall and the status bar is 10 pixels tall so there is no perfect rendering
	top = LCD_HEIGHT - lcdFontHeightGet() - 2;

	// we draw the bitmap first because it may include pixels of various shades; the subsequent rectangle will cover those up
	lcdBitmap(_test_col1, top, menu_ball10, false);
	lcdBitmap(LCD_WIDTH - _test_col1 - TRIPLET_FROM_ACTUAL(MENU_BALL10_WIDTH), top, menu_ball10, false);

	lcdColorSet(LCD_BLACK, LCD_BLACK);
	lcdRectangle(_test_col1 + TRIPLET_FROM_ACTUAL((MENU_BALL10_WIDTH/2)), top, LCD_WIDTH - (_test_col1 * 2) - TRIPLET_FROM_ACTUAL((MENU_BALL10_WIDTH)), lcdFontHeightGet() + 2, LCD_FILLED);

	top += 1;
	lcdColorSet(LCD_WHITE, LCD_BLACK);
	lcdPutStringAt("Bat:", BAT_POSITION, top);
	lcdPutStringAt("Num:", NUM_POSITION, top);
	lcdPutStringAt("RFx:", RFX_POSITION, top);
	lcdColorSet(LCD_BLACK, LCD_WHITE);
}



/* ---
#### Arduino-like code

The SRXEcore and the sample smoketest are developed and maintained using VSCode+PlatformIO.
For users of the Arduino IDE, the following code is broken out to mimic the structure of
Arduino projects.

`smoketest_setup()` contains the contents found in Arduino `setup()` and
`smoketest_loop()` contains the contents found in Arduino `loop()`.
There are also a number of global variables to persist data between `setup` and `loop`
as well between calls to `loop`.

--- */

// we need a number of variables to persist between the setup() and the loop() and between successive calls to the loop()
static unsigned long _update_timer;
static unsigned long _keyscan_timer;
static uint8_t _test_key;				// we need to know the most recent key press outside of the loop()
static int8_t _rx_tx_mode = 2; 			// start in TX mode so our default is testing the whole system
static uint8_t _rf_channel = 0;
static uint16_t _test_counter = 0;


/*
int smoketest_setup()

This is very similar to the Arduino model of setup() and loop().
Perform the setup of all SRXE Core library services.

*/

void smoketest_setup() {
/* ---
#### Initialization of the Functional Areas

Before using any of the SRXEcore functions, the _init_ procedure for each subsystem must be called.
```C
*/
	ledsInit(); // uses the JTAG pads directly or via the Enigma Development Adapter
	uartInit(); // uses the JTAG pads directly or via the Enigma Development Adapter

	clockInit();
	powerInit();
	rfInit(1);
	randomInit(); // (must be after RF)
	kbdInit();
	lcdInit();
/*
```
--- */

	for (uint8_t i = 0; i < LEDS_COUNT; i++) {
		ledOn(i);
		clockDelay(500);
	}

	_test_key = 0;
	_update_timer = clockMillis();
	_keyscan_timer = _update_timer;

/* ---
#### Display the Screen Contents

Once the various systems have been initialized, we may start to display content on the LCD.

In this demo, we render a keyboard map.
This is later used to visually verify each key is functioning.

We also render some status information including
the build date of the firmware,
battery status,
RF transceiver status,
current TX and RX messages,
and a 1-second counter.

To demonstrate the graphics, we render text in severals sizes and a box for each color value.

We also use the composite UI function to render a menu with `UI_MENU_ROUND_END` styling.

The status bar combines a filled rectangle and bitmap circles for a rounded-rectangle result.

--- */

	_initial_display_content(); // re-paint display
}


/*
#### int smoketest_loop()

This is very similar to the Arduino model of setup() and loop().
Perform the loop for the smoke test.

*/

void smoketest_loop() {
/* ---
For every iteration of the loop, we check to see if there has been a long-press of the power button.
In the demo, this event is used to indicate the device should be put to sleep. It will wake when the button is pressed again.

The code performs a series of pre-sleep tasks to conserve power,
including shutting down the RF transceiver,
instructing the LCD to power down, and turning off any LEDs.
Then it initiates the system level sleep.
When it wakes, it will reverse the steps as part of its post-sleep tasks.

--- */

	if (powerButtonPressed()) {
		uartPutStringNL("Powering down");
		if (_rx_tx_mode) rfTerm(); // turn off RF transceiver
		lcdSleep();				  // turn off LCD
		ledsOff();

		powerSleep(); // actual powerdown sleep mode

		lcdWake();				   // restart LCD
		if (_rx_tx_mode) rfInit(1); // re-initialize RF Transceiver

		_update_timer = clockMillis();
		_keyscan_timer = _update_timer;
		uartPutStringNL("Waking up");

		_initial_display_content(); // re-paint display
	}

/* ---
There are two counters with use the library clock to perform actions at given intervals.

The first counter performs updates to the status information once each 1 second (1000 milliseconds).

This set of actions include updating the battery, RF transceiver state, and counter, in the status bar.
It also will transmit the current counter depending on the state of the RF transceiver.

During this interval, the demo will also optionally transmit its counter value with the RF transceiver.

The demo lets the user set the RF Transceiver to one of four modes:
**OFF**, **RX only**, **RX with TX**, and **Echo**
(where the code will retransmit whatever it has received).

_When a new device powers up, the initial state is **RX with TX**.
With another devcie in **Echo**, the display of the RF messages on the new device will have
the RX message the same as the TX message. This is a quick and easy way to test the RF hardware on a device._

--- */

	if (clockMillis() >= _update_timer) {
		_update_timer = clockMillis() + PERIODIC_INTERVAL;
		_test_counter++;

		// Update bottom status bar
		lcdFontSet(FONT1);
		lcdColorSet(LCD_WHITE, LCD_BLACK);

		// Battery state
		uint16_t voltage = powerBatteryLevel();

		lcdPositionSet(BAT_POSITION + 10, LCD_HEIGHT - lcdFontHeightGet() - 1);
		printDevicePrintf(PRINT_LCD, "%d.%02dV", voltage / 1000, (voltage % 1000) / 10);

		lcdPositionSet(NUM_POSITION + 10, LCD_HEIGHT - lcdFontHeightGet() - 1);
		printDevicePrintf(PRINT_LCD, "%5u", _test_counter);

		lcdPositionSet(RFX_POSITION + 10, LCD_HEIGHT - lcdFontHeightGet() - 1);
		switch (_rx_tx_mode) {
			case 0: {	lcdPutString("OFF");	} break;
			case 1: {	lcdPutString("RX ");	} break;
			case 2: {	lcdPutString("TX ");	} break;
			case 3: {	lcdPutString("EC ");	} break;
		}

		// RF diagnostics and any Rx/TX data; displayed above status bar

		lcdColorSet(LCD_BLACK, LCD_WHITE);
		lcdFontSet(FONT1);

		uint16_t val = _test_counter;
		// Tx, Rx, Tx, Loopback, or Echo
		if (_rx_tx_mode == 3 /*EC*/) {
			val = 0xFFFF;
			if (rfAvailable() > 0) {
				ledOn(2);
				uint8_t* p = rf_receive_test();
				lcdPutStringAt("RE: ", BAT_POSITION, LCD_HEIGHT - ((lcdFontHeightGet() + 1) * 2));
				lcdPutString((char*)p); // horizontal position is already updated
				val = atoi((char*)p);
				ledOff(2);
			}
		}
		if ((_rx_tx_mode == 2 /*TX*/) || (_rx_tx_mode == 3 /*EC*/)) {
			if (val != 0xFFFF) {
				// detect if we do not actually have the RF transceiver
				if (rfInited())	ledOn(1);
				else			ledOn(0);
				printDevicePrintf(PRINT_RF, "%05u", val);

				// display what we transmitted
				lcdPositionSet(BAT_POSITION + (lcdFontWidthGet() * 12), LCD_HEIGHT - ((lcdFontHeightGet() + 1) * 2));
				if (_rx_tx_mode == 3 /*EC*/)
					lcdPutString("TE: ");
				else
					lcdPutString("TX: ");
				// LCD position is already updated
				printDevicePrintf(PRINT_LCD, "%05u", val);
				if (rfInited())	ledOff(1);
				else			ledOff(0);
			}
		}
	}

	if (_rx_tx_mode != 3 /*EC*/) {
		if (rfAvailable() > 0) {
			ledOn(2);
			uint8_t *p = rf_receive_test();
			if (p[0]) {
				lcdFontSet(FONT1);
				lcdPutStringAt("RX:", BAT_POSITION, LCD_HEIGHT - ((lcdFontHeightGet() + 1) * 2));
				lcdPutString((char*)p); // horizontal position is already updated
			}
			ledOff(2);
		}
	}

/* ---
The second counter performs keyboard scanning and keyboard related actions once each 10 milliseconds.

If a key has been pressed, the first task is to update the keyboard map we displayed at the start.
For each key press, we blank out the corresponding location on the map.
Pressing every key on the keyboard will blank every location on the map.
If an location is not cleared, then it indicates that key has malfunctioned and
there may be internal damage to the SRXE circuit board.

The **UP** and **DOWN** of the four-way button on the keyboard is used to control the LCD contrast.

The **LEFT** and **RIGHT** of the four-way button on the keyboard is used to change the RF transceiver mode.

--- */

	// Every KEYSCAN_RATE
	// Scan keyboard
	// and test for some "special" keypress
	if (clockMillis() >= _keyscan_timer) {
		_keyscan_timer = clockMillis() + KEYSCAN_RATE;

		// clear the previous text and draw new key
		uint16_t key_details = kbdGetKeyDetails();
		uint8_t key = (key_details) & 0xFF;
		uint8_t details = (key_details >> 8) & 0xFF;

		uint8_t col = ((details >> 4) & 0xF) - 1;
		uint8_t row = ((details) & 0xF) - 1;

		lcdFontSet(FONT3);
		if (details != 0) {
			lcdPutStringAt(" ", _test_col1 + (col * lcdFontWidthGet()), KB_TOP + (row * lcdFontHeightGet()));
		}

		if (key) {
			_test_key = key;	// remember the key for use outside the loop()

			// action linked to some keypress

			// adjust LCD contrast
			if (key == KEY_UP)	// arrow pad up
				lcdContrastIncrease();
			if (key == KEY_DOWN)	// arrow pad down
				lcdContrastDecrease();

			lcdFontSet(FONT1);

			// detect RF mode changes
			int8_t old_mode = _rx_tx_mode;
			if (key == KEY_LEFT)
				_rx_tx_mode--;
			if (key == KEY_RIGHT)
				_rx_tx_mode++;

			if (old_mode != _rx_tx_mode) {
				// clear the TX and RX screen area
				lcdPutStringAt("            ", BAT_POSITION,                            LCD_HEIGHT - ((lcdFontHeightGet() + 1) * 2));
				lcdPutStringAt("            ", BAT_POSITION + (lcdFontWidthGet() * 12), LCD_HEIGHT - ((lcdFontHeightGet() + 1) * 2));

				if (_rx_tx_mode < 0) _rx_tx_mode = 3;
				if (_rx_tx_mode > 3) _rx_tx_mode = 0;

				if ((old_mode == 0) && (_rx_tx_mode != 0))
					rfInit(1);
				if ((old_mode != 0) && (_rx_tx_mode == 0))
					rfTerm();
			}
		}
	}
}


/*
#### int smoketest()

Perform a test of the SRXE device hardware will also demonstrating many of the functions of the SRXEcore library.
This is a wrapper to smoketest_setup() and smoketest_loop() for user when developing with VSCode+PlatformIO

*/

int smoketest(bool forever) {
	bool loop = true;

	// if not forever then the caller must have already performed the initialization
	if (!forever) {
		_rf_channel = rfInited();
		// if the rf is already initialized, terminate it and then restart it on channel 1
		if (_rf_channel) {
			rfTerm();
			_delay_ms(50);
			rfInit(1);
		}
		else
			rfInit(1);
	} else {
		smoketest_setup();
	}

	do {
		smoketest_loop();

		if (!forever) {
			// allow exit from smoketest
			if (_test_key == KEY_MENU10SY) {
				if (rfInited()) {
					rfTerm();
					_delay_ms(50);
				}
				if (_rf_channel) {
					rfInit(_rf_channel);
				}
				loop = false;
			}
		}
	} while (loop);

	return 0;
}

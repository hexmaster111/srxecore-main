/* ************************************************************************************
* File:    lcdbase.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## LCD
**Draw Primatives, Text, and Composite UI**

The SRXE LCD is 384\*136 pixels with four levels of greyscale.
The LCD Driver operates on three pixels at a time, stored in a single byte.
This code refers to a three-pixel byte as a **triplet**.

The physical LCD is 384\*136. However, it is only addressable by triplets.
All of the exposed functions in this library operate on a screen
coordinate system of 128\*136.

There is one issue with this hardware implementation - all vertical lines are _fat_.
The `lcdRectangle()` and `lcdVerticalLine()` code attempts to addressing this error.
_You can judge for yourself if they are successful._

Use `LCD_WIDTH` and `LCD_HEIGHT` for the extents of the screen.

There are definitions for the colors to improve code:
`LCD_BLACK`, `LCD_DARK`, `LCD_LIGHT`, and `LCD_WHITE`.

--------------------------------------------------------------------------
--- */


#ifndef __SRXE_LCDBASE_
#define __SRXE_LCDBASE_

#include "common.h"


// --------------------------------------------------------------------------------------------

#include "fonts.h"

// Display info
#define LCD_WIDTH_ACTUAL	384	// never used, but here for posterity
#define LCD_WIDTH  128	// 0..127 = 128 ==> 128 * 3 = 384
#define LCD_HEIGHT 136	// 0..135 = 136
#define LCD_DRIVER_HEIGHT 160

typedef char (*char_callback)(char);

// depending on the type of block write command, the larges required buffer will vary; for a horizontal line is the LCD_WIDTH and for a vertical line is it LCD_HEIGHT

#define TRIPLET_SIZE		3
#define TRIPLET_OFFSET(n)	(((n) % TRIPLET_SIZE) ? (TRIPLET_SIZE - ((n) % TRIPLET_SIZE)) : 0)					// if not on a triplet boundary, delta to next boundary
#define TRIPLET_CEILING(n)	(((n) % TRIPLET_SIZE) ? ((n) + (TRIPLET_SIZE - ((n) % TRIPLET_SIZE))) : (n))		// if not on a triplet boundary, increase to next boundary
#define TRIPLET_FLOOR(n)	(((n) % TRIPLET_SIZE) ? ((n) - ((n) % TRIPLET_SIZE)) : (n))							// if not on a triplet boundary, decrease to next boundary
#define TRIPLET_ROUND(n)	(((n) % TRIPLET_SIZE) ? ((((n) % TRIPLET_SIZE) == 1) ? ((n)-1) : ((n) + 1)) : (n))	// if not on a triplet boundary, round up/down to nearest boundary

#define TRIPLET_FROM_ACTUAL(n)	((n) / TRIPLET_SIZE)
#define TRIPLET_TO_ACTUAL(n)	((n) * TRIPLET_SIZE)

// we use the FONTOBJECT to allow fonts to be loaded dynamically
// NOTE to internal developers: these values are in real pixels, not triplets

typedef struct _FONTOBJECT {
	const unsigned char *data;
	uint8_t width;
	uint8_t height;
	uint8_t widthbytes;
	uint8_t charbytes;
	uint8_t scale;	// a bit field
} FONTOBJECT;

enum {
	FONT1 = 0,
	FONT2,
	FONT3,
	FONT4
} FONT_NUM;

#define FONTS_MAX 4
FONTOBJECT _srxe_fonts[FONTS_MAX];

// definitions for 2X scaling a font width and/or height
// the code can actually perform any multiple but 2X is pretyt much the only practical use case

#define FONT_DEFAULT_SCALE 0x0
#define FONT_DOUBLE_WIDTH 0x1
#define FONT_DOUBLE_HEIGHT 0x2
#define FONT_DOUBLED (FONT_DOUBLE_WIDTH | FONT_DOUBLE_HEIGHT)

// --------------------------------------------------------------------------------------------

// definitions for fill functions
#define LCD_ERASE	2
#define LCD_FILLED	1
#define LCD_HOLLOW	0

#define LCD_CORRECT_COLOR(c) (((c) == 0b11011011) ? 0b11111111 : (c))	// if a color is "almost" white, correct it; FYI: it's not clear if this is needed

// colors
#define LCD_BLACK	0x3
#define LCD_DARK	0x2
#define LCD_LIGHT	0x1
#define LCD_WHITE	0x0

// 3 pixels are packed into a byte in the for bbxbbxbb where 'x' is not used and should be 0
static uint8_t _lcd_color_to_byte[4] = {0b00000000, 0b00100101, 0b10010010, 0b11111111};

// --------------------------------------------------------------------------------------------

// 10 levels of contrast with 5 as the middle (this biases the contrast to be darker)
#define LCD_CONTRAST_MIN		1
#define LCD_CONTRAST_DEFAULT	10
#define LCD_CONTRAST_MAX		20

static uint8_t _lcd_contrast;

// --------------------------------------------------------------------------------------------

// these private globals are so that text can be rendered with a simple putChar() style function
static int _lcd_position_x, _lcd_position_y;
static uint8_t _lcd_color_bg, _lcd_color_fg;	// colors
uint8_t _srxe_active_font_num;

// --------------------------------------------------------------------------------------------

static uint8_t _lcd_scroll_offset, _lcd_scroll_area;

// --------------------------------------------------------------------------------------------

static uint8_t _lcd_init;

// --------------------------------------------------------------------------------------------

#ifdef SCREEN_GRABBER

// WARNING: does not support scrolling at this time

static uint8_t LCD_STREAM_GRABBER_FLAG = 0;
#ifdef SCREEN_GRABBER_MANUAL
static uint8_t LCD_STREAM_GRABBER_ACTIVE = 0;
#else
static uint8_t LCD_STREAM_GRABBER_ACTIVE = 1;
#endif

void LCD_STREAM_GRABBER_ACTIVATE() {
	ledOn(0);
	uartPutStringNL("Screen Grabber Enabled");
	LCD_STREAM_GRABBER_ACTIVE = true;
}
void LCD_STREAM_GRABBER_DEACTIVATE() {
	ledOff(0);
	uartPutStringNL("Screen Grabber Disabled");
	LCD_STREAM_GRABBER_ACTIVE = false;
}

void LCD_STREAM_GRABBER_START(int x, int y, int cx, int cy) {
	if (LCD_STREAM_GRABBER_ACTIVE) {
		ledOn(1);
		uartPutString("[[");
		uartPutByte(x);
		uartPutByte(y);
		uartPutByte(cx);
		uartPutByte(cy);
		LCD_STREAM_GRABBER_FLAG = true;
	}
}
void LCD_STREAM_GRABBER_SKIP() {
	if (LCD_STREAM_GRABBER_ACTIVE) {
		LCD_STREAM_GRABBER_FLAG = false;
		_delay_ms(5);	// let the UART and screen grabber time to process
	}
}

void LCD_STREAM_GRABBER(uint8_t b) {
	if (LCD_STREAM_GRABBER_FLAG) {
		uartPutByte(b);
		_delay_us(500); // we were overrunning the UART with data; this duration is approx 1/2 of a character
	}
}

void LCD_STREAM_GRABBER_STOP() {
	if (LCD_STREAM_GRABBER_ACTIVE) {
		ledOff(1);
		uartPutString("]]");
		_delay_ms(10);	// let the UART and screen grabber time to process
		LCD_STREAM_GRABBER_FLAG = false;
	}
}

void LCD_STREAM_GRABBER_NEW() {
	if (LCD_STREAM_GRABBER_ACTIVE)
		uartPutString("[]");
}

void LCD_STREAM_GRABBER_GRAB() {
	if (LCD_STREAM_GRABBER_ACTIVE) {
		ledOn(2);
		uartPutString("][");
		_delay_ms(10);	// let the UART and screen grabber time to process
		ledOff(2);
	}
}

#else
#define LCD_STREAM_GRABBER_ACTIVATE()			((void) 0)
#define LCD_STREAM_GRABBER_DEACTIVATE()			((void) 0)
#define LCD_STREAM_GRABBER_START(x, y, cx, cy)	((void) 0)
#define LCD_STREAM_GRABBER_SKIP()				((void) 0)
#define LCD_STREAM_GRABBER(b)					((void) 0)
#define LCD_STREAM_GRABBER_STOP()				((void) 0)
#define LCD_STREAM_GRABBER_NEW()				((void) 0)
#define LCD_STREAM_GRABBER_GRAB()				((void) 0)
#endif

// --------------------------------------------------------------------------------------------

// only used internally for communicating with the driver
typedef enum {
	MODE_DATA = 0,
	MODE_COMMAND
} DC_MODE;

// Sets the D/C pin to data or command mode
static void _lcd_set_mode(int iMode) {
	srxeDigitalWrite(LCD_DC, (iMode == MODE_DATA));
} /* _lcd_set_mode() */

//
// Write a one byte command to the LCD controller
//
static void _lcd_write_command(unsigned char c) {
	srxeDigitalWrite(LCD_CS, LOW);
	_lcd_set_mode(MODE_COMMAND);
	_srxe_spi_transfer(c);
	_lcd_set_mode(MODE_DATA);
	srxeDigitalWrite(LCD_CS, HIGH);
} /* _lcd_write_command() */

// Write a block of data to the LCD
// Length can be anything from 1 to 17404 (whole display)
void _lcd_write_data_block(uint8_t* data, uint8_t len) {
	srxeDigitalWrite(LCD_CS, LOW);
	for (uint8_t i = 0; i < len; i++) {
		_srxe_spi_transfer(data[i]);
		LCD_STREAM_GRABBER(data[i]);
	}
	srxeDigitalWrite(LCD_CS, HIGH);
}


//
// Power on the LCD; these are the ordered initialization commands
//
const unsigned char _lcd_power_up_commands[] PROGMEM = {
	1, 0x01,						 // soft reset
	99, 120,						 // 120ms delay
									 //	2, 0xD7, 0x9F,			// disable Auto Read
									 //	99, 10,					// 10ms delay
	1, 0x11,						 // sleep out
	1, 0x28,						 // display off
	99, 50,							 // 50ms delay
									 //	3, 0xc0, 0xB9, 0x00,	// Vop = 0xB9
	3, 0xc0, 0xf8, 0x00,			 // Vop = 0xF0
	2, 0xc3, 0x04,					 // BIAS = 1/10
	2, 0xc4, 0x05,					 // Booster = x6
	2, 0xd0, 0x1d,					 // Enable analog circuit
									 //	2, 0xb3, 0x00,					 // Set FOSC divider
	2, 0xb5, 0x00,					 // N-Line = 0
									 //	2, 0xb5, 0x8b,					 // N-Line = 0
	1, 0x38,						 // Set grayscale mode (0x39 = monochrome mode)
	2, 0x3a, 0x02,					 // Enable DDRAM interface
	2, 0x36, 0x00,					 // Scan direction setting
	2, 0xB0, 0x9f,					 // Duty setting (0x87?)
									 //	2, 0xB4, 0xA0,			// partial display
	5, 0x30, 0x00, 0x00, 0x00, 0x77, // partial display area
	5, 0xf0, 0x12, 0x12, 0x12, 0x12, // 77Hz frame rate in all temperatures
	1, 0x20,						 // Display inversion off
	1, 0x29,						 // Display ON
	0};
const unsigned char _lcd_power_down_commands[] PROGMEM = {
	1, 0x28, // display off
	1, 0x10, // sleep in
	99, 120, // 120ms delay
	99, 80,	 // another 80 to get to 200 ?
	0};

void _lcd_run_commands(const unsigned char *pList) {
	uint8_t cmd_buffer[4];
	uint8_t val, count, len = 1;

	while (len != 0) {
		len = pgm_read_byte(pList++);

		if (len != 0) {
			val = pgm_read_byte(pList++);

			if (len == 99) // 99 means perform a delay
				clockDelay(val);
			else {
				// send command with optional data
				_lcd_write_command(val);
				count = len - 1;
				if (count != 0) {
					memcpy_P(cmd_buffer, pList, count);
					pList += count;
					_lcd_write_data_block(cmd_buffer, count);
				}
			}
		}
	}
}

//
// Send commands to position the "cursor" to the given
// row and column
// Opens a window in the display RAM
// the windows starts @ x, y and it is cx wide and cy tall
// all the subsequent write cycles will be done in that window
//
// set th area of the LDC driver buffer which will be affected by subsequent data write operations
void _lcd_set_active_area(int x, int y, int cx, int cy) {
	if (!_lcd_init) return;
	uint8_t cmd_buffer[4];

	if (x > (LCD_WIDTH - 1) || y > (LCD_DRIVER_HEIGHT - 1) || cx > LCD_WIDTH || cy > LCD_DRIVER_HEIGHT) {
		return; // invalid
	}

	_lcd_write_command(0x2a);	  	// set column address
	cmd_buffer[0] = 0;				  	// start column high uint8_t
	cmd_buffer[1] = x;			  		// start column low uint8_t
	cmd_buffer[2] = 0;				  	// end col high uint8_t
	cmd_buffer[3] = (x + cx - 1); 		// end col low uint8_t
	_lcd_write_data_block(cmd_buffer, 4);
	_lcd_write_command(0x2b); 		// set row address
	cmd_buffer[0] = 0;			  		// start row high uint8_t
	cmd_buffer[1] = y;			  		// start row low uint8_t
	cmd_buffer[2] = 0;			  		// end row high uint8_t
	cmd_buffer[3] = y + cy - 1;	  		// end row low uint8_t
	_lcd_write_data_block(cmd_buffer, 4);
	_lcd_write_command(0x2c); 	// write RAM

	LCD_STREAM_GRABBER_START(x, y, cx, cy);
} /* _lcd_set_active_area() */

void _lcd_end_active_area() {
	LCD_STREAM_GRABBER_STOP();
}

// --------------------------------------------------------------------------------------------
// Public Functions
// --------------------------------------------------------------------------------------------
void lcdFontConfig(uint8_t id, const unsigned char *data, uint8_t width, uint8_t height, uint8_t width_bytes, uint8_t char_bytes, uint8_t scale);
void lcdFontClone(uint8_t target_id, uint8_t source_id, uint8_t scale);

//
// Initializes the LCD controller
// Parameters: GPIO pin numbers used for the CS/DC/RST control lines
//

// not used; not tested
void lcdPixelsAllOn() {
	if (!_lcd_init) return;
	_lcd_write_command(0b00100011); // AP=1 all pixes on
}

// not used; not tested
void lcdPixelsAllOff() {
	if (!_lcd_init) return;
	_lcd_write_command(0b00100010); // AP=0 all pixes onff
}

// not used; not tested
void lcdInvertOn() {
	if (!_lcd_init) return;
	_lcd_write_command(0b00100001); // INV=1 invert
}

// not used; not tested
void lcdInvertOff() {
	if (!_lcd_init) return;
	_lcd_write_command(0b00100000); // INV=0 normal
}



/* ---
#### void lcdFill(uint8_t color)

Fill the entire screen with the specified color.

--- */
void lcdFill(uint8_t ucData) {
	if (!_lcd_init) return;

	int y;
	uint8_t temp[128];

	LCD_STREAM_GRABBER_NEW();

	_lcd_set_active_area(0, 0, LCD_WIDTH, LCD_HEIGHT);
	LCD_STREAM_GRABBER_SKIP();	// we do not need to attempt and send all the blanking data

	for (y = 0; y < LCD_HEIGHT; y++) {
		memset(temp, ucData, LCD_WIDTH);		  // have to do this because the uint8_ts get overwritten
		_lcd_write_data_block(temp, LCD_WIDTH); // fill with data uint8_t
	}

	_lcd_end_active_area();
}

/* ---
#### void lcdClearScreen()

Fill the entire screen with the current background color.
--- */

#define lcdClearScreen() lcdFill(_lcd_color_to_byte[_lcd_color_bg])


/* ---
#### void lcdColorSet(uint8_t fore_color, uint8_t back_color)

Set the foreground and background colors used for subsequent operations.

--- */
void lcdColorSet(uint8_t fg, uint8_t bg) {
	if (fg > LCD_BLACK)		fg = LCD_BLACK;
	if (bg > LCD_BLACK)		bg = LCD_BLACK;

#if 0
	// while a user could conceivable want to do this, we are going to assume they made a mistake
	if (fg == bg) {
		fg = LCD_BLACK;
		bg = LCD_WHITE;
	}
#endif
	_lcd_color_fg = fg;
	_lcd_color_bg = bg;
}

/* ---
#### uint8_t lcdColorTripletGetF()

Get the current foreground color as a three-byte triplet.
--- */
uint8_t lcdColorTripletGetF() {
	return _lcd_color_to_byte[_lcd_color_fg];
}

/* ---
#### uint8_t lcdColorTripletGetB()

Get the current background color as a three-byte triplet.
--- */
uint8_t lcdColorTripletGetB() {
	return _lcd_color_to_byte[_lcd_color_bg];
}

/* ---
#### void lcdPositionSet(uint8_t x, uint8_t y)

Set the current position on the screen used for subsequent operations.

_Note: Text draw operations will update the horizontal position._

**Notes:**
Horizontal dimensions are in display triplets, not real pixels.
Vertical dimensions are always in real pixels.
--- */
void lcdPositionSet(int x, int y) {
	// we could set x and y to the upper limits, but setting them to zero makes debugging easer
	if (x > LCD_WIDTH)
		x = 0;
	if (y > LCD_HEIGHT)
		y = 0;

	_lcd_position_x = x;
	_lcd_position_y = y;
}

/* ---
#### uint8_t lcdPositionGetX()

Get the current horizontal position on the screen.

**Note:** horizontal dimensions are in display triplets, not real pixels._
--- */
int lcdPositionGetX() {
	return _lcd_position_x;
}

/* ---
#### uint8_t lcdPositionGetY()

Get the current vertical position on the screen.

**Note:** vertical dimensions are always in real pixels._
--- */
int lcdPositionGetY() {
	return _lcd_position_y;
}



#define _VOP_CENTER 250
#define _VOP_RANGE 100
#define _VOP_MIN (_VOP_CENTER - (_VOP_RANGE / 2))

/* ---
#### uint8_t lcdContrastGet()

Get the current LCD contrast level (a value between 1 and 10)

--- */
uint8_t lcdContrastGet() {
	return _lcd_contrast;
}


/* ---
#### void lcdContrastSetRaw()

Set the LCD contrast to a specified level (0 .. 256)

This is a special case function and probably is not the one you want.
--- */

void lcdContrastSetRaw(uint8_t val) {
	if (!_lcd_init) return;

	// input values are 0..256
	// the system values are 0x00:0x60 (96) thru 0x01:0x68 (360)
	// this is a range of 264
	// however, at 210 it is completely faded out and at at 310 it is completely filled in
	// so the entire +/- 128 really isn't visible but it's an easy number


	uint8_t cmd_buffer[2];
	cmd_buffer[1] = 0x00;

	uint16_t sys_val = (_VOP_CENTER - 128) + val;

	if (sys_val > 255) {
		cmd_buffer[1] = 0x01;
	}
	cmd_buffer[0] = (sys_val & 0xff);

	_lcd_write_command(0b11000000); // C0 command = 0xset Vop value
	_lcd_write_data_block(cmd_buffer, 2);
}


/* ---
#### void lcdContrastSet()

Set the LCD contrast to a specified level ( 1 .. 10)

There are definitions for the contrast to improve code:
`LCD_CONTRAST_MIN`, `LCD_CONTRAST_DEFAULT`, and `LCD_CONTRAST_MAX`.
--- */
int lcdPutChar(char c);	// temporary forward declaration

void lcdContrastSet(uint8_t val) {
	if (!_lcd_init) return;

	if (val < LCD_CONTRAST_MIN) val = LCD_CONTRAST_MIN;
	if (val > LCD_CONTRAST_MAX) val = LCD_CONTRAST_MAX;

	// input values are 1..10
	// the system values are 0x00:0x60 (96) thru 0x01:0x68 (360)
	// this is a range of 264
	// however, at 210 it is completely faded out and at at 310 it is completely filled in
	// assume 250 is the middle and +/- 50 is usable

	uint8_t cmd_buffer[2];
	cmd_buffer[1] = 0x00;

	uint16_t sys_val = _VOP_MIN + ((_VOP_RANGE / LCD_CONTRAST_MAX) * val);

	if (sys_val > 255) {
		cmd_buffer[1] = 0x01;
	}
	cmd_buffer[0] = (sys_val & 0xff);


	_lcd_write_command(0b11000000); // set Vop value
	_lcd_write_data_block(cmd_buffer, 2);
	//_delay_ms(20);	// the datasheet doesn't say anytying about this but with this delay (or some slow code like UART)
	//printDevicePrintf(PRINT_UART, "lcdContrast %2x:%2x (%d:%d) ==> %d\n", cmd_buffer[1], cmd_buffer[0], _lcd_contrast, val, sys_val);
	_lcd_contrast = val;
}

/* ---
#### void lcdContrastReset()

Reset the LCD to the `LCD_CONTRAST_DEFAULT` level.
--- */
void lcdContrastReset() {
	lcdContrastSet(LCD_CONTRAST_DEFAULT);
}

/* ---
#### void lcdContrastDecrease()

Decrease the LCD contrast one level.
--- */
void lcdContrastDecrease() {
	if (!_lcd_init) return;
	if (_lcd_contrast > LCD_CONTRAST_MIN)
		lcdContrastSet(_lcd_contrast-1);
}

/* ---
#### void lcdContrastIncrease()

Increase the LCD contrast one level.
--- */
void lcdContrastIncrease() {
	if (!_lcd_init) return;
	if (_lcd_contrast < LCD_CONTRAST_MAX)
		lcdContrastSet(_lcd_contrast+1);
}


/* ---
#### void lcdWake()

Perform wake-up operations after LCD has been put to sleep.
This is used as part of a re-initialization sequence after the SRXE has returned from its power-off sleep state.
--- */
void lcdWake(void) {
	if (!_lcd_init) return;
	_lcd_run_commands(_lcd_power_up_commands);
} /* SRXEPowerUp() */

/* ---
#### void lcdSleep()

Perform shutdown operations of the LCD.
This is used ahead of the SRXE entering power-off sleep state.
--- */
void lcdSleep() {
	if (!_lcd_init) return;

	lcdClearScreen(); // fill memory with zeros to go to lowest power mode
	_lcd_run_commands(_lcd_power_down_commands);
} /* SRXEPowerDown() */


/* ---
#### bool lcdInit()

Must be called to initialize the LCD prior to using any other LCD functions.
--- */

int lcdInit() {

	_srxe_spi_init();

	srxePinMode(LCD_CS, OUTPUT);
	//srxePinMode(FLASH_CS, OUTPUT);
	srxeDigitalWrite(LCD_CS, HIGH);

	// srxeDigitalWrite(LCD_CS, LOW); // leave CS low forever so it's always ready for use

	srxePinMode(LCD_DC, OUTPUT);
	srxePinMode(LCD_RESET, OUTPUT);

	// Start by reseting the LCD controller
	srxeDigitalWrite(LCD_RESET, HIGH);
	_delay_ms(50);
	srxeDigitalWrite(LCD_RESET, LOW);
	_delay_ms(5);
	srxeDigitalWrite(LCD_RESET, HIGH); // take it out of reset
	_delay_ms(150);						  // datasheet says it must be at least 120ms

	_lcd_scroll_area = LCD_HEIGHT;

	_lcd_init = true;

	lcdWake(); // turn on and initialize the display

	lcdContrastReset();
	lcdClearScreen();

	// initialize fonts
	memset(_srxe_fonts, 0, sizeof(FONTOBJECT) * FONTS_MAX);
	#ifndef CUSTOM_FONTS
	lcdFontConfig(FONT1, font_6X8_P,  FONT_6X8_WIDTH,  FONT_6X8_HEIGHT,  FONT_6X8_WIDTHBYTES,  FONT_6X8_CHARBYTES,  FONT_DEFAULT_SCALE);
	lcdFontConfig(FONT2, font_8X14_P, FONT_8X14_WIDTH, FONT_8X14_HEIGHT, FONT_8X14_WIDTHBYTES, FONT_8X14_CHARBYTES, FONT_DEFAULT_SCALE);
	lcdFontClone(FONT3, FONT1, FONT_DOUBLED);
	lcdFontClone(FONT4, FONT2, FONT_DOUBLED);
	#endif

	_lcd_color_fg = LCD_BLACK;
	_lcd_color_bg = LCD_WHITE;
	_lcd_position_x = 0;
	_lcd_position_y = 0;

	return true;
}

#endif // __SRXE_LCDBASE_

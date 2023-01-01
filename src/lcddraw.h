/* ************************************************************************************
* File:	lcddraw.h
* Date:	2020.05.04
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

### LCD Draw
**Draw Graphic Primatives**

--------------------------------------------------------------------------
--- */

#ifndef __SRXE_LCDDRAW_
#define __SRXE_LCDDRAW_

#include "common.h"
#include "lcdbase.h"

/* ---
#### void lcdHorizontalLine(int x, int y, int length, int thickness)

Draw a horizontal line with a given thickness.

**Note:** Horizontal dimensions are in display triplets, not real pixels.
--- */
void lcdHorizontalLine(int x, int y, int length, int thickness) {
	if (!_lcd_init) return;

	uint8_t bTemp[128];
	memset(bTemp, lcdColorTripletGetF(), length);

	_lcd_set_active_area(x, y, length, thickness);
	for (int i = 0; i < thickness; i++) {
		_lcd_write_data_block(bTemp, length);
	}
	_lcd_end_active_area();
}

/* ---
#### void lcdVerticalLine(int x, int y, int length, int thickness)

Draw a vertical line. Thickness from 1 to 3 is supported.

**Note:** Vertical dimensions are always in real pixels.
--- */

void lcdVerticalLine(int x, int y, int height, int thickness) {
	if (!_lcd_init) return;
	uint8_t bTemp[136];

	// implement thickness = try repeating entire instruction set first, then see if it can be optimized like with horizontal

	uint8_t fg = lcdColorTripletGetF();
	uint8_t bg = lcdColorTripletGetB();

	if (thickness > 3) thickness = 3;

	uint8_t color = (fg) | (bg); // use all pixels of triplet

	if (thickness == 1)	color = (fg & 0b00011100) | (bg & 0b11100011); // use middle pixel of triplet
	if (thickness == 2)	color = (fg & 0b11111100) | (bg & 0b00000011); // use left and middle pixel of triplet
	memset(bTemp, color, height);

	_lcd_set_active_area(x, y, 1, height);	// 1 = one triplet
	_lcd_write_data_block(bTemp, height);
	_lcd_end_active_area();
}

/* ---
#### void lcdRectangle(int x, int y, int width, int height, bool filled)

Draw a rectangle - hollow, filled, or erase area.

**Notes:**
A _filled_ rectangle is filled with the background color.
Horizontal dimensions are in display triplets, not real pixels.
Vertical dimensions are always in real pixels.
--- */
void lcdRectangle(int x, int y, int cx, int cy, uint8_t mode) {
	if (!_lcd_init) return;

	uint8_t bTemp[136];	// a full length vertical line is the maximum data block we will write

#if 0
	if (x < 0 || x > 127 || y < 0 || y > 135) return;
	if (x + cx > 127 || y + cy > 135) return;
#else
	// clipping
	if (x < 0) x = 0;
	if (x > (LCD_WIDTH - 1))		x = LCD_WIDTH - 1;
	if (y < 0)						y = 0;
	if (y > (LCD_HEIGHT - 1))		y = LCD_HEIGHT - 1;
	if (x + cx > (LCD_WIDTH - 1))	cx = (LCD_WIDTH) - x;	// should this have the '-1'
	if (y + cy > (LCD_HEIGHT))		cy = (LCD_HEIGHT) - y;	// should this have the '-1'
#endif

	uint8_t fg, bg;
	fg = lcdColorTripletGetF();
	bg = lcdColorTripletGetB();

	if ((mode == LCD_FILLED) || (mode == LCD_ERASE)) {
		_lcd_set_active_area(x, y, cx, cy);
		for (uint8_t i = 0; i < cy; i++) {
			memset(bTemp, bg, cx);
			_lcd_write_data_block(bTemp, cx);
		}
		_lcd_end_active_area();
	}

	uint8_t color; // color bit for a triplet
	if (mode != LCD_ERASE) {
		// Left
		_lcd_set_active_area(x, y, 1, cy);
		color = (fg & 0b11100000) | (bg & 0b00011111);	// left pixel
		memset(bTemp, color, cy);
		_lcd_write_data_block(bTemp, cy);
		_lcd_end_active_area();

		// Right
		_lcd_set_active_area((x + cx) - 1, y, 1, cy);
		color = (fg & 0b00000011) | (bg & 0b11111100);	// right pixel
		memset(bTemp, color, cy);
		_lcd_write_data_block(bTemp, cy);
		_lcd_end_active_area();

		// Top
		_lcd_set_active_area(x, y, cx, 1);
		memset(bTemp, fg, cx);
		_lcd_write_data_block(bTemp, cx);
		_lcd_end_active_area();

		// Bottom
		_lcd_set_active_area(x, y + cy - 1, cx, 1);
		memset(bTemp, fg, cx);
		_lcd_write_data_block(bTemp, cx);
		_lcd_end_active_area();
	}

	_lcd_end_active_area();
}


/* ---
#### void lcdBitmap(int x, int y, const uint8_t *bitmap, bool invert)

Draw a bitmap. The color map may be inverted.

The bitmap must be run-length-encoded using the `bitmap_gen.py` tool.
The tool provides instructions on how to create a source graphic which will converted correctly.

**Notes:**
Horizontal dimensions are in display triplets, not real pixels.
Vertical dimensions are always in real pixels.
--- */
void lcdBitmap(int x, int y, const uint8_t *btmp, bool invert) {
	if (!_lcd_init) return;

	int width, height, index = 0;
	unsigned char length, value;

	width = pgm_read_byte_near(btmp + index++) + (pgm_read_byte_near(btmp + index++) << 8);
	height = pgm_read_byte_near(btmp + index++) + (pgm_read_byte_near(btmp + index++) << 8);

	_lcd_set_active_area(x, y, TRIPLET_FROM_ACTUAL(width), height);

	srxeDigitalWrite(LCD_CS, LOW);
	while ((length = pgm_read_byte_near(btmp + index++))) {
		value = pgm_read_byte_near(btmp + index++);
		if (invert)
			value = ~value;
		for (unsigned char count = 0; count < length; count++) {
			_srxe_spi_transfer(value);
			LCD_STREAM_GRABBER(value);
		}
	}
	srxeDigitalWrite(LCD_CS, HIGH);

	_lcd_end_active_area();
}

/* ---
#### void lcdScrollSet(...)

Initialize an area for vertical scrolling. The scrolling operation is performed with `lcdScrollLines()`.

The parameters are:
 - int TA - number of pixel lines for the top fixed area
 - int SA - number of pixel lines for the scroll area
 - int BA - number of pixel lines for the bottom fixed area

The sum of TA + SA + BA must equal 160. 160 is the LCD driver height (not the LCD height).

**Warning:** This code has not been tested.
--- */
void lcdScrollSet(int TA, int SA, int BA) {
	if (!_lcd_init) return;
	uint8_t cmd_buffer[3];

	if ((TA + SA + BA) != 160)
		return;

	cmd_buffer[0] = (uint8_t)TA;
	cmd_buffer[1] = (uint8_t)SA;
	cmd_buffer[2] = (uint8_t)BA;
	_lcd_write_command(0x33); // set scroll area
	_lcd_write_data_block(cmd_buffer, 3);

	_lcd_scroll_area = (uint8_t)SA;
}

/* ---
#### void lcdScrollLines(int count)

Scroll the _scroll area_ a given number of pixel lines. The _scroll area_, `SA`,  must already be set using `lcdScrollSet()`.

**Warning:** This code has not been tested.
--- */
void lcdScrollLines(int count) {
	if (!_lcd_init) return;
	uint8_t b;

	//  _lcd_scroll_offset = (_lcd_scroll_offset + iLines) % LCD_HEIGHT;
	_lcd_scroll_offset = (_lcd_scroll_offset + count) % _lcd_scroll_area;
	_lcd_write_command(0x37); // set scroll start line
	b = (uint8_t)_lcd_scroll_offset;
	_lcd_write_data_block(&b, 1);
} /* SRXEScroll() */

/* ---
#### void lcdScrollReset(int count)

Reset the scroll area to the whole screen.

**Warning:** This code has not been tested.
--- */
void lcdScrollReset(void) {
	if (!_lcd_init) return;
	uint8_t b;
	lcdScrollSet(0, 160, 0); // restore default values

	_lcd_scroll_offset = 0;
	_lcd_write_command(0x37); // scroll start address
	b = 0;
	_lcd_write_data_block(&b, 1);
} /* SRXEcdScrollReset() */

#endif // __SRXE_LCDDRAW_

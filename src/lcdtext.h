/* ************************************************************************************
* File:	lcdtext.h
* Date:	2020.05.04
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

### LCD Text
**Draw Text**

The text functions of the SRXEcore use bitmap fonts which have been generated with the `font_gen.py` tool.
The tool is documented at the end of this README.
There are four available slots for fonts.
While an application may use more than four fonts, only four may be active at a time.
The font slots are identified as `FONT1`, `FONT2`, `FONT3`, and `FONT4`.

To specify your own fonts, add `#define CUSTOM_FONTS` before including the library code. If you define your own fonts,
you will need to sue the `lcdFontConfig()` function and optionally use `lcdFontClone()`.
If `CUSTOM_FONTS` is not defined, the SRXEcore will load default fonts.

--------------------------------------------------------------------------
--- */



#ifndef __SRXE_LCDTEXT_
#define __SRXE_LCDTEXT_

#include "common.h"
#include "lcdbase.h"


/* ---
#### void lcdFontConfig(...)

Fonts are managed by an internal `FONTOBJECT`. Up to four fonts (FONTS_MAX) may be active at any time.
This function initializes one of the four slots.

The input parameters are:
- uint8_t font_ID - one of `FONT1`, `FONT2`, `FONT3`, or `FONT4`
- const unsigned char* data - the PROGMEM data from a font `.h` file
- uint8_t width - defined in the font `.h` file
- uint8_t height - defined in the font `.h` file
- uint8_t width_bytes - defined in the font `.h` file
- uint8_t char_bytes - defined in the font `.h` file
- uint8_t scale - `FONT_DEFAULT_SCALE`, `FONT_DOUBLE_WIDTH`, `FONT_DOUBLE_HEIGHT`, or `FONT_DOUBLED`

**Notes:
Font dimension parameters are in real pixels, not display triplets.
Vertical dimensions are always in real pixels.
--- */
void lcdFontConfig(uint8_t id, const unsigned char* data, uint8_t width, uint8_t height, uint8_t width_bytes, uint8_t char_bytes, uint8_t scale) {
	// initialize a font
	if (id >= FONTS_MAX)
		return;

	_srxe_fonts[id].data = data;
	_srxe_fonts[id].width = width;
	_srxe_fonts[id].height = height;
	_srxe_fonts[id].widthbytes = width_bytes;
	_srxe_fonts[id].charbytes = char_bytes;
	_srxe_fonts[id].scale = scale;
}

/* ---
#### void lcdFontClone(...)

One font may use an existing font to save data.
This is referred to as font cloning.
The only difference between the two fonts will be any scaling.

This function initializes one of the four slots using the data of another slot.

In theory, three font slots could all be clones of a singel slot.
In practice, often the `FONT1` and `FONT2` slots are real data
with `FONT3` being a clone of `FONT1` and `FONT4` being a clone of `FONT2`.

The input parameters are:
- uint8_t target_ID - one of `FONT1`, `FONT2`, `FONT3`, or `FONT4` which will share configuration from another font
- uint8_t source_ID - one of `FONT1`, `FONT2`, `FONT3`, or `FONT4` which provides the source configuration
- uint8_t scale - `FONT_DEFAULT_SCALE`, `FONT_DOUBLE_WIDTH`, `FONT_DOUBLE_HEIGHT`, or `FONT_DOUBLED` forthe new font
--- */
void lcdFontClone(uint8_t target_id, uint8_t source_id, uint8_t scale) {
	// initialize a font
	if ((source_id >= FONTS_MAX) || (target_id >= FONTS_MAX) || (source_id == target_id))
		return;

	memcpy((void*)&(_srxe_fonts[target_id]), (void*)&(_srxe_fonts[source_id]), sizeof(FONTOBJECT));
	_srxe_fonts[target_id].scale = scale;
}

/* ---
#### void lcdFontSet(uint8_t font_id)

Set the active font for subsequent text operations.

There are definitions for the font IDs to improve code:
`FONT1`, `FONT2`, `FONT3`, and `FONT4`.
--- */
int8_t lcdFontSet(uint8_t id) {
	if (id >= FONTS_MAX)
		return -1;
	_srxe_active_font_num = id;
	return id;
}

/* ---
#### uint8_t lcdFontGet()

Get the active font ID being used for text operations.

The return value used definitions for the font IDs to improve code:
`FONT1`, `FONT2`, `FONT3`, and `FONT4`.
--- */
int8_t lcdFontGetNum() {
	return _srxe_active_font_num;
}

// get a point to the font object of the active font
FONTOBJECT* _lcd_font_get_pointer() {
	return &(_srxe_fonts[_srxe_active_font_num]);
}

/* ---
#### uint8_t lcdFontWidthGet()

Get the width of a character from the active font.

**Note:** width is in display triplets, not real pixels._
--- */
uint8_t lcdFontWidthGet() {
	uint8_t w = _srxe_fonts[_srxe_active_font_num].width * (_srxe_fonts[_srxe_active_font_num].scale & FONT_DOUBLE_WIDTH ? 2 : 1);
	return TRIPLET_FROM_ACTUAL(TRIPLET_CEILING(w));
}

/* ---
#### uint8_t lcdFontHeightGet()

Get the height of a character from the active font.

**Note:** vertical dimensions are always in real pixels._
--- */
uint8_t lcdFontHeightGet() {
	return _srxe_fonts[_srxe_active_font_num].height * (_srxe_fonts[_srxe_active_font_num].scale & FONT_DOUBLE_HEIGHT ? 2 : 1);
}

/* ---
#### uint8_t lcdTextWidthGet(const char* text)

Get the width of the text using characters from the active font.

**Note:** width is in display triplets, not real pixels._
--- */
uint16_t lcdTextWidthGet(const char *text) {
	uint16_t len = strlen(text);
	len *= lcdFontWidthGet();
	return len;
}



/* ---
#### int lcdPutChar(char c)

Display a character at the current LCD position, using the current font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the character displayed.

Use `lcdPositionSet()`, `lcdFontSet()`, and `lcdColorSet()` as necessary, prior to using the function.

The current position is updated by this function.
--- */
int lcdPutChar(char c) {
	// The initial location, font, and color(s) must already be set before using this function
	// eg: lcdPositionSet(x, y); lcdColorSet(fg, bg); lcdFontSet(id);

	int x = lcdPositionGetX();
	int y = lcdPositionGetY();

	uint8_t fg = lcdColorTripletGetF() & 0x3;
	uint8_t bg = lcdColorTripletGetB() & 0x3;

	if (fg > 3) fg = 3;
	if (bg > 3) bg = 3;

	FONTOBJECT *font = _lcd_font_get_pointer();

	uint8_t font_width = font->width;
	uint8_t font_multiplier_width = ((font->scale & FONT_DOUBLE_WIDTH) ? 2 : 1);
	uint8_t glyph_width = font_width * font_multiplier_width;
	uint8_t font_height = font->height;
	uint8_t font_multiplier_height = ((font->scale & FONT_DOUBLE_HEIGHT) ? 2 : 1);
	uint8_t glyph_height = font_height * font_multiplier_height;
	uint8_t font_widthbytes = font->widthbytes;
	uint8_t font_charbytes = font->charbytes;

	// NOTE: padding will be 0, 1, or 2; if it is 2, then we split the padding before and after the glyph
	uint8_t padding = TRIPLET_OFFSET(glyph_width);	// this is the number of pixels to get to the next triplet boundary

	// if the character will not fit, then we error out
	if ((glyph_width + TRIPLET_TO_ACTUAL(x)) > LCD_WIDTH_ACTUAL)
		return -1;

	// NOTE:    font data is width (bits) first by height (bits)
	//          the pixel bits are stored mirrored (the low bit is the left most pixel)
	// WARNING: font width must be a multiple of 3 ... well, actually we pad the end with background color

	// the code used a LCD bitmap buffer and pointer (bp), as well as a character bitmap buffer and pointer (cp)
	// the font is 1-bit per pixel aka 8 pixels per byte and the LCD screen is 3 pixels per byte (3bits-3bits-2bits) - the code calls this a 'triplet'

	// BUG there is some problem with FONT4

	uint16_t lcd_bitmap_size = TRIPLET_FROM_ACTUAL(glyph_width + padding) * glyph_height;
	uint8_t lcd_bitmap[lcd_bitmap_size], *bp, bp_counter;									// lcd bitmap and pointer
	uint8_t font_bytes[font_charbytes], *cp, cb, cb_multiplier_width, cb_multiplier_height; // font character buffer and pointer and current byte
	uint8_t triplet;
	uint8_t pixel; // this is the index of the current pixel; we used to use the 'k' loop variable but now we might already have a pixels before the loop starts

	bp = lcd_bitmap;
	bp_counter = 0;
	cb_multiplier_width = cb_multiplier_height = 1;

	// the font data character set starts at char(32) so we subtract that value from the byte code
	// get pointer to the character's pixel bytes in PROGMEM
	cp = (unsigned char *)&(font->data[(c - 32) * (font_charbytes)]);
	memcpy_P(font_bytes, cp, font_charbytes); // then copy from PROGMEM to local buffer
	cp = font_bytes;						  // then update the pointer to the local buffer
	cb = cp[0];

	// process each byte of pixels of the character
	for (int j = 0; j < font_charbytes;) {
		triplet = bg;
		pixel = 0;

		if (padding == 2) { // when padding == 2 we put one pixel before and one pixel after
			// shift the output byte left 3 to make room for the next pixel
			triplet <<= 3;
			// clear low 3 bits
			triplet &= 0b11111000;
			triplet |= bg;
			pixel++;
		}

		// process a single row of the font character bitmap
		for (int k = 0; k < (font_width * font_multiplier_width); k++) {

			// after every 3 pixels, we need to start the next triplet
			if (pixel && !(pixel % TRIPLET_SIZE)) {
				// WTF - try cleaning up the triplet
				triplet = LCD_CORRECT_COLOR(triplet);
				*bp = triplet;
				bp++;
				triplet = bg;
				bp_counter++;
				if (bp_counter >= lcd_bitmap_size) {
					return -1;
				}
			}

			// do we need to get the next pixel byte from the input
			// if its not the (very beginning) and (if either (we have process 8 pixel bits) or (we have reach the end of a row of the font))
			if (k && !(k % (8 * font_multiplier_width))) {
				j++;
				cb = cp[j];
			}

			// shift the output byte left 3 to make room for the next pixel
			triplet <<= TRIPLET_SIZE;
			// clear low 3 bits
			triplet &= 0b11111000;

			// if the low pixel bit is 1, then we set the low two bits of the bitmap byte to fg
			triplet |= (cb & 0x1) ? fg : bg;
			pixel++;

			// if wer are multiplying the width, then we repeat the same pixel
			if (cb_multiplier_width < font_multiplier_width) {
				cb_multiplier_width++;	// repeat the same pixel
			} else {
				cb_multiplier_width = 1;
				cb = cb >> 1; 			// discard the used pixel bit
			}
		} // end of for loop of one row of pixels pixels from the character bitmap data

		// pad out the end of the triplet
		if (padding) {
			// shift the output byte left 3 to make room for the next pixel
			triplet <<= 3;
			// clear low 3 bits
			triplet &= 0b11111000;
			triplet |= bg;
		}
		// WTF - try cleaning up the triplet
		triplet = LCD_CORRECT_COLOR(triplet);
		*bp = triplet;
		bp++;
		bp_counter++;
		// throw away any remaining bits and start with a new byte from the source pixels
		j++;

		if (cb_multiplier_height < font_multiplier_height) {
			cb_multiplier_height++;
			j -= font_widthbytes;
		} else {
			cb_multiplier_height = 1;
		}

		cb = cp[j];

	} // end of for loop of character bitmap data

	// assuming we did everything correctly, the bitmap is now loaded up
	_lcd_set_active_area(x, y, TRIPLET_FROM_ACTUAL(glyph_width + padding), glyph_height);
	_lcd_write_data_block(lcd_bitmap, lcd_bitmap_size); // write character pattern
	_lcd_end_active_area();

	// update position
	x += TRIPLET_FROM_ACTUAL(glyph_width + padding);
	lcdPositionSet(x, y);

	return x;
}


/* ---
#### int lcdPutString(char* string)

Display a string at the current LCD position, using the current font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the string displayed.

Use `lcdPositionSet()`, `lcdFontSet()`, and `lcdColorSet()` as necessary, prior to using the function.

The current position is updated by this function.
--- */
int lcdPutString(const char *message) //small font 576 bytes
{
	if (!_lcd_init) return -1;

	int w, width = 0;
	int text_len = strlen(message);

	for (int i = 0; i < text_len; i++) {
		w = lcdPutChar(message[i]);
		if (w < 0)
			break;
		width += w;
	}

	return w;
}

/* ---
#### int lcdPutStringAt(char* string, int x, int y)

Display a string at the specified position, using the current font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the string displayed.

Use `lcdFontSet()`, and `lcdColorSet()` as necessary, prior to using the function.

The current position is updated by this function.

__This is a convenience function which combines `lcdPositionSet()` and `lcdPutString()`.__
--- */
int lcdPutStringAt(const char *message, int x, int y) //small font 576 bytes
{
	if (!_lcd_init) return -1;

	lcdPositionSet(x, y);
	return lcdPutString(message);
}

/* ---
#### int lcdPutStringAtWith(char* string, int x, int y, uint8_t font_id, uint8_t fg, uint8_t bg)

Display a string at the specified position, using the specified font, and colors.

Return -1 if the character was not displayed, otherwise it returns the width of the string displayed.

The current position is updated by this function.

__This is a convenience function which combines `lcdPositionSet()`, `lcdFontSet()`, `lcdColorSet()`, and `lcdPutString()`.__
--- */
int lcdPutStringAtWith(const char *message, int x, int y, uint8_t fid, uint8_t fg, uint8_t bg) //small font 576 bytes
{
	if (!_lcd_init) return -1;

	lcdFontSet(fid);
	lcdColorSet(fg, bg);
	lcdPositionSet(x, y);
	return lcdPutString(message);
}

#endif // __SRXE_LCDTEXT_

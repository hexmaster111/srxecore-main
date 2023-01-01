/* ************************************************************************************
* File:    lcdui.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

### LCD UI
**Draw Composite UI Elements**

The SRXE LCD functions are broken into layers. The UI layer is to top most set of
functions and is intended to make building user interfaces easier and consistent.

**FYI:** The portion of SRXECore is likely to evolve as the first developers start
using the library and new UI functions are identified.

--------------------------------------------------------------------------
--- */


// bitmap file used for round ends of background for a menu item


#define MENU_BALL18_WIDTH 18
#define MENU_BALL18_HEIGHT 18

const uint8_t menu_ball18[] PROGMEM ={
	0x12, 0x00, // image width=18
	0x12, 0x00, // image height=18
	0X02,0x00,0X01,0x52,0X01,0x91,0X03,0x00,0X01,0x0b,0X02,0xff,0X01,0xe8,0X02,0x00,
	0X01,0x9f,0X02,0xff,0X01,0xfe,0X01,0x00,0X01,0x02,0X04,0xff,0X01,0x80,0X01,0x0b,
	0X04,0xff,0X01,0xe8,0X01,0x1f,0X04,0xff,0X01,0xfc,0X01,0x5f,0X04,0xff,0X01,0xfd,
	0X01,0x9f,0X04,0xff,0X01,0xfe,0X01,0x9f,0X04,0xff,0X01,0xfe,0X01,0x9f,0X04,0xff,
	0X01,0xfe,0X01,0x9f,0X04,0xff,0X01,0xfe,0X01,0x5f,0X04,0xff,0X01,0xfd,0X01,0x1f,
	0X04,0xff,0X01,0xfc,0X01,0x0b,0X04,0xff,0X01,0xe8,0X01,0x02,0X04,0xff,0X01,0x80,
	0X01,0x00,0X01,0x9f,0X02,0xff,0X01,0xfe,0X02,0x00,0X01,0x0b,0X02,0xff,0X01,0xe8,
	0X03,0x00,0X01,0x52,0X01,0x89,0X02,0x00,0X00,0x00
};
// Total bytes from original bitmap: 324
// Total bytes to RLE datafrom original bitmap: 59

#define UI_MENU_COUNT 10

enum {
	UI_MENU_CLEAR = 0,
	UI_MENU_RECTANGLE,
	UI_MENU_ROUND_END,
	UI_MENU_ROUNDED
} LCD_MENU_SHAPES;

#define _MENU_POSITION1			11	// centerline of first menu position
#define _MENU_POSITION_DELTA	29	// vertical space from centerline to centerline
#define _MENU_PADDING 			1
#define _MENU_TIGHT					// pull menu text to edge of screen

/* ---
#### void uiMenu()

Render soft menu labels adjacent to the 5 buttons on either side of the screen.

The input parameters are:
- const char* menus[] - an array of 10 pointers to strings to be used as menu labels
- const char* title - optional title to display top center; use NULL to omit the title
- *uint8_t menu_shape - one of `UI_MENU_CLEAR`, `UI_MENU_RECTANGLE`, `UI_MENU_ROUND_END`, or `UI_MENU_ROUNDED`
- bool clear - flag to clear an area slightly larger than the space used for the soft menus

Returns the width used for rendering a menu item

**Notes:**

The array must contain 10 elements.
An element may be a NULL pointer to indicate no menu for that position.
The menu positions are number from top to bottom on the left 0..4 and on the right from 5..9

The function used DEFAULT_MENU_FONT._
--- */
uint8_t uiMenu(const char* menus[], const char* title, uint8_t menu_shape, bool clear) {
	uint8_t x, y, mwidth, mheight, xd;
	bool lmenu = false, rmenu = false;

	uint8_t saved_font = lcdFontGetNum();
	uint8_t saved_fg = lcdColorTripletGetF();
	uint8_t saved_bg = lcdColorTripletGetB();

	lcdFontSet(DEFAULT_MENU_FONT);

	mwidth = 0;
	for (uint8_t i = 0; i < UI_MENU_COUNT;  i++) {
		if (menus[i] != NULL) {
			x = lcdTextWidthGet(menus[i]);
			if (x > mwidth)
				mwidth = x;	// find the longest menu test

			if (x && i < 5)		lmenu = true;
			if (x && i > 4)		rmenu = true;
		}
	}

	switch (menu_shape) {
		case UI_MENU_CLEAR:
		case UI_MENU_RECTANGLE: {
			x = lcdFontWidthGet(); 	// the text offset is based on the font width
			mwidth += TRIPLET_FROM_ACTUAL(x);
			xd = x / 2;
			mheight = lcdFontHeightGet() + (lcdFontHeightGet() / 2);
		} break;
		case UI_MENU_ROUND_END:
		case UI_MENU_ROUNDED: {
			mwidth += TRIPLET_FROM_ACTUAL(MENU_BALL18_WIDTH);
			mheight = MENU_BALL18_HEIGHT;			//h >= lcdFontHeightGet() + _MENU_PADDING;
			xd = MENU_BALL18_WIDTH / 2; 			// the text offset is half the width of the bitmap graphic allow for good visuals and space for the graphic itself
		} break;
	}
	xd = TRIPLET_FROM_ACTUAL(TRIPLET_ROUND(xd));	// move to nearest TRIPLET boundary

	if (clear) {
		uint8_t padded = mwidth + _MENU_PADDING;
		lcdColorSet(LCD_BLACK, LCD_WHITE);
		// clear the left and right sides for the menus
		lcdRectangle(0, 0, padded, LCD_HEIGHT, LCD_ERASE);
		lcdRectangle(LCD_WIDTH - padded, 0, padded, LCD_HEIGHT, LCD_ERASE);
	}

	if (title != NULL) {
		uint8_t save = lcdFontGetNum();
		lcdFontSet(FONT3);
		lcdColorSet(LCD_BLACK, LCD_WHITE);

		// center title on screen
		x = LCD_WIDTH;
		x -= lcdTextWidthGet(title);
		x /= 2;

		// bias title if menus only on one side
		if (lmenu && !rmenu)	x += mwidth / 2;
		if (!lmenu && rmenu)	x -= mwidth / 2;

		lcdRectangle(0, 0, LCD_WIDTH, lcdFontHeightGet() + 2, LCD_ERASE);
		lcdPutStringAt(title, x, 0);
		lcdFontSet(save);
	}

	x = 0;
	y = _MENU_POSITION1 - (mheight/2);

	// render each menu item
	for (uint8_t i = 0; i < UI_MENU_COUNT;  i++) {
		// second half of menu items are on the right
		if (i == 5) {
			y = _MENU_POSITION1 - (mheight/2);
		}
		if (i >= 5) {
			x = LCD_WIDTH - mwidth;
		}
		if (menus[i] != NULL) {
			// draw menu item background
			if ((y + mheight) > LCD_HEIGHT)
				y = LCD_HEIGHT - mheight;

			switch (menu_shape) {
				case UI_MENU_CLEAR: {
					lcdColorSet(LCD_BLACK, LCD_WHITE); // text color will be normal
				} break;
				case UI_MENU_RECTANGLE: {
					lcdColorSet(LCD_WHITE, LCD_BLACK);	// text color will need to be inverted
					lcdRectangle(x, y, mwidth, mheight, LCD_ERASE);
				} break;
				case UI_MENU_ROUND_END: {
					lcdColorSet(LCD_WHITE, LCD_BLACK);	// text color will need to be inverted
					// rounded end has circle on one end
					if (i < 5) {
						// rounded end is on the right for left side menus
						// we draw the bitmap first because it may include pixels of various shades; the subsequent rectangle will cover those up
						lcdBitmap(x + (mwidth - TRIPLET_FROM_ACTUAL(MENU_BALL18_WIDTH)), y, menu_ball18, false);
						lcdRectangle(x, y, mwidth - xd, mheight, LCD_ERASE);
					} else {
						lcdBitmap(x, y, menu_ball18, false);
						lcdRectangle(x + xd, y, mwidth - xd, mheight, LCD_ERASE);
					}
				} break;
				case UI_MENU_ROUNDED: {
					lcdColorSet(LCD_WHITE, LCD_BLACK);	// text color will need to be inverted
					// we draw the bitmap first because it may include pixels of various shades; the subsequent rectangle will cover those up
					lcdBitmap(x, y, menu_ball18, false);
					lcdBitmap(x + (mwidth - TRIPLET_FROM_ACTUAL(MENU_BALL18_WIDTH)), y, menu_ball18, false);
					lcdRectangle(x + xd, y, mwidth - (xd * 2), mheight, LCD_ERASE);
				} break;
			}
#ifdef _MENU_TIGHT
			if (i >= 5) {
				// right justify the right menu
				x = LCD_WIDTH - lcdTextWidthGet(menus[i]) - (xd * 2);
			}
#endif
			// render menu text
			lcdPutStringAt(menus[i], x + xd, (y + (_MENU_PADDING * TRIPLET_SIZE)) -1);
		}
		y += _MENU_POSITION_DELTA;
	}

	lcdFontSet(saved_font);
	lcdColorSet(saved_fg, saved_bg);

	return mwidth;
}


/* ---
#### void uiTextBox()

Render multi-line text.

The input parameters are:
- char* buffer[] - buffer to receive user input
- char* label[] - label to proceed input field (NULL to ignore)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint8_t w - width of input area
- uint8_t h - height of input area
- uint8_t render_flags - flags to control how user input is displayed

Render flags (_not all implemented_):
```C
*/
#define UI_TEXTBOX_WRAP           0x01    // wrap input onto multiple lines (currently assumed)
#define UI_TEXTBOX_INLINE_LABEL   0x02    // label will render at start of line (currently assumed)
/*
```

**Note:** the wrap feature does not use any word break algorithm.

_The function uses the current font and color._
--- */
void uiTextBox(char* buffer, char* label, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t flags) {

	flags = UI_TEXTBOX_WRAP | UI_TEXTBOX_INLINE_LABEL;

	// TODO for Chippy's sake, render the input text block more efficiently than this
	lcdRectangle(x, y, w, h, LCD_ERASE);
	char *p = buffer;
	uint8_t dy = 0;
	while (dy < h) {
		lcdPositionSet(x, y + dy);

		if (dy == 0) {
			// first line and we want to render the label
			// this assumes the label is not longer than the available width
			if (label) {
				lcdPutString(label);
				// lcdPutChar(':');
				lcdPutChar(' ');
			}
		}
		while (lcdPositionGetX() < (x + w)) {
			if (!(*p))
				break;
			lcdPutChar(*p++);
		}
		if (!(*p))
			break;
		dy += lcdFontHeightGet();
	}
}

/* ---
#### void uiLinesBox()

Render multi-line text.

The input parameters are:
- char* lines[] - array of text lines (last array item must be NULL)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint8_t w - width of input area
- uint8_t h - height of input area

**Notes:** There are some special markup supported.
- FONT3 and Black on White text is used
- lines begining with the vertical bar (|) will be centered
- lines begining with the pound symbol (#) will use FONT2
- lines begining with the underscore symbol (_) will be at the bottom of the screen
- special markup may be used together

--- */
void uiLinesBox (const char* lines[], uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool clear) {
	uint8_t saved_font = lcdFontGetNum();
	uint8_t saved_fg = lcdColorTripletGetF();
	uint8_t saved_bg = lcdColorTripletGetB();
	lcdFontSet(FONT3);
	lcdColorSet(LCD_BLACK, LCD_WHITE);

	uint8_t dh = lcdFontHeightGet();
	uint8_t dy = 0;
	uint8_t dw = 0;

	if (clear)
		lcdRectangle(x, y, w, h, LCD_ERASE);

	uint8_t i = 0;
	while (lines[i] != NULL) {
		uint8_t font_change = false;
		const char *p = lines[i];
		dw = 0;
		dy = 0;

		bool special_chars;
		do {
			special_chars = false;

			if (*p == '#') {
				special_chars = true;
				p++; // skip the special character
				// drop font size
				font_change = true;
				lcdFontSet(FONT2);
			}

			if (*p == '|') {
				special_chars = true;
				p++; // skip the special character
				// center this text
				dw = lcdTextWidthGet(p);
				if (dw < w)
					dw = (w - dw) / 2;
				else
					dw = 0;
			}

			if (*p == '_') {
				special_chars = true;
				p++; // skip the special character
				// center this text
				dy = LCD_HEIGHT - y - lcdFontHeightGet();
			}
		} while (special_chars);

		lcdPutStringAt(p, x + dw, y + dy);

		if (font_change) {
			font_change = false;
			lcdFontSet(FONT3);
		}

		y += dh;
		i++;
	}

	lcdFontSet(saved_font);
	lcdColorSet(saved_fg, saved_bg);
}




/* ---
#### void uiInputBox()

Render multi-line input field.

The input parameters are:
- char* buffer[] - buffer to receive user input
- uint8_t size - length of buffer
- char* label[] - label to proceed input field (NULL to ignore)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint8_t w - width of input area
- uint8_t h - height of input area
- uint8_t render_flags - flags to control how user input is displayed - see `uiTextBox()`
- char_callback todo - an call back to perform actions on the current keyboard character (NULL to ignore)

Returns the number of characters input.

**Notes:**

Input ends when size-1 is reached or user hits ENTER or ESC.
Buffer is erased at start of input.
Buffer is erased if user hits ESC.
If no flags restrict input, then all input is allows. Flags may be combined.

_The function uses the current font and color._
--- */
uint8_t uiInputBox(char* buffer, uint8_t size, char* label, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t flags, char_callback todo) {
	uint8_t count = 0;
	uint8_t key = 0;
	bool dirty = true;

	flags = UI_TEXTBOX_WRAP | UI_TEXTBOX_INLINE_LABEL;

	memset(buffer, 0, size);

	while (count < (size - 1)) {

		if (dirty)
			uiTextBox(buffer, label, x, y, w, h, flags);
		dirty = false;

		key = kbdGetKeyWait();

		// don't delete back past start
		if ((key == KEY_DEL) && !count)
			continue;

		// run callback (if available)
		if (todo)
			key = (*todo)(key);

		// callback may alter character and even NOP it
		if (key == KEY_NOP)
			continue;

		dirty = true;

		if (key == KEY_ESC) {
			lcdRectangle(x, y, w, h, LCD_ERASE);
			memset(buffer, 0, size);
			count = 0;
			break;
		}
		if (key == KEY_ENTER)
			break;

		if (key == KEY_DEL) {
			if (count) {
				count--;
				buffer[count] = 0;
			}
		}
		else {
			buffer[count] = key;
			count++;
		}
	}

	buffer[count] = 0;
	return count;
}


/* ---
#### void uiInputField()

Render multi-line input field.

The input parameters are:
- char* buffer[] - buffer to receive user input
- uint8_t size - length of buffer
- char* label[] - label to proceed input field (NULL to ignore)
- char* initial[] - initial value (NULL to ignore)
- uint8_t x - left position of input area
- uint8_t y - top of input area
- uint16_t flags - TBD
- char_callback todo - an call back to perform actions on the current keyboard character (NULL to ignore)

Render flags:
```C
*/
#define UI_INPUTFIELD_FIXED       0x01    // field is fixed length and will automatically return when full
/*
```
Returns the number of characters input.

**Notes:**

Input ends when size-1 is reached, input matches `len` or user hits ESC.
Buffer is erased at start of input and optionally initialized with `default`.

_The function uses the current font and color._
--- */
uint8_t uiInputField(char* buffer, uint8_t size, char* label, char* initial, uint8_t len, uint8_t x, uint8_t y, uint16_t flags, char_callback todo) {
	uint8_t w;
	bool dirty = true;
	uint8_t c;

	memset(buffer, 0, size);

	w = (len + 2) * lcdFontWidthGet();
	if (label)
		w += (strlen(label) + 1) * lcdFontWidthGet();

	if (initial)
		memcpy(buffer, initial, len);

	uint8_t i = 0;
	while (i < len) {
		if (dirty) {
			lcdRectangle(x, y, w, lcdFontHeightGet(), LCD_ERASE);
			lcdPositionSet(x, y);
			if (label)
				printDevicePrintf(PRINT_LCD, "%s [%-*s]", label, len, buffer);
			else
				printDevicePrintf(PRINT_LCD, "[%-*s]", len, buffer);
			dirty = false;
		}

		c = kbdGetKeyWait();

		// TODO need to support DEL

		if ((c == KEY_ENTER) || (c == KEY_ESC)) {
			if ((flags & UI_INPUTFIELD_FIXED) & (strlen(buffer) == len))
				break;
		}

		// run callback (if available); callback may alter character and even NOP it
		if (todo)
			c = (*todo)(c);

		if (c == KEY_NOP)	continue;

		// valid key; store it
		buffer[i] = c;
		i++;
		dirty = true;
	}

	lcdRectangle(x, y, w, lcdFontHeightGet(), LCD_ERASE);
	return i;
}


#if 0

const char* test_message1[] = {
	"This is the first line",
	"A second line is needed",
	0
};

void uisMessageBox(const char* msg[]) {
	uint16_t x, y, w = 0, h = 0, temp;

	// count number of lines; find longest line and finally multiple line count by font height
	for (int i = 0; msg[i] != 0; i++) {
		temp = lcdGetTextWidth(msg[i], FONT_3);
		if (temp > w)
			w = temp;
		h++;
	}

	h *= lcdGetFontHeight(FONT_3);

	// create box at center - maxwidth/2, center - totalheight/2
	x = (LCD_WIDTH - w) / 2;
	y = (LCD_HEIGHT - h) / 2;

	w += x % 3;
	x -= x % 3;

#define BORDER_OFFSET 12
#define BLANK_OFFSET  (BORDER_OFFSET + 3)
	lcdRectangle(x - BLANK_OFFSET, y - BLANK_OFFSET, w + (2 * BLANK_OFFSET), h + (2 * BLANK_OFFSET), 0, 1);
	// render each line of the message starting at center - maxwidth/2
	for (int i = 0; msg[i] != 0; i++) {
		lcdPutString(x, y + (lcdGetFontHeight(FONT_3) * i), msg[i], FONT_3, 3, 0);
	}
	lcdRectangle(x - BORDER_OFFSET, y - BORDER_OFFSET, w + (2 * BORDER_OFFSET), h + (2 * BORDER_OFFSET), 3, 0);


	// leave message up for 'at most' n seconds; use 0 to mean indefinitely
	wait_for_user(0);
}
#endif

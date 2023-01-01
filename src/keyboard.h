/* ************************************************************************************
* File:    keyboard.h
* Date:    2021.08.12
* Author:  Bradan Lane Studio
*
* This content may be redistributed and/or modified as outlined under the MIT License
*
* ************************************************************************************/

/* ---

## Keyboard
**Keyboard input**

The SRXE keyboard has three mode - **normal**, **shift**, and **symbol**.
The keyboard also includes the ten soft menu keys. These are located five on either side of the LCD screen,
and the is a four-way button in the lower right.

The original keyboard code from **fdufnews** have been extended with **shift** and **symbol** menus as well
as mappings for most available keys. It is strongly recommended to use the key definitions to avoid confusion.
The mapping of ENTER and DEL have been restored.

**FYI:** The ENTER key is not marked on the SRXE keyboard.
It is located in the center of the 4-way navigation pad.
Pressing the center of the NAV pad will generate the ENTER key.

--------------------------------------------------------------------------
--- */


#ifndef __SRXE_KEYBOARD_
#define __SRXE_KEYBOARD_

#include "common.h"



#define UPPERCASE(key)	( (((key) >= 'a')&&((key) <= 'z')) ? ((key) - 'a' + 'A') : (key) )
#define ISLETTER(key) 	( ((((key) >= 'a')&&((key) <= 'z')) || (((key) >= 'A')&&((key) <= 'Z'))) ? true : false)
#define ISDIGIT(key)	( (((key) >= '0')&&((key) <= '9')) ? true : false)
#define ISPRINTABLE(key)( (((key) >= ' ')&&((key) <= '~')) ? true : false)

// --------------------------------------------------------------------------------------------------------
// Keyboard
// --------------------------------------------------------------------------------------------------------

// Keyboard info
#define ROWS 6
#define COLS 10

/* ---
There are definitions for the various special keys. They are:
```C
*/
#define KEY_NOP             0
#define KEY_SHIFT           KEY_NOP
#define KEY_SYM             KEY_NOP

#define KEY_DEL             0x08
#define KEY_ENTER           0x0D
#define KEY_ESC             0x1B

#define KEY_MENU1           0x80
#define KEY_MENU2           0x81
#define KEY_MENU3           0x82
#define KEY_MENU4           0x83
#define KEY_MENU5           0x84
#define KEY_MENU6           0x85
#define KEY_MENU7           0x86
#define KEY_MENU8           0x87
#define KEY_MENU9           0x88
#define KEY_MENU10          0x89
#define KEY_LEFT            0x8A
#define KEY_RIGHT           0x8B
#define KEY_UP              0x8C
#define KEY_DOWN            0x8D

#define KEY_MENU1SH         0x90
#define KEY_MENU2SH         0x91
#define KEY_MENU3SH         0x92
#define KEY_MENU4SH         0x93
#define KEY_MENU5SH         0x94
#define KEY_MENU6SH         0x95
#define KEY_MENU7SH         0x96
#define KEY_MENU8SH         0x97
#define KEY_MENU9SH         0x98
#define KEY_MENU10SH        0x99
#define KEY_PGUP            0x9A
#define KEY_PGDN            0x9B
#define KEY_HOME            0x9C
#define KEY_END             0x9D



#define KEY_MENU1SY         0xA0
#define KEY_MENU2SY         0xA1
#define KEY_MENU3SY         0xA2
#define KEY_MENU4SY         0xA3
#define KEY_MENU5SY         0xA4
#define KEY_MENU6SY         0xA5
#define KEY_MENU7SY         0xA6
#define KEY_MENU8SY         0xA7
#define KEY_MENU9SY         0xA8
#define KEY_MENU10SY        0xA9
#define KEY_CONTUP          0xAE
#define KEY_CONTDN          0xAF


#define KEY_FRAC            0xB0
#define KEY_ROOT            0xB1
#define KEY_EXPO            0xB2
#define KEY_ROOX            0xB3
#define KEY_BASE            0xB4
#define KEY_PI              0xB5
#define KEY_THETA           0xB6
#define KEY_DEG             0xB7
#define KEY_LE              0xB8
#define KEY_GE              0xB9

#define KEY_GRAB            0xFC    // when SCREEN_GRABBER has been defined
#define KEY_GRABON          0xFD    // when SCREEN_GRABBER has been defined
#define KEY_GRABOFF         0xFE    // when SCREEN_GRABBER has been defined

#define KEY_MENU            0xFF
/*
```
--- */

// Logical Layout:
//                ROW1|ROW2|ROW3|ROW4|ROW5|ROW6|ROW7|ROW8|ROW9|ROW10
//           COL1    1|   2|   3|   4|   5|   6|   7|   8|   9|    0
//           COL2    Q|   W|   E|   R|   T|   Y|   U|   I|   O|    P
//           COL3    A|   S|   D|   F|   G|   H|   J|   K|   L|  Del
//           COL4 Shft|   Z|   X|   C|   V|   B|   N|Down|Entr|   Up
//           COL5  Sym|Frac|Root| Exp| Spc|   ,|   .|   M|Left|Right
//           COL6  SK1| SK2| SK3| SK4| SK5| SK6| SK7| SK8| SK9| SK10
//
// SK# are screen keys with top to bottom 1-5 on left and 6-10 on right

/* ---
There are keyboard maps for normal, with-shift, and with-symbol keys. They are:
```C
*/
uint8_t _kbd_normal_keys[] = {
	'1',        '2',        '3',        '4',        '5',        '6',        '7',        '8',        '9',        '0',
	'q',        'w',        'e',        'r',        't',        'y',        'u',        'i',        'o',        'p',
	'a',        's',        'd',        'f',        'g',        'h',        'j',        'k',        'l',        KEY_DEL,
	KEY_SHIFT,  'z',        'x',        'c',        'v',        'b',        'n',        KEY_DOWN,   KEY_ENTER,  KEY_UP,
	KEY_SYM,    KEY_FRAC,   KEY_ROOT,   KEY_EXPO,   ' ',        ',',        '.',        'm',        KEY_LEFT,   KEY_RIGHT,
	KEY_MENU1,  KEY_MENU2,  KEY_MENU3,  KEY_MENU4,  KEY_MENU5,  KEY_MENU6,  KEY_MENU7,  KEY_MENU8,  KEY_MENU9,  KEY_MENU10
};

uint8_t _kbd_shift_keys[] = {
	'1',        '2',        '3',        '4',        '5',        '6',        '7',        '8',        '9',        '0',
	'Q',        'W',        'E',        'R',        'T',        'Y',        'U',        'I',        'O',        'P',
	'A',        'S',        'D',        'F',        'G',        'H',        'J',        'K',        'L',        KEY_ESC,
	KEY_SHIFT,  'Z',        'X',        'C',        'V',        'B',        'N',        KEY_PGDN,   KEY_ENTER,  KEY_PGUP,
	KEY_SYM,    KEY_GRABOFF,KEY_GRAB,   KEY_GRABON ,'_',        ',',        '.',        'M',        KEY_HOME,   KEY_END,
	KEY_MENU1SH,KEY_MENU2SH,KEY_MENU3SH,KEY_MENU4SH,KEY_MENU5SH,KEY_MENU6SH,KEY_MENU7SH,KEY_MENU8SH,KEY_MENU9SH,KEY_MENU10SH
};

uint8_t _kbd_symbol_keys[] = {
	'!',        KEY_PI,     KEY_THETA,  '$',        '%',        KEY_DEG,    '\'',       '\"',       '(',        ')',
	'Q',        'W',        'E',        'R',        'T',        'Y',        'U',        ';',        '[',        ']',
	'=',        '+',        '-',        'F',        'G',        'H',        'J',        ':',        '?',        KEY_ESC,
	KEY_SHIFT,  '*',        '/',        'C',        'V',        'B',        KEY_LE,     KEY_CONTDN,   KEY_ENTER,  KEY_CONTUP,
	KEY_SYM,    KEY_FRAC,   KEY_ROOX,   KEY_BASE,   KEY_MENU,   '<',        '>',        KEY_GE,     KEY_HOME,   KEY_END,
	KEY_MENU1SY,KEY_MENU2SY,KEY_MENU3SY,KEY_MENU4SY,KEY_MENU5SY,KEY_MENU6SY,KEY_MENU7SY,KEY_MENU8SY,KEY_MENU9SY,KEY_MENU10SY
};
/*
```
--- */


const uint8_t _kb_row_pins[ROWS] = {0xe6, 0xb7, 0xb6, 0xb5, 0xb4, 0xe0};
const uint8_t _kb_col_pins[COLS] = {0xe4, 0xf1, 0xf3, 0xe2, 0xe1, 0xd7, 0xa0, 0xa5, 0xd5, 0xd4};
static uint8_t _new_keymap[COLS];					  // bits indicating pressed keys
static uint8_t _old_keymap[COLS];					  // previous map to look for pressed/released keys
static uint16_t _last_key;							  // most recent key detected by scan

static uint32_t _kb_debounce;
#define KBD_DEBOUNCE_INTERVAL 10 // milliseconds
//
// Scan the rows and columns and store the results in the key map
//
void _kbd_scan_kb(void) {
	uint8_t col, row;

	// save current keymap to compare for pressed/released keys
	memcpy(_old_keymap, _new_keymap, sizeof(_new_keymap));

	if (_kb_debounce > clockMillis())
		return;
	_kb_debounce = clockMillis() + KBD_DEBOUNCE_INTERVAL;

	#if 0	// this is done once in kbdInit()
	for (row = 0; row < ROWS; row++) {
		srxePinMode(_kb_row_pins[row], INPUT_PULLUP);
	}
	#endif

	for (col = 0; col < COLS; col++) {
		_new_keymap[col] = 0;
		srxePinMode(_kb_col_pins[col], OUTPUT); // make the column GND
		srxeDigitalWrite(_kb_col_pins[col], LOW);

		for (row = 0; row < ROWS; row++) {
			if (srxeDigitalRead(_kb_row_pins[row]) == LOW) {
				_new_keymap[col] |= (1 << row); // set a bit for this pressed key
			}
		} // for r

		srxeDigitalWrite(_kb_col_pins[col], HIGH); // reset the column to non-conductive
		srxePinMode(_kb_col_pins[col], INPUT);
	} // for c

#if 1	// prioritize the center pad of the NAV buttons
	// treat multi-NAV (aka button mashing) activation as ENTER
	// the NAV pad has 5 locations in the keymap: C7R3, C8R3, C8R4, C9R3, and C9R4
	// of these, C8R3 is the center
	uint8_t multi_nav = 0;
	if (_new_keymap[7] & (1 << 3)) multi_nav++;
	if (_new_keymap[8] & (1 << 3)) multi_nav++;
	if (_new_keymap[8] & (1 << 4)) multi_nav++;
	if (_new_keymap[9] & (1 << 3)) multi_nav++;
	if (_new_keymap[9] & (1 << 4)) multi_nav++;
	if (multi_nav > 1)
		_new_keymap[8] |= (1 << 3);

	// the NAV pad has 5 locations in the keymap: C7R3, C8R3, C8R4, C9R3, and C9R4
	// of these, C8R3 is the center
	if (_new_keymap[8] & (1<<3)) {
		// if the center pad is pressed, we clear all the others
		// clear C7R3, C8R4, and C9
		_new_keymap[7] &= ~(1 << 3);
		_new_keymap[8] &= ~(1 << 4);
		_new_keymap[9] &= ~((1 << 3) | (1 << 4));
	}
#endif

} /* _srxe_scan_kb() */

//
// Return a pointer to the internal column-wise key map
// (10 bytes with 6 bits each) each byte represents a column
//
uint8_t *_kbd_get_keymap(void) {
	return _new_keymap;
}


/* ---
#### void kbdInit()

Initialize the keyboard for scanning.

Must be called to initialize the keyboard prior to using any other keyboard functions.
--- */

void kbdInit() {
#if 1
	for (int r = 0; r < ROWS; r++) {
		srxePinMode(_kb_row_pins[r], INPUT_PULLUP);
	}
#else
	for (int r = 0; r < ROWS; r++) {
		srxePinMode(_kb_row_pins[r], INPUT_PULLUP);
	}
	for (int c = 0; c < COLS; c++) {
		srxePinMode(_kb_col_pins[c], INPUT_PULLUP);
	}
#endif
	_last_key = 0;
	_kb_debounce = 0;
}

/* ---
#### uint8_t kbdGetKeyDetails()

Return a 16 bit value with the low 8 bits representing the most current key press.
The high 8 bits are 4 bits for the column and 4 bits for the row.
This will ignore the current key if it has already been reported.
Returns KEY_NOP if no key _(or no new key)_ is pressed.

**Note:** The row and column values are 1's based to allow zero to represent no data.

--- */
uint16_t kbdGetKeyDetails() {
	uint8_t *pKeys;
	uint8_t shift_key, sym_key;
	uint8_t row_bit;
	uint8_t row, col;
	uint8_t details = 0;


	_kbd_scan_kb();

	// grab the shift/sym state first
	shift_key = _new_keymap[0] & 0x08;
	if (shift_key) details = (((0+1) << 4) & 0xF0) | ((3+1) & 0xF);
	sym_key = _new_keymap[0] & 0x10;
	if (sym_key) details = (((0+1) << 4) & 0xF0) | ((4+1) & 0xF);

	for (col = 0; col < COLS; col++) {
		for (row = 0; row < ROWS; row++) {
			row_bit = 1 << row;
			// if key is pressed and key was not pressed, process it
			if ((_new_keymap[col] & row_bit) == row_bit && (_old_keymap[col] & row_bit) == 0) {
				// make sure it's not shift/sym
				if (col == 0 && (row == 3 || row == 4)) // shift/sym, ignore because we already have their state
					continue;

				// valid key, adjust it and return
				pKeys = _kbd_normal_keys;
				if (shift_key)
					pKeys = _kbd_shift_keys;
				else if (sym_key)
					pKeys = _kbd_symbol_keys;

				details = (((col+1) << 4) & 0xF0) | ((row+1) & 0xF);
				_last_key = (details << 8) | (pKeys[(row * COLS) + col]);

				powerSleepUpdate();	// we centralize staying away to any keyboard input

#ifdef SCREEN_GRABBER
			// the following three keys are special use for capturing screens
			if ((_last_key & 0xFF) == KEY_GRAB) {
				// trigger screen grab
				LCD_STREAM_GRABBER_GRAB();
			}
			if ((_last_key & 0xFF) == KEY_GRABON) {
				// enable screen capture stream
				LCD_STREAM_GRABBER_ACTIVATE();
			}
			if ((_last_key & 0xFF) == KEY_GRABOFF) {
				// disable screen capture stream
				LCD_STREAM_GRABBER_DEACTIVATE();
			}
#endif
			//if (_last_key & 0xFF) printDevicePrintf(PRINT_UART, "(%2d,%2d) %02X\n", (_last_key >> 12) & 0xF, (_last_key >> 8) & 0xF, _last_key & 0xFF);

			return _last_key;
			}
		}
	}
	return ((details << 8) & 0xFF00);	// this will return the shift or sym key details if either has been pressed by itself
}

/* ---
#### uint8_t kbdGetKey()

Return the most current key press. This will ignore the current key if it has already been reported.
Returns KEY_NOP if no key _(or no new key)_ is pressed.

--- */
uint8_t kbdGetKey() {
	return kbdGetKeyDetails() & 0xFF;
}

/* ---
#### uint8_t kbdGetKeyWait()

Return the current key press.  This will ignore the current key if it has already been reported.
if no key _(or no new key)_ is pressed, it will wait.

--- */
uint8_t kbdGetKeyWait() {
	uint8_t c;

	while (!(c = kbdGetKey()))
		;
	return c;
}

#endif /// __SRXE_KEYBOARD_

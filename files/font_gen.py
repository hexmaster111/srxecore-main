"""

/* ***************************************************************************
* File:    font_gen.py
* Date:    2021.08.10
* Author:  Bradan Lane STUDIO
*
* This content may be redistributed and/or modified as outlined
* under the MIT License
*
* ******************************************************************************/

/* ---
# SMART Response XE Font Generation

Getting fonts to look good on the SRXE LCD is 25% trial and error, 33% subjective, 50% rendering engine, and 10% your code.

This `font_gen.py` program generates header files that are specific to the SRXEcore text renderer. It was not written to
match any previous format. The program will output a series of header files with bitmap data for a font at a designated W*H.
The output data is _width first_ to match the way the rendering engine on the SRXEcore works.
Also, _width first_ is how the LCD bitmap on a SRXE is formatted.

The general workflow for generating new font files is as follows:
 - find a font that will look good at the resolution you need. The best source is [Old School Fonts](https://int10h.org/oldschool-pc-fonts/)
 - adjust the parameters (ID number, point size, y offset, width, and height) until the resulting `PNG` looks good
 - include the generated header file(s) in your code

The SRXEcore will compile out any rendering engine code not used so only include the generated headers you will actually use.

### FONTS

Only use monospace fonts. This code does not attempt to clean up and space out proportional fonts ... at least not very well.
Ideally, the font width will be a multiple of 3. This will look best on the SRXE LCD display. The rendering engine will add whitespace
between characters to pad out to the 3-pixel boundary required by the LCD driver.

Fonts are somewhat subjective - especially at small sizes. Some people may have difficulty reading some fonts.
Here are the best fonts I was able to find:

|LABEL|WxH|DESCRIPTION|
|:-----|:-----:|:-----|
|TINY|6x8|The HP100LX is possible the best tiny font available|
|SMALL|8x12|the Toshiba 8x14 can be tweeked quite a bit to make an excellent small font|
|SMALL|9x15|the IBM XGA AI 7x15 is a clean contemporary looking font|
|MEDIUM|12x18|the IBM XGA AI 12x23 makes a clean contemporary looking medium font|
|LARGE|15x28|the IBM XGA AI 7x15 (doubled) actually looks a bit better that the 12x23|

**Memory Usage:** If memory usage is a concern, the the `TINY` may be doubled to make a `MEDIUM` and the `SMALL` may be doubled to make a `LARGE`.
The Toshiba font is better for doubling than the IBM font.

_See the `lcdFontConfig()` and `lcdFontClone()` functions for details on doubling.

### Notes

Most fonts drop the underscore character to the very bottom of the font cell and below any other glyph pixels.
This results in the need to make a font taller than it would otherwise need to be. For this reason, this program
forces the underscore to the bottom most available row of the font. This hack means its OK to clip the bottom of the
font, if the only thing that will be lost is the underscore.

The `font_gen.py` program is based on the work by Jared Sanson (jared@jared.geek.nz).
This program requires `PIL` (Python Imaging Library) to generate a `PNG` of the characters.
The `PNG` is chunked to make the bitmaps for each character.
Only the characters from SPACE (32) through Tilda (127) are rasterized.

--------------------------------------------------------------------------
--- */

"""

from PIL import Image, ImageFont, ImageDraw
import os.path

FONT_DIR = "./"

# FONT = {'fname':r'font_file_name.ttf', 'def:'1,2,3, or 4', 'size':point_size, 'yoff':0, 'w':pixel_width, 'h':pixel_height}

FONT6X8		= {'fname':r'HP100LX_6x8.ttf',		'def':'6X8',	'size':8,	'yoff':0,	'w':6,	'h':8}
FONT8X14	= {'fname':r'ToshibaSat_8x14.ttf',	'def':'8X14',	'size':16,	'yoff':-2,	'w':8,	'h':12}
FONT7X15	= {'fname':r'IBM_XGA-AI_7x15.ttf',	'def':'7X15',	'size':16,	'yoff':0,	'w':7,	'h':15}
FONT12X23	= {'fname':r'IBM_XGA-AI_12x23.ttf',	'def':'12X23',	'size':24,	'yoff':-1,	'w':12,	'h':18}

FONTS = [ FONT6X8, FONT8X14, FONT7X15, FONT12X23]


# WARNING: Support for variable-width character fonts is not available

for FONT in FONTS:
	FONT_FILE = FONT['fname']
	FONT_SIZE = FONT['size']
	FONT_DEFINE = FONT.get('def', 'unknown')

	FONT_Y_OFFSET = FONT.get('yoff', 0)

	CHAR_WIDTH = FONT.get('w', 5)	# 5 is the default if the key value is missing
	CHAR_HEIGHT = FONT.get('h', 8)	# 8 is the default if the key value is missing

	# only get the character glyphs from SPACE through tilda
	FONT_BEGIN = ' '
	FONT_END = '~'


	FONTSTR = ''.join(chr(x) for x in range(ord(FONT_BEGIN), ord(FONT_END)+1))

	OUTPUT_NAME = 'font_' + FONT_DEFINE
	OUTPUT_PNG = OUTPUT_NAME + '.png'
	OUTPUT_H = OUTPUT_NAME + '.h'

	GLYPH_WIDTH = CHAR_WIDTH

	WIDTH = GLYPH_WIDTH * len(FONTSTR)
	HEIGHT = CHAR_HEIGHT

	img = Image.new("RGBA", (WIDTH, HEIGHT), (255,255,255))
	fnt = ImageFont.truetype(FONT_DIR + FONT_FILE, FONT_SIZE)
	drw = ImageDraw.Draw(img)

	for i in range(len(FONTSTR)):
		drw.text((i*GLYPH_WIDTH,FONT_Y_OFFSET), FONTSTR[i], (0,0,0), font=fnt)

	img.save(OUTPUT_PNG)

	#### Convert to C-header format
	f = open(OUTPUT_H, 'w')
	num_chars = len(FONTSTR)

	f.write('\n')
	f.write('// GENERATED FILE - DO NOT EDIT\n')
	f.write('// To change fonts, edit and run python3 font_gen.py\n')
	f.write('// generated from font file: "%s"\n' % (FONT_FILE))
	f.write('\n')
	# f.write('#define FONT_%s %s\n\n' % (FONT_DEFINE.upper(), FONT_DEFINE.upper()))
	f.write('#define FONT_%s_WIDTH %d\n' % (FONT_DEFINE.upper(), CHAR_WIDTH))
	f.write('#define FONT_%s_HEIGHT %d\n' % (FONT_DEFINE.upper(), CHAR_HEIGHT))
	f.write('#define FONT_%s_WIDTHBYTES %d\n' % (FONT_DEFINE.upper(), int((CHAR_WIDTH + 7) / 8)))
	f.write('#define FONT_%s_CHARBYTES %d\n' % (FONT_DEFINE.upper(), (int((CHAR_WIDTH + 7) / 8) * CHAR_HEIGHT)))
	f.write('\n')
	f.write('// NOTE: Data is width (bits) first by height (bits) to match the SRXE LCD processing.\n')
	f.write('//       The bits are stored where the low bit is the left most pixel to be easier for SRXE processing.\n')
	f.write('//       This has the positive of being flexible for any font and the negative of using more data space.\n')
	f.write('//       Ideally for data size, the font width would be a multiple of 8, but LCD wants font width to be a multiple of 3.\n')
	f.write('\n')

	f.write('\nconst unsigned char font_' + FONT_DEFINE + '_P [] PROGMEM = {\n')

	for i in range(num_chars):
		ints = []

		for y in range(CHAR_HEIGHT):
			val = 0
			x = 0
			offset = i*GLYPH_WIDTH
			shift = 0

			for j in range(CHAR_WIDTH):
				# if we are on the last row of the underscore, force the pixels
				rgb = img.getpixel((j+offset,y))
				if (y == (CHAR_HEIGHT - 1)) and (i == (95 - 32)) and (j != 0):
					rgb = [0, 0, 0]
				val = val | ((1 << shift) if rgb[0] < 127 else 0)
				shift += 1
				if shift % 8 == 0:
					# store byte
					ints.append('0x%.2x' % (val))
					val = 0
					shift = 0

			if shift:
				# we have bit(s) we have not stored
				ints.append('0x%.2x' % (val))
				val = 0
				shift = 0

		c = FONTSTR[i]
		if c == '\\': c = '"\\"' # bugfix

		f.write('\t%s, // %3d %s\n' % (','.join(ints), ord(c[0]), c))

	f.write('\t%s\n' % (','.join(['0x00']*CHAR_WIDTH)))
	f.write('};\n\n')

	# f.write('FONTOBJECT font_%s_object = {font_%s_P, FONT_%s,  FONT_%s_WIDTH, FONT_%s_HEIGHT, FONT_%s_WIDTHBYTES, FONT_%s_CHARBYTES};\n' % (FONT_DEFINE.upper(), FONT_DEFINE.upper(), FONT_DEFINE.upper(), FONT_DEFINE.upper(), FONT_DEFINE.upper(), FONT_DEFINE.upper(), FONT_DEFINE.upper()))
	# f.write('\n')

	f.close()

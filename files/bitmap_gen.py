"""

/* ***************************************************************************
* File:    bitmap_gen.py
* Date:    2020.10.31
* Author:  Bradan Lane STUDIO
*
* This content may be redistributed and/or modified as outlined
* under the MIT License
*
* ******************************************************************************/

/* ---
# SMART Response XE Bitmaps

Bitmaps are memory intensive but when used judiciously, they can be smaller than attempted to create the same results as draw functions.
Bitmaps are also great for icons and other small graphics.

This `bitmap_gen.py` program generates run-length-encoded (RLE) data as a header file.
While RLE is pretty common, this program required the source bitmap to be correctly formatted.

### Source Bitmaps

The general workflow for generating a new bitmap file is as follows _(these instructions use GIMP)_:
 - The maximum graphic is 384x136. _Smaller graphics are fine._
 - Create your graphic. It's OK to use layers. Graphics layers can not use transparency.
 - Keep text in black & white.
 - Convert any image layers layers to grayscale using **Color -> Desaturate -> Desaturate**.
 - Increase contrast of any image image layers using **Colors -> Levels** and shifting the center node to the right and the right node to the left. _This will be somewhat of a trial and error process_.
 - Set image color mode to indexed using **Image -> Mode -> Indexed** with **Maximum Colors 4** and dithering set to **Floyd-Steinberg Reduced Color Bleeding**.
 - Export file with `.raw` extension and use **Standard** and **Normal** options.
 - Delete and `.pal` files.

**Note:** The SRXE requires draw operations on a 3-pixel boundary. If the RAW bitmap width is not a multiple of 3, it will be padded with background.

### Usage

`python3 bitmap_gen.py` _source-file_ _output-file_ _width_ _height_

|PARAMETER|DESCRIPTION|
|:-----|:-----|
|source file|4 indexed monochrome bitmap file in RAW format|
|output file| header file output|
|width|pixel width of source bitmap|
|height|pixel height of source bitmap|

### Example

`python3 bitmap_gen.py` menu_ball18.raw ball18.h 18 18`

--------------------------------------------------------------------------
--- */

"""

#!/usr/bin/python2

# Generate a bitmap for the SMART Response XE
# RLE variant
from array import array
import argparse
import os

#list defining the 4 colors for any of the 3 pixels
# The color list is inverted comparing to the one in the SMART Response library
# that's because "min" and "max" in the graphics file are reverse of the SRXE
color = [0xFF, 0x92, 0x49, 0x00]

#list defining the mask to access the 3 pixels
mask = [0xE0, 0x1C, 0x03]


#define a parser for the command line
parser = argparse.ArgumentParser(description='process a raw bitmap')
parser.add_argument('filein', help='bitmap file in raw format to convert')
parser.add_argument('fileout', help='.h file')
parser.add_argument('width',type=int, nargs=1, help='width of bitmap')
parser.add_argument('height',type=int, nargs=1, help='height of bitmap')
parser.add_argument('--version', action='version', version='%(prog)s 1.0')

#parse the command line
result = parser.parse_args()
dict = vars(result)
width = dict['width'][0]
height = dict['height'][0]
infilename = dict['filein']
outfilename = dict['fileout']

padding = 0
if (width % 3 > 0):
	padding = 3 - (width % 3)
	print('WARNING: bitmap width is %d which is not on 3-pixel boundary. A %d pixel padding has been added.\n' % (width, padding))

basename = os.path.splitext(os.path.basename(infilename))[0]

fileout = open(outfilename,'w')

fileout.write('\n')
fileout.write('// GENERATED FILE - DO NOT EDIT\n')
fileout.write('// To change bitmaps, edit and run python3 bitmap_gen.py\n')
fileout.write('// generated from RAW bitmap file: "%s"\n' % (infilename))
fileout.write('\n')
fileout.write('// NOTE: Data is run length encoded width (bits) first by height (bits).\n')
fileout.write('//       each byte represents up to 3 pixels - called a TRIPLET - to match the LCD driver of the SRXE.\n')
fileout.write('\n')
if (padding):
	fileout.write('// WARNING: the bitmap width is %d which is not on 3-pixel boundary. A %d pixel padding has been added.\n' % (width, padding))
	fileout.write('\n')
fileout.write('\n')
fileout.write('#ifndef __%s_\n' % (basename.upper()))
fileout.write('#define __%s_\n' % (basename.upper()))
fileout.write('\n')
fileout.write('\n')

# let's write a header with the name of the image source
fileout.write('// Original file {}\n\n#include <avr/pgmspace.h>\n\n'.format(infilename))
fileout.write('\n')
# name the bitmap with the name of the output file

name = os.path.splitext(os.path.basename(outfilename))

fileout.write('#define %s_WIDTH %d\n' % (basename.upper(), (width + padding)))
fileout.write('#define %s_HEIGHT %d\n' % (basename.upper(), height))
fileout.write('\n')

#fileout.write('const uint8_t bitmap_{0}[] PROGMEM ='.format(name[0]))
fileout.write('const uint8_t {0}[] PROGMEM ='.format(name[0]))
fileout.write('{\n\t')

#put size of the picture as a header of the bitmap
fileout.write('0x{0:02x}, 0x{1:02x}, // image width={2}\n\t'.format((width+padding) & 0xff, (width+padding)>>8 & 0xFF, (width+padding)))
fileout.write('0x{0:02x}, 0x{1:02x}, // image height={2}\n\t'.format(height & 0xff, height>>8 & 0xFF, height))

with open( infilename, 'rb' ) as filein:
	data = array( 'B', filein.read() ) # buffer the file


count = 0	# count of byte processed
countr = 0	# count of byte read
countw = 0	# count byte written

first = 1 # Flag telling that we have just started
countRLE = 0 # count number of same bytes
prev_char = 4 # hold value of current running length (4 is an impossible value in the context)

# bytearray used to hold 3 pixels at a time
ba = bytearray()
for byte in data:
	if byte > 4: # skip 0xFF high order bytes
		continue

	ba.insert(0,byte) # push the pixel in the array
	count = count + 1
	countr = countr + 1

	if (padding and (countr % width == 0)):
		for i in range(padding):
			ba.insert(0,3) # push a blank pixel in the array
			count = count + 1

	if(count%3==0):  # every 3 pixels concatenate them into one byte
		pix = color[ba.pop()] & mask[0] | color[ba.pop()] & mask[1] | color[ba.pop()] & mask[2]
		if pix != prev_char:
			if countRLE != 0: # if we are not starting encoding write the previous running length
				fileout.write('0X{0:02X},0x{1:02x},'.format(countRLE, prev_char)) # write the byte to the file in ascii hex
				if(countw % 8 == 7):
					fileout.write('\n\t')  # every 16 bytes written newline
				countw +=1

			countRLE = 1 # restart RLE count
			prev_char = pix # new pix we are counting
			first = 0
		else:
			countRLE +=1
			if countRLE==254: # if running length is to roll over write value and reset count
				fileout.write('0X{0:02X},0x{1:02x},'.format(countRLE, prev_char)) # write the byte to the file in ascii hex
				if(countw % 8 == 7):
					fileout.write('\n\t')  # every 16 bytes written newline
				countw +=1
				countRLE = 0


fileout.write('0X{0:02X},0x{1:02x},'.format(countRLE, prev_char))
fileout.write('0X00,0x00\n};')

fileout.write('\n')
fileout.write('// Total bytes from original bitmap: %d\n' % (count))
fileout.write('// Total bytes to RLE datafrom original bitmap: %d\n' % (countw))
fileout.write('\n')
fileout.write('#endif // __%s_\n' % (basename.upper()))
fileout.write('\n')

fileout.close()


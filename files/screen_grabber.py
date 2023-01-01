"""

/* ***************************************************************************
* File:    screen_grabber.py
* Date:    2021.08.10
* Author:  Bradan Lane STUDIO
*
* This content may be redistributed and/or modified as outlined
* under the MIT License
*
* ******************************************************************************/

/* ---
# SMART Response XE Screen Grabber

Monitor the UART output for screen data and write it to PGM image files.

This `screen_grabber.py` works in conjunction with the SRXEcore when it has been compiled with `SCREEN_GRABBER` flag defined.
The SRXEcore - _when compiled with `SCREEN_GRABBER`_ - will stream buffer events over UART as well as stream the data written to the LCD.

By default the screen data streaming is automatic when `SCREEN_GRABBER` has been defined.

Define `SCREEN_GRABBER_MANUAL` to compile in functionality but have it remain idle.
Use `LCD_SCREEN_GRABBER_ACTIVATE()` and `LCD_SCREEN_GRABBER_DEACTIVATE()` to manually enable/disable the functionality.
This is most useful if tied to menu or hot keys on the keyboard.
It may also be helpful to tie `LCD_SCREEN_GRABBER_GRAB()` to a menu or hot key to trigger `screen_grabber.py`.

**CAUTION:** The data streaming **really really really** slows down the screen operations _(did we say really? we mean really)_
so this capability should only be used to generate screen shots and then then the SRXEcore should be recompiled without the `SCREEN_GRABBER` flag
or use the `SCREEN_GRABBER_MANUAL` option.

The UI for this program is rough but _it-is-what-it-is_. There are only two commands: **G** and **Q** - both of which must be followed by `[ENTER]`
to be processed.

The system will capture the current screen prior to `lcdClearScreen()`. Alternatively, the current screen may be saved using the **G** command.

**NOTE:** In addition to the UI being lazy, the Serial port selection is coded. You will need to edit `screen_grabber.py` if the default device does
not match your environment.

--------------------------------------------------------------------------
--- */

"""


import serial
import sys
import threading

import time
from datetime import datetime


START_MARK = 0x5B # '['
STOP_MARK = 0x5D # ']'
PIXIES = ['.', '-', 'X', '#']
PORT = "/dev/ttyUSB0"

RUN_ID = datetime.now().strftime("%Y%m%d%H%M%S")

print ('initializing serial port ...')
port = serial.Serial(PORT, baudrate=9600, timeout=0.1)
print ('... serial port {} initialized'.format(PORT))
print('')


SCREEN_WIDTH = 128
SCREEN_HEIGHT = 136
screen = [0] * (SCREEN_WIDTH * SCREEN_HEIGHT)
x = 0
y = 0
w = SCREEN_WIDTH - 1
h = SCREEN_HEIGHT - 1
pixel = 0
special = 0
active = False
screen_counter = 1
dirty = False


# --------------------------------------------------------------
# Keyboard Handler for action keys
# --------------------------------------------------------------

_kbd_key = ''

def getkey():
	global _kbd_key
	if _kbd_key != '':
		key = _kbd_key
		_kbd_key = ''
		return key
	return 0

def keyboard_tread():
	global _kbd_key
	loop = True
	while loop:
		ch = sys.stdin.read(1)
		ch = ch.upper()
		if (ch == 'G'):
			_kbd_key = ch
		elif (ch == 'Q'):
			_kbd_key = ch
			loop = False

def setup_keyboard_handler():
	poller = threading.Thread(target=keyboard_tread)
	poller.start()
# --------------------------------------------------------------
# --------------------------------------------------------------



def compute_position(x, y, w, h, pixel):
	# we are dealing with an x,y,w,d area within the overall 0,0,W,D buffer
	if int(pixel / w) > h:
		print('clipped: {:d}, {:d} for {:d}, {:d} with {:d}'.format(x, y, w, h, pixel))
		return -1
	pos = (y + int(pixel / w)) * (SCREEN_WIDTH)
	pos += x + (pixel % w)
	if pos >= (SCREEN_WIDTH * SCREEN_HEIGHT):
		print('error: X{:d}, Y{:d} W{:d}, W{:d} with P{:d} ==> {:d}'.format(x, y, w, h, pixel, pos))
		pos = -1
	return pos


def show_buffer():
	for i in range(SCREEN_HEIGHT):
		for j in range(SCREEN_WIDTH):
			triplet = screen[(i * (SCREEN_WIDTH)) + j]
			p1 = ((triplet >> 6) & 0x3)
			p2 = ((triplet >> 3) & 0x3)
			p3 = (triplet & 0x3)
			print('{}{}{}'.format(PIXIES[p1], PIXIES[p2], PIXIES[p3]), end = '', flush = True)
		print('')


def write_screen(buffer, num):
	# open file
	# write header
	# write data as ascii pixels (with values of 0..3)
	# close file
	outfilename = f'srxe_screen_{RUN_ID}_{num:06d}.pgm'
	print('Creating screen shot {}'.format(outfilename))

	fileout = open(outfilename,'w')
	fileout.write('P2\n')
	fileout.write('# {}\n'.format(outfilename))
	fileout.write('384 136\n')
	fileout.write('3\n')
	for i in range(SCREEN_HEIGHT):
		for j in range(SCREEN_WIDTH):
			triplet = screen[(i * (SCREEN_WIDTH)) + j]
			p1 = 3 - ((triplet >> 6) & 0x3)		# invert colors
			p2 = 3 - ((triplet >> 3) & 0x3)		# invert colors
			p3 = 3 - (triplet & 0x3)			# invert colors
			fileout.write('{:d} {:d} {:d} '.format(p1, p2, p3))
		fileout.write('\n')
	fileout.write('\n')
	fileout.close()



# ---------------------------------------------------------------------
# main()
# ---------------------------------------------------------------------


setup_keyboard_handler()

print('beginning ... ([G]rab current, [Q]uit')
print('FYI: I was too lazy and Python is too f***** up so hit ENTER after a command')

while True:
	key = getkey()
	if (key == 'Q'):
		print('ending ...')
		break
	if (key == 'G'):
		# frame grab
		pixel = 0
		# output the buffer as a PGM file
		if dirty:
			write_screen(screen, screen_counter)
			screen_counter += 1
			dirty = False


	while port.in_waiting:
		ch = port.read()
		ch = ord(ch)

		if special != 0:
			if (special == START_MARK) and (ch == START_MARK):
				##print('active area ?\n')
				# start of an area
				special = 0
				active = True
				pixel = 0
				# get new x, y, w, d values
				ch = port.read()
				x = ord(ch)
				ch = port.read()
				y = ord(ch)
				ch = port.read()
				w = ord(ch)
				ch = port.read()
				h = ord(ch)
				#print('start active area at {:d}, {:d} for {:d}, {:d}'.format(x, y, w, h))
				break

			elif (special == STOP_MARK) and (ch == STOP_MARK):
				# end of an area
				special = 0
				active = False
				pixel = 0
				x = 0
				y = 0
				w = SCREEN_WIDTH - 1
				h = SCREEN_HEIGHT - 1
				#print('end active area')
				#if dirty: show_buffer()
				break

			elif (special == STOP_MARK) and (ch == START_MARK):
				# grap current screen
				special = 0
				active = False
				# output the buffer as a PGM file
				if dirty:
					write_screen(screen, screen_counter)
					screen_counter += 1
					dirty = False
					print('continue with screen ... ([G]rab current, [Q]uit')
				break

			elif (special == START_MARK) and (ch == STOP_MARK):
				# new screen
				special = 0
				active = False
				pixel = 0
				# output the buffer as a PGM file
				if dirty:
					write_screen(screen, screen_counter)
					screen_counter += 1
					dirty = False
					print('beginning new screen ... ([G]rab current, [Q]uit')
				# clear the buffer
				for i in range (SCREEN_WIDTH * SCREEN_HEIGHT):
					screen[i] = 0
				break

			else:
				# the special character must have been just data
				if active:
					pos = compute_position(x, y, w, h, pixel)
					if pos >= 0:
						screen[pos] = special
						pixel += 1
						dirty = True
				special = 0
		# end special character processing

		if (ch == STOP_MARK) or (ch == START_MARK):
			special = ch
		else:
			if active:
				pos = compute_position(x, y, w, h, pixel)
				if pos >= 0:
					screen[pos] = ch
					pixel += 1
					dirty = True
				'''
				triplet = ch
				p1 = (triplet >> 6) & 0x3
				p2 = (triplet >> 3) & 0x3
				p3 = triplet & 0x3

				print('{}{}{}'.format(PIXIES[p1], PIXIES[p2], PIXIES[p3]), end = '', flush = True)
				if not (pixel % (w)):
					print('')
				'''
			else:
				# echo any other messages to the screen
				print('{:c}'.format(ch), end = '', flush = True)


		#print('{:02X}'.format(ch), end = '', flush = True)

if dirty:
	write_screen(screen, screen_counter)
	screen_counter += 1
	dirty = False

print('ended\n')


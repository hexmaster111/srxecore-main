/* ************************************************************************************
 * File:    main.c
 * Date:    2021.08.12
 * Author:  Bradan Lane Studio
 *
 * This content may be redistributed and/or modified as outlined under the MIT License
 *
 * ************************************************************************************/

/* ---

# SMART Response XE Core Library

The SMART Response XE (SRXE) is an existing hardware device from around 2011.
It was used to a greater or lesser extent in schools as a tool for educators
to send questions to students, and the students - each with an SRXE - would
respond. The educator could see who answered, tabular correct / wrong answers, etc.

The basic hardware consists of a device, approximately 120mm * 90mm * 20mm in size.
- It is powered by 4 AAA batteries.
- It has a 384x136 LCD display with 4 levels of greyscale.
- It has a full chicklet keyboard.
- It uses an ATMega128RFA1 chip with 128KB of FLASH.
- Because it uses the ATMega128RFA1, there is also a low power 2.4GHz RF transceiver.
- There is also a 128KB FLASH chip for storing extra data.

While the specification for the RF transceiver suggests a range up to 30m,
real world testing of the SRXE in a device-to-device configuration provides a
reliable range of only 7.5m.

The hardware was short lived. A large number of SRXE devices are on the used market.
When acquiring SRXE hardware, its very common to find old batteries and corrosion.
Used SRXE devices need to be thuroughly cleaned and tested.

_See the example code in [smoketest.h](#smart-response-xe-device-smoketest-and-demo) for a suitable hardware test._

**Why Another Library:** This library was created for a specifc project using multiple existing open source projects.
However, nearly all of those projects were poorly documented and had bugs or undocumented assumptions.

This library integrates content from those other sources. It also adds detailed documentation and a significant example.
Further, the example is a comprehensive hardware smoketest for the SMART Response XE. Given many of the devices may have hidden
corrosion damage, this smoketest is a valuble resource in and of itself. This library also adds helpful UI builder functions
such as menus for the soft keys, and pop-up message boxes. The UI builder functions will likely grow over time.

The SRXEcore library includes support for the optional **Enigma Development Adapter** - a custom PCB to easily program the SRXE device.
The adapter also provides LEDs and access to 4 extra GPIO pads directly accessable through the battery compartment.

![Enigma Development Adapter](https://gitlab.com/bradanlane/srxecore/-/raw/main/docs/pcb_512.png)

_Files related to the **Enigma Development Adapter** are located in the `docs` folder._

_The [font_gen.py](#smart-response-xe-font-generation) and [bitmap_gen.py](#smart-response-xe-bitmaps) tools are documented at the end._

Feel free to jump to a specific section:
[Clock](#clock) [Power](#power) [EEPROM](#eeprom) [FLASH](#flash) [RF Transceiver](#rf) [Keyboard](#keyboard)
[LCD](#lcd) [PrintF](#printf) [UART](#uart) [LEDs](#leds)

Complete Table of Contents:

[TOC]

--------------------------------------------------------------------------
--- */

// #define SHOW_SMOKETEST_DEMO

#ifdef SHOW_SMOKETEST_DEMO
#define SRXECORE_DEBUG
#include "smoketest.h"
#endif

#ifndef SHOW_SMOKETEST_DEMO
#include "simple_kernal/kernal_main.h"

const char *_softkey_menu_names[] = {"<--", NULL, NULL, NULL, NULL, "-->", NULL, NULL, NULL, NULL};

long debug_call_count = 0;

uint8_t debug_event_handler(uint8_t *event)
{
	// kernal_panic("Event handler called with", event, false);

	// check if the event contains a keyPress
	if (event && KERNAL_EVENT_FLAG__KEYPRESS)
	{
		lcdClearScreen();
	}

	debug_call_count++;

	lcdFontSet(FONT2);

	// Print the message
	lcdPutStringAt("Update Flags:", 30, lcdFontHeightGet());

	char buff[10];

	sprintf_(buff, "%08b", *event);
	lcdPutStringAt(buff, 30, lcdFontHeightGet() * 2);
	itoa(debug_call_count, buff, 10);
	lcdPutStringAt(buff, 30, lcdFontHeightGet() * 3);
	sprintf_(buff, "%c", _last_key);
	uiMenu(_softkey_menu_names, NULL, UI_MENU_ROUND_END, false);

	
	*event = 0;
	return 0;
}

#endif
int main()
{

#ifdef SHOW_SMOKETEST_DEMO
	smoketest(true); // true means 'forever'
#endif

#ifndef SHOW_SMOKETEST_DEMO
	Kernal_RegisterKernalEventHandler(debug_event_handler);
	Kernal_SetRefreshTimer(500);

	kernal_main();

#endif
	while (1)
	{
	}
	return 0;
}
#ifndef __PANIC_H__
#define __PANIC_H__
#include "printf.h"

void _reboot(void)
{
    asm volatile("jmp 0");
}

#define HSK_DEBUG

void _panic(const char *sender, const char *msg, int error_code, bool halt)
{
    // Clear the screen
    lcdClearScreen();

    lcdFontSet(FONT2);

    // Print the message
    lcdPutStringAt(sender, 0, 0);
    lcdPutStringAt(msg, 0, lcdFontHeightGet() * 1);

    // Print the error code
    char error_code_str[10];
    itoa(error_code, error_code_str, 10);
    lcdPutStringAt("Error Code:", 0, lcdFontHeightGet() * 2);
    lcdPutStringAt(error_code_str, 0, lcdFontHeightGet() * 3);

    if (halt)
    {

        lcdPutStringAt("Press power to shutdown", 0, lcdFontHeightGet() * 4);
        lcdPutStringAt("Press any other key to reboot", 0, lcdFontHeightGet() * 5);

        // park the processor
        while (true)
        {
            if (powerButtonPressed())
            {

                lcdSleep();
                powerSleep();
                _reboot();
            }

            if (kbdGetKey() != 0)
            {
                _reboot();
            }
        };
    }
    else
    {
        lcdPutStringAt("Press any key to countinue", 0, lcdFontHeightGet() * 4);
        while (kbdGetKey() == 0)
            ;
    }
}

void kernal_panic(const char *msg, int error_code, bool halt)
{
    _panic("Kernal Panic", msg, error_code, halt);
}

void debug_panic(const char *msg, int code, bool halt)
{
#ifdef HSK_DEBUG
    _panic("Debug Panic",msg, code, halt);
#endif
}

#endif
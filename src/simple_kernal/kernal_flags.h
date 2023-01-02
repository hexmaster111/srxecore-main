

#define KERNAL_EVENT_KEYPRESS  0x01 // The user pressed a key, run GetLastPressedKey() to see what it was,
#define KERNAL_EVENT_WAKEUP    0x02   // The device was woken up from sleep, the app should redraw the screen
#define KERNAL_EVENT_SLEEP     0x03    // The device is about to go to sleep, the app should save any data it needs to
#define KERNAL_EVENT_TIMER     0x04    // The timer has expired, the app should do whatever it needs to do on a timer
#define KERNAL_EVENT_BATTERY   0x05  // The battery voltage has changed



#include "_avr_includes.h"
#include "_srxe_includes.h"
#include "panic.h"

#define INACTIVITY_TIMEOUT 600000

#define KEYSCAN_INTERVAL 10
static unsigned long _keyscan_timer;
static unsigned long _last_key_pressed_time;

static unsigned long _timer = 0;
static unsigned long _timer_interval = 0;

#define BATTERY_CHECK_INTERVAL 1000
static unsigned long _battery_check_timer;
uint16_t _last_battery_level = 0;

// Thease flags are used to tell the application what event has occured, the kernal sets the flags
uint8_t kernal_event_flags = 0;
#define KERNAL_EVENT_FLAG__KEYPRESS 0b00000001 // The user pressed a key, run GetLastPressedKey() to see what it was,
#define KERNAL_EVENT_FLAG__WAKEUP 0b00000010   // The device was woken up from sleep, the app should redraw the screen
#define KERNAL_EVENT_FLAG__SLEEP 0b00000100    // The device is about to go to sleep, the app should save any data it needs to
#define KERNAL_EVENT_FLAG__TIMER 0b00001000    // The timer has expired, the app should do whatever it needs to do on a timer
#define KERNAL_EVENT_FLAG__BATTERY 0b00010000  // The battery voltage has changed

uint8_t (*_app_event_handler)(uint8_t *event_flags) = NULL;

//******************************** Kernal Exposed API Functions ********************************

// Returns the last key pressed
uint8_t Kernal_GetLastPressedKey(void)
{
    return _last_key & 0xFF;
}

// This should be used for things that dont require alot of accuracy, its a lazy way to keep pumping the app events
void Kernal_SetRefreshTimer(unsigned long interval_ms)
{
    _timer_interval = interval_ms;
    _timer = clockMillis();
}

// This function is called by the app to tell the kernal what function to call when an event occurs,
// the kernal will call the function when the event occurs
void Kernal_RegisterKernalEventHandler(uint8_t (*app_event_handler)(uint8_t *event_flags))
{
    _app_event_handler = app_event_handler;
    if (_app_event_handler == NULL)
        kernal_panic("Event handler got set to null", 2, true);
}

//******************************** Internal Functions ********************************

int _do_event_handling(void)
{
    int error_code = 0;

    if (kernal_event_flags == 0)
        return 0; // no events to handles

    if (_app_event_handler != NULL)
        error_code = _app_event_handler(&kernal_event_flags);
    else
        kernal_panic("No app event registered", 1, true);

    return error_code;
}

void _kernal_init(void)
{
    clockInit();
    powerInit();
    kbdInit();
    lcdInit();
    _keyscan_timer = clockMillis();
}

void _do_appsafe_sleep()
{
    // We are about to sleep, let the app know
    kernal_event_flags |= KERNAL_EVENT_FLAG__SLEEP;
    _do_event_handling();
    lcdSleep();
    // sleeping now
    powerSleep();
    // We are now awake
    lcdWake();
    _last_key_pressed_time = _keyscan_timer = clockMillis();
    kernal_event_flags |= KERNAL_EVENT_FLAG__WAKEUP;
}

int _handle_keypress_checks(void)
{

    // Power button handling
    // TODO: This should be its own ~thing~, something to manage the enabled systems and their power states
    if (powerButtonPressed())
    {
        _do_appsafe_sleep();
    }

    // Keyscan handling
    if (clockMillis() >= _keyscan_timer)
    {
        _keyscan_timer = clockMillis() + KEYSCAN_INTERVAL;

        uint16_t key = kbdGetKey();
        if (key != KEY_NOP)
        {
            _last_key_pressed_time = clockMillis();

            uint8_t key_code = key & 0xFF;

            // handle kernal actions from a keypress
            switch (key_code)
            {
            case KEY_CONTUP:
                lcdContrastIncrease();
                break;

            case KEY_CONTDN:
                lcdContrastDecrease();
                break;

            default:
                kernal_event_flags |= KERNAL_EVENT_FLAG__KEYPRESS;
                break;
            }
        }
    }

    // Battery voltage handling
    if (clockMillis() >= _battery_check_timer)
    {
        _battery_check_timer = clockMillis() + BATTERY_CHECK_INTERVAL;

        if (powerBatteryLevel() != _last_battery_level)
        {
            kernal_event_flags |= KERNAL_EVENT_FLAG__BATTERY;
            _last_battery_level = powerBatteryLevel();
        }
    }

    // Inactivity timeout handling
    if (clockMillis() >= _last_key_pressed_time + INACTIVITY_TIMEOUT)
    {
        _last_key_pressed_time = clockMillis();
        _do_appsafe_sleep();
    }

    // Generic Timer handling
    if (_timer_interval > 0 && clockMillis() >= _timer)
    {
        _timer = clockMillis() + _timer_interval;
        kernal_event_flags |= KERNAL_EVENT_FLAG__TIMER;
    }

    return 0;
}

int kernal_main(void)
{
    _kernal_init();
    kernal_event_flags |= KERNAL_EVENT_FLAG__WAKEUP;

    int kernal_loop_status = 0;

    while (kernal_loop_status == 0)
    {
        kernal_loop_status = _handle_keypress_checks();
        if (kernal_loop_status != 0)
            break;
        kernal_loop_status = _do_event_handling();
        if (kernal_loop_status != 0)
            break;
    }

    if (kernal_loop_status != 0)
        kernal_panic("Main exited with code", kernal_loop_status, true);

    return kernal_loop_status;
}
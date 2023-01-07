

#include "_avr_includes.h"
#include "_srxe_includes.h"
#include "panic.h"
#include "../queue.h"
#include "kernal_flags.h"

typedef struct
{
    uint8_t error; // non zero if there was an error
    const char *error_message;
} KERNAL_EVENT_HANDLER_RETURN;

typedef struct
{
    uint8_t event_id;
    uint16_t event_data;
} KMSG;

typedef KERNAL_EVENT_HANDLER_RETURN (*KERNAL_EVENT_HANDLER)(KMSG *);

QUEUE_TEMPLATE(KMSG);

Queue_KMSG *_kernal_message_queue;

#define INACTIVITY_TIMEOUT 600000
#define KEYSCAN_INTERVAL 10

static unsigned long _keyscan_timer;
static unsigned long _last_key_pressed_time;

static unsigned long _timer = 0;
static unsigned long _timer_interval = 0;

#define BATTERY_CHECK_INTERVAL 1000
static unsigned long _battery_check_timer;
uint16_t _last_battery_level = 0;

//******************************** Kernal Exposed API Functions ********************************

// This should be used for things that dont require alot of accuracy, its a lazy way to keep pumping the app events
void Kernal_SetRefreshTimer(unsigned long interval_ms)
{
    _timer_interval = interval_ms;
    _timer = clockMillis();
}

KERNAL_EVENT_HANDLER _event_handler = NULL;

bool KernalRegisterEventHandler(KERNAL_EVENT_HANDLER event_handler)
{
    if (_event_handler == NULL)
    {
        _event_handler = event_handler;
        return true;
    }
    return false;
}

//******************************** Internal Functions ********************************

void _kernal_init(void)
{
    _kernal_message_queue = createQueue_KMSG(10);

    clockInit();
    powerInit();
    kbdInit();
    lcdInit();
    _keyscan_timer = clockMillis();

    Enqueue_KMSG(_kernal_message_queue, (KMSG){.event_id = KERNAL_EVENT_WAKEUP});
}

void _do_appsafe_sleep()
{
    Enqueue_KMSG(_kernal_message_queue, (KMSG){.event_id = KERNAL_EVENT_SLEEP});

    // TODO: Wait for some sort of response from the app, or a timeout then sleep fornow crash with not implemented
    kernal_panic("Sleep saving not implemented", 0, false);

    lcdSleep();
    // sleeping now
    powerSleep();
    // We are now awake
    lcdWake();
    _last_key_pressed_time = _keyscan_timer = clockMillis();

    Enqueue_KMSG(_kernal_message_queue, (KMSG){.event_id = KERNAL_EVENT_WAKEUP});
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

            case KEY_MENU10SY:
                //This is gonna be the kernal debug key
            break;

            default:
                Enqueue_KMSG(_kernal_message_queue, (KMSG){.event_id = KERNAL_EVENT_KEYPRESS, .event_data = key});
                break;
            }
        }
    }
    return 0;
}

int _handle_timer_checks(void)
{
    // Generic Timer handling
    if (_timer_interval != 0 && clockMillis() >= _timer)
    {
        _timer = clockMillis() + _timer_interval;
        Enqueue_KMSG(_kernal_message_queue, (KMSG){.event_id = KERNAL_EVENT_TIMER, .event_data = _timer});
    }
    return 0;
}

int _handle_inactivity_timeout(void)
{
    if (clockMillis() >= _last_key_pressed_time + INACTIVITY_TIMEOUT)
    {
        _last_key_pressed_time = clockMillis();
        _do_appsafe_sleep();
    }
    return 0;
}

int _handle_battery_checks(void)
{
    // Battery voltage handling
    if (clockMillis() >= _battery_check_timer)
    {
        _battery_check_timer = clockMillis() + BATTERY_CHECK_INTERVAL;

        if (powerBatteryLevel() != _last_battery_level)
        {
            _last_battery_level = powerBatteryLevel();

            Enqueue_KMSG(_kernal_message_queue, (KMSG){.event_id = KERNAL_EVENT_BATTERY, .event_data = _last_battery_level});
        }
    }
    return 0;
}

void _kernal_check_for_changes(void)
{
    int status = 0;

    status = _handle_keypress_checks();
    if (status != 0)
        kernal_panic("Error in keypress checks", status, true);

    status = _handle_battery_checks();
    if (status != 0)
        kernal_panic("Error in battery checks", status, true);

    status = _handle_inactivity_timeout();
    if (status != 0)
        kernal_panic("Error in inactivity timeout", status, true);

    status = _handle_timer_checks();
    if (status != 0)
        kernal_panic("Error in timer checks", status, true);
}


/// @brief Gets the next message from the kernal, if there is no message avalable, this call will block until there is one
/// @param msg The message to fill
/// @return Continue pumping messages + error codes
bool KernalGetMessage(KMSG *msg)
{

    while (QueueEmpty_KMSG(_kernal_message_queue))
    {
        _kernal_check_for_changes();
    }

    *msg = front_KMSG(_kernal_message_queue);
    if (msg == NULL)
        debug_panic("Kernal message queue is empty but we have elements in it", 0, true);
    Dequeue_KMSG(_kernal_message_queue);

    return true;
}

void DispatchMessage(KMSG *msg)
{
    if (_event_handler == NULL)
        kernal_panic("No event handler registered", 0, true);

    KERNAL_EVENT_HANDLER_RETURN ret = _event_handler(msg);

    if (ret.error != 0)
        kernal_panic("Error in event handler", ret.error, true);
}

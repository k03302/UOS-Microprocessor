#include "common.h"
#include "led.h"
#include <assert.h>

void led_init(void)
{
    LED_DDR = LED_ALL_BITS;
    led_clear();
}

void led_clear(void)
{
    LED_BASE = 0x00;
}

void led_accumulate_print(int value, int start, int end)
{
    assert(start < end);

    if (value >= start)
    {
        LED_BASE = 0xff;
        return;
    }

    if (value <= end)
    {
        LED_BASE = 0x00;
        return;
    }

    int led = 0x00;

    int interval = (end - start) >> 3;
    value -= start;
    while (value > 0)
    {
        value -= interval;
        led = (led << 1) | 0x01;
    }
    LED_BASE = led;
}

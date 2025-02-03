#include "peripherals/led8.h"
#include "common.h"

void led8_init(void)
{
    LED_DDR = LED_ALL_BITS;
    led8_clear();
}

void led8_clear(void)
{
    LED_BASE = 0x00;
}

void led8_accumulate_print(int value, int start, int end)
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

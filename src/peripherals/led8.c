#include "peripherals/led8.h"
#include "common.h"

static enum LED8Owner current_owner = LED8_OWNER_NONE;

void led8_init(void)
{
    LED_DDR = LED_ALL_BITS;
    led8_clear(LED8_OWNER_NONE);
}

int led8_lock(enum LED8Owner owner)
{
    if (current_owner < owner)
    {
        current_owner = owner;
        return 1;
    }
    return 0;
}

void led8_unlock(enum LED8Owner owner)
{
    if (owner == current_owner)
    {
        current_owner = LED8_OWNER_NONE;
    }
}

int led8_clear(enum LED8Owner owner)
{
    if (owner == current_owner)
    {
        LED_BASE = 0x00;
        return 1;
    }
    else
    {
        return 0;
    }
}

int led8_accumulate_print(enum LED8Owner owner, int value, int start, int end)
{
    assert(start < end);
    if (owner != current_owner)
    {
        return 0;
    }

    if (value >= start)
    {
        LED_BASE = 0xff;
        return 0;
    }

    if (value <= end)
    {
        LED_BASE = 0x00;
        return 0;
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

    return 1;
}

#include "peripherals/moodlight.h"
#include "common.h"

void moodlight_init()
{
    RGB_LED_DDR = RGB_LED_ALL_BITS;
    moodlight_set(0);
}

void moodlight_set(int turn_on)
{
    if (turn_on)
    {
        RGB_LED_BASE = RGB_LED_ALL_BITS;
    }
    else
    {
        RGB_LED_BASE = 0x00;
    }
}

void moodlight_toggle()
{
    RGB_LED_BASE ^= RGB_LED_ALL_BITS;
}

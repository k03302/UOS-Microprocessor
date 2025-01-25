#include "rgbled.h"
#include "common.h"

void rgb_led_init()
{
    RGB_LED_DDR = RGB_LED_ALL_BITS;
    rgb_led_set(0);
}

void rgb_led_set(int turn_on)
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

void rgb_led_toggle()
{
    RGB_LED_BASE ^= RGB_LED_ALL_BITS;
}

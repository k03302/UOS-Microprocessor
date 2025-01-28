#include "peripherals/knob.h"
#include "common.h"

void knob_init(void)
{
    ROTARY_ENCODER_DDR = 0x00;
}

static int knob_check_S1(void)
{
    return (ROTARY_ENCODER_BASE & ROTARY_ENCODER_S1_BIT) == 0 ? 1 : 0;
}

static int knob_check_S2(void)
{
    return (ROTARY_ENCODER_BASE & ROTARY_ENCODER_S2_BIT) == 0 ? 1 : 0;
}

static int last_S1 = 0;
static int last_S2 = 0;

enum KnobTurnDirection knob_check(void)
{
    int current_S1 = knob_check_S1();
    int current_S2 = knob_check_S2();
    enum KnobTurnDirection direction = KNOB_NONE;

    if (current_S1 != last_S1)
    {
        if (last_S1 == 1 && current_S1 == 0 && current_S2 == 0)
        {
            direction += KNOB_CLOCKWISE;
        }
    }

    if (current_S2 != last_S2)
    {
        if (last_S2 == 1 && current_S2 == 0 && current_S1 == 0)
        {
            direction += KNOB_COUNTERCLOCKWISE;
        }
    }

    last_S1 = current_S1;
    last_S2 = current_S2;

    return direction;
}

int knob_is_turned(void)
{
    return knob_check() != KNOB_NONE ? 1 : 0;
}
#include "common.h"
#include "system.h"
#include "rgbled.h"
#include "fnd.h"
#include "led.h"
#include "knob.h"
#include "adc_ctrl.h"
#include "utils.h"

static int system_attributes[SYSTEM_ATTRIBUTE_END];

void system_init()
{
    led_init();
    timer_init();
    fnd_init();
    adc_init(ADC_CHANNEL_CDS);
    knob_init();
    sei();

    system_attributes[SOUND_THRESHOLD] = 800;
    system_attributes[SOUND_THRESHOLD_MIN] = 500;
    system_attributes[SOUND_THRESHOLD_MAX] = 1000;
    system_attributes[SOUND_THRESHOLD_ADJUST_AMOUNT] = 10;
    system_attributes[CLAP_MIN_DURATION] = 10;
    system_attributes[CLAP_MAX_DURATION] = 30;
    system_attributes[CLAP_MIN_GAP] = 200;
    system_attributes[CLAP_MAX_GAP] = 1000;
    system_attributes[CLAP_TOGGLE_WAIT] = 1000;
    system_attributes[FND_NEXT_DIGIT_PERIOD] = 1;
    system_attributes[FND_UPDATE_PERIOD] = 100;
    system_attributes[FND_UPDATE_TIMEOUT] = 2000;
    system_attributes[ADC_CHANGE_TIMEOUT] = 10;
    system_attributes[CDS_NIGHT_THRESHOLD] = 600;
    system_attributes[CDS_DAY_THRESHOLD] = 900;
    system_attributes[CDS_CHECK_PERIOD] = 1000;
}

int system_get_attribute(enum SystemAttribute attr)
{
    assert(attr >= 0 && attr < SYSTEM_ATTRIBUTE_END);
    return system_attributes[attr];
}

int system_set_attribute(enum SystemAttribute attr, int value)
{
    if (attr == SOUND_THRESHOLD)
    {
        system_attributes[SOUND_THRESHOLD] = clamp(value, system_attributes[SOUND_THRESHOLD_MIN], system_attributes[SOUND_THRESHOLD_MAX]);
        return 1;
    }
    return 0;
}
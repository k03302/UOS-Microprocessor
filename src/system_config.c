#include "system_config.h"
#include "utils/math.h"

static int system_attributes[SA_SYSTEM_ATTRIBUTE_END];

void system_init_config()
{
    system_attributes[SA_SOUND_THRESHOLD] = 800;
    system_attributes[SA_SOUND_THRESHOLD_MIN] = 500;
    system_attributes[SA_SOUND_THRESHOLD_MAX] = 1000;
    system_attributes[SA_SOUND_THRESHOLD_ADJUST_AMOUNT] = 10;
    system_attributes[SA_CLAP_MIN_DURATION] = 10;
    system_attributes[SA_CLAP_MAX_DURATION] = 30;
    system_attributes[SA_CLAP_MIN_GAP] = 200;
    system_attributes[SA_CLAP_MAX_GAP] = 1000;
    system_attributes[SA_CLAP_TOGGLE_WAIT] = 1000;
    system_attributes[SA_CLAP_CALMDOWN_WAIT] = 500;
    system_attributes[SA_FND_NEXT_DIGIT_PERIOD] = 1;
    system_attributes[SA_FND_UPDATE_PERIOD] = 100;
    system_attributes[SA_FND_UPDATE_TIMEOUT] = 2000;
    system_attributes[SA_ADC_CHANGE_TIMEOUT] = 10;
    system_attributes[SA_CDS_NIGHT_THRESHOLD] = 600;
    system_attributes[SA_CDS_DAY_THRESHOLD] = 900;
    system_attributes[SA_CDS_CHECK_PERIOD] = 1000;
}

int system_get_attribute(enum SystemAttribute attr)
{
    assert(attr >= 0 && attr < SA_SYSTEM_ATTRIBUTE_END);
    return system_attributes[attr];
}

int system_set_attribute(enum SystemAttribute attr, int value)
{
    if (attr == SA_SOUND_THRESHOLD)
    {
        system_attributes[SA_SOUND_THRESHOLD] = clamp(value, system_attributes[SA_SOUND_THRESHOLD_MIN], system_attributes[SA_SOUND_THRESHOLD_MAX]);
        return 1;
    }
    return 0;
}
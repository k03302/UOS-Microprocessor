#include "lamp_sm.h"
#include "clap_sm.h"
#include "system.h"
#include "adc_ctrl.h"
#include "rgbled.h"
#include "common.h"

enum LampState
{
    LAMP_DAY,
    LAMP_NIGHT,
    LAMP_CHECK_SOUND,
    LAMP_ADC_CHANGE_SOUND,
    LAMP_ADC_CHANGE_LIGHT,
    LAMP_WAIT
};

/*
램프 상태머신 관련 변수
*/
static enum LampState lamp_mode = LAMP_DAY;
static long long adc_change_start_timestamp = 0;
static long long sound_adc_start_timestamp = 0;
static long long clap_toggle_timestamp = 0;

void lamp_initialize()
{
    lamp_mode = LAMP_DAY;
    adc_change_start_timestamp = 0;
    sound_adc_start_timestamp = 0;
    clap_toggle_timestamp = 0;
}

void lamp_state_machine()
{
    int light_value_realtime;

    switch (lamp_mode)
    {

    case LAMP_DAY:
        light_value_realtime = adc_read(ADC_CHANNEL_CDS);
        if (light_value_realtime < system_get_attribute(CDS_NIGHT_THRESHOLD))
        {
            rgb_led_set(1);
            lamp_mode = LAMP_NIGHT;
        }
        break;

    case LAMP_NIGHT:
        light_value_realtime = adc_read(ADC_CHANNEL_CDS);
        if (light_value_realtime > system_get_attribute(CDS_DAY_THRESHOLD))
        {
            rgb_led_set(0);
            lamp_mode = LAMP_DAY;
        }
        else
        {
            adc_change_start_timestamp = timer_get_time();
            adc_init(2);
            lamp_mode = LAMP_ADC_CHANGE_SOUND;
        }
        break;

    case LAMP_ADC_CHANGE_SOUND:
    case LAMP_ADC_CHANGE_LIGHT:
        if (timer_get_time() - adc_change_start_timestamp > system_get_attribute(ADC_CHANGE_TIMEOUT))
        {
            lamp_mode = (lamp_mode == LAMP_ADC_CHANGE_SOUND) ? LAMP_CHECK_SOUND : LAMP_NIGHT;
            sound_adc_start_timestamp = timer_get_time();
        }
        break;

    case LAMP_WAIT:
        if (timer_get_time() - clap_toggle_timestamp > system_get_attribute(CLAP_TOGGLE_WAIT))
        {
            lamp_mode = LAMP_CHECK_SOUND;
        }
        break;

    case LAMP_CHECK_SOUND:
        clap_state_machine();
        if (timer_get_time() - sound_adc_start_timestamp > system_get_attribute(CDS_CHECK_PERIOD))
        {
            adc_change_start_timestamp = timer_get_time();
            adc_init(0);
            lamp_mode = LAMP_ADC_CHANGE_LIGHT;
        }
        else if (clap_is_finished())
        {
            clap_initialize();
            rgb_led_toggle();

            lamp_mode = LAMP_WAIT;
            clap_toggle_timestamp = timer_get_time();
        }
        break;
    }
}
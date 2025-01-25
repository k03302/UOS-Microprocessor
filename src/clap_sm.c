#include "clap_sm.h"
#include "common.h"
#include "system_config.h"
#include "adc_ctrl.h"
#include "led.h"

enum ClapState
{
    CLAP_START,
    CLAP_FIRST_RISE,
    CLAP_FIRST_DROP,
    CLAP_SECOND_RISE,
    CLAP_SECOND_DROP
};

/*
박수 상태머신 관련 변수
*/
static enum ClapState clap_state = CLAP_START;
static long long clap_start_timestamp = 0;
static long long clap_end_timestamp = 0;

int clap_state_machine_finished()
{
    return clap_state == CLAP_SECOND_DROP;
}

void clap_state_machine_initialize()
{
    clap_state = CLAP_START;

    long long current_time = timer_get_time();
    clap_start_timestamp = current_time;
    clap_end_timestamp = current_time;
}

void clap_state_machine()
{
    assert(adc_get_current_channel() == ADC_CHANNEL_SOUND);

    int sound_value_realtime = adc_read(ADC_CHANNEL_SOUND);
    int sound_threshold = system_get_attribute(SOUND_THRESHOLD);

    switch (clap_state)
    {

    case CLAP_START:
        if (sound_value_realtime > sound_threshold)
        {
            clap_start_timestamp = timer_get_time();
            clap_state = CLAP_FIRST_RISE;
        }
        break;

    case CLAP_FIRST_RISE:
    case CLAP_SECOND_RISE:
        if (sound_value_realtime < sound_threshold)
        {
            clap_end_timestamp = timer_get_time();
            int clap_duration = clap_end_timestamp - clap_start_timestamp;

            if (clap_duration < system_get_attribute(CLAP_MIN_DURATION))
            {
            }
            else if (clap_duration > system_get_attribute(CLAP_MAX_DURATION))
            {
                clap_state = CLAP_START;
            }
            else
            {
                clap_state = (clap_state == CLAP_FIRST_RISE) ? CLAP_FIRST_DROP : CLAP_SECOND_DROP;
            }
        }
        break;

    case CLAP_FIRST_DROP:
        if (sound_value_realtime > sound_threshold)
        {
            clap_start_timestamp = timer_get_time();
            int clap_gap = clap_start_timestamp - clap_end_timestamp;

            if (clap_gap < system_get_attribute(CLAP_MIN_GAP))
            {
            }
            else if (clap_gap > system_get_attribute(CLAP_MAX_GAP))
            {
                clap_state = CLAP_FIRST_RISE;
            }
            else
            {
                clap_state = CLAP_SECOND_RISE;
            }
        }
        break;

    case CLAP_SECOND_DROP:
        led_clear();
        break;
    }
}
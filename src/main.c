#include "common.h"
#include "rgbled.h"
#include "fnd.h"
#include "led.h"
#include "knob.h"
#include "adc.h"
#include "utils.h"
#include "system.h"

enum SystemState
{
    SYSTEM_RUN,
    SYSTEM_SET
};

enum LampState
{
    LAMP_DAY,
    LAMP_NIGHT,
    LAMP_CHECK_SOUND,
    LAMP_ADC_CHANGE_SOUND,
    LAMP_ADC_CHANGE_LIGHT,
    LAMP_WAIT
};

enum ClapState
{
    CLAP_START,
    CLAP_FIRST_RISE,
    CLAP_FIRST_DROP,
    CLAP_SECOND_RISE,
    CLAP_SECOND_DROP
};

/*
시스템 상태머신 관련 변수
*/
enum SystemState system_mode = SYSTEM_RUN;
int sound_threshold = 800;
int sound_threshold_display = 800;
long long fnd_update_timestamp = 0;
long long led_indicator_toggle_timestamp = 0;
long long fnd_print_update_timestamp = 0;
long long threshold_update_timestamp = 0;

/*
램프 상태머신 관련 변수
*/
enum LampState lamp_mode = LAMP_DAY;
long long adc_change_start_timestamp = 0;
long long sound_adc_start_timestamp = 0;
long long clap_toggle_timestamp = 0;

/*
박수 상태머신 관련 변수
*/
enum ClapState clap_state = CLAP_START;
long long clap_start_timestamp = 0;
long long clap_end_timestamp = 0;

/*
    박수 상태 머신은 현재 아날라고 입력이 sound(ADC2)를 이용하고 있음을 가정한다.
*/
void clap_state_machine()
{
    int sound_value_realtime = adc_read(ADC_CHANNEL_SOUND);

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
        else if (clap_state == CLAP_SECOND_DROP)
        {
            clap_state = CLAP_START;
            rgb_led_toggle();

            lamp_mode = LAMP_WAIT;
            clap_toggle_timestamp = timer_get_time();
        }
        break;
    }
}

void system_state_machine()
{
    enum KnobTurnDirection turn_direction = knob_check();

    switch (system_mode)
    {
    case SYSTEM_RUN:
        lamp_state_machine();

        if (turn_direction != NONE)
        {
            system_mode = SYSTEM_SET;

            threshold_update_timestamp = timer_get_time();
            fnd_update_timestamp = timer_get_time();
        }
        break;

    case SYSTEM_SET:
        led_accumulate_print(sound_threshold_display, system_get_attribute(SOUND_THRESHOLD_MIN), system_get_attribute(SOUND_THRESHOLD_MAX));

        fnd_set_print_value(sound_threshold_display);

        // fnd에 포시할 숫자를 일정 주기마다 업데이트
        if (timer_get_time() - fnd_update_timestamp > system_get_attribute(FND_UPDATE_PERIOD))
        {
            fnd_update_timestamp = timer_get_time();
            sound_threshold_display = sound_threshold;
        }

        // 역치 조절을 한 후 시간이 경과했을 때 SYSTEM_RUN으로 변경
        if (timer_get_time() - threshold_update_timestamp > system_get_attribute(FND_UPDATE_TIMEOUT))
        {
            fnd_clear();
            led_clear();
            system_mode = SYSTEM_RUN;
        }

        if (turn_direction == CLOCKWISE)
        {
            threshold_update_timestamp = timer_get_time();

            sound_threshold = clamp(sound_threshold + system_get_attribute(SOUND_THRESHOLD_ADJUST_AMOUNT), system_get_attribute(SOUND_THRESHOLD_MIN), system_get_attribute(SOUND_THRESHOLD_MAX));
        }
        else if (turn_direction == COUNTERCLOCKWISE)
        {
            threshold_update_timestamp = timer_get_time();

            sound_threshold = clamp(sound_threshold - system_get_attribute(SOUND_THRESHOLD_ADJUST_AMOUNT), system_get_attribute(SOUND_THRESHOLD_MIN), system_get_attribute(SOUND_THRESHOLD_MAX));
        }

        break;
    }
}

int main()
{
    system_init();
    while (1)
    {
        system_state_machine();
    }
}
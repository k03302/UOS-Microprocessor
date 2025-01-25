#include "common.h"
#include "rgbled.h"
#include "fnd.h"
#include "led.h"
#include "knob.h"
#include "adc_ctrl.h"
#include "utils.h"
#include "system.h"
#include "clap_sm.h"

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

/*
시스템 상태머신 관련 변수
*/
enum SystemState system_mode = SYSTEM_RUN;
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

void system_state_machine()
{
    enum KnobTurnDirection turn_direction = knob_check();
    int sound_threshold;

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
        sound_threshold = system_get_attribute(SOUND_THRESHOLD);

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

        if (turn_direction != NONE)
        {
            if (turn_direction == CLOCKWISE)
            {
                threshold_update_timestamp = timer_get_time();

                system_set_attribute(SOUND_THRESHOLD,
                                     clamp(sound_threshold + system_get_attribute(SOUND_THRESHOLD_ADJUST_AMOUNT), system_get_attribute(SOUND_THRESHOLD_MIN), system_get_attribute(SOUND_THRESHOLD_MAX)));
            }
            else if (turn_direction == COUNTERCLOCKWISE)
            {
                threshold_update_timestamp = timer_get_time();

                system_set_attribute(SOUND_THRESHOLD,
                                     clamp(sound_threshold - system_get_attribute(SOUND_THRESHOLD_ADJUST_AMOUNT), system_get_attribute(SOUND_THRESHOLD_MIN), system_get_attribute(SOUND_THRESHOLD_MAX)));
            }
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
#include "common.h"
#include "rgbled.h"
#include "fnd.h"
#include "led.h"
#include "knob.h"
#include "adc_ctrl.h"
#include "utils.h"
#include "system.h"
#include "lamp_sm.h"

enum SystemState
{
    SYSTEM_RUN,
    SYSTEM_SET
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
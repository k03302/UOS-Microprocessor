#include "system_sm.h"
#include "common.h"
#include "system_config.h"
#include "knob.h"
#include "lamp_sm.h"
#include "led.h"
#include "fnd.h"
#include "adc_ctrl.h"

enum SystemState
{
    SYSTEM_RUN,
    SYSTEM_SET
};

static enum SystemState system_mode = SYSTEM_RUN;
static int sound_threshold_display = 0;
static struct watch fnd_update_watch;
static struct watch threshold_update_watch;
static int initialize_done = 0;

void system_state_machine_initialize()
{
    system_mode = SYSTEM_RUN;

    timer_get_watch(&fnd_update_watch, system_get_attribute(SA_FND_UPDATE_PERIOD));
    timer_get_watch(&threshold_update_watch, system_get_attribute(SA_FND_UPDATE_TIMEOUT));

    timer_init();
    led_init();
    fnd_init();
    knob_init();
    sei();

    system_init_config();
    lamp_state_machine_initialize();

    sound_threshold_display = system_get_attribute(SA_SOUND_THRESHOLD);
    initialize_done = 1;
}

void system_state_machine()
{
    assert(initialize_done);
    enum KnobTurnDirection turn_direction = knob_check();
    int sound_threshold;
    int adjust_amount;

    switch (system_mode)
    {
    case SYSTEM_RUN:
        lamp_state_machine();

        if (turn_direction != KNOB_NONE)
        {
            system_mode = SYSTEM_SET;

            timer_update_watch(&threshold_update_watch);
            timer_update_watch(&fnd_update_watch);
        }
        break;

    case SYSTEM_SET:
        sound_threshold = system_get_attribute(SA_SOUND_THRESHOLD);

        led_accumulate_print(sound_threshold_display, system_get_attribute(SA_SOUND_THRESHOLD_MIN), system_get_attribute(SA_SOUND_THRESHOLD_MAX));

        fnd_set_print_value(sound_threshold_display);

        // fnd에 포시할 숫자를 일정 주기마다 업데이트
        if (timer_check_update_watch(&fnd_update_watch))
        {
            sound_threshold_display = sound_threshold;
        }

        // 역치 조절을 한 후 시간이 경과했을 때 SYSTEM_RUN으로 변경
        if (timer_check_watch(&threshold_update_watch))
        {
            fnd_clear();
            led_clear();
            system_mode = SYSTEM_RUN;
            break;
        }

        // knob이 돌려졌을 때 역치 조절
        if (turn_direction != KNOB_NONE)
        {
            timer_update_watch(&threshold_update_watch);
            adjust_amount = system_get_attribute(SA_SOUND_THRESHOLD_ADJUST_AMOUNT);
            if (turn_direction == KNOB_CLOCKWISE)
            {
                system_set_attribute(SA_SOUND_THRESHOLD,
                                     min(sound_threshold + adjust_amount, system_get_attribute(SA_SOUND_THRESHOLD_MAX)));
            }
            else if (turn_direction == KNOB_COUNTERCLOCKWISE)
            {
                system_set_attribute(SA_SOUND_THRESHOLD,
                                     max(sound_threshold - adjust_amount, system_get_attribute(SA_SOUND_THRESHOLD_MIN)));
            }
        }

        break;
    }
}
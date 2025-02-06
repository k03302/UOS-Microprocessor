#include "state_machines/system_sm.h"
#include "common.h"
#include "system_config.h"
#include "peripherals/knob.h"
#include "peripherals/fnd.h"
#include "peripherals/timer1.h"
#include "peripherals/led8.h"
#include "utils/watch.h"

enum SystemState
{
    SYSTEM_IDLE,
    SYSTEM_SET
};

static enum SystemState system_mode = SYSTEM_IDLE;
static int sound_threshold_display = 0;
static int sound_threshold_min = 0;
static int sound_threshold_max = 1024;
static int sound_adjust_amount = 5;
static struct watch fnd_update_watch;
static struct watch threshold_update_watch;
static int initialize_done = 0;

void system_state_machine_initialize()
{
    system_mode = SYSTEM_IDLE;

    watch_init(&fnd_update_watch, system_get_attribute(SA_FND_UPDATE_PERIOD));
    watch_init(&threshold_update_watch, system_get_attribute(SA_FND_UPDATE_TIMEOUT));

    led8_init();
    fnd_init();
    knob_init();
    sei();

    system_init_config();

    sound_threshold_display = system_get_attribute(SA_SOUND_THRESHOLD);
    sound_threshold_min = system_get_attribute(SA_SOUND_THRESHOLD_MIN);
    sound_threshold_max = system_get_attribute(SA_SOUND_THRESHOLD_MAX);
    sound_adjust_amount = system_get_attribute(SA_SOUND_THRESHOLD_ADJUST_AMOUNT);
    initialize_done = 1;
}

void system_state_machine()
{
    assert(initialize_done);
    enum KnobTurnDirection turn_direction = knob_check();
    int sound_threshold;

    switch (system_mode)
    {
    case SYSTEM_IDLE:

        if (turn_direction != KNOB_NONE)
        {
            system_mode = SYSTEM_SET;

            fnd_start();
            led8_lock(LED8_OWNER_SYSTEM_SM);
            watch_start(&threshold_update_watch);
            watch_start(&fnd_update_watch);
        }
        break;

    case SYSTEM_SET:
        sound_threshold = system_get_attribute(SA_SOUND_THRESHOLD);

        led8_accumulate_print(LED8_OWNER_SYSTEM_SM, sound_threshold_display, sound_threshold_min, sound_threshold_max);

        fnd_set_print_value(sound_threshold_display);

        // fnd에 포시할 숫자를 일정 주기마다 업데이트
        if (watch_check_restart(&fnd_update_watch))
        {
            sound_threshold_display = sound_threshold;
        }

        // 역치 조절을 한 후 시간이 경과했을 때 SYSTEM_RUN으로 변경
        if (watch_check(&threshold_update_watch))
        {
            fnd_end();
            led8_clear(LED8_OWNER_SYSTEM_SM);
            led8_unlock(LED8_OWNER_SYSTEM_SM);
            system_mode = SYSTEM_IDLE;
            break;
        }

        // knob이 돌려졌을 때 역치 조절
        if (turn_direction != KNOB_NONE)
        {
            watch_start(&threshold_update_watch);

            if (turn_direction == KNOB_CLOCKWISE)
            {
                system_set_attribute(SA_SOUND_THRESHOLD,
                                     min(sound_threshold + sound_adjust_amount, sound_threshold_max));
            }
            else if (turn_direction == KNOB_COUNTERCLOCKWISE)
            {
                system_set_attribute(SA_SOUND_THRESHOLD,
                                     max(sound_threshold - sound_adjust_amount, sound_threshold_min));
            }
        }

        break;
    }
}
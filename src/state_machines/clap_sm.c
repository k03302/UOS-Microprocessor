#include "state_machines/clap_sm.h"
#include "common.h"
#include "system_config.h"
#include "peripherals/timer.h"
#include "utils/watch.h"

// 박수의 시작과 끝 타임스탬프를 저장
static long long start_timestamp = 0;
static long long end_timestamp = 0;

// 박수 상태
enum ClapState
{
    CLAP_START,         // 초기 상태
    CLAP_FIRST_TOP,     // 첫 번째로 소리가 역치 초과
    CLAP_FIRST_BOTTOM,  // 첫 번째로 역치 미만으로 하강
    CLAP_SECOND_TOP,    // 두 번째로 소리가 역치 초과
    CLAP_SECOND_BOTTOM, // 두 번째로 역치 미만으로 하강
    CLAP_END,           // 박수가 인식된 종료 상태
    CLAP_STATE_COUNT    // 상태 개수
};

static enum ClapState current_state = CLAP_START;
static int initialize_done = 0;
static int current_sound_value = 0;
static int threshold = 0;

static struct watch clap_clamdown_watch;

// 상태 함수
static void clap_start(void);
static void clap_top_common(void); // CLAP_FIRST_TOP, CLAP_SECOND_TOP에서 상태함수 재사용
static void clap_first_bottom(void);
static void clap_second_bottom(void);
static void clap_end(void);

static StateFuncNoParam state_table[CLAP_STATE_COUNT] = {
    clap_start,
    clap_top_common,
    clap_first_bottom,
    clap_top_common,
    clap_second_bottom,
    clap_end};

int clap_state_machine_finished(void)
{
    return current_state == CLAP_END;
}

void clap_state_machine_initialize(void)
{
    current_state = CLAP_START;
    start_timestamp = 0;
    end_timestamp = 0;
    threshold = system_get_attribute(SA_SOUND_THRESHOLD);
    initialize_done = 1;

    watch_init(&clap_clamdown_watch, system_get_attribute(SA_CLAP_CALMDOWN_WAIT));
}

void clap_state_machine(int sound_value_realtime)
{
    assert(initialize_done);
    current_sound_value = sound_value_realtime;
    state_table[current_state]();
}

// CLAP_START 상태 함수
static void clap_start(void)
{
    assert(current_state == CLAP_START);

    // 소리가 역치 초과
    if (current_sound_value > threshold)
    {
        start_timestamp = timer_get_tick();
        current_state = CLAP_FIRST_TOP;
    }
}

// CLAP_FIRST_TOP, CLAP_SECOND_TOP 상태 함수
static void clap_top_common(void)
{
    assert(current_state == CLAP_FIRST_TOP || current_state == CLAP_SECOND_TOP);

    // 소리가 역치 미만으로 하강
    if (current_sound_value < threshold)
    {
        end_timestamp = timer_get_tick();
        int duration = end_timestamp - start_timestamp;

        if (duration < system_get_attribute(SA_CLAP_MIN_DURATION))
        {
            // 박수 스파이크가 너무 짧게 지속됨. 무시
        }
        else if (duration > system_get_attribute(SA_CLAP_MAX_DURATION))
        {
            // 박수 스파이크가 너무 길게 지속됨. 박수 아님 판정. 시작 상태로 복귀
            current_state = CLAP_START;
        }
        // 정상적인 박수 스파이크 인식
        else
        {
            if (current_state == CLAP_FIRST_TOP)
            {
                current_state = CLAP_FIRST_BOTTOM;
            }
            else
            {
                current_state = CLAP_SECOND_BOTTOM;
                watch_start(&clap_clamdown_watch);
            }
        }
    }
}

// CLAP_FIRST_BOTTOM 상태 함수
static void clap_first_bottom(void)
{
    assert(current_state == CLAP_FIRST_BOTTOM);

    // 소리가 역치 초과
    if (current_sound_value > threshold)
    {
        start_timestamp = timer_get_tick();
        int gap = start_timestamp - end_timestamp;

        if (gap < system_get_attribute(SA_CLAP_MIN_GAP))
        {
            // 너무 짧은 박수 간 간격. 무시
        }
        else if (gap > system_get_attribute(SA_CLAP_MAX_GAP))
        {
            // 너무 긴 박수 간 간격. 첫 번째 박수 스파이크로 간주
            current_state = CLAP_FIRST_TOP;
        }
        else
        {
            // 정상적인 첫 번째 박수 간 간격 인식
            current_state = CLAP_SECOND_TOP;
        }
    }
}

// CLAP_SECOND_BOTTOM 상태 함수
static void clap_second_bottom(void)
{
    assert(current_state == CLAP_SECOND_BOTTOM);

    if (watch_check(&clap_clamdown_watch))
    {
        current_state = CLAP_END;
    }
}

// CLAP_END 상태 함수
static void clap_end(void)
{
    assert(current_state == CLAP_END);
}
#include "clap_sm.h"
#include "common.h"
#include "system_config.h"
#include "adc_ctrl.h"
#include "led.h"

/*
    다음과 같은 패턴의 2회의 박수를 인식한다.
    CLAP_SECOND_BOTTOM 상태가 되면 finished 상태가 된다.
    박수를 인식하는 조건은 다음과 같다.

         /\            /\
        /  \          /  \
       /    \        /    \
------/      \------/      \------

    1. 박수소리가 너무 오래 지속되면 일반 소음으로 간주하여 초기화한다.
    2. 박수소리가 너무 짧게 지속되면 지속시간이 확보될 때까지 무시한다.
    3. 두 번째 박수가 너무 늦게 시작하면 첫 번째 박수로 간주한다.
*/

// 박수의 시작과 끝 타임스탬프를 저장
static long long start_timestamp = 0;
static long long end_timestamp = 0;

// 박수 상태
enum ClapState
{
    CLAP_START,         // 대기 상태
    CLAP_FIRST_TOP,     // 첫 번째로 소리가 역치 초과
    CLAP_FIRST_BOTTOM,  // 첫 번째로 역치 미만으로 하강
    CLAP_SECOND_TOP,    // 두 번째로 소리가 역치 초과
    CLAP_SECOND_BOTTOM, // 두 번째로 역치 미만으로 하강
    CLAP_STATE_COUNT    // 상태 개수
};

static enum ClapState current_state = CLAP_START;

typedef void (*ClapStateFunc)(void);

// 상태 함수
static void state_start(void);
static void state_top_common(void);
static void state_first_bottom(void);
static void state_second_bottom(void);

static ClapStateFunc state_table[CLAP_STATE_COUNT] = {
    state_start,
    state_top_common,
    state_first_bottom,
    state_top_common,
    state_second_bottom};

int clap_state_machine_finished(void)
{
    return current_state == CLAP_SECOND_BOTTOM;
}

void clap_state_machine_initialize(void)
{
    current_state = CLAP_START;
    start_timestamp = 0;
    end_timestamp = 0;
}

void clap_state_machine(void)
{
    assert(adc_get_current_channel() == ADC_CHANNEL_SOUND);
    state_table[current_state]();
}

// STATE_START 상태 함수
static void state_start(void)
{
    assert(current_state == CLAP_START);
    int sound_value = adc_read(ADC_CHANNEL_SOUND);
    int threshold = system_get_attribute(SOUND_THRESHOLD);

    // 소리가 역치 초과
    if (sound_value > threshold)
    {
        start_timestamp = timer_get_time();
        current_state = CLAP_FIRST_TOP;
    }
}

// STATE_FIRST_TOP, STATE_SECOND_TOP 상태 함수
static void state_top_common(void)
{
    assert(current_state == CLAP_FIRST_TOP || current_state == CLAP_SECOND_TOP);
    int sound_value = adc_read(ADC_CHANNEL_SOUND);
    int threshold = system_get_attribute(SOUND_THRESHOLD);

    // 소리가 역치 미만으로 하강
    if (sound_value < threshold)
    {
        end_timestamp = timer_get_time();
        int duration = end_timestamp - start_timestamp;

        if (duration < system_get_attribute(CLAP_MIN_DURATION))
        {
            // 박수 스파이크가 너무 짧게 지속됨. 무시
        }
        else if (duration > system_get_attribute(CLAP_MAX_DURATION))
        {
            // 박수 스파이크가 너무 길게 지속됨. 박수 아님 판정. 시작 상태로 복귀
            current_state = CLAP_START;
        }
        else
        {
            // 정상적인 박수 스파이크 인식
            current_state = (current_state == CLAP_FIRST_TOP) ? CLAP_FIRST_BOTTOM : CLAP_SECOND_BOTTOM;
        }
    }
}

// STATE_FIRST_BOTTOM 상태 함수
static void state_first_bottom(void)
{
    assert(current_state == CLAP_FIRST_BOTTOM);
    int sound_value = adc_read(ADC_CHANNEL_SOUND);
    int threshold = system_get_attribute(SOUND_THRESHOLD);

    // 소리가 역치 초과
    if (sound_value > threshold)
    {
        start_timestamp = timer_get_time();
        int gap = start_timestamp - end_timestamp;

        if (gap < system_get_attribute(CLAP_MIN_GAP))
        {
            // 너무 짧은 박수 간 간격. 무시
        }
        else if (gap > system_get_attribute(CLAP_MAX_GAP))
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

// STATE_SECOND_BOTTOM 상태 함수
static void state_second_bottom(void)
{
    assert(current_state == CLAP_SECOND_BOTTOM);
    led_clear();
}
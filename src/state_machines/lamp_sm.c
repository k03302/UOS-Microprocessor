#include "state_machines/lamp_sm.h"
#include "state_machines/clap_sm.h"
#include "state_machines/day_sm.h"
#include "system_config.h"
#include "peripherals/adc_ctrl.h"
#include "peripherals/moodlight.h"
#include "peripherals/timer1.h"
#include "peripherals/led8.h"
#include "utils/watch.h"
#include "common.h"

// 램프 상태
enum LampState
{
    LAMP_CHECK_LIGHT,      // 조도를 인식하는 상태
    LAMP_CHECK_SOUND,      // 박수 소리를 인식하는 상태
    LAMP_ADC_CHANGE_SOUND, // ADC 채널 변경 후 안정화 대기 상태
    LAMP_ADC_CHANGE_LIGHT, // ADC 채널 변경 후 안정화 대기 상태
    LAMP_STATE_COUNT
};

static enum LampState lamp_mode = LAMP_CHECK_LIGHT;
static struct watch adc_change_watch;
static struct watch sound_check_watch;
static int initialize_done = 0; // 초기화 함수를 호출했는지 여부

// 상태 함수
static void lamp_check_light(void);
static void lamp_check_sound(void);
static void lamp_adc_change_sound(void);
static void lamp_adc_change_light(void);

static StateFuncNoParam state_table[LAMP_STATE_COUNT] = {
    lamp_check_light,
    lamp_check_sound,
    lamp_adc_change_sound,
    lamp_adc_change_light};

void lamp_state_machine_initialize()
{
    initialize_done = 1;
    lamp_mode = LAMP_CHECK_LIGHT;

    watch_init(&adc_change_watch, system_get_attribute(SA_ADC_CHANGE_TIMEOUT));
    watch_init(&sound_check_watch, system_get_attribute(SA_CDS_CHECK_PERIOD));

    adc_init(ADC_CHANNEL_CDS);
    led8_init();
    moodlight_init();
    clap_state_machine_initialize();
    day_state_machine_initialize();
}

void lamp_state_machine()
{
    assert(initialize_done);
    state_table[lamp_mode]();
}

/*
    @brief
    LAMP_ADC_CHANGE_SOUND 상태 함수
    잠시 대기 후 LAMP_CHECK_SOUND 상태로 전환
    ADC 채널 변경 후 안정화 시간 확보
*/
static void lamp_adc_change_sound(void)
{
    if (watch_check(&adc_change_watch))
    {
        watch_start(&sound_check_watch);
        lamp_mode = LAMP_CHECK_SOUND;
    }
}

/*
    @brief
    LAMP_ADC_CHANGE_LIGHT 상태 함수
    잠시 대기 후 LAMP_CHECK_LIGHT 상태로 전환
    ADC 채널 변경 후 안정화 시간 확보
*/
static void lamp_adc_change_light(void)
{
    if (watch_check(&adc_change_watch))
    {
        lamp_mode = LAMP_CHECK_LIGHT;
    }
}

/*
    @brief
    LAMP_CHECK_SOUND 상태 함수
    사운드센서 값을 읽어서 clap_state_machine에 전달
    박수 인식 시 rgb led를 토글 및 빛 감지 시작
    또는 일정 시간 경과 시 빛 감지 시작
*/
static void lamp_check_sound(void)
{
    int sound_value_realtime = adc_read(ADC_CHANNEL_SOUND);
    clap_state_machine(sound_value_realtime);

    if (clap_state_machine_finished())
    {
        clap_state_machine_initialize();
        moodlight_toggle();

        watch_start(&adc_change_watch);
        adc_init(ADC_CHANNEL_CDS);
        lamp_mode = LAMP_ADC_CHANGE_LIGHT;
    }
    else if (watch_check(&sound_check_watch))
    {
        watch_start(&adc_change_watch);
        adc_init(ADC_CHANNEL_CDS);
        lamp_mode = LAMP_ADC_CHANGE_LIGHT;
    }
}

/*
    @brief
    LAMP_CHECK_LIGHT 상태 함수
*/
static void lamp_check_light(void)
{
    int light_value_realtime = adc_read(ADC_CHANNEL_CDS);
    day_state_machine(light_value_realtime);

    adc_init(ADC_CHANNEL_SOUND);
    watch_start(&adc_change_watch);
    lamp_mode = LAMP_ADC_CHANGE_SOUND;
}

// day_state_machine에서 day->night 전환 시 호출되는 콜백 구현
void day_sm_day2night_callback(void)
{
    moodlight_set(1);
}

// day_state_machine에서 night->day 전환 시 호출되는 콜백 구현
void day_sm_night2day_callback(void)
{
    moodlight_set(0);
}
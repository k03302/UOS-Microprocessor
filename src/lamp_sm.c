#include "lamp_sm.h"
#include "clap_sm.h"
#include "system_config.h"
#include "adc_ctrl.h"
#include "rgbled.h"
#include "common.h"

// 램프 상태
enum LampState
{
    LAMP_DAY,              // 낮으로 간주되는 상태
    LAMP_NIGHT,            // 밤으로 간주되는 상태
    LAMP_CHECK_SOUND,      // 박수 소리를 인식하는 상태
    LAMP_ADC_CHANGE_SOUND, // ADC 채널 변경 후 안정화 대기 상태
    LAMP_ADC_CHANGE_LIGHT, // ADC 채널 변경 후 안정화 대기 상태
    LAMP_WAIT,             // 박수 소리를 인식한 후 대기 상태
    LAMP_STATE_COUNT
};

static enum LampState lamp_mode = LAMP_DAY;
static struct watch adc_change_watch;
static struct watch cds_check_watch;
static struct watch clap_toggle_watch;
static int initialize_done = 0; // 초기화 함수를 호출했는지 여부

// 상태 함수
static void state_day(void);
static void state_night(void);
static void state_adc_change_sound(void);
static void state_adc_change_light(void);
static void state_wait(void);
static void state_check_sound(void);

static StateFuncNoParam state_table[LAMP_STATE_COUNT] = {
    state_day,
    state_night,
    state_check_sound,
    state_adc_change_sound,
    state_adc_change_light,
    state_wait};

void lamp_state_machine_initialize()
{
    initialize_done = 1;
    lamp_mode = LAMP_DAY;

    timer_get_watch(&adc_change_watch, system_get_attribute(ADC_CHANGE_TIMEOUT));
    timer_get_watch(&cds_check_watch, system_get_attribute(CDS_CHECK_PERIOD));
    timer_get_watch(&clap_toggle_watch, system_get_attribute(CLAP_TOGGLE_WAIT));

    adc_init(ADC_CHANNEL_CDS);
    rgb_led_init();
    clap_state_machine_initialize();
}

void lamp_state_machine()
{
    assert(initialize_done);
    state_table[lamp_mode]();
}

/*
    @brief
    LAMP_DAY 상태 함수
    조도를 확인하여 역치보다 낮으면 LAMP_NIGHT 상태로 전환
*/
static void state_day(void)
{
    int light_value_realtime = adc_read(ADC_CHANNEL_CDS);
    if (light_value_realtime < system_get_attribute(CDS_NIGHT_THRESHOLD))
    {
        rgb_led_set(1);
        lamp_mode = LAMP_NIGHT;
    }
}

/*
    @brief
    LAMP_NIGHT 상태 함수
    조도를 확인하여 역치보다 높으면 LAMP_DAY 상태로 전환
    역치보다 낮으면 LAMP_ADC_CHANGE_SOUND 상태로 전환
*/
static void state_night(void)
{
    int light_value_realtime = adc_read(ADC_CHANNEL_CDS);
    if (light_value_realtime > system_get_attribute(CDS_DAY_THRESHOLD))
    {
        rgb_led_set(0);
        lamp_mode = LAMP_DAY;
    }
    else
    {
        timer_update_watch(&cds_check_watch);
        adc_init(ADC_CHANNEL_SOUND);
        lamp_mode = LAMP_ADC_CHANGE_SOUND;
    }
}

/*
    @brief
    LAMP_ADC_CHANGE_SOUND 상태 함수
    잠시 대기 후 LAMP_CHECK_SOUND 상태로 전환
    ADC 채널 변경 후 안정화 시간 확보
*/
static void state_adc_change_sound(void)
{
    if (timer_check_update_watch(&adc_change_watch))
    {
        lamp_mode = LAMP_CHECK_SOUND;
    }
}

/*
    @brief
    LAMP_ADC_CHANGE_LIGHT 상태 함수
    잠시 대기 후 LAMP_NIGHT 상태로 전환
    ADC 채널 변경 후 안정화 시간 확보
*/
static void state_adc_change_light(void)
{
    if (timer_check_update_watch(&adc_change_watch))
    {
        lamp_mode = LAMP_NIGHT;
    }
}

/*
    @brief
    LAMP_WAIT 상태 함수
    잠시 대기 후 LAMP_CHECK_SOUND 상태로 전환
    박수소리 2회 인식 후 일정 시간 동안 대기하기 위함
*/
static void state_wait(void)
{
    if (timer_check_watch(&clap_toggle_watch))
    {
        lamp_mode = LAMP_CHECK_SOUND;
    }
}

/*
    @brief
    LAMP_CHECK_SOUND 상태 함수
    박수 소리를 인식하여 LAMP_WAIT 상태로 전환
*/
static void state_check_sound(void)
{
    clap_state_machine();
    if (timer_check_watch(&cds_check_watch))
    {
        timer_update_watch(&adc_change_watch);
        adc_init(ADC_CHANNEL_CDS);
        lamp_mode = LAMP_ADC_CHANGE_LIGHT;
    }
    else if (clap_state_machine_finished())
    {
        clap_state_machine_initialize();
        rgb_led_toggle();

        lamp_mode = LAMP_WAIT;
        timer_update_watch(&clap_toggle_watch);
    }
}
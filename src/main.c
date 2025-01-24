#include "common.h"

/*
광감지센서 상수
ADC0 이용
value: 0 ~ 1023
*/
#define MIN_DAY_LIGHT 600   // 밤이 되기 위한 조도 역치 (0~1023)
#define MAX_NIGHT_LIGHT 900 // 낮이 되기 위한 조도 역치 (0~1023)

/*
사운드센서 상수
ADC2 이용
value: 0 ~ 1023
일상소음: 300~600
근접 박수소리: 800~900
*/
#define MAX_ADJUSTABLE_SOUND 1000 // 최대로 조절할 수 있는 박수소리 역치 (0~1023)
#define MIN_ADJUSTABLE_SOUND 500  // 최소로 조절할 수 있는 박수소리 역치 (0~1023)
#define SOUND_ADJUST_AMOUNT 10    // 한 번에 조절할 수 있는 박수소리 역치 (0~1023)
#define MIN_CLAP_DURATION 10      // 박수 스파이크 지속시간 최소 (msec)
#define MAX_CLAP_DURATION 30      // 박수 스파이크 지속시간 최대 (msec)
#define MIN_CLAP_GAP 200          // 박수 간격 최소(msec)
#define MAX_CLAP_GAP 1000         // 박수 간격 최대(msec)

/*
상태머신 상수
*/
#define FND_NEXT_DIGIT_PERIOD 1       // FND에 다음 digit을 출력하는 주기 (msec)
#define FND_UPDATE_PERIOD 100         // FND에 출력할 수치를 업데이트하는 주기 (msec)
#define THRESHOLD_UPDATE_TIMEOUT 2000 // 박수소리 역치 디스플레이를 끝내는 시간 (msec)
#define ADC_CHANGE_TIMEOUT 10         // ADC 입력원을 변경할 때의 대기시간 (msec)
#define CLAP_TOGGLE_TIMEOUT 1000      // SET모드를 끝내기 위한 대기시간간
#define LIGHT_CHECK_PERIOD 1000       // 조도를 감지하는 주기 (msec)

#define SYSTEM_RUN 0
#define SYSTEM_SET 1

#define LAMP_DAY 0
#define LAMP_NIGHT 1
#define LAMP_CHECK_SOUND 2
#define LAMP_ADC_CHANGE_SOUND 3
#define LAMP_ADC_CHANGE_LIGHT 4
#define LAMP_WAIT 5

#define CLAP_START 0
#define CLAP_FIRST_RISE 1
#define CLAP_FIRST_DROP 2
#define CLAP_SECOND_RISE 3
#define CLAP_SECOND_DROP 4

// 각 숫자를 나타내기 위한 위한 FND 상태
volatile unsigned char fnd_digit[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F};

/*
시스템 상태머신 관련 변수
*/
int system_mode = 0;
int sound_threshold = 800;
int sound_threshold_display = 800;
int last_knob_A = 0;
int last_knob_B = 0;
int fnd_digit_index = 0;
long long fnd_next_digit_timestamp = 0;
long long fnd_update_timestamp = 0;
long long led_indicator_toggle_timestamp = 0;
long long fnd_print_update_timestamp = 0;
long long threshold_update_timestamp = 0;

/*
램프 상태머신 관련 변수
*/
int lamp_mode = 0;
long long adc_change_start_timestamp = 0;
long long sound_adc_start_timestamp = 0;
long long clap_toggle_timestamp = 0;

/*
박수 상태머신 관련 변수수
*/
int clap_state = 0;
long long clap_start_timestamp = 0;
long long clap_end_timestamp = 0;

// 현재 시간 (msec), 타이머 인터럽트에 의해 업데이트
volatile long long timestamp = 0;

// 타이머 인터럽트 함수
ISR(TIMER1_COMPA_vect)
{
    timestamp++;
}

/*
초기화 함수
*/
void led_init();
void adc_init(int channel);
void knob_init();
void timer1_init();
void fnd_init();

/*
유틸리티 함수
*/
int clamp(int value, int min, int max);

unsigned int read_adc();

void fnd_print_one_digit(int value, int digit_index);
void fnd_clear();

int get_led_interval(int start, int end);
void led_accumulate_print(int value, int start, int end);
void led_print_clear();

void set_rgb_led(int turn_on);
void toggle_rgb_led();

int knob_check_A();
int knob_check_B();

/*
상태 머신 함수
*/
void clap_state_machine();   // 박수 상태
void lamp_state_machine();   // 램프 상태
void system_state_machine(); // 시스템 상태태

void led_init()
{
    DDRA = 0xFF;
    DDRB = 0xE0;
}

void adc_init(int channel)
{
    ADMUX = 0x00;
    ADMUX |= 0x0F & channel;

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void knob_init()
{
    DDRE = 0x00;
}

void timer1_init()
{
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
    OCR1A = 25;
    TIMSK |= (1 << OCIE1A);
}

void fnd_init()
{
    DDRC = 0xFF;
    DDRG = 0x0F;
}

int clamp(int value, int min, int max)
{
    if (value > max)
    {
        return max;
    }
    if (value < min)
    {
        return min;
    }
    return value;
}

void fnd_print_one_digit(int value, int digit_index)
{
    unsigned char fnd_value[4] = {
        fnd_digit[value % 10],
        fnd_digit[(value / 10) % 10],
        fnd_digit[(value / 100) % 10],
        fnd_digit[(value / 1000) % 10]};

    PORTC = fnd_value[digit_index];
    PORTG = 0x01 << digit_index;
}

void fnd_clear()
{
    for (int i = 0; i < 4; i++)
    {
        PORTC = 0x00;
        PORTG = 0x0F;
    }
}

void led_print_clear()
{
    PORTA = 0x00;
}

int get_led_interval(int start, int end)
{
    return (end - start) >> 3;
}

void led_accumulate_print(int value, int start, int end)
{
    int led = 0x00;
    int interval = get_led_interval(start, end);
    value -= start;
    while (value > 0)
    {
        value -= interval;
        led <<= 1;
        led |= 1;
    }
    PORTA = led;
}

void set_rgb_led(int turn_on)
{
    if (turn_on)
    {
        PORTB = 0xE0;
    }
    else
    {
        PORTB = 0x00;
    }
}

void toggle_rgb_led()
{
    PORTB ^= 0xE0;
}

unsigned int read_adc()
{
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADIF)) == 0)
        ;
    return ADC;
}

int knob_check_A()
{
    return (PINE & 0x80) == 0 ? 1 : 0;
}

int knob_check_B()
{
    return (PINE & 0x40) == 0 ? 1 : 0;
}

/*
    박수 상태 머신은 현재 아날라고 입력이 sound(ADC2)를 이용하고 있음을 가정한다.
*/
void clap_state_machine()
{
    int sound_value_realtime = read_adc();

    switch (clap_state)
    {

    case CLAP_START:
        if (sound_value_realtime > sound_threshold)
        {
            clap_start_timestamp = timestamp;
            clap_state = CLAP_FIRST_RISE;
        }
        break;

    case CLAP_FIRST_RISE:
    case CLAP_SECOND_RISE:
        if (sound_value_realtime < sound_threshold)
        {
            clap_end_timestamp = timestamp;
            int clap_duration = clap_end_timestamp - clap_start_timestamp;

            if (clap_duration < MIN_CLAP_DURATION)
            {
            }
            else if (clap_duration > MAX_CLAP_DURATION)
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
            clap_start_timestamp = timestamp;
            int clap_gap = clap_start_timestamp - clap_end_timestamp;

            if (clap_gap < MIN_CLAP_GAP)
            {
            }
            else if (clap_gap > MAX_CLAP_GAP)
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
        led_print_clear();
        break;
    }
}

void lamp_state_machine()
{
    int light_value_realtime;

    switch (lamp_mode)
    {

    case LAMP_DAY:
        light_value_realtime = read_adc();
        if (light_value_realtime < MIN_DAY_LIGHT)
        {
            set_rgb_led(1);
            lamp_mode = LAMP_NIGHT;
        }
        break;

    case LAMP_NIGHT:
        light_value_realtime = read_adc();
        if (light_value_realtime > MAX_NIGHT_LIGHT)
        {
            set_rgb_led(0);
            lamp_mode = LAMP_DAY;
        }
        else
        {
            adc_change_start_timestamp = timestamp;
            adc_init(2);
            lamp_mode = LAMP_ADC_CHANGE_SOUND;
        }
        break;

    case LAMP_ADC_CHANGE_SOUND:
    case LAMP_ADC_CHANGE_LIGHT:
        if (timestamp - adc_change_start_timestamp > ADC_CHANGE_TIMEOUT)
        {
            lamp_mode = (lamp_mode == LAMP_ADC_CHANGE_SOUND) ? LAMP_CHECK_SOUND : LAMP_NIGHT;
            sound_adc_start_timestamp = timestamp;
        }
        break;

    case LAMP_WAIT:
        if (timestamp - clap_toggle_timestamp > CLAP_TOGGLE_TIMEOUT)
        {
            lamp_mode = LAMP_CHECK_SOUND;
        }
        break;

    case LAMP_CHECK_SOUND:
        clap_state_machine();
        if (timestamp - sound_adc_start_timestamp > LIGHT_CHECK_PERIOD)
        {
            adc_change_start_timestamp = timestamp;
            adc_init(0);
            lamp_mode = LAMP_ADC_CHANGE_LIGHT;
        }
        else if (clap_state == CLAP_SECOND_DROP)
        {
            clap_state = CLAP_START;
            toggle_rgb_led();

            lamp_mode = LAMP_WAIT;
            clap_toggle_timestamp = timestamp;
        }
        break;
    }
}

void system_state_machine()
{
    int current_knob_A, current_knob_B;

    switch (system_mode)
    {
    case SYSTEM_RUN:
        lamp_state_machine();

        current_knob_A = knob_check_A();
        current_knob_B = knob_check_B();
        if (current_knob_A != last_knob_A || current_knob_B != last_knob_B)
        {
            system_mode = SYSTEM_SET;

            fnd_digit_index = 0;
            threshold_update_timestamp = timestamp;
            fnd_next_digit_timestamp = timestamp;
            fnd_update_timestamp = timestamp;
        }
        last_knob_A = current_knob_A;
        last_knob_B = current_knob_B;
        break;

    case SYSTEM_SET:
        led_accumulate_print(sound_threshold_display, MIN_ADJUSTABLE_SOUND, MAX_ADJUSTABLE_SOUND);

        //
        if (timestamp - fnd_next_digit_timestamp > FND_NEXT_DIGIT_PERIOD)
        {
            fnd_next_digit_timestamp = timestamp;
            fnd_print_one_digit(sound_threshold_display, fnd_digit_index);
            fnd_digit_index = (fnd_digit_index + 1) % 4;
        }

        // fnd에 포시할 숫자를 일정 주기마다 업데이트
        if (timestamp - fnd_update_timestamp > FND_UPDATE_PERIOD)
        {
            fnd_update_timestamp = timestamp;
            sound_threshold_display = sound_threshold;
        }

        // 역치 조절을 한 후 시간이 경과했을 때 SYSTEM_RUN으로 변경
        if (timestamp - threshold_update_timestamp > THRESHOLD_UPDATE_TIMEOUT)
        {
            fnd_clear();
            led_print_clear();
            system_mode = SYSTEM_RUN;
        }

        current_knob_A = knob_check_A();
        current_knob_B = knob_check_B();

        // 로터리 엔코더의 회전 방향이 순방향인지 체크
        if (current_knob_A != last_knob_A)
        {
            threshold_update_timestamp = timestamp;

            if (last_knob_A == 1 && current_knob_A == 0)
            {
                if (current_knob_B == 0)
                {
                    sound_threshold = clamp(sound_threshold + SOUND_ADJUST_AMOUNT, MIN_ADJUSTABLE_SOUND, MAX_ADJUSTABLE_SOUND);
                }
            }
        }

        // 로터리 엔코더의 회전 방향이 역방향인지 체크
        if (current_knob_B != last_knob_B)
        {
            threshold_update_timestamp = timestamp;

            if (last_knob_B == 1 && current_knob_B == 0)
            {
                if (current_knob_A == 0)
                {
                    sound_threshold = clamp(sound_threshold - SOUND_ADJUST_AMOUNT, MIN_ADJUSTABLE_SOUND, MAX_ADJUSTABLE_SOUND);
                }
            }
        }

        last_knob_A = current_knob_A;
        last_knob_B = current_knob_B;

        break;
    }
}

int main()
{
    led_init();
    timer1_init();
    fnd_init();
    adc_init(0);
    knob_init();
    sei();

    while (1)
    {
        system_state_machine();
    }
}
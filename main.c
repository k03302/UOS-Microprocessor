#define F_CPU 1600000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define MIN_CLAP_DURATION 10
#define MAX_CLAP_DURATION 30
#define MIN_CLAP_GAP 200
#define MAX_CLAP_GAP 1000

#define MIN_DAY_LIGHT 600
#define MAX_NIGHT_LIGHT 900

#define MAX_ADJUSTABLE_SOUND 1000
#define MIN_ADJUSTABLE_SOUND 500
#define SOUND_ADJUST_AMOUNT 10

#define FND_NEXT_DIGIT_PERIOD 1
#define FND_UPDATE_PERIOD 100
#define THRESHOLD_UPDATE_TIMEOUT 2000
#define LED_INDICATOR_TOGGLE_PERIOD 1000
#define ADC_CHANGE_TIMEOUT 10
#define CLAP_TOGGLE_TIMEOUT 1000
#define LIGHT_CHECK_PERIOD 1000

volatile unsigned char fnd_digit[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F};

volatile long long timestamp = 0;

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

int lamp_mode = 0;
long long adc_change_start_timestamp = 0;
long long sound_adc_start_timestamp = 0;
long long clap_toggle_timestamp = 0;

int clap_state = 0;
long long clap_start_timestamp = 0;
long long clap_end_timestamp = 0;

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

void fnd_print(int value)
{

    unsigned char fnd_value[4] = {
        fnd_digit[value % 10],
        fnd_digit[(value / 10) % 10],
        fnd_digit[(value / 100) % 10],
        fnd_digit[(value / 1000) % 10]};

    for (int i = 0; i < 4; i++)
    {
        PORTC = fnd_value[i];
        PORTG = 0x01 << i;
        _delay_ms(1);
    }
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

void fnd_turn_off()
{
    for (int i = 0; i < 4; i++)
    {
        PORTC = 0x00;
        PORTG = 0x0F;
    }
}

void led_turn_off()
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

void led_toggle_print(int value, int start, int end)
{
    int interval = get_led_interval(start, end);
    int led_num = value / interval;

    PORTA ^= (1 << led_num);
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


ISR(TIMER1_COMPA_vect)
{
    timestamp++;
}

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

/*
    clap_state_machine works only when adc is for sound.
*/
void clap_state_machine()
{
    int sound_value_realtime = read_adc();
	
    switch (clap_state)
    {

    case 0:
        if (sound_value_realtime > sound_threshold)
        {
            clap_start_timestamp = timestamp;
            clap_state = 1;
        }
        break;

    case 1:
    case 3:
        if (sound_value_realtime < sound_threshold)
        {
            clap_end_timestamp = timestamp;
            int clap_duration = clap_end_timestamp - clap_start_timestamp;

            if (clap_duration < MIN_CLAP_DURATION)
            {
            }
            else if (clap_duration > MAX_CLAP_DURATION)
            {
                clap_state = 0;
            }
            else
            {
                clap_state = (clap_state == 1) ? 2 : 4;
            }
        }
        break;

    case 2:
        if (sound_value_realtime > sound_threshold)
        {
            clap_start_timestamp = timestamp;
            int clap_gap = clap_start_timestamp - clap_end_timestamp;

            if (clap_gap < MIN_CLAP_GAP)
            {
            }
            else if (clap_gap > MAX_CLAP_GAP)
            {
                clap_state = 1;
            }
            else
            {
                clap_state = 3;
            }
        }
        break;

    case 4:
        break;
    }
}

void lamp_state_machine()
{
    int light_value_realtime;

    switch (lamp_mode)
    {

    case 0:
        light_value_realtime = read_adc();
        if (light_value_realtime < MIN_DAY_LIGHT)
        {
            set_rgb_led(1);
            lamp_mode = 1;
        }
        break;

    case 1:
        light_value_realtime = read_adc();
        if (light_value_realtime > MAX_NIGHT_LIGHT)
        {
            set_rgb_led(0);
            lamp_mode = 0;
        }
        else
        {
            adc_change_start_timestamp = timestamp;
            adc_init(2);
            lamp_mode = 2;
        }
        break;

    case 2:
    case 4:
        if (timestamp - adc_change_start_timestamp > ADC_CHANGE_TIMEOUT)
        {
            lamp_mode = (lamp_mode == 2) ? 3 : 1;
            sound_adc_start_timestamp = timestamp;
        }
        break;

    case 5:
        if (timestamp - clap_toggle_timestamp > CLAP_TOGGLE_TIMEOUT)
        {
            lamp_mode = 3;
        }
        break;

    case 3:
        clap_state_machine();
        if (timestamp - sound_adc_start_timestamp > LIGHT_CHECK_PERIOD)
        {
            adc_change_start_timestamp = timestamp;
            adc_init(0);
            lamp_mode = 4;
        }
        else if (clap_state == 4)
        {
            clap_state = 0;
            toggle_rgb_led();

            lamp_mode = 5;
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
    case 0:
        lamp_state_machine();

        current_knob_A = knob_check_A();
        current_knob_B = knob_check_B();
        if (current_knob_A != last_knob_A || current_knob_B != last_knob_B)
        {
            system_mode = 1;

            fnd_digit_index = 0;
            threshold_update_timestamp = timestamp;
            fnd_next_digit_timestamp = timestamp;
            fnd_update_timestamp = timestamp;
            led_indicator_toggle_timestamp = timestamp;
        }
        last_knob_A = current_knob_A;
        last_knob_B = current_knob_B;
        break;

    case 1:
        led_accumulate_print(sound_threshold_display, MIN_ADJUSTABLE_SOUND, MAX_ADJUSTABLE_SOUND);

        if (timestamp - fnd_next_digit_timestamp > FND_NEXT_DIGIT_PERIOD)
        {
            fnd_next_digit_timestamp = timestamp;
            fnd_print_one_digit(sound_threshold_display, fnd_digit_index);
            fnd_digit_index = (fnd_digit_index + 1) % 4;
        }

        if (timestamp - fnd_update_timestamp > FND_UPDATE_PERIOD)
        {
            fnd_update_timestamp = timestamp;
            sound_threshold_display = sound_threshold;
        }

        if (timestamp - led_indicator_toggle_timestamp > LED_INDICATOR_TOGGLE_PERIOD)
        {
            led_indicator_toggle_timestamp = timestamp;
            led_toggle_print(sound_threshold, MIN_ADJUSTABLE_SOUND, MAX_ADJUSTABLE_SOUND);
        }

        if (timestamp - threshold_update_timestamp > THRESHOLD_UPDATE_TIMEOUT)
        {
            fnd_turn_off();
            led_turn_off();
            system_mode = 0;
        }

        current_knob_A = knob_check_A();
        current_knob_B = knob_check_B();
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
        // int sound_value_realtime = read_adc();
        // fnd_print(sound_value_realtime);
        // clap_state_machine(sound_value_realtime);
        // PORTA = clap_state;

        // lamp_state_machine();
        // PORTA = lamp_mode;

        // if (timestamp - fnd_next_digit_timestamp > 0)
        // {
        //     fnd_next_digit_timestamp = timestamp;
        //     fnd_print_one_digit(1234, fnd_digit_index);
        //     fnd_digit_index = (fnd_digit_index + 1) % 4;
        // }

        system_state_machine();
    }
}
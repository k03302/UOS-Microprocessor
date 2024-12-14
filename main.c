#define F_CPU 1600000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define MIN_CLAP_DURATION 1
#define MAX_CLAP_DURATION 3
#define MIN_CLAP_GAP 20
#define MAX_CLAP_GAP 100

#define LED_INDICATOR_TOGGLE_PERIOD 100
#define ADC_CHANGE_TIMEOUT 1
#define CLAP_TOGGLE_TIMEOUT 100
#define LIGHT_CHECK_PERIOD 100
#define MIN_DAY_LIGHT 600
#define MAX_NIGHT_LIGHT 900



volatile unsigned char fnd_digit[10] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66,
	0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

volatile long long timestamp = 0;
volatile int sound_threshold = 800;


volatile int lamp_mode = 0;
long long adc_change_start_timestamp = 0;
long long sound_adc_start_timestamp = 0;
long long clap_toggle_timestamp = 0;
long long led_indicator_toggle_timestamp = 0;
long long fnd_print_update_timestamp = 0;

volatile int clap_state = 0;
long long clap_start_timestamp = 0;
long long clap_end_timestamp = 0;



void fnd_print(int value) {
	unsigned char fnd_value[4] = {
		fnd_digit[value % 10],
		fnd_digit[(value / 10) % 10],
		fnd_digit[(value / 100) % 10],
		fnd_digit[(value / 1000) % 10]
	};

	for (int i = 0; i < 4; i++) {
		PORTC = fnd_value[i];
		PORTG = 0x01 << i;
		_delay_ms(1);
	}
}

void led_accumulate_print(int value, int start, int end) {
	int led = 0x00;
	int interval = (end - start) >> 3;
	value -= start;
	while(value > 0) {
		value -= interval;
		led <<= 1;
		led |= 1;
	}
	PORTA = led;
}

void led_toggle_print(int value, int start, int end) {
	int interval = (end - start) >> 3;
	int led_num = value / interval;

	PORTA ^= (1 << led_num);
}

void set_rgb_led(int turn_on)
{
	if(turn_on)
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


ISR(TIMER1_COMPA_vect) {
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



void timer1_init() {
	TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
	OCR1A = 250;
	TIMSK |= (1 << OCIE1A);
}

void fnd_init() {
	DDRC = 0xFF;
	DDRG = 0x0F;
}


/*
	clap_state_machine works only when adc is for sound.
*/
void clap_state_machine()
{
	int sound_value_realtime = read_adc();
	switch(clap_state)
	{
		
		case 0:
		if(sound_value_realtime > sound_threshold)
		{
			clap_start_timestamp = timestamp;
			clap_state = 1;
		}
		break;
		
		case 1: case 3:
		if(sound_value_realtime < sound_threshold)
		{
			clap_end_timestamp = timestamp;
			int clap_duration = clap_end_timestamp - clap_start_timestamp;
			
			if(clap_duration < MIN_CLAP_DURATION)
			{
				
			}
			else if(clap_duration > MAX_CLAP_DURATION)
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
		if(sound_value_realtime > sound_threshold)
		{
			clap_start_timestamp = timestamp;
			int clap_gap = clap_start_timestamp - clap_end_timestamp;
			
			if(clap_gap < MIN_CLAP_GAP)
			{
				
			}
			else if(clap_gap > MAX_CLAP_GAP)
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
	int adc_change_waited, sound_check_duration, clap_toggle_waited;
	
	
	switch(lamp_mode)
	{
		
	case 0:
		light_value_realtime = read_adc();
		if(light_value_realtime < MIN_DAY_LIGHT)
		{
			set_rgb_led(1);
			lamp_mode = 1;
		}
	break;
	
	case 1:
		light_value_realtime = read_adc();
		if(light_value_realtime > MAX_NIGHT_LIGHT)
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
	
	case 2: case 4:
		adc_change_waited = timestamp - adc_change_start_timestamp;
		if (adc_change_waited > ADC_CHANGE_TIMEOUT)
		{
			lamp_mode = (lamp_mode == 2) ? 3 : 1;
			sound_adc_start_timestamp = timestamp;
		}
	break;
	
	case 5:
		clap_toggle_waited = timestamp - clap_toggle_timestamp;
		if(clap_toggle_waited > CLAP_TOGGLE_TIMEOUT)
		{
			lamp_mode = 3;
		}
	break;
	
	case 3:
		clap_state_machine();
		sound_check_duration = timestamp - sound_adc_start_timestamp;
		if(sound_check_duration > LIGHT_CHECK_PERIOD)
		{
			adc_change_start_timestamp = timestamp;
			adc_init(0);
			lamp_mode = 4;	
		}
		else if(clap_state == 4)
		{
			clap_state = 0;
			toggle_rgb_led();

			lamp_mode = 5;
			clap_toggle_timestamp = timestamp;
		}
	break;
	}
}




int main() {
	led_init();
	timer1_init();
	fnd_init();
	adc_init(0);
	sei();

	while (1) {	
		//int sound_value_realtime = read_adc();
		//fnd_print(sound_value_realtime);
		//clap_state_machine(sound_value_realtime);
		//PORTA = clap_state;
	
		lamp_state_machine();
		PORTA = lamp_mode;
	}
}
#define F_CPU 1600000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char fnd_digit[10] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66,
	0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

volatile long long millisec = 0;
volatile int light_threshold = 300;
volatile int sound_threshold = 800;
volatile int adc_value_realtime = 0;
volatile int adc_value_display = 0;

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

void led_print(int value, int start, int end) {
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


unsigned int read_adc()
{
	ADCSRA |= (1 << ADSC);
	while ((ADCSRA & (1 << ADIF)) == 0)
	;
	return ADC;
}

ISR(TIMER1_COMPA_vect) {
	adc_value_display = read_adc();
	millisec++;
}

void led_init()
{
	DDRA = 0xFF;
}

void adc_init(int channel)
{
	ADMUX = 0x00;
	ADMUX |= 0x0F & channel;
	
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}


void timer1_init() {
	// CTC mode, prescaler 64
	TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
	OCR1A = 2500;
	TIMSK |= (1 << OCIE1A);
}

void fnd_init() {
	DDRC = 0xFF;
	DDRG = 0x0F;
}


int main() {
	led_init();
	timer1_init();
	fnd_init();
	adc_init(2);
	sei();


	while (1) {
		adc_value_realtime = read_adc();
		led_print(adc_value_realtime, 500, 1000);
		fnd_print(adc_value_display);
	}
}
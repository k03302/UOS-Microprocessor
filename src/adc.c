#include "common.h"
#include "adc.h"

static enum AdcChannel current_channel;

void adc_init(enum AdcChannel channel)
{
    ADMUX = 0x00;

    current_channel = channel;

    switch (channel)
    {
    case ADC_CHANNEL_CDS:
        ADMUX |= 0x0F & CDS_BIT;
        break;
    case ADC_CHANNEL_SOUND:
        ADMUX |= 0x0F & SOUND_SENSOR_BIT;
        break;
    }

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

int adc_read(enum AdcChannel channel)
{
    assert(channel == current_channel);

    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADIF)) == 0)
        ;
    return ADC;
}
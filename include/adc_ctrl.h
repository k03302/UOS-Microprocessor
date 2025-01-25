#ifndef ADC_H
#define ADC_H
enum AdcChannel
{
    ADC_CHANNEL_CDS,
    ADC_CHANNEL_SOUND
};

/*
    @brief
    ADC 초기화 함수
*/
void adc_init(enum AdcChannel channel);

/*
    @brief
    ADC 변환 결과를 읽어오는 함수

    @return
    - ADC 변환 결과 값

    @note
    adc channel을 바꾼 뒤 곧바로 호출되면 잘못된 값을 리턴할 수 있습니다.
    adc_init과 adc_read는 일정 시간 간격을 두고 호출해야 합니다.
    시스템 내부에서는 SA_ADC_CHANGE_TIMEOUT 만큼의 시간을 두고 호출합니다.
*/
int adc_read(enum AdcChannel channel);

/*
    @brief
    현재 설정된 ADC 채널을 반환
*/
enum AdcChannel adc_get_current_channel();

#endif
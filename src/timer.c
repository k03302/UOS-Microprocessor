#include <avr/io.h>
#include "timer.h"

static volatile long long timestamp = 0;

// 타이머 인터럽트 함수
ISR(TIMER1_COMPA_vect)
{
    timestamp++;
}

void timer_init(void)
{
    // 타이머1을 CTC 모드로 동작시키고, 시스템 클럭을 64로 나눈 속도로 타이머가 증가하도록 설정
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);

    // 타이머가 0부터 25까지 카운트한 뒤 인터럽트가 발생하도록 설정
    OCR1A = 25;

    // 타이머1의 비교 일치 인터럽트(OCIE1A)를 활성화하는 설정
    TIMSK |= (1 << OCIE1A);
}

unsigned long long timer_get_time(void)
{
    return timestamp;
}

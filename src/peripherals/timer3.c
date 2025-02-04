#include "peripherals/timer3.h"
#include "common.h"

static volatile int occupied = 0; // 타이머를 사용하고 있는지 여부
static volatile int finished = 1; // 타이머가 끝났는지 여부

ISR(TIMER3_COMPA_vect)
{
    finished = 1;

    // TODO
    // 타이머를 멈춤
}

int timer3_start_us(int us)
{
    if (occupied == 1)
    {
        return 0;
    }

    occupied = 1;
    finished = 0;

    // 타이머1을 CTC 모드로 동작시키고, 시스템 클럭을 8로 나눈 속도로 타이머가 증가하도록 설정
    TCCR3B |= (1 << WGM12) | (1 << CS31);

    // 타이머가 0부터 25까지 카운트한 뒤 인터럽트가 발생하도록 설정
    OCR3A = 25;

    // 타이머3의 비교 일치 인터럽트(OCIE1A)를 활성화하는 설정
    TIMSK |= (1 << OCIE3A);
    return 1;
}

int timer3_check(void)
{
    if (finished)
    {
        occupied = 0;
    }
    return occupied;
}
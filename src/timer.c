#include <avr/io.h>
#include <avr/interrupt.h>
#include <assert.h>
#include "timer.h"

#define TIMER_COUNT 10

struct TimerEvent
{
    unsigned long long interval;
    unsigned long long next_trigger_time;
    void (*callback)(void);
    int is_interval; // 1 for interval, 0 for timeout
    int is_active;   // 1 if the timer is active, 0 otherwise
};

static struct TimerEvent timer_events[TIMER_COUNT] = {0};

static volatile long long timestamp = 0;

// 타이머 인터럽트 함수
ISR(TIMER1_COMPA_vect)
{
    timestamp++;
    for (int i = 0; i < TIMER_COUNT; i++)
    {
        if (timer_events[i].is_active && timestamp >= timer_events[i].next_trigger_time && timer_events[i].callback)
        {
            timer_events[i].callback();
            if (timer_events[i].is_interval)
            {
                timer_events[i].next_trigger_time += timer_events[i].interval;
            }
            else
            {
                timer_events[i].is_active = 0; // timeout인 경우 비활성화
            }
        }
    }
}

void timer_get_watch(struct watch *w, unsigned int wait_time)
{
    assert(w != (void *)0);
    w->start_timestamp = timer_get_time();
    w->wait_time = wait_time;
}

int timer_check_watch(struct watch *w)
{
    assert(w != (void *)0);

    if (timer_get_time() - w->start_timestamp >= w->wait_time)
    {
        return 1; // 시간이 경과함
    }
    return 0; // 시간이 경과하지 않음
}

void timer_update_watch(struct watch *w)
{
    assert(w != (void *)0);

    w->start_timestamp = timer_get_time();
}

int timer_check_update_watch(struct watch *w)
{
    assert(w != (void *)0);

    if (timer_get_time() - w->start_timestamp >= w->wait_time)
    {
        w->start_timestamp = timer_get_time();
        return 1; // 시간이 경과함
    }
    return 0; // 시간이 경과하지 않음
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

int timer_set_timeout(unsigned long long interval, void (*callback)(void))
{
    if (interval == 0)
    {
        callback();
        return 1;
    }

    int i;
    for (i = 0; i < TIMER_COUNT; i++)
    {
        if (!timer_events[i].is_active)
        {
            timer_events[i].interval = interval;
            timer_events[i].next_trigger_time = timestamp + interval;
            timer_events[i].callback = callback;
            timer_events[i].is_interval = 0; // timeout
            timer_events[i].is_active = 1;
            return 1; // 성공적으로 등록됨
        }
    }
    return 0;
}

int timer_set_interval(unsigned long long interval, void (*callback)(void))
{
    if (interval == 0)
    {
        return 0;
    }

    int i;
    for (i = 0; i < TIMER_COUNT; i++)
    {
        if (!timer_events[i].is_active)
        {
            timer_events[i].interval = interval;
            timer_events[i].next_trigger_time = timestamp + interval;
            timer_events[i].callback = callback;
            timer_events[i].is_interval = 1; // interval
            timer_events[i].is_active = 1;
            return 1; // 성공적으로 등록됨
        }
    }
    return 0;
}

void timer_clear_interval(void (*callback)(void))
{
    int i;
    for (i = 0; i < TIMER_COUNT; i++)
    {
        if (timer_events[i].callback == callback)
        {
            timer_events[i].is_active = 0;
        }
    }
}

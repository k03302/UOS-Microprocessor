#include "peripherals/timer.h"
#include "common.h"

static volatile unsigned long long tick = 0;
TimerEvent *head = NULL;

// 타이머 인터럽트 함수
ISR(TIMER1_COMPA_vect)
{
    tick++;
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
    return tick;
}

void timer_create_event(struct TimerEvent *event, unsigned int timeout, int is_periodic, void (*callback)(void))
{
    if (event == NULL)
    {
        return;
    }
    event->timeout = timeout;
    event->is_periodic = is_periodic;
    event->callback = callback;
}

int timer_register_handler(TimerEvent *event)
{
    assert(event != NULL && event->callback != NULL);
    if (event->timeout == 0)
    {
        event->callback();
        if (event->is_periodic == 0)
        {
            return 0;
        }
        else
        {
            return -1; // periodic event with interval 0 is impossible. return error code
        }
    }

    event->next_trigger_time = tick + event->timeout;
    event->next = NULL;

    if (head == NULL)
    {
        head = event;
    }
    else if (head->next_trigger_time >= event->next_trigger_time)
    {
        event->next = head;
        head = event;
    }
    else
    {
        TimerEvent *current = head;
        while (current->next != NULL && current->next->next_trigger_time < event->next_trigger_time)
        {
            current = current->next;
        }
        event->next = current->next;
        current->next = event;
    }

    return 0;
}

void timer_unregister_handler(TimerEvent *event)
{
    assert(event != NULL);

    TimerEvent *current = head;
    TimerEvent *prev = NULL;
    while (current != NULL)
    {
        if (current == event)
        {
            if (prev == NULL)
            {
                head = current->next;
            }
            else
            {
                prev->next = current->next;
            }
        }
        else
        {
            prev = current;
        }
        current = current->next;
    }
}

void timer_process_due_events()
{
    TimerEvent *current = head;
    while (current != NULL && current->next_trigger_time <= tick)
    {
        if (current->callback)
        {
            current->callback();
        }

        head = current->next;

        if (current->is_periodic)
        {
            timer_register_handler(current);
        }

        current = current->next;
    }
}
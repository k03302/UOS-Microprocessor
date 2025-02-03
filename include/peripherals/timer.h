#ifndef TIMER_H
#define TIMER_H

/*
    @brief
    타이머 초기화
*/
void timer_init(void);

typedef struct TimerEvent
{
    unsigned long long timeout;
    unsigned long long next_trigger_time;
    void (*callback)(void);
    int is_periodic; // 1 for periodic, 0 for one time
    struct TimerEvent *next;
} TimerEvent;

/*
    @brief
    타임스탬프 반환 (msec)
*/
unsigned long long timer_get_time(void);

/*
    @brief
    새로운 타이머이벤트 생성
*/
void timer_create_event(struct TimerEvent *event, unsigned int timeout, int is_periodic, void (*callback)(void));

/*
    @brief
    interval만큼의 시간이 지난 후 callback 함수 호출

    @return
    성공 시 0, 실패 시 에러 코드
*/
int timer_register_handler(struct TimerEvent *event);

/*
    @brief
    event에 해당하는 모든 등록을 취소
*/
void timer_unregister_handler(struct TimerEvent *event);

/*
    @brief
    timeout이 경과된 모든 이벤트를 실행
    periodic 이벤트는 다시 등록됨
*/
void timer_process_due_events();

#endif // TIMER_H
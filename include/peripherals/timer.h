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

struct watch
{
    unsigned long long start_timestamp;
    unsigned int wait_time;
};

/*
    @brief
    watch 구조체 초기화 및 시작 시간 설정
*/
void timer_get_watch(struct watch *w, unsigned int wait_time);

/*
    @brief
    wait_time만큼 시간이 경과했는지 확인

    @return
    시간이 경과했으면 1, 그렇지 않으면 0
*/
int timer_check_watch(struct watch *w);

/*
    @brief
    start_timestamp를 현재 시간으로 갱신
*/
void timer_update_watch(struct watch *w);

/*
    @brief
    wait_time만큼 시간이 경과했는지 확인
    경과했으면 start_timestamp를 현재 시간으로 갱신
*/
int timer_check_update_watch(struct watch *w);

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
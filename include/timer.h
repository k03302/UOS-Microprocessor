#ifndef TIMER_H
#define TIMER_H

/*
    @brief
    타이머 초기화
*/
void timer_init(void);

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
    interval만큼의 시간이 지난 후 callback 함수 호출

    @return
    성공 시 1, 실패 시 0
*/
int timer_set_timeout(unsigned long long interval, void (*callback)(void));

/*
    @brief
    interval만큼의 시간마다 callback 함수 호출

    @return
    성공 시 1, 실패 시 0
*/
int timer_set_interval(unsigned long long interval, void (*callback)(void));

/*
    @brief
    callback에 대해 timer_set_interval를 취소
*/
void timer_clear_interval(void (*callback)(void));

#endif // TIMER_H
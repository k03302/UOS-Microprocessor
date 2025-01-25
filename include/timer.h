#ifndef TIMER_H
#define TIMER_H

/*
    @brief
    타이머 초기화
*/
void timer_init(void);

/*
    @brief
    타임스탬프 반환 (타임스탬프의 스케일은 내부 상수에 의해 결정)
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
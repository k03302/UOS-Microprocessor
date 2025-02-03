#ifndef WATCH_H
#define WATCH_H

struct watch
{
    unsigned long long start_timestamp;
    unsigned int wait_time;
};

/*
    @brief
    watch 구조체 초기화 및 시작 시간 설정
*/
void watch_init(struct watch *w, unsigned int wait_time);

/*
    @brief
    wait_time만큼 시간이 경과했는지 확인

    @return
    시간이 경과했으면 1, 그렇지 않으면 0
*/
int watch_check(struct watch *w);

/*
    @brief
    start_timestamp를 현재 시간으로 갱신
*/
void watch_start(struct watch *w);

/*
    @brief
    wait_time만큼 시간이 경과했는지 확인
    경과했으면 start_timestamp를 현재 시간으로 갱신
*/
int watch_check_restart(struct watch *w);

#endif
#ifndef TIMER3_H
#define TIMER3_H

/*
    @brief
    timer3을 시작하고 대기시간 설정
    timer3 점유

    @return
    timer3가 점유해있으면 0 아니면 1
*/
int timer3_start_us(int us);

/*
    @brief
    timer3이 대기시간을 초과했는지 판별
    이 함수로 대기시관 초과를 확인했을 시 timer3에 대한 점유가 해제된 후 멈춤

    @return
    timer3이 대기시간을 초과했으면 1 아니면 0
*/
int timer3_check(void);

#endif
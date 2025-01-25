#ifndef KNOB_H
#define KNOB_H

enum KnobTurnDirection
{
    COUNTERCLOCKWISE = -1,
    CLOCKWISE = 1,
    NONE = 0
};

/*
    @brief
    로터리 엔코더 핀 초기화 함수
*/
void knob_init(void);

/*
    @brief
    로터리 엔코더가 마지막 체크 이후 어떤 방향으로 회전했는지 리턴하는 함수

    @return
    - COUNTERCLOCKWISE: 반시계 방향으로 회전
    - CLOCKWISE: 시계 방향으로 회전
    - NONE: 회전하지 않음
*/
enum KnobTurnDirection knob_check(void);

/*
    @brief
    로터리 엔코더가 마지막 체크 이후 회전했는지 여부를 리턴하는 함수

    @return
    - 0: 회전하지 않음
    - 1: 회전함
*/
int knob_is_turned(void);

#endif
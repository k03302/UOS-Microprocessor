#ifndef LED_H
#define LED_H

// led8에 대한 소유권을 나타내는 열거체
// 숫자가 높을수록 소유 권한이 높음
enum LED8Owner
{
    LED8_OWNER_NONE = 0,
    LED8_OWNER_CLAP_SM = 1,
    LED8_OWNER_SYSTEM_SM = 2
};

/*
    @brief
    LED 초기화 함수
*/
void led8_init(void);

/*
    @brief
    led8에 대한 소유권을 주장

    @return
    소유권을 얻었으면 1, 실패했으면 0
*/
int led8_lock(enum LED8Owner owner);

/*
    @brief
    led8에 대한 소유권을 포기
*/
void led8_unlock(enum LED8Owner owner);

/*
    @brief
    LED 를 모두 끔

    @param
    owner: 소유자

    @return
    성공 시 1, 실패 시 0
*/
int led8_clear(enum LED8Owner owner);

/*
    @brief
    value 값을 start ~ end 범위에 따라 8개의 LED 에 표시

    @param
    owner: 소유자
    value: 표시할 값 (start ~  end)
    start: 최소값
    end: 최대값

    @return
    성공 시 1 실패 시 0
*/
int led8_accumulate_print(enum LED8Owner owner, int value, int start, int end);

#endif
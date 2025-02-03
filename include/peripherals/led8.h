#ifndef LED_H
#define LED_H

/*
    @brief
    LED 초기화 함수
*/
void led8_init(void);

/*
    @brief
    RGB LED 를 모두 끔
*/
void led8_clear(void);

/*
    @brief
    value 값을 start ~ end 범위에 따라 8개의 LED 에 표시

    @param
    value: 표시할 값 (start ~  end)
    start: 최소값
    end: 최대값
*/
void led8_accumulate_print(int value, int start, int end);

#endif
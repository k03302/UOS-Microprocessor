#ifndef RGB_LED_H
#define RGB_LED_H

/*
    @brief
    RGB LED 초기화 함수
*/
void rgb_led_init(void);

/*
    @brief
    RGB LED 상태 설정

    @param turn_on 1: 켜기, 0: 끄기
*/
void rgb_led_set(int turn_on);

/*
    @brief
    RGB LED 상태 토글
*/
void rgb_led_toggle();

#endif
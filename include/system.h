#ifndef SYSTEM_H
#define SYSTEM_H

enum SystemAttribute
{
    /*
    사운드센서 상수
    value: 0 ~ 1023
    일상소음: 300~600
    근접 박수소리: 800~900
    */
    SOUND_THRESHOLD = 0,           // 현재 설정된 박수소리 역치 (adjustable)
    SOUND_THRESHOLD_DISPLAY,       // 현재 표시되고 있는 박수소리 역치 (adjustable)
    SOUND_THRESHOLD_MIN,           // 설정 가능한 박수소리 역치의 최솟값
    SOUND_THRESHOLD_MAX,           // 설정 가능한 박수소리 역치의 최댓값
    SOUND_THRESHOLD_ADJUST_AMOUNT, // knob을 돌릴 때 박수소리 역치를 조절하는 단위

    /*
    광감지센서 상수
    value: 0 ~ 1023
    */
    CDS_NIGHT_THRESHOLD, // 밤으로 전환하는 조도 역치
    CDS_DAY_THRESHOLD,   // 낮으로 전환하는 조도 역치
    CDS_CHECK_PERIOD,    // 조도를 감지하는 주기 (msec)

    CLAP_MIN_DURATION, // 박수 스파이크 지속시간 최소 (msec)
    CLAP_MAX_DURATION, // 박수 스파이크 지속시간 최대 (msec)
    CLAP_MIN_GAP,      // 박수 간격 최소 (msec)
    CLAP_MAX_GAP,      // 박수 간격 최대 (msec)
    CLAP_TOGGLE_WAIT,  // SET모드를 끝내기 위한 대기시간 (msec)

    FND_NEXT_DIGIT_PERIOD, // FND에 다음 digit을 출력하는 주기 (msec)
    FND_UPDATE_PERIOD,     // FND에 출력할 수치를 업데이트하는 주기 (msec)
    FND_UPDATE_TIMEOUT,    // 박수소리 역치 디스플레이를 끝내는 시간 (msec)

    ADC_CHANGE_TIMEOUT, // ADC 입력원을 변경할 때의 대기시간 (msec)

    SYSTEM_ATTRIBUTE_END
};

/*
    @brief
    시스템 초기화 함수
*/
void system_init();

/*
    @brief
    시스템의 속성값 조회
*/
int system_get_attribute(enum SystemAttribute attr);

/*
    @brief
    시스템의 속성값 설정
    고정속성의 경우 설정 시도 시 실패

    @return
    성공 시 1, 실패 시 0
*/
int system_set_attribute(enum SystemAttribute attr, int value);

#endif
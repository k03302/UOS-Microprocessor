#include "fnd.h"
#include "common.h"
#define FND_REFRESH_PERIOD 1 // FND 각 digit을 갱신하는 주기
#define FND_DIGIT_COUNT 4    // FND의 digit 개수

static volatile unsigned char fnd_digit[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F};

static int print_value;
static int current_digit_index = 0;

/*
    @brief
    print_value의 current_digit_index번째 digit을 출력하는 함수
    함수 호출 후 current_digit_index를 업데이트
*/
static void fnd_print_current_digit()
{
    int digit_value = (print_value / (int)pow(10, current_digit_index)) % 10;
    FND_DATA_BASE = fnd_digit[digit_value];
    FND_SELECT_BASE = 0x01 << current_digit_index;

    if (++current_digit_index >= FND_DIGIT_COUNT)
    {
        current_digit_index = 0;
    }
}

void fnd_init(void)
{
    FND_DATA_DDR = FND_DATA_ALL_BITS;
    FND_SELECT_DDR = FND_SELECT_ALL_BITS;
    fnd_clear();
}

void fnd_set_print_value(int value)
{
    print_value = value;
    current_digit_index = 0;
    timer_set_interval(FND_REFRESH_PERIOD, fnd_print_current_digit);
}

void fnd_clear(void)
{
    timer_clear_interval(fnd_set_print_value);
    FND_DATA_BASE = 0x00;
    FND_SELECT_BASE = FND_SELECT_ALL_BITS;
}

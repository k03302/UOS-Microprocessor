#include "peripherals/fnd.h"
#include "peripherals/timer1.h"
#include "common.h"
#include "system_config.h"
#define FND_DIGIT_COUNT 4 // FND의 digit 개수

static volatile unsigned char fnd_digit[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F};

static int print_value;
static int current_digit_index = 0;

static struct TimerEvent timer_event;

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
    FND_DATA_BASE = 0x00;
    FND_SELECT_BASE = FND_SELECT_ALL_BITS;
}

void fnd_set_print_value(int value)
{
    print_value = value;
    current_digit_index = 0;
}

void fnd_start()
{
    timer_event.callback = fnd_print_current_digit;
    timer_event.is_periodic = 1;
    timer_event.timeout = system_get_attribute(SA_FND_UPDATE_PERIOD);

    timer1_register_handler(&timer_event);
}

void fnd_end(void)
{
    timer1_unregister_handler(&timer_event);
    FND_DATA_BASE = 0x00;
    FND_SELECT_BASE = FND_SELECT_ALL_BITS;
}

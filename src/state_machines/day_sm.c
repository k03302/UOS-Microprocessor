#include "state_machines/day_sm.h"
#include "system_config.h"

// 외부 조도 상태
enum DayState
{
    DAY_NULL, // 확정할 수 없는 상태
    DAY_DAY,  // 낮인 상태 (조도가 역치보다 높은 상태)
    DAY_NIGHT // 밤인 상태 (조도가 역치보다 낮은 상태)
};

static enum DayState day_state = DAY_NULL;
static int day_threshold = 0;
static int night_threshold = 0;

void day_state_machine_initialize(void)
{
    day_state = DAY_NULL;
    day_threshold = system_get_attribute(SA_CDS_DAY_THRESHOLD);
    night_threshold = system_get_attribute(SA_CDS_NIGHT_THRESHOLD);
}

void day_state_machine(int light_value)
{
    switch (day_state)
    {
    case DAY_NULL:
        if (light_value > night_threshold)
        {
            day_state = DAY_DAY;
        }
        else
        {
            day_state = DAY_NIGHT;
        }
        break;

    case DAY_DAY:
        if (light_value < night_threshold)
        {
            day_sm_day2night_callback();
            day_state = DAY_NIGHT;
        }
        break;

    case DAY_NIGHT:
        if (light_value > night_threshold)
        {
            day_sm_night2day_callback();
            day_state = DAY_DAY;
        }
        break;
    }
}
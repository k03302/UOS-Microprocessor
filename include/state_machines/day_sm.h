#ifndef DAY_SM_H
#define DAY_SM_H

void day_state_machine_initialize(void);

void day_state_machine(int light_value);

// day_state_machine에서 day->night 전환 시 호출되는 콜백
// 외부에서 구현해야 함
extern void day_sm_day2night_callback(void);

// day_state_machine에서 night->day 전환 시 호출되는 콜백
// 외부에서 구현해야 함
extern void day_sm_night2day_callback(void);

#endif
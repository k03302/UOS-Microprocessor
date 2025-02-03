#ifndef DAY_SM_H
#define DAY_SM_H

void day_state_machine_initialize(void);

void day_state_machine(int light_value);

extern void day_sm_day2night_callback(void);

extern void day_sm_night2day_callback(void);

#endif
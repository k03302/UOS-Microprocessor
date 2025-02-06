#include "system_config.h"
#include "peripherals/timer1.h"
#include "state_machines/system_sm.h"
#include "state_machines/sensor_sm.h"

int main()
{
    timer1_init();
    system_state_machine_initialize();
    sensor_state_machine_initialize();
    while (1)
    {
        timer1_process_due_events();
        system_state_machine();
        sensor_state_machine();
    }
}
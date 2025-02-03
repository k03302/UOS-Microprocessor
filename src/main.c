#include "system_config.h"
#include "peripherals/timer1.h"
#include "state_machines/system_sm.h"
#include "state_machines/lamp_sm.h"

int main()
{
    system_state_machine_initialize();
    lamp_state_machine_initialize();
    while (1)
    {
        timer1_process_due_events();
        system_state_machine();
        lamp_state_machine();
    }
}
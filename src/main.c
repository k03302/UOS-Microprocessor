#include "system_config.h"
#include "state_machines/system_sm.h"

int main()
{
    system_state_machine_initialize();
    while (1)
    {
        system_state_machine();
    }
}
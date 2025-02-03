#include "peripherals/timer.h"
#include "utils/watch.h"
#include "assert.h"

void watch_init(struct watch *w, unsigned int wait_time)
{
    assert(w != (void *)0);
    w->start_timestamp = timer_get_tick();
    w->wait_time = wait_time;
}

int watch_check(struct watch *w)
{
    assert(w != (void *)0);

    if (timer_get_tick() - w->start_timestamp >= w->wait_time)
    {
        return 1; // 시간이 경과함
    }
    return 0; // 시간이 경과하지 않음
}

void watch_start(struct watch *w)
{
    assert(w != (void *)0);

    w->start_timestamp = timer_get_tick();
}

int watch_check_restart(struct watch *w)
{
    assert(w != (void *)0);

    if (timer_get_tick() - w->start_timestamp >= w->wait_time)
    {
        w->start_timestamp = timer_get_tick();
        return 1; // 시간이 경과함
    }
    return 0; // 시간이 경과하지 않음
}

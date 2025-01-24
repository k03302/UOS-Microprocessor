#ifndef TIMER_H
#define TIMER_H

void timer_init(void);
unsigned long long timer_get_time(void);
int timer_set_timeout(unsigned long long interval, void (*callback)(void));
int timer_set_interval(unsigned long long interval, void (*callback)(void));
void timer_clear_interval(void (*callback)(void));

#endif // TIMER_H
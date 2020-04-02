#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

struct timer_instance{
	int hours;
	int minutes;
	void *priv_data;
};

int timer_init(struct timer_instance* inst, void (* timer_cb)(void *timer_cb_ctx), void *timer_cb_ctx);
void timer_close(struct timer_instance *inst);
void set_timer(struct timer_instance *inst, bool is_periodic);

#endif /* TIMER_H */

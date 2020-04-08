#ifndef TIMER_HANDLER_H
#define TIMER_HANDLER_H
#include <time.h>

enum timer_state {
	TIMER_OFF = 0,
	TIMER_ON,
	TIMER_ERROR = -1
};

struct timer_handler {
	struct tm on_time;
	struct tm off_time;
	void *priv_data;
};

int timer_handler_init(struct timer_handler *hndl,
		       void (* timer_cb)(void *ctx, enum timer_state state),
		       void *ctx);

void timer_handler_close(struct timer_handler *hndl);
#endif /* TIMER_HANDLER_H */

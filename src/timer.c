#define DEBUG
#include "timer.h"

#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <log.h>

#define CLOCK_ID CLOCK_REALTIME
#define MAX_HOURS 500
#define MAX_MINUTES 59
#define SECONDS_IN_MINUTE 60
#define MINUTES_IN_HOURS 60

struct timer_data {
	struct itimerspec trigger;
	timer_t timerid;
	struct sigevent sev;
	bool is_periodic;
	void *timer_cb_ctx;
	void (*timer_cb)(void *timer_cb_ctx);
};

static void timer_handler(union sigval sv)
{
	struct timer_data *data = sv.sival_ptr;
	data->timer_cb(data->timer_cb_ctx);

	if (data->is_periodic) {
		timer_settime(data->timerid, 0, &data->trigger, NULL);
	}
}

static int to_seconds(struct timer_instance *inst)
{
	unsigned int seconds;

	if (inst->minutes > MAX_MINUTES ||
	    inst->hours > MAX_HOURS)
		return -1;

	seconds = inst->minutes * SECONDS_IN_MINUTE;
	seconds += inst->hours * MINUTES_IN_HOURS * SECONDS_IN_MINUTE;
	seconds += inst->seconds;

	return seconds;
}

int timer_init(struct timer_instance* inst, void (* timer_cb)(void *timer_cb_ctx), void *timer_cb_ctx)
{
	struct timer_data *data = NULL;

	data = malloc(sizeof(struct timer_data));
	if (!data)
		return -1;

	memset(&data->trigger, 0, sizeof(data->trigger));
	memset(&data->sev, 0, sizeof(data->sev));

	data->timer_cb = timer_cb;
	data->timer_cb_ctx = timer_cb_ctx;

	data->sev.sigev_notify = SIGEV_THREAD;;
	data->sev.sigev_notify_function = &timer_handler;
	data->sev.sigev_value.sival_ptr = data;

	timer_create(CLOCK_ID, &data->sev, &data->timerid);

	inst->priv_data = data;

	return 0;
}

void timer_close(struct timer_instance *inst)
{
	struct timer_data *data = inst->priv_data;

	timer_delete(data->timerid);
	free(data);
}

void set_timer(struct timer_instance *inst, bool is_periodic)
{
	struct timer_data *data = inst->priv_data;

	data->trigger.it_value.tv_sec = to_seconds(inst);
	pr_debug("trigger time: %li\n", data->trigger.it_value.tv_sec);
	timer_settime(data->timerid, 0, &data->trigger, NULL);

	data->is_periodic = is_periodic;
}

#ifdef TIMER_TEST
#include <unistd.h>

void  print_time()
{
  time_t rawtime;
  struct tm * timeinfo;

  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ( "Current local time and date: %s", asctime (timeinfo) );
}

static void test_timer_cb(void *ctx)
{
	char *str = ctx;
	printf("%s\n", str);
	print_time();;
}

int main()
{
	struct timer_instance inst1;
	struct timer_instance inst2;
	char str1[] = "TIMER1 DONE!!!";
	char str2[] = "TIMER2 DONE!!!";

	inst1.hours = 0;
	inst1.minutes = 1;
	inst1.seconds = 0;

	inst2.hours = 0;
	inst2.minutes = 2;
	inst2.seconds = 0;

	timer_init(&inst1, test_timer_cb, str1);
	timer_init(&inst2, test_timer_cb, str2);
	print_time();
	set_timer(&inst1, true);
	set_timer(&inst2, true);

	sleep(181);
	print_time();

	timer_close(&inst1);
	timer_close(&inst2);
}
#endif /* TIMER_TEST */

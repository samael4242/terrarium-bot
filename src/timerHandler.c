#define DEBUG
#include "timerHandler.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <log.h>
#include "timer.h"

#define SEC_IN_DAY 86400
#define MIN_IN_HOUR 60
#define SEC_IN_MIN 60

struct timer_handler_priv {
	struct timer_instance inst;
	enum timer_state state;
	struct tm off_interval;
	struct tm on_interval;
	struct tm ftrg;
	void *timer_ctx;
	void (*timer_cb)(void *timer_ctx, enum timer_state state);
};

static void timer_handler_cb(void *ctx)
{
	struct timer_handler_priv *handler = ctx;
	struct tm *trigger;

	if (handler->state == TIMER_OFF) {
		handler->state = TIMER_ON;
		trigger = &handler->on_interval;
	} else {
		handler->state = TIMER_OFF;
		trigger = &handler->off_interval;
	}

	handler->inst.hours = trigger->tm_hour;
	handler->inst.minutes = trigger->tm_min;
	handler->inst.seconds = trigger->tm_sec;

	handler->timer_cb(handler->timer_ctx, handler->state);

	set_timer(&handler->inst, false);
}

static time_t to_sec(struct tm *tm)
{
	return  tm->tm_hour * MIN_IN_HOUR * SEC_IN_MIN +
		tm->tm_min * SEC_IN_MIN +
		tm->tm_sec;
}

static time_t compare_tm_sec(struct tm *tm1, struct tm *tm2)
{
	return to_sec(tm1) - to_sec(tm2);
}

static double compare_tm(struct tm *tm1, struct tm *tm2)
{
	time_t t1, t2;

	t1 = mktime(tm1);
	pr_debug("t1: %li, errno: %s\n", t1, strerror(errno));
	pr_debug("tm1: hour: %i, min: %i, sec: %i\n",
			tm1->tm_hour, tm1->tm_min, tm1->tm_sec);
	t2 = mktime(tm2);
	pr_debug("t2: %li errno: %s\n", t2, strerror(errno));
	pr_debug("tm2: hour: %i, min: %i, sec: %i\n",
			tm2->tm_hour, tm2->tm_min, tm2->tm_sec);

	return difftime(t1, t2);
}

/* tm1 > tm2 */
static int get_state_and_interval(struct timer_handler_priv *ctx,
				  struct tm *tm1, struct tm *tm2,
				  bool tm1_is_off)
{
	time_t ct = time(NULL);
	struct tm ctime = *localtime(&ct);
	struct tm ctm1, ctm2;
	struct tm *interval1, *interval2;
	double compare1 = 0, compare2 = 0;
	enum timer_state intr_tm1 = tm1_is_off ? TIMER_ON : TIMER_OFF;
	enum timer_state intr_tm1_tm2 = tm1_is_off ? TIMER_OFF : TIMER_ON;

	memcpy(&ctm1, &ctime, sizeof(struct tm));
	memcpy(&ctm2, &ctime, sizeof(struct tm));

	if (tm1_is_off) {
		interval1 = &ctx->off_interval;
		interval2 = &ctx->on_interval;
	} else {
		interval2 = &ctx->off_interval;
		interval1 = &ctx->on_interval;
	}

	ctm1.tm_hour = tm1->tm_hour;
	ctm1.tm_min = tm1->tm_min;
	ctm1.tm_sec = tm1->tm_sec;

	ctm2.tm_mday += 1;
	mktime(&ctm2);
	ctm2.tm_hour = tm2->tm_hour;
	ctm2.tm_min = tm2->tm_min;
	ctm2.tm_sec = tm2->tm_sec;

	compare1 = compare_tm(&ctime, &ctm1);
	compare2 = compare_tm(&ctime, &ctm2);

	if (compare1 < 0) {
		ctx->state = intr_tm1;
		ctx->ftrg.tm_sec = compare1 * -1;
	} else if (compare1 == 0 || (compare1 > 0 && compare2 < 0)) {
		ctx->state = intr_tm1_tm2;
		ctx->ftrg.tm_sec = compare2 * -1;
	} else {
		ctx->state = TIMER_ERROR;
		pr_err("Inccorrect time intervals set\n");
		return -1;
	}

	mktime(&ctx->ftrg);

	interval1->tm_sec = compare_tm_sec(tm1, tm2);
	interval2->tm_sec = SEC_IN_DAY -
		interval1->tm_sec;

	pr_debug("interval1->tm_sec: %i\n", interval1->tm_sec);
	pr_debug("interval2->tm_sec: %i\n", interval2->tm_sec);

	pr_debug("tm1 in sec: %li\n", to_sec(tm1));
	pr_debug("tm2 in sec: %li\n", to_sec(tm2));
	pr_debug("diff: %i\n", interval1->tm_sec);

	mktime(interval1);
	mktime(interval2);

	ctx->timer_cb(ctx->timer_ctx, ctx->state);

	return 0;
}

static int set_current_state_and_interval(struct timer_handler *hndl)
{
	double compare = 0.;
	bool off_time_is_bigger;
	struct timer_handler_priv *ctx = hndl->priv_data;

	compare = compare_tm_sec(&hndl->off_time, &hndl->on_time);
	if (compare == 0) {
		pr_err("OFF and ON time is same\n");
		return -1;
	} else if (compare > 0) {
		off_time_is_bigger = true;
	} else {
		off_time_is_bigger = false;
	}

	if(off_time_is_bigger)
		return get_state_and_interval(ctx, &hndl->off_time,
					      &hndl->on_time,
					      off_time_is_bigger);
	else
		return get_state_and_interval(ctx, &hndl->on_time,
					      &hndl->off_time,
					      off_time_is_bigger);

	return 0;
}

int timer_handler_init(struct timer_handler *hndl,
		       void (* timer_cb)(void *ctx, enum timer_state state),
		       void *ctx)
{
	struct timer_handler_priv *ptime_hndl;

	ptime_hndl = calloc(1, sizeof(struct timer_handler_priv));
	hndl->priv_data = ptime_hndl;

	ptime_hndl->timer_cb = timer_cb;
	ptime_hndl->timer_ctx = ctx;

	if (set_current_state_and_interval(hndl))
		goto err;

	timer_init(&ptime_hndl->inst, timer_handler_cb, ptime_hndl);

	ptime_hndl->inst.hours = ptime_hndl->ftrg.tm_hour;
	ptime_hndl->inst.minutes = ptime_hndl->ftrg.tm_min;
	ptime_hndl->inst.seconds = ptime_hndl->ftrg.tm_sec;

	set_timer(&ptime_hndl->inst, false);

	return 0;
err:
	free(ptime_hndl);
	return -1;
}

void timer_handler_close(struct timer_handler *hndl)
{
	struct timer_handler_priv *ptime_hndl = hndl->priv_data;

	timer_close(&ptime_hndl->inst);
	free(ptime_hndl);
}

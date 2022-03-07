#include "ytimer.h"

/*
 * ytimer_new()
 * Create a timer object.
 */
ytimer_t *ytimer_new() {
	ytimer_t *timer;

	timer = malloc0(sizeof(ytimer_t));
	return (timer);
}
/*
 * ytimer_del()
 * Delete a previously created timer object.
 */
void ytimer_del(ytimer_t *timer) {
	free0(timer);
}
/*
 * ytimer_start()
 * Start the time measure.
 */
bool ytimer_start(ytimer_t *timer) {
	if (!timer || timer->state == RUNNING)
		return (false);
	gettimeofday(&timer->start, NULL);
	timer->state = RUNNING;
	return (true);
}
/*
 * ytimer_stop()
 * Stop the time measure.
 */
bool ytimer_stop(ytimer_t *timer) {
	if (!timer || timer->state == STOPPED)
		return (false);
	gettimeofday(&timer->end, NULL);
	if (timer->start.tv_usec > timer->end.tv_usec) {
		timer->end.tv_usec += 1000000;
		timer->end.tv_sec--;
	}
	timer->state = STOPPED;
	return (true);
}
/*
 * ytimer_reset()
 * Reset a running time measure.
 */
bool ytimer_reset(ytimer_t *timer) {
	if (!timer)
		return (false);
	gettimeofday(&timer->start, NULL);
	timer->state = RUNNING;
	return (true);
}
/*
 * ytimer_get_sec()
 * Return the time measure in seconds.
 */
long ytimer_get_sec(ytimer_t *timer) {
	long result;

	if (!timer || timer->state == RUNNING)
		return (-1);
	result = timer->end.tv_sec - timer->start.tv_sec;
	return (result);
}
/*
 * ytimer_get_usec()
 * Return the time measure in micro-seconds.
 */
long ytimer_get_usec(ytimer_t *timer) {
	long result;

	if (!timer || timer->state == RUNNING)
		return (-1);
	result = (timer->end.tv_sec - timer->start.tv_sec) * 1e6;
	result += (timer->end.tv_usec - timer->start.tv_usec);
	return (result);
}


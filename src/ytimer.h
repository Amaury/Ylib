/**
 * @header	ytimer.h
 *		Function for time measurement.
 * @version	1.0.0 Aug 12 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#pragma once

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <sys/time.h>
#include "y.h"

/**
 * @typedef	ytimer_state_t
 *		Describe the current state of a timer.
 * @constant	STOPPED	The timer is stopped.
 * @constant	RUNNING	The timer is running.
 */
typedef enum {
	STOPPED = 0,
	RUNNING
} ytimer_state_t;

/**
 * @typedef	ytimer_t
 *		Timer object.
 * @field	state	Current timer state.
 * @field	start	Time of time measure's beginning.
 * @field	end	Time of time measure's ending.
 */
typedef struct {
	ytimer_state_t state;
	struct timeval start;
	struct timeval end;
} ytimer_t;

/**
 * @function	ytimer_new
 *		Create a timer object.
 * @return	A pointer to the allocated timer object.
 */
ytimer_t *ytimer_new(void);

/**
 * @function	ytimer_del
 *		Delete a previously created timer object.
 * @param	timer	A pointer to the timer object.
 */
void ytimer_del(ytimer_t *timer);

/*!
 * @function	ytimer_start
 *		Start the time measure.
 * @param	timer	A pointer to the timer object.
 * @return	false if the timer was already running, true otherwise.
 */
bool ytimer_start(ytimer_t *timer);

/*!
 * @function	ytimer_stop
 *		Stop the time measure.
 * @param	timer	A pointer to the timer object.
 * @return	false if the timer was already stopped, true otherwise.
 */
bool ytimer_stop(ytimer_t *timer);

/*!
 * @function	ytimer_reset
 *		Reset a running time measure.
 * @param	timer	A pointer to the timer object.
 * @return	false is the timer pointer is NULL, true otherwise.
 */
bool ytimer_reset(ytimer_t *timer);

/*!
 * @function	ytimer_get_sec
 *		Return the time measure in seconds.
 * @param	timer	A pointer to the timer object.
 * @return	The number of seconds between timer start and stop,
 *		or -1 if the timer is still running.
 */
long ytimer_get_sec(ytimer_t *timer);

/*!
 * @function	ytimer_get_usec
 *		Return the time measure in micro-seconds.
 * @param	timer	A pointer to the timer object.
 * @return	The number of micro-seconds between timer start and stop,
 *		or -1 if the timer is still running.
 */
long ytimer_get_usec(ytimer_t *timer);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */


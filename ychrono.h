/* Process this file with the HeaderBrowser tool (http://www.headerbrowser.org)
   to create documentation. */
/*!
 * @header	ychrono.h
 *		Function for time measurement.
 * @version	1.0.0 Aug 12 2003
 * @author	Amaury Bouchard <amaury@amaury.net>
 */
#ifndef __YCHRONO_H__
#define __YCHRONO_H__

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif /* __cplusplus || c_plusplus */

#include <sys/time.h>
#include "ydefs.h"

/*!
 * @enum	ychrono_state_e
 *		Describe the current state of a chrono.
 * @constant	STOPPED	The chrono is stopped.
 * @constant	RUNNING	The chrono is running.
 */
enum ychrono_state_e
{
  STOPPED = 0,
  RUNNING
};

/*! @typedef ychrono See enum ychrono_state_e. */
typedef enum ychrono_state_e ychrono_state_t;

/*!
 * @struct	ychrono_s
 *		Chrono object.
 * @field	state	Current chrono state.
 * @field	start	Time of time measure's beginning.
 * @field	end	Time of time measure's ending.
 */
struct ychrono_s
{
  ychrono_state_t state;
  struct timeval start;
  struct timeval end;
};

/*! @typedef ychrono_t See struct ychrono_s. */
typedef struct ychrono_s ychrono_t;

/*!
 * @function	ychrono_new
 *		Create a chrono object.
 * @return	A pointer to the chrono object.
 */
ychrono_t *ychrono_new(void);

/*!
 * @function	ychrono_del
 *		Delete a previously created chrono object.
 * @param	chrono	A pointer to the chrono object.
 */
void ychrono_del(ychrono_t *chrono);

/*!
 * @function	ychrono_start
 *		Start the time measure.
 * @param	chrono	A pointer to the chrono object.
 * @return	YFALSE if the chrono was already running, YTRUE otherwise.
 */
ybool_t ychrono_start(ychrono_t *chrono);

/*!
 * @function	ychrono_stop
 *		Stop the time measure.
 * @param	chrono	A pointer to the chrono object.
 * @return	YFALSE if the chrono was already stopped, YTRUE otherwise.
 */
ybool_t ychrono_stop(ychrono_t *chrono);

/*!
 * @function	ychrono_reset
 *		Reset a running time measure.
 * @param	chrono	A pointer to the chrono object.
 * @return	YFALSE is the chrono pointer is NULL, YTRUE otherwise.
 */
ybool_t ychrono_reset(ychrono_t *chrono);

/*!
 * @function	ychrono_get_sec
 *		Return the time measure in seconds.
 * @param	chrono	A pointer to the chrono object.
 * @return	The number of seconds between chrono start and stop,
 *		or -1 if the chrono is still running.
 */
long ychrono_get_sec(ychrono_t *chrono);

/*!
 * @function	ychrono_get_usec
 *		Return the time measure in micro-seconds.
 * @param	chrono	A pointer to the chrono object.
 * @return	The number of micro-seconds between chrono start and stop,
 *		or -1 if the chrono is still running.
 */
long ychrono_get_usec(ychrono_t *chrono);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif /* __cplusplus || c_plusplus */

#endif /* __YCHRONO_H__ */

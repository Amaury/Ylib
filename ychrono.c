#include "ychrono.h"

/*
** ychrono_new()
** Create a chrono object.
*/
ychrono_t *ychrono_new()
{
  ychrono_t *chrono;

  chrono = malloc0(sizeof(ychrono_t));
  return (chrono);
}

/*
** ychrono_del()
** Delete a previously created chrono object.
*/
void ychrono_del(ychrono_t *chrono)
{
  free0(chrono);
}

/*
** ychrono_start()
** Start the time measure.
*/
ybool_t ychrono_start(ychrono_t *chrono)
{
  if (!chrono || chrono->state == RUNNING)
    return (YFALSE);
  gettimeofday(&chrono->start, NULL);
  chrono->state = RUNNING;
  return (YTRUE);
}

/*
** ychrono_stop()
** Stop the time measure.
*/
ybool_t ychrono_stop(ychrono_t *chrono)
{
  if (!chrono || chrono->state == STOPPED)
    return (YFALSE);
  gettimeofday(&chrono->end, NULL);
  if (chrono->start.tv_usec > chrono->end.tv_usec)
    {
      chrono->end.tv_usec += 1000000;
      chrono->end.tv_sec--;
    }
  chrono->state = STOPPED;
  return (YTRUE);
}

/*
** ychrono_reset()
** Reset a running time measure.
*/
ybool_t ychrono_reset(ychrono_t *chrono)
{
  if (!chrono)
    return (YFALSE);
  gettimeofday(&chrono->start, NULL);
  chrono->state = RUNNING;
  return (YTRUE);
}

/*
** ychrono_get_sec()
** Return the time measure in seconds.
*/
long ychrono_get_sec(ychrono_t *chrono)
{
  long result;

  if (!chrono || chrono->state == RUNNING)
    return (-1);
  result = chrono->end.tv_sec - chrono->start.tv_sec;
  return (result);
}

/*
** ychrono_get_usec()
** Return the time measure in micro-seconds.
*/
long ychrono_get_usec(ychrono_t *chrono)
{
  long result;

  if (!chrono || chrono->state == RUNNING)
    return (-1);
  result = (chrono->end.tv_sec - chrono->start.tv_sec) * 1e6;
  result += (chrono->end.tv_usec - chrono->start.tv_usec);
  return (result);
}

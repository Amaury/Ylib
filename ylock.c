#include "ylock.h"

/*
** ylock()
** Lock a file.
*/
yerr_t ylock(const char *filename)
{
  return (ylock_delay(filename, YLOCK_DEFAULT_DELAY));
}

/*
** ylock_delay()
** Lock a file. Waits eventually during a specified time.
*/
yerr_t ylock_delay(const char *filename, int delay)
{
  int fd;
  ystr_t ys;

  if (!filename)
    {
      YLOG_ADD(YLOG_ERR, "Empty filename parameter.");
      return (YEUNDEF);
    }
  if (!(ys = ys_new(filename)))
    {
      YLOG_ADD(YLOG_ERR, "Memory allocation error.");
      return (YEUNDEF);
    }
  ys_cat(&ys, YLOCK_FILE_SUFFIX);
  if ((fd = open(ys, O_CREAT | O_EXCL | O_WRONLY)) == -1)
    {
      sleep(delay);
      if ((fd = open(ys, O_CREAT | O_EXCL | O_WRONLY)) == -1)
	{
	  YLOG_ADD(YLOG_WARN, "Can't create lock file '%s'.", ys);
	  ys_del(&ys);
	  return (YEAGAIN);
	}
    }
  ys_printf(&ys, "%d", getpid());
  write(fd, ys, ys_len(ys));
  close(fd);
  ys_del(&ys);
  return (YENOERR);
}

/*
** yunlock()
** Unlock the given file.
*/
yerr_t yunlock(const char *filename)
{
  struct stat st;
  ystr_t ys;
  yerr_t this_error = YENOERR;

  if (!filename)
    {
      YLOG_ADD(YLOG_ERR, "Empty filename parameter.");
      return (YEUNDEF);
    }
  if (!(ys = ys_new(filename)))
    {
      YLOG_ADD(YLOG_ERR, "Memory allocation error.");
      return (YEUNDEF);
    }
  ys_cat(&ys, YLOCK_FILE_SUFFIX);
  if (stat(filename, &st))
    {
      YLOG_ADD(YLOG_WARN, "Can't find lock file '%s'.", ys);
      this_error = YENOENT;
    }
  else if (unlink(filename))
    {
      YLOG_ADD(YLOG_WARN, "Can't remove lock file '%s'.", ys);
      this_error = YEACCES;
    }
  ys_del(&ys);
  return (this_error);
}
